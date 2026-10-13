#include "manual_fps_trace.hpp"
#ifdef DORA3_CRASH_LOG
#include "native_game_loop.hpp"
#include "native_scene_policy.hpp"
#include "native_cutscene.hpp"
#include "native_minigames.hpp"
#include "native_gameplay_preview.hpp"
#include "native_model_animation.hpp"
#include "native_frame_bridge.hpp"
#include "guest_storage.hpp"
#include <array>
#include <algorithm>
#include <bit>
#include <chrono>
#include <cstdarg>
#include <cstdio>
#include <mutex>
#include <unordered_map>
#include <vector>
extern "C" int dora3_overlay_loaded(std::uint32_t);
extern "C" int dora3_native_loop_cutscene_active();
extern "C" int dora3_native_loop_minigame_active();
namespace dora3::manual_trace {
namespace {
using Clock=std::chrono::steady_clock;
std::mutex output_mutex;
const auto started=Clock::now();
unsigned segment=0;std::uint64_t sequence=0;
int mode=-1,limit=0,wide=-1,hud=-1;
struct Gate {unsigned segment=~0u,count=0;std::uint64_t time=0;};
std::unordered_map<std::uint64_t,Gate> gates;
std::uint64_t elapsed(){return std::uint64_t(std::chrono::duration_cast<std::chrono::microseconds>(Clock::now()-started).count());}
void prefix(const char* kind) {
    const auto wall=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    std::fprintf(stderr,"MF151 n=%llu segment=%u t_us=%llu wall_ms=%lld mode=%s limit=%d wide=%d hud=%d %s ",
        (unsigned long long)++sequence,segment,(unsigned long long)elapsed(),(long long)wall,mode==0?"Original":mode==2?"Manual":"Unset",limit,wide,hud,kind);
}
bool pointer(std::uint32_t a,unsigned n){return !(a&3)&&a>=0x80000000u&&std::uint64_t(a)+n<=0x80800000ull;}
std::uint32_t word(std::uint8_t* rdram,std::uint32_t a){return std::uint32_t(MEM_W(0,std::int32_t(a)));}
double real(std::uint8_t* ram,std::uint32_t a){return double(std::bit_cast<float>(word(ram,a)));}
bool world(std::uint8_t* ram){return ram&&dora3_overlay_loaded(0x70A50);}
bool credits(std::uint8_t* ram){return ram&&dora3_overlay_loaded(0x635C0)&&word(ram,0x801A4514)==0x801C267C;}
bool masks(std::uint8_t* ram){return world(ram)||credits(ram);}
void actor(std::uint8_t* rdram,std::uint32_t a,const char* stage,const char* family,unsigned slot,std::uint32_t control=0) {
    if(!pointer(a,0xCC))return;
    const auto p=word(rdram,a+0x98);if(!pointer(p,0xF4))return;
    const auto m=word(rdram,a+0x74),m2=word(rdram,a+0x7C);
    auto& animation=dora3::native_time::game_model_animation();
    log("POSE stage=%s family=%s slot=%u actor=%08X state=%d physics=%08X type=%d physflags=%u,%u,%u phase=%d parent=%d xyz=%.9g,%.9g,%.9g local=%.9g,%.9g,%.9g angles=%.9g,%.9g,%.9g heading=%.9g scale=%.9g,%.9g,%.9g matrixT=%.9g,%.9g,%.9g dirty=%u model=%08X clip=%.9g model2=%08X clip2=%.9g allocation=%llu run=%llu control=%08X ratesZ=%.9g ctrlZ=%.9g ctrlflag=%u",
        stage,family,slot,a,int(word(rdram,a+0x94)),p,int(MEM_H(0x20,std::int32_t(p))),unsigned(MEM_BU(0xC8,std::int32_t(p))),unsigned(MEM_BU(0xC9,std::int32_t(p))),unsigned(MEM_BU(0xCA,std::int32_t(p))),int(MEM_H(0x34,std::int32_t(p))),int(MEM_H(0x7C,std::int32_t(p))),
        real(rdram,a+0x9C),real(rdram,a+0xA0),real(rdram,a+0xA4),real(rdram,a),real(rdram,a+4),real(rdram,a+8),real(rdram,a+0xC),real(rdram,a+0x10),real(rdram,a+0x14),real(rdram,p+4),real(rdram,a+0x18),real(rdram,a+0x1C),real(rdram,a+0x20),real(rdram,a+0x54),real(rdram,a+0x58),real(rdram,a+0x5C),unsigned(MEM_BU(0x6A,std::int32_t(a))),
        m,pointer(m,0x80)?real(rdram,m+0x64):0.,m2,pointer(m2,0x80)?real(rdram,m2+0x64):0.,(unsigned long long)animation.lifetime_token(m),(unsigned long long)animation.run_token(m),control,control?real(rdram,control+0x20):0.,control?real(rdram,control+0x38):0.,control?unsigned(MEM_HU(6,std::int32_t(control))):0u);
}
}
bool gate(unsigned channel,unsigned slot,std::uint64_t interval_us,unsigned max_count) {
    std::lock_guard lock(output_mutex);auto& g=gates[(std::uint64_t(channel)<<32)|slot];
    const auto now=elapsed();if(g.segment!=segment)g={segment,0,0};
    if(g.count>=max_count||(g.count&&now-g.time<interval_us))return false;
    ++g.count;g.time=now;return true;
}
void log(const char* format,...) {
    std::lock_guard lock(output_mutex);prefix("DATA");
    va_list args;va_start(args,format);std::vfprintf(stderr,format,args);va_end(args);std::fputc('\n',stderr);
}
void settings(int new_mode,int fps,int aspect,int new_hud,const char* reason) {
    std::lock_guard lock(output_mutex);
    if(mode==new_mode&&limit==fps&&wide==aspect&&hud==new_hud)return;
    const int old_mode=mode,old_limit=limit;
    mode=new_mode;limit=fps;wide=aspect;hud=new_hud;++segment;
    prefix("FPS_MARKER");std::fprintf(stderr,"from=%s/%d to=%s/%d reason=%s frame_trace_budget_restarted=1\n",old_mode==0?"Original":old_mode==2?"Manual":"Unset",old_limit,mode==0?"Original":"Manual",limit,reason);
}
void frame(std::uint8_t* rdram,const char* stage) {
    if(!rdram)return;
    const bool post=stage[0]=='p';const bool w=world(rdram),race=dora3_overlay_loaded(0x1961B0);
    if(gate(1,unsigned(post),500000)) {
        const auto* policy=select_native_scene_policy(rdram);
        log("FRAME stage=%s native_build=%d requested=%u banks=%d gameplay=%d cutscene=%d minigame=%d eligible_policy=%s fallback=%s update=%08X draw=%08X divider=%d scene=%d scripted=%d pending=%08X,%08X pause=%d,%d graphics=%d,%d copy=%d hold=%d,%d origin=%.9g,%.9g,%.9g",
            stage,dora3_native_frame_build_active(),dora3_requested_native_fps(),dora3_native_loop_owns_frame_banks(),dora3_native_loop_gameplay_active(),dora3_native_loop_cutscene_active(),dora3_native_loop_minigame_active(),policy?policy->name:"none",native_scene_fallback_reason(rdram),word(rdram,0x801A4500),word(rdram,0x801A4514),int(word(rdram,0x801A39DC)),int(MEM_H(0,std::int32_t(0x801B36B8))),w?int(word(rdram,0x80218AD8)):-1,w?word(rdram,0x802170EC):0,w?word(rdram,0x80217104):0,w?int(word(rdram,0x8021D484)):-1,w?int(word(rdram,0x8021D494)):-1,int(word(rdram,0x801AA648)),int(word(rdram,0x801A9500)),int(word(rdram,0x801A9C88)),int(word(rdram,0x801A48E0)),int(word(rdram,0x801B1400)),real(rdram,0x800DAE54),real(rdram,0x800FE878),real(rdram,0x800DD3F0));
    }
    if(w&&(dora3_overlay_loaded(0x10DA30)||dora3_overlay_loaded(0x111E70)||dora3_overlay_loaded(0x11D570)))
        for(unsigned i=30;i<dora3::distance::common_end;++i) {
            const auto a=0x804B1CD0u+i*0xCC,p=word(rdram,a+0x98);
            if(!word(rdram,a+0x94)||!pointer(p,0xF4))continue;
            const int type=MEM_H(0x20,std::int32_t(p));
            if(((type==446||type==447)&&dora3_overlay_loaded(0x10DA30))||((type>=454&&type<=469)&&dora3_overlay_loaded(0x111E70))||((type>=472&&type<=477)&&dora3_overlay_loaded(0x11D570)))
                if(gate(2+unsigned(post),i,16000,600))actor(rdram,a,stage,"boss",i);
        }
    if(credits(rdram)&&gate(20,unsigned(post),16000,600)) {
        log("CREDITS stage=%s native_build=%d line=%u machine_phase=%u phase=%.9g bob=%.9g shutter=%u",stage,dora3_native_frame_build_active(),word(rdram,0x801C69D4),word(rdram,0x801C69D0),real(rdram,0x801C69CC),real(rdram,0x801C69F4),word(rdram,0x801C69EC));
        for(unsigned i=0;i<7;++i) {
            const auto a=0x804B1CD0u+(i==6?10u:i)*0xCC;
            if(!word(rdram,a+0x94))continue;
            log("CREW stage=%s slot=%u actor=%08X xyz=%.9g,%.9g,%.9g world=%.9g,%.9g,%.9g matrixT=%.9g,%.9g,%.9g",stage,i,a,real(rdram,a),real(rdram,a+4),real(rdram,a+8),real(rdram,a+0x9C),real(rdram,a+0xA0),real(rdram,a+0xA4),real(rdram,a+0x54),real(rdram,a+0x58),real(rdram,a+0x5C));
        }
    }
    if(race)for(unsigned i=0;i<4;++i)if(gate(4+unsigned(post),i,16000,600)) {
        const auto d=0x801CCAD0u+i*0xA8,c=0x801A99F8u+i*0x34,a=0x804B1CD0u+(i+1)*0xCC;
        log("RACE stage=%s view=%u native_build=%d active=%d race_state=%d driver=%08X speed=%.9g square=%.9g brake=%.9g,%.9g lateral=%.9g heading=%.9g total=%.9g rates=%.9g,%.9g,%.9g,%.9g projected=%.9g,%.9g snapshot=%.9g,%.9g walls=%u xyz=%.9g,%.9g,%.9g origin=%.9g,%.9g,%.9g camera=%.9g,%.9g,%.9g eye=%.9g,%.9g,%.9g camangles=%.9g,%.9g,%.9g buttons=%u",
            stage,i,dora3_native_frame_build_active(),dora3_native_loop_minigame_active(),int(MEM_H(0,std::int32_t(0x801CCA94))),d,real(rdram,d+8),real(rdram,d+4),real(rdram,d+0xC),real(rdram,d+0x10),real(rdram,d+0x14),real(rdram,d+0x1C),real(rdram,d+0x3C),real(rdram,d+0x24),real(rdram,d+0x28),real(rdram,d+0x2C),real(rdram,d+0x30),real(rdram,d+0x34),real(rdram,d+0x38),real(rdram,d+0x50),real(rdram,d+0x58),unsigned(MEM_HU(0x5C,std::int32_t(d))),real(rdram,a+0x9C),real(rdram,a+0xA0),real(rdram,a+0xA4),real(rdram,0x801CC9E8+4*i),real(rdram,0x801CCAB0+4*i),real(rdram,0x801CD028+4*i),real(rdram,c),real(rdram,c+4),real(rdram,c+8),real(rdram,c+0xC),real(rdram,c+0x10),real(rdram,c+0x14),real(rdram,c+0x18),real(rdram,c+0x1C),real(rdram,c+0x30),unsigned(MEM_HU(0,std::int32_t(0x801A48EC+i*6))));
    }
    if(w&&word(rdram,0x80218AD8)!=0)for(unsigned i=0;i<60;++i) {
        const auto control=0x80218B10u+i*0xD4;
        if(MEM_H(4,std::int32_t(control))&&gate(6+unsigned(post),i,16000,600))actor(rdram,word(rdram,control),stage,"script",i,control);
    }
}
void scope(std::uint8_t* rdram,recomp_context* c,unsigned role,unsigned origin,unsigned aspect) {
    if(!rdram||!c||!masks(rdram)||!gate(8,role,100000,900))return;
    const auto sp=std::uint32_t(c->r29);std::array<std::uint32_t,6> stack{};
    if(pointer(sp,0x28))for(unsigned i=0;i<6;++i)stack[i]=word(rdram,sp+0x10+4*i);
    log("OVERLAY role=%u origin=%X aspect=%u caller=%08X native_build=%d a=%08X,%08X,%08X,%08X f12=%.9g f14=%.9g stack10_24=%08X,%08X,%08X,%08X,%08X,%08X shutter_mode=%d cursor=%08X",role,origin,aspect,std::uint32_t(c->r31),dora3_native_frame_build_active(),std::uint32_t(c->r4),std::uint32_t(c->r5),std::uint32_t(c->r6),std::uint32_t(c->r7),double(c->f12.fl),double(c->f14.fl),stack[0],stack[1],stack[2],stack[3],stack[4],stack[5],int(std::int16_t(stack[4])),word(rdram,0x801B1424));
}
void display_list(std::uint8_t* rdram,std::uint32_t begin,std::uint32_t end) {
    if(!masks(rdram)||end<=begin||!dora3::distance::frame_pointer(begin,std::uint64_t(end)-begin))return;
    const bool room=world(rdram)&&MEM_H(0,std::int32_t(0x801B36B8))==311;
    const bool ending=credits(rdram),detailed=room||ending;
    if(!gate(9,room?311u:ending?635u:0u,500000,detailed?12:48))return;
    // Read emitted commands only. Resolve the actual RSP segment bindings;
    // test150 missed segmented nested material lists and the middle rows.
    const auto readable=[&](std::uint32_t a,unsigned n){return pointer(a,n)||dora3::distance::frame_pointer(a,n);};
    std::array<std::uint32_t,16> segments{};
    const auto absolute=[](std::uint32_t a){return a>=0x80000000u?a:(a&0x1FFFFFFFu)|0x80000000u;};
    const auto resolve=[&](std::uint32_t a){
        const auto segment=a>>24;
        return segment<16&&segments[segment]?segments[segment]+(a&0xFFFFFFu):absolute(a);
    };
    unsigned visited=0,records=0,images=0,rectangles=0;std::uint64_t hash=1469598103934665603ull;
    struct List{std::uint32_t pc,end;unsigned depth;};std::vector<List> todo{{begin,end,0}};
    struct Row{std::uint32_t pc,a,b,resolved;unsigned depth;};std::vector<Row> rows;rows.reserve(2048);
    while(!todo.empty()&&visited<16000) {
        auto list=todo.back();todo.pop_back();
        for(auto p=list.pc;p+8<=list.end&&visited<16000;p+=8) {
            if(!readable(p,8))break;
            const auto a=word(rdram,p),b=word(rdram,p+4),op=a>>24;++visited;
            hash=(hash^a)*1099511628211ull;hash=(hash^b)*1099511628211ull;
            if(op==0xDB&&(a&0x00FF0000u)==0x00060000u) {
                const auto index=(a&0xFFFFu)/4;if(index<16)segments[index]=absolute(b);
            }
            if(op==0xFD)++images;if(op==0xE4||op==0xE5||op==0xF6)++rectangles;
            if(op==0xFD||op==0xED||op==0xE4||op==0xE5||op==0xF6||op==0x64||
               (detailed&&(op==0xDB||op==0xDA||op==0xDC||op==0x01||op==0xFC||op==0xFA||op==0xFB||op==0xF9||op==0xF2||op==0xDE)))
                rows.push_back({p,a,b,resolve(b),list.depth});
            if(op==0xDF)break;
            if(op==0xDE&&list.depth<8) {
                const auto target=resolve(b);
                if(readable(target,8)) {
                    if(!(a&0x10000))todo.push_back({p+8,list.end,list.depth});
                    const auto bound=pointer(target,8)?std::min(target+0x10000u,0x80800000u):target+0x10000u;
                    todo.push_back({target,bound,list.depth+1});break;
                }
            }
        }
    }
    for(unsigned i=0;i<rows.size();++i)if((detailed&&(i<2048||rows.size()-i<=128))||(!detailed&&(i<32||rows.size()-i<=96))) {
        const auto& row=rows[i];const auto op=row.a>>24;
        log("DL pc=%08X depth=%u index=%u/%zu op=%02X w0=%08X w1=%08X resolved=%08X xy0=%u,%u xy1=%u,%u",row.pc,row.depth,i,rows.size(),op,row.a,row.b,row.resolved,(row.b>>12)&0xFFF,row.b&0xFFF,(row.a>>12)&0xFFF,row.a&0xFFF);++records;
        if(detailed&&op==1) {
            const unsigned count=(row.a>>12)&255;
            if(count&&count<=32&&readable(row.resolved,count*16)) {
                std::uint64_t identity=1469598103934665603ull;
                for(unsigned j=0;j<count*4;++j)identity=(identity^word(rdram,row.resolved+j*4))*1099511628211ull;
                log("VERTICES pc=%08X address=%08X count=%u hash=%016llX",row.pc,row.resolved,count,(unsigned long long)identity);
                for(unsigned j=0;j<std::min(count,4u);++j) {
                    const auto v=std::int32_t(row.resolved+j*16);
                    log("VERTEX pc=%08X index=%u xyz=%d,%d,%d st=%d,%d rgba=%u,%u,%u,%u",row.pc,j,int(MEM_H(0,v)),int(MEM_H(2,v)),int(MEM_H(4,v)),int(MEM_H(8,v)),int(MEM_H(10,v)),unsigned(MEM_BU(12,v)),unsigned(MEM_BU(13,v)),unsigned(MEM_BU(14,v)),unsigned(MEM_BU(15,v)));
                }
            }
        }
        if(detailed&&op==0xDA&&(row.a&1)&&readable(row.resolved,64)) {
            const auto fixed=[&](unsigned index){const auto v=std::int32_t(row.resolved);return double(std::int32_t((std::uint32_t(std::uint16_t(MEM_H(index*2,v)))<<16)|std::uint16_t(MEM_H(index*2+32,v))))/65536.;};
            log("PROJECTION pc=%08X address=%08X flags=%u m00=%.9g m11=%.9g m22=%.9g m23=%.9g m32=%.9g m33=%.9g",row.pc,row.resolved,row.a&255,fixed(0),fixed(5),fixed(10),fixed(11),fixed(14),fixed(15));
        }
    }
    log("DL_SUMMARY begin=%08X end=%08X words=%u textures=%u rectangles=%u rows=%u hash=%016llX native_build=%d room311=%d credits=%d",begin,end,visited,images,rectangles,records,(unsigned long long)hash,dora3_native_frame_build_active(),room,ending);
}
}
#endif
