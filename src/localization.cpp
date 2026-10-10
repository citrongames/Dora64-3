#include "localization.hpp"
#include "diagnostics.hpp"
#include "recomp.h"
#include "recomp_context_copy.hpp"
#ifdef DORA3_LIVE_RT64
#include "json/json.hpp"
#include <SDL.h>
#include "stb/stb_image.h"
#include "librecomp/addresses.hpp"
#include <cstring>
#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <unordered_map>

extern "C" {
int dora3_overlay_loaded(uint32_t);
void dora3_common_glyph_draw(uint8_t*,recomp_context*);
void dora3_scaled_texture_rect(uint8_t*,recomp_context*);
}
namespace {
using Json=nlohmann::json;
using Bytes=std::vector<uint8_t>;
constexpr uint32_t arena=dora3::localization::arena_begin;
constexpr uint32_t plain_slot=arena+0x5000,npc_slot=arena+0x5800,widget_slot=arena+0x6000;
constexpr unsigned slot_size=0x800;
struct Entry {
    std::string id;
    uint32_t rom=0,vram=0,section=0,key=0,context=0;
    unsigned cells=0;
    Bytes original;
    std::vector<std::string> controls;
};
struct Pack {std::unordered_map<uint32_t,Bytes> strings;std::array<unsigned,8> choice{};bool choice_ready=false;};
struct Script {uint32_t rom,end;};
std::vector<dora3::localization::Language> choices{{"ja","Original Japanese"}};
std::vector<Pack> packs(1);
std::vector<Entry> entries;
std::unordered_multimap<uint32_t,size_t> by_address;
std::unordered_map<uint32_t,size_t> by_rom,by_context;
std::vector<Script> scripts;
std::unordered_map<std::string,uint8_t> glyphs;
std::map<uint16_t,unsigned> pair_indices;
std::vector<uint16_t> pairs;
std::array<uint8_t,192*192/2> atlas{};
bool font_ready=false;
std::atomic<unsigned> selected{0};

// Guest-thread state. Native images copy the scratch bytes and never publish
// text advancement. Language packs/pair IDs remain immutable until process exit.
unsigned npc_language=0,widget_language=0;
uint32_t script_rom=0,script_base=0,pending_script=0;
int32_t& word(uint8_t* rdram,uint32_t a){return MEM_W(0,int32_t(a));}
uint8_t byte(uint8_t* rdram,uint32_t a){return MEM_BU(0,int32_t(a));}
bool pointer(uint32_t a,size_t n){return a>=0x80000000u&&uint64_t(a)+n<=0x80800000ull;}
Json read(const std::filesystem::path& path){std::ifstream in(path);Json j;in>>j;return j;}
const std::map<std::string,uint16_t> commands{
    {"end",0x8f49},{"close",0x8aae},{"next",0x91b1},{"white",0x9492},
    {"red",0x90d4},{"yellow",0x89a9},{"blue",0x90c2},{"wait",0x8e9f},
    {"jump",0x94f2},{"choice",0x939a},{"yes",0x905e},{"no",0x8b55},{"cursor",0x897e}};
void half(Bytes& out,uint16_t value){out.push_back(uint8_t(value>>8));out.push_back(uint8_t(value));}
std::optional<Bytes> encode(const std::string& text,const Entry& entry) {
    Bytes out;std::vector<std::string> controls;std::vector<uint8_t> run;unsigned cells=0;
    std::vector<std::pair<size_t,uint16_t>> pending;
    auto flush=[&] {
        for(size_t i=0;i<run.size();i+=2) {
            const uint16_t pair=uint16_t(run[i])<<8|(i+1<run.size()?run[i+1]:glyphs.at(" "));
            pending.emplace_back(out.size(),pair);half(out,0);++cells;
        }
        run.clear();
    };
    for(size_t i=0;i<text.size();) {
        if(text[i]=='{') {
            flush();const auto end=text.find('}',i);if(end==std::string::npos)return {};
            const auto token=text.substr(i+1,end-i-1);controls.push_back(token);
            const auto colon=token.find(':');const auto name=token.substr(0,colon);
            const auto cmd=commands.find(name);if(cmd==commands.end())return {};
            half(out,cmd->second);
            if(colon!=std::string::npos) {
                if(name!="jump"&&name!="yes"&&name!="no")return {};
                const auto digits=token.substr(colon+1);if(digits.empty()||digits.size()>4)return {};
                for(char c:digits){if(c<'0'||c>'9')return {};out.push_back(uint8_t(c));}
            }
            i=end+1;
        } else {
            const unsigned c=uint8_t(text[i]);const size_t n=c<0x80?1:(c&0xe0)==0xc0?2:(c&0xf0)==0xe0?3:(c&0xf8)==0xf0?4:0;
            if(!n||i+n>text.size())return {};
            auto found=glyphs.find(text.substr(i,n));if(found==glyphs.end())return {};
            run.push_back(found->second);i+=n;
        }
    }
    flush();
    // Validate before interning: a rejected custom record must not consume IDs.
    // Commands, branch arguments and their order match the ROM-qualified index.
    if(controls!=entry.controls||cells>entry.cells||out.size()+2>slot_size)return {};
    if(pairs.size()+pending.size()>0x4000)return {};
    for(const auto& [offset,pair]:pending) {
        auto [it,added]=pair_indices.emplace(pair,unsigned(pairs.size()));
        if(added)pairs.push_back(pair);
        const auto value=uint16_t(0x4000+it->second);
        out[offset]=uint8_t(value>>8);out[offset+1]=uint8_t(value);
    }
    if(entry.original.size()>=2&&entry.original[entry.original.size()-2]==0&&entry.original.back()==0)half(out,0);
    return out;
}
// Immutable PNG replacements are converted once to native CI8/TLUT. The original
// sprite routine retains its clipping, scale, alpha, layer and strip handling.
// Identity compares the complete ROM palette + texels, never a transient slot.
struct Sprite {
    unsigned width=0,height=0,format=0,palette_size=0;
    Bytes original, palette;
    std::vector<std::array<unsigned,4>> regions;
    std::string file;
    bool material=false;
};
struct SpriteImage {Bytes pixels,palette,indexed;uint32_t address=0,palette_address=0,material_address=0;};
std::vector<Sprite> sprites;
std::vector<std::vector<SpriteImage>> sprite_packs(1);
std::unordered_multimap<uint64_t,size_t> sprite_dimensions;
uint64_t sprite_key(unsigned w,unsigned h,unsigned f){return (uint64_t(w)<<32)|(uint64_t(h)<<8)|f;}
Bytes decode_resource(std::span<const char> rom,size_t record) {
    auto be=[&](size_t at){if(at+4>rom.size())throw std::runtime_error("Texture ROM bounds");return uint32_t(uint8_t(rom[at]))<<24|uint32_t(uint8_t(rom[at+1]))<<16|uint32_t(uint8_t(rom[at+2]))<<8|uint8_t(rom[at+3]);};
    const auto compressed=be(record),packed=be(record+8),size=be(record+12);
    if(compressed>1||be(record+4)||!size||size>0x100000||record+16+packed>rom.size())throw std::runtime_error("Texture resource header");
    Bytes out;out.reserve(size);std::array<uint8_t,1024> ring{};unsigned write=0x3be;
    size_t pos=record+16,end=pos+packed;
    auto emit=[&](uint8_t c){if(out.size()>=size)throw std::runtime_error("Texture resource length");out.push_back(c);ring[write]=c;write=(write+1)&1023;};
    if(!compressed){while(pos<end)emit(uint8_t(rom[pos++]));}
    else while(pos<end&&out.size()<size){const auto flags=uint8_t(rom[pos++]);for(unsigned bit=0;bit<8&&pos<end&&out.size()<size;++bit){
        if(flags&(1u<<bit))emit(uint8_t(rom[pos++]));else{
            if(pos+2>end)throw std::runtime_error("Texture compressed bounds");
            const auto lo=uint8_t(rom[pos++]),hi=uint8_t(rom[pos++]);const unsigned index=lo|((hi&0xc0)<<2),count=(hi&63)+3;
            for(unsigned n=0;n<count;++n)emit(ring[(index+n)&1023]);
        }
    }}
    if(out.size()!=size||size<16||std::memcmp(out.data(),"BMP",3))throw std::runtime_error("Texture BMP header");
    return out;
}
void load_sprite_index(const std::filesystem::path& root,std::span<const char> rom) {
    if(!std::filesystem::exists(root/"texture_index.json"))return;
    const auto doc=read(root/"texture_index.json");
    for(const auto& row:doc.at("entries")) {
        Sprite s;s.file=row.at("file");
        if(s.file.empty()||s.file.find_first_of("/\\:")!=std::string::npos||s.file.find("..")!=std::string::npos)continue;
        auto bytes=decode_resource(rom,row.at("record").get<size_t>());
        auto be=[&](unsigned at){return uint32_t(bytes[at])<<24|uint32_t(bytes[at+1])<<16|uint32_t(bytes[at+2])<<8|bytes[at+3];};
        if(bytes[3]!=5&&bytes[3]!=6)continue;
        s.width=be(8);s.height=be(12);s.format=bytes[3]-5;
        if(!s.width||!s.height||s.width>512||s.height>256)continue;
        s.palette_size=(((unsigned(bytes[6])<<8)|bytes[7])*2+7)&~7u;
        const size_t count=s.width*s.height/(s.format?1:2);
        if(!s.palette_size||s.palette_size>(s.format?512u:32u)||16+s.palette_size+count>bytes.size())continue;
        s.original.assign(bytes.begin()+16,bytes.begin()+16+s.palette_size+count);
        s.palette.assign(bytes.begin()+16,bytes.begin()+16+s.palette_size);
        s.material=row.value("material",false);
        if(row.contains("regions")) {
            s.regions=row.at("regions").get<std::vector<std::array<unsigned,4>>>();
            bool valid=true;for(const auto& r:s.regions)valid &= r[2]&&r[3]&&uint64_t(r[0])+r[2]<=s.width&&uint64_t(r[1])+r[3]<=s.height;
            if(!valid)continue;
        }
        // Same byte order as RDRAM so the hot path can compare contiguous bytes.
        if(s.original.size()%4)continue;
        for(size_t i=0;i<s.original.size();i+=4){std::swap(s.original[i],s.original[i+3]);std::swap(s.original[i+1],s.original[i+2]);}
        sprite_dimensions.emplace(sprite_key(s.width,s.height,s.format),sprites.size());sprites.push_back(std::move(s));
    }
}
std::vector<SpriteImage> load_sprite_pack(const std::filesystem::path& root) {
    std::vector<SpriteImage> result(sprites.size());
    for(size_t i=0;i<sprites.size();++i) {
        const auto& s=sprites[i];std::ifstream in(root/"textures"/s.file,std::ios::binary);
        if(!in)continue;
        std::vector<char> file((std::istreambuf_iterator<char>(in)),{});if(file.empty()||file.size()>4*1024*1024)continue;
        int w=0,h=0;auto* rgba=stbi_load_from_memory(reinterpret_cast<const stbi_uc*>(file.data()),int(file.size()),&w,&h,nullptr,4);
        if(!rgba)continue;
        if(w==int(s.width)&&h==int(s.height)){
            auto& image=result[i].pixels;
            // The original strip loader requests one guard row/word. Supply
            // transparent padding, including on the last strip, in our buffer.
            image.reserve(size_t(w)*size_t(h+1)*2+16);
            for(int p=0;p<w*h;++p) {
                const auto* c=rgba+p*4;
                uint16_t color=uint16_t(c[0]>>3)<<11|uint16_t(c[1]>>3)<<6|uint16_t(c[2]>>3)<<1|(c[3]>=128);
                bool replaced=s.regions.empty();
                for(const auto& r:s.regions)if(unsigned(p%w)>=r[0]&&unsigned(p%w)<r[0]+r[2]&&unsigned(p/w)>=r[1]&&unsigned(p/w)<r[1]+r[3])replaced=true;
                if(!replaced) {
                    const unsigned packed=s.original[(s.palette_size+(s.format?p:p/2))^3u];
                    const unsigned index=s.format?packed:(packed>>(p%2?0:4))&15;
                    color=uint16_t(s.palette[index*2])<<8|s.palette[index*2+1];
                }
                half(image,color);
            }
            if(s.material) {
                // World signs retain their original CI format/TLUT and UVs.
                // Map only these explicit immutable resources into that palette.
                auto& indexed=result[i].indexed;indexed.resize(size_t(w)*size_t(h)/(s.format?1:2)+32,0);
                for(int p=0;p<w*h;++p) {
                    const unsigned color=unsigned(image[p*2])<<8|image[p*2+1];unsigned best=0,best_score=UINT32_MAX;
                    for(unsigned k=0;k<s.palette.size()/2;++k) {
                        const unsigned q=unsigned(s.palette[k*2])<<8|s.palette[k*2+1];
                        const int dr=int(color>>11&31)-int(q>>11&31),dg=int(color>>6&31)-int(q>>6&31),db=int(color>>1&31)-int(q>>1&31);
                        const unsigned score=(color&1)!=(q&1)?100000u:unsigned(dr*dr+dg*dg+db*db);
                        if(score<best_score){best_score=score;best=k;}
                    }
                    if(s.format)indexed[p]=uint8_t(best);else indexed[p/2]|=uint8_t(best<<(p%2?0:4));
                }
            }
            // The unused SDK direct-colour branch is RGBA32 (FD18), not
            // RGBA16. Use its proven CI8 branch: <=1500 texels per strip fit
            // beside the TLUT, even for wide/partially clipped menu textures.
            Bytes pixels(size_t(w)*size_t(h+1)+32,0),palette;
            std::unordered_map<uint16_t,unsigned> colors{{0,0}};half(palette,0);bool valid=true;
            for(int p=0;p<w*h;++p) {
                uint16_t color=uint16_t(image[p*2])<<8|image[p*2+1];if(!(color&1))color=0;
                auto [it,added]=colors.emplace(color,unsigned(colors.size()));
                if(added){if(it->second>=256){valid=false;break;}half(palette,color);}
                pixels[p]=uint8_t(it->second);
            }
            if(valid){palette.resize(512+32,0);image=std::move(pixels);result[i].palette=std::move(palette);}
            else {image.clear();result[i].indexed.clear();DORA3_LOG(stderr,"LOCALIZATION sprite needs <=256 native colours: %s\n",s.file.c_str());}
        }
        stbi_image_free(rgba);
    }
    return result;
}

bool matches(uint8_t* ram,uint32_t address,const Entry& e) {
    if(!pointer(address,e.original.size()))return false;
    for(size_t i=0;i<e.original.size();++i)if(byte(ram,address+uint32_t(i))!=e.original[i])return false;
    return true;
}
const Entry* resolve(uint8_t* ram,uint32_t address) {
    const auto [begin,end]=by_address.equal_range(address);
    for(auto it=begin;it!=end;++it) {
        const auto& e=entries[it->second];
        if(dora3_overlay_loaded(e.section)&&matches(ram,address,e))return &e;
    }
    if(script_base&&address>=script_base) {
        const uint64_t origin=uint64_t(script_rom)+address-script_base;
        if(origin<=UINT32_MAX)if(auto it=by_rom.find(uint32_t(origin));it!=by_rom.end()) {
            const auto& e=entries[it->second];
            if(e.section==0&&matches(ram,address,e))return &e;
        }
    }
    return nullptr;
}
uint32_t translate(uint8_t* rdram,uint32_t source,unsigned language,uint32_t destination,uint32_t context=0) {
    if(language==0||language>=packs.size()||!font_ready)return source;
    const Entry* entry=nullptr;
    if(context)if(auto it=by_context.find(context);it!=by_context.end()) {
        const auto& candidate=entries[it->second];
        if(dora3_overlay_loaded(candidate.section)&&matches(rdram,source,candidate))entry=&candidate;
    }
    if(!entry)entry=resolve(rdram,source);if(!entry)return source;
    const auto it=packs[language].strings.find(entry->key);if(it==packs[language].strings.end())return source;
    for(size_t i=0;i<it->second.size();++i)MEM_B(0,int32_t(destination+i))=it->second[i];
    return destination;
}
void load_font(const std::filesystem::path& root) {
    const auto config=read(root/"dialogue_font.json");
    if(config.at("tile_width")!=12||config.at("tile_height")!=12)throw std::runtime_error("Expected 12px dialogue font");
    for(auto it=config.at("glyphs").begin();it!=config.at("glyphs").end();++it) {
        const int code=it.value().get<int>();if(code<0||code>255)throw std::runtime_error("Invalid font code");
        glyphs.emplace(it.key(),uint8_t(code));
    }
    if(!glyphs.contains(" "))throw std::runtime_error("Missing space glyph");
    const auto path=root/std::filesystem::u8path(config.at("atlas").get<std::string>());
    std::ifstream input(path,std::ios::binary);
    std::vector<char> data((std::istreambuf_iterator<char>(input)),{});
    if(data.empty()||data.size()>4*1024*1024)throw std::runtime_error("Invalid dialogue font bitmap");
    auto* stream=SDL_RWFromConstMem(data.data(),int(data.size()));
    auto* bmp=stream?SDL_LoadBMP_RW(stream,1):nullptr;
    if(!bmp)throw std::runtime_error("Missing dialogue font bitmap");
    auto* rgba=SDL_ConvertSurfaceFormat(bmp,SDL_PIXELFORMAT_RGBA32,0);SDL_FreeSurface(bmp);
    if(!rgba)throw std::runtime_error("Cannot read dialogue font bitmap");
    if(rgba->w!=192||rgba->h!=192){SDL_FreeSurface(rgba);throw std::runtime_error("Expected 192x192 font bitmap");}
    for(int y=0;y<192;++y)for(int x=0;x<192;++x) {
        const auto* p=static_cast<const uint8_t*>(rgba->pixels)+y*rgba->pitch+x*4;
        const unsigned light=(unsigned(p[0])+p[1]+p[2])/3;
        // Dora3: index0 transparent; 10..15 white-to-black ramp, shared by
        // its red/blue/yellow palettes. Keep the donor's hand-drawn bitmap.
        const uint8_t value=light>=192?10:light>=64?13:0;
        atlas[(y*192+x)/2]|=uint8_t(value<<(x%2?0:4));
    }
    SDL_FreeSurface(rgba);font_ready=true;
}
}
namespace dora3::localization {
void initialize(const std::filesystem::path& root,std::span<const char> rom) {
    try {
        load_font(root);
        try {load_sprite_index(root,rom);}
        catch(const std::exception& e) {sprites.clear();sprite_dimensions.clear();DORA3_LOG(stderr,"LOCALIZATION textures skipped: %s\n",e.what());}
        for(const char* filename:{"dialogue_index.json","game_text_index.json"}) {
            const auto index=read(root/filename);
            for(const auto& row:index.at("entries")) {
                Entry e;e.id=row.at("id");e.rom=row.at("rom");e.vram=row.at("vram");e.section=row.at("section_rom");
                e.key=row.value("key",e.rom);e.context=row.value("context_vram",0u);
                e.cells=row.at("max_cells");if(e.cells>24)continue;e.controls=row.at("controls").get<std::vector<std::string>>();
                const auto length=row.at("length").get<size_t>();
                if(length<2||length>=slot_size||uint64_t(e.rom)+length>rom.size())continue;
                e.original.assign(rom.begin()+e.rom,rom.begin()+e.rom+length);
                if(e.context)by_context.emplace(e.context,entries.size());
                else {by_rom.emplace(e.rom,entries.size());if(e.vram)by_address.emplace(e.vram,entries.size());
                    // Some menus draw a copied string, not its ROM-linked address.
                    // Limit aliases to catalogued buffers; resolve still requires
                    // the active overlay and every original byte, including NUL.
                    for(const auto& address:row.value("copied_to",Json::array()))
                        if(pointer(address.get<uint32_t>(),e.original.size()))
                            by_address.emplace(address.get<uint32_t>(),entries.size());}
                entries.push_back(std::move(e));
            }
        }
        const auto script_index=read(root/"scripts.json");
        for(const auto& s:script_index.at("scripts"))scripts.push_back({s.at("rom"),s.at("end")});
        const auto manifest=read(root/"languages.json");
        for(const auto& language:manifest.at("languages")) {
          try {
            const std::string code=language.at("code");
            if(code=="ja"||code.empty()||code.find_first_not_of("abcdefghijklmnopqrstuvwxyz0123456789-_")!=std::string::npos)continue;
            Pack pack;unsigned rejected=0;
            // The eight original choice cells and cursor positions are retained.
            const auto choice_file=root/code/"choices.json";
            if(std::filesystem::exists(choice_file)) {
                const auto doc=read(choice_file);Entry e;e.cells=8;
                if(auto bytes=encode(doc.at("cells").get<std::string>(),e);bytes&&bytes->size()==16) {
                    for(unsigned i=0;i<8;++i)pack.choice[i]=256+((unsigned((*bytes)[2*i])<<8)|(*bytes)[2*i+1])-0x4000;
                    pack.choice_ready=true;
                }
            }
            for(const char* filename:{"dialogue.json","game_text.json"}) {
                const auto file=root/code/filename;if(!std::filesystem::exists(file))continue;
                const auto document=read(file);
                for(const auto& row:document.at("entries")) {
                    if(!row.contains("id")||!row["id"].is_string()||!row.contains("translation")||!row["translation"].is_string()) {++rejected;continue;}
                    const std::string id=row.at("id"),text=row.at("translation");if(text.empty())continue;
                    const auto e=std::find_if(entries.begin(),entries.end(),[&](const Entry& e){return e.id==id;});
                    if(e==entries.end())continue;
                    if(auto encoded=encode(text,*e))pack.strings.emplace(e->key,std::move(*encoded));else ++rejected;
                }
            }
            DORA3_LOG(stderr,"LOCALIZATION %s: %zu strings, %u rejected\n",code.c_str(),pack.strings.size(),rejected);
            auto images=load_sprite_pack(root/code);
            choices.push_back({code,language.at("name")});packs.push_back(std::move(pack));
            sprite_packs.push_back(std::move(images));
          } catch(const std::exception& e) {DORA3_LOG(stderr,"LOCALIZATION pack skipped: %s\n",e.what());}
        }
    } catch(const std::exception& e) {
        DORA3_LOG(stderr,"LOCALIZATION: %s; Japanese fallback\n",e.what());
        choices.resize(1);packs.resize(1);sprite_packs.resize(1);selected=0;
    }
}
const std::vector<Language>& languages(){return choices;}
std::string system_language() {
    std::string result="ja";
    if(auto* locales=SDL_GetPreferredLocales()) {
        for(auto* locale=locales;locale->language;++locale) {
            auto it=std::find_if(choices.begin(),choices.end(),[&](const Language& l){return l.code==locale->language;});
            if(it!=choices.end()){result=it->code;break;}
        }
        SDL_free(locales);
    }
    return result;
}
std::string select(const std::string& code) {
    for(unsigned i=0;i<choices.size();++i)if(choices[i].code==code){selected=i;return code;}
    selected=0;return "ja";
}
std::string current_language(){return choices[selected.load()].code;}
void reset(){npc_language=widget_language=selected.load();script_base=script_rom=pending_script=0;for(auto& pack:sprite_packs)for(auto& image:pack){image.address=0;image.palette_address=0;image.material_address=0;}}
void initialize_memory(uint8_t* rdram) {
    for(size_t i=0;i<atlas.size();++i)MEM_B(0,int32_t(arena+i))=atlas[i];
    for(auto& pack:sprite_packs)for(auto& image:pack) {
        if(image.pixels.empty())continue;
        auto* memory=static_cast<uint8_t*>(recomp::alloc(rdram,image.pixels.size()));
        if(!memory)throw std::bad_alloc{};
        const auto address=uint64_t(memory-rdram)+0x80000000ull;
        if(address+image.pixels.size()>0xa0000000ull)throw std::runtime_error("Localized sprite outside extended RDRAM");
        image.address=uint32_t(address);
        for(size_t i=0;i<image.pixels.size();++i)MEM_B(0,int32_t(image.address+i))=image.pixels[i];
        auto* palette=static_cast<uint8_t*>(recomp::alloc(rdram,image.palette.size()));
        if(!palette)throw std::bad_alloc{};
        const auto palette_address=uint64_t(palette-rdram)+0x80000000ull;
        if(palette_address+image.palette.size()>0xa0000000ull)throw std::runtime_error("Localized palette outside extended RDRAM");
        image.palette_address=uint32_t(palette_address);
        for(size_t i=0;i<image.palette.size();++i)MEM_B(0,int32_t(image.palette_address+i))=image.palette[i];
        if(!image.indexed.empty()) {
            auto* pixels=static_cast<uint8_t*>(recomp::alloc(rdram,image.indexed.size()));
            if(!pixels)throw std::bad_alloc{};
            const auto a=uint64_t(pixels-rdram)+0x80000000ull;
            if(a+image.indexed.size()>0xa0000000ull)throw std::runtime_error("Localized material outside extended RDRAM");
            image.material_address=uint32_t(a);
            for(size_t i=0;i<image.indexed.size();++i)MEM_B(0,int32_t(image.material_address+i))=image.indexed[i];
        }
    }
}
}
extern "C" int dora3_localization_material(uint8_t* ram,recomp_context* ctx) {
    const unsigned language=selected.load();if(!language||language>=sprite_packs.size())return 0;
    const uint32_t header=uint32_t(ctx->r4);if(!pointer(header,16))return 0;
    const auto fmt=byte(ram,header+3);if(fmt!=5&&fmt!=6)return 0;
    const auto range=sprite_dimensions.equal_range(sprite_key(uint32_t(word(ram,header+8)),uint32_t(word(ram,header+12)),fmt-5));
    for(auto it=range.first;it!=range.second;++it) {
        const auto& sprite=sprites[it->second];const auto& image=sprite_packs[language][it->second];
        if(!sprite.material||!image.material_address||!pointer(header+16,sprite.original.size()))continue;
        if(std::memcmp(ram+((header+16)&0x1fffffff),sprite.original.data(),sprite.original.size()))continue;
        ctx->r2=int32_t(image.material_address);return 1;
    }
    return 0;
}
extern "C" int dora3_localization_sprite(uint8_t* ram,recomp_context* ctx) {
    const unsigned language=selected.load();if(!language||language>=sprite_packs.size())return 0;
    const uint32_t sp=uint32_t(ctx->r29);if(!pointer(sp,0x24))return 0;
    const unsigned format=uint32_t(word(ram,sp+0x20));if(format>1)return 0;
    const auto range=sprite_dimensions.equal_range(sprite_key(uint32_t(ctx->r6),uint32_t(ctx->r7),format));
    const uint32_t pixels=uint32_t(word(ram,sp+0x18)),palette=uint32_t(word(ram,sp+0x1c));
    for(auto it=range.first;it!=range.second;++it) {
        const auto& sprite=sprites[it->second];const auto& image=sprite_packs[language][it->second];
        const auto bytes=sprite.original.size()-sprite.palette_size;
        if(!image.address||!pointer(pixels,bytes)||!pointer(palette,sprite.palette_size))continue;
        if(std::memcmp(ram+(palette&0x1fffffff),sprite.original.data(),sprite.palette_size)||
           std::memcmp(ram+(pixels&0x1fffffff),sprite.original.data()+sprite.palette_size,bytes))continue;
        recomp_context draw{};dora3::copy_recomp_context(draw,*ctx);
        word(ram,sp+0x18)=int32_t(image.address);word(ram,sp+0x1c)=int32_t(image.palette_address);word(ram,sp+0x20)=1;
        // Extended immutable pointers bypass the game-RAM ROM matcher. Original
        // widescreen/cadence hooks execute exactly once, in the actual draw.
        dora3_scaled_texture_rect(ram,&draw);
        word(ram,sp+0x18)=int32_t(pixels);word(ram,sp+0x1c)=int32_t(palette);word(ram,sp+0x20)=int32_t(format);
        dora3::copy_recomp_context(*ctx,draw);return 1;
    }
    return 0;
}
extern "C" void dora3_localization_context_plain(uint8_t* ram,recomp_context* ctx) {
    const uint32_t slot=uint32_t(ctx->r3),source=uint32_t(ctx->r4);
    if(pointer(slot,4)&&uint32_t(word(ram,slot))==source)
        ctx->r4=int32_t(translate(ram,source,selected.load(),plain_slot,slot));
}
extern "C" void dora3_localization_plain(uint8_t* ram,recomp_context* ctx) {
    ctx->r4=int32_t(translate(ram,uint32_t(ctx->r4),selected.load(),plain_slot));
}
extern "C" void dora3_localization_dialogue_begin(uint8_t*,recomp_context*){npc_language=selected.load();}
extern "C" void dora3_localization_dialogue_line(uint8_t* ram,recomp_context* ctx) {
    const uint32_t state=uint32_t(ctx->r4);
    if(pointer(state,0x18))word(ram,state+0x14)=int32_t(translate(ram,uint32_t(word(ram,state+0x14)),npc_language,npc_slot));
}
extern "C" void dora3_localization_script_load(uint8_t*,recomp_context* ctx) {
    pending_script=0;
    for(const auto& s:scripts)if(s.rom==uint32_t(ctx->r4)&&s.end==uint32_t(ctx->r5)){pending_script=s.rom;break;}
}
extern "C" void dora3_localization_bind(uint8_t*,recomp_context* ctx) {
    script_rom=pending_script;script_base=pending_script?uint32_t(ctx->r5):0;widget_language=selected.load();
}
extern "C" void dora3_localization_widget_line(uint8_t* ram,recomp_context*) {
    word(ram,0x801B8D44)=int32_t(translate(ram,uint32_t(word(ram,0x801B8D44)),widget_language,widget_slot));
}
extern "C" void dora3_localization_choice(uint8_t*,recomp_context* ctx) {
    if(!font_ready||npc_language==0||npc_language>=packs.size())return;
    const auto& pack=packs[npc_language];const unsigned cell=uint32_t(ctx->r16);
    if(!pack.choice_ready||cell>=8)return;
    const unsigned glyph=pack.choice[cell];ctx->r4=(glyph%16)*12;ctx->r5=(glyph/16)*12;
}
extern "C" int dora3_localization_glyph(uint8_t* ram,recomp_context* ctx) {
    const uint32_t address=uint32_t(ctx->r4);
    if(address<plain_slot||address+2>widget_slot+slot_size)return 0;
    const unsigned code=(unsigned(byte(ram,address))<<8)|byte(ram,address+1);
    if(code<0x4000||code-0x4000>=pairs.size())return 0;
    ctx->r2=256+code-0x4000;return 1;
}
extern "C" int dora3_localization_draw_pair(uint8_t* ram,recomp_context* ctx) {
    const unsigned x=uint32_t(ctx->r4),y=uint32_t(ctx->r5);
    if(y<192||x>=192||x%12||y%12)return 0;
    const unsigned index=y/12*16+x/12-256;if(index>=pairs.size())return 0;
    const uint16_t pair=pairs[index];
    const auto old_texture=word(ram,uint32_t(ctx->r29)+0x10);
    const float screen_x=std::bit_cast<float>(uint32_t(ctx->r6));
    // Only argument registers and the caller-owned stack argument change.
    // Recursive calls have normal atlas coordinates, so bypass this adapter.
    word(ram,uint32_t(ctx->r29)+0x10)=int32_t(arena);
    for(unsigned n=0;n<2;++n) {
        const unsigned glyph=(pair>>(n?0:8))&255;if(glyph==0)continue;
        recomp_context draw{};dora3::copy_recomp_context(draw,*ctx);
        draw.r4=(glyph%16)*12;draw.r5=(glyph/16)*12;
        draw.r6=int32_t(std::bit_cast<uint32_t>(screen_x+n*6.f));
        dora3_common_glyph_draw(ram,&draw);
    }
    word(ram,uint32_t(ctx->r29)+0x10)=old_texture;return 1;
}

#else
// Headless research builds retain the original ROM behaviour. This does not
// run or validate that target; it only keeps the shared generated hooks linked.
namespace dora3::localization {
void reset(){} void initialize_memory(uint8_t*){}
}
extern "C" {
void dora3_localization_plain(uint8_t*,recomp_context*){}
void dora3_localization_context_plain(uint8_t*,recomp_context*){}
void dora3_localization_dialogue_begin(uint8_t*,recomp_context*){}
void dora3_localization_dialogue_line(uint8_t*,recomp_context*){}
void dora3_localization_script_load(uint8_t*,recomp_context*){}
void dora3_localization_bind(uint8_t*,recomp_context*){}
void dora3_localization_widget_line(uint8_t*,recomp_context*){}
int dora3_localization_material(uint8_t*,recomp_context*){return 0;}
int dora3_localization_sprite(uint8_t*,recomp_context*){return 0;}
void dora3_localization_choice(uint8_t*,recomp_context*){}
int dora3_localization_glyph(uint8_t*,recomp_context*){return 0;}
int dora3_localization_draw_pair(uint8_t*,recomp_context*){return 0;}
}
#endif
