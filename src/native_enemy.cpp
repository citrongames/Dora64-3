#include "manual_fps_trace.hpp"
#include "graphics_profile.hpp"
#ifdef DORA3_CRASH_LOG
#include "crash_context.hpp"
#include <chrono>
#include <cstdio>
#endif
#include "performance_profile.hpp"
#include "guest_storage.hpp"
#include "native_slot_reset.hpp"
#include "native_enemy.hpp"
#include "native_frame_bridge.hpp"
#include "native_gameplay_preview.hpp"
#include "native_scene_objects.hpp"
#include "native_enemy_projection_math.hpp"
#include "native_model_animation.hpp"
#include "recomp_context_copy.hpp"
#include <array>
#include <algorithm>
#include <bit>
#include <cmath>
#include <complex>
#include <optional>
#include <memory>
#include <stdexcept>
#include <vector>
#include <map>
#include <tuple>

extern "C" int dora3_overlay_loaded(uint32_t);
extern "C" void dora3_s33_remaining_8021FED8(uint8_t*,recomp_context*);
using namespace dora3::native_time;
namespace {
constexpr uint32_t pool=0x804B34B8u;
constexpr uint32_t stride=0xCCu;
using Definition=dora3::scene_objects::Definition;
// Registered callback/contact closures have explicit ROM identity.
// All218 reviewed enemy/AI owners are registered, including linked parts,
// projectiles, seven boss families and their shared outbound effects.
constexpr std::array definitions{
    Definition{264,0x12FD90,0x8022DC90,0x8022D750},
    Definition{265,0x12FD90,0x8022E52C,0x8022DD90},
    Definition{266,0x0CEC60,0x8021FAE4,0x8021F1D0},
    Definition{267,0x0CEC60,0x80220830,0x8021FBFC},
    Definition{268,0x0CEC60,0x80220CD0,0x802208D0},
    Definition{341,0x0CEC60,0x8021E29C,0x8021D750},
    Definition{342,0x0CEC60,0x8021EC7C,0x8021E3B4},
    Definition{343,0x0CEC60,0x8021F044,0x8021ED04},
    Definition{344,0x0CEC60,0x8021F044,0x8021ED04},
    Definition{345,0x0CEC60,0x8021F0A8,0x8021F0A0},
    Definition{346,0x0CEC60,0x80223748,0x80222B60},
    Definition{347,0x0CEC60,0x8022438C,0x80223854},
    Definition{269,0x0D9910,0x80220358,0x8021FCD0},
    Definition{270,0x0D9910,0x80220BAC,0x80220458},
    Definition{271,0x0D9910,0x8022164C,0x80220CC4},
    Definition{359,0x0EDDB0,0x8021E600,0x8021D920},
    Definition{360,0x0EDDB0,0x8021F428,0x8021E794},
    Definition{361,0x0EDDB0,0x8021F618,0x8021F4B0},
    Definition{362,0x0EDDB0,0x80220538,0x8021F874},
    Definition{363,0x0EDDB0,0x802207C8,0x802205D8},
    Definition{387,0x0EDDB0,0x80220E10,0x80220900},
    Definition{388,0x0EDDB0,0x802218A8,0x80220F98},
    Definition{389,0x0EDDB0,0x80222608,0x80221B78},
    Definition{390,0x0EDDB0,0x80223744,0x80222818},
    Definition{391,0x0EDDB0,0x80223880,0x80223878},
    Definition{277,0x0D9910,0x8021E288,0x8021D760},
    Definition{278,0x0D9910,0x8021F444,0x8021E3D0},
    Definition{279,0x0D9910,0x8021F77C,0x8021F4E4},
    Definition{280,0x0D9910,0x8021FBA4,0x8021F8A0},
    Definition{297,0x0CEC60,0x80221898,0x80220DD8},
    Definition{298,0x0CEC60,0x802223E4,0x80221AD4},
    Definition{299,0x0CEC60,0x80222A74,0x80222478},
    Definition{272,0x12FD90,0x8022F0EC,0x8022E68C},
    Definition{273,0x12FD90,0x8022F56C,0x8022F174},
    Definition{274,0x121280,0x8022DE08,0x8022D768},
    Definition{275,0x121280,0x8022F314,0x8022DF0C},
    Definition{276,0x121280,0x8022F67C,0x8022F3A8},
    Definition{312,0xEBAC0,0x8021E620,0x8021D7DC},
    Definition{313,0xEBAC0,0x8021E850,0x8021E6C0},
    Definition{314,0xEBAC0,0x8021F39C,0x8021E97C},
    Definition{315,0xEBAC0,0x8021F804,0x8021F490},
    Definition{316,0x12FD90,0x8022FF00,0x8022F658},
    Definition{317,0x12FD90,0x80230384,0x802300A0},
    Definition{287,0xDFC40,0x8021E8C4,0x8021D768},
    Definition{288,0xDFC40,0x8021F070,0x8021E9E8},
    Definition{289,0xDFC40,0x80220190,0x8021F0F8},
    Definition{308,0x142670,0x8022F598,0x8022EC2C},
    Definition{309,0x142670,0x8022FB84,0x8022F75C},
    Definition{310,0x142670,0x80230310,0x8022FE08},
    Definition{311,0xEDDB0,0x8021F67C,0x8021F674},
    Definition{320,0xD5D10,0x802210A8,0x802208A4},
    Definition{321,0x11F1C0,0x8022F3F4,0x8022EF54},
    Definition{322,0x12E230,0x8022DC10,0x8022D768},
    Definition{323,0x12E230,0x8022E4B4,0x8022DD68},
    Definition{324,0x12E230,0x8022E7C0,0x8022E5B8},
    Definition{325,0x129A30,0x8022D958,0x8022D750},
    Definition{326,0x125180,0x8022DA28,0x8022D750},
    Definition{374,0x12FD90,0x80230558,0x802306E0},
    Definition{375,0x12FD90,0x80230558,0x802306E0},
    Definition{376,0x12FD90,0x80230558,0x802306E0},
    Definition{377,0x1561E0,0x8023F628,0x8023F1CC},
    Definition{378,0x1561E0,0x8023F628,0x8023F1CC},
    Definition{379,0x126BA0,0x8022FDE0,0x8022FDD8},
    Definition{380,0x126BA0,0x8022FAB8,0x8022FAB0},
    Definition{381,0x126BA0,0x8022FBF0,0x8022FBE8},
    Definition{354,0x150C60,0x8023DC5C,0x8023DA9C},
    Definition{355,0x150C60,0x8023D7C4,0x8023D6D8},
    Definition{356,0x150C60,0x8023D8D8,0x8023D820},
    Definition{357,0x14D4C0,0x8023E4C0,0x8023E348},
    Definition{358,0x14D4C0,0x8023E13C,0x8023E134},
    Definition{392,0x129A30,0x80231014,0x80230F9C},
    Definition{364,0x126BA0,0x8022DEA8,0x8022D77C},
    Definition{365,0x126BA0,0x8022E460,0x8022DFD0},
    Definition{370,0x121280,0x8022FDD8,0x8022FDD0},
    Definition{371,0x121280,0x802304C0,0x80230080},
    Definition{372,0x121280,0x80230840,0x802305D0},
    Definition{373,0x121280,0x80230FA0,0x80230914},
    Definition{318,0x142670,0x8022DE70,0x8022D768},
    Definition{319,0x142670,0x8022EA84,0x8022DF88},
    Definition{281,0xD9910,0x80222458,0x80221764},
    Definition{282,0xD9910,0x802236D0,0x80222564},
    Definition{283,0xDFC40,0x80220DF4,0x8022029C},
    Definition{284,0xDFC40,0x802213E4,0x80220F00},
    Definition{285,0xDFC40,0x80221740,0x80221464},
    Definition{286,0xDFC40,0x802218BC,0x80221794},
    Definition{290,0x11F1C0,0x8022DB0C,0x8022D750},
    Definition{291,0x11F1C0,0x8022E94C,0x8022DC54},
    Definition{292,0x11F1C0,0x8022EE20,0x8022E9EC},
    Definition{293,0xDFC40,0x8022219C,0x802219A8},
    Definition{294,0xDFC40,0x80222AB4,0x80222AAC},
    Definition{295,0xDFC40,0x80222A2C,0x8022229C},
    Definition{296,0xDFC40,0x80222B68,0x80222AAC},
    Definition{382,0x129A30,0x80230568,0x80230244},
    Definition{383,0x129A30,0x802305D4,0x802305C4},
    Definition{384,0x129A30,0x80230878,0x80230614},
    Definition{385,0x129A30,0x80230B80,0x80230974},
    Definition{386,0x129A30,0x80230EC8,0x80230C68},
    Definition{398,0x129A30,0x8023179C,0x80231538},
    Definition{393,0x1561E0,0x8024046C,0x802403D8},
    Definition{394,0x70A50,0x801E5D84,0x801E5D7C},
    Definition{395,0x12E230,0x8022ECBC,0x8022E908},
    Definition{396,0x12E230,0x8022ED4C,0x8022ED44},
    Definition{397,0x12E230,0x8022F054,0x8022EDB8},
    Definition{327,0x125180,0x8022E4C8,0x8022DBEC},
    Definition{328,0x125180,0x8022E81C,0x8022E550},
    Definition{329,0x125180,0x8022EC34,0x8022E878},
    Definition{339,0xE5480,0x802234D0,0x80222D90},
    Definition{340,0xE5480,0x802239C0,0x80223558},
    Definition{330,0xE5480,0x8021E240,0x8021D7A8},
    Definition{331,0xE5480,0x8021E994,0x8021E3C8},
    Definition{332,0xE5480,0x8021F5D0,0x8021EA94},
    Definition{333,0xE5480,0x8021FB68,0x8021F670},
    Definition{334,0xE5480,0x802209C4,0x8021FC54},
    Definition{335,0xE5480,0x80220F54,0x80220A4C},
    Definition{336,0xE5480,0x80221B54,0x8022105C},
    Definition{337,0xE5480,0x80222AA0,0x80221C6C},
    Definition{338,0xE5480,0x80222C74,0x80222B40},
    Definition{366,0x129A30,0x8022EC80,0x8022E028},
    Definition{367,0x129A30,0x8022EEC0,0x8022EE70},
    Definition{368,0x129A30,0x8022FDB8,0x8022EF68},
    Definition{369,0x129A30,0x80230080,0x8022FE4C},
    Definition{348,0xD5D10,0x8021E118,0x8021D8A4},
    Definition{349,0xD5D10,0x8021E7E0,0x8021E230},
    Definition{350,0xD5D10,0x8021F578,0x8021E944},
    Definition{351,0xD5D10,0x80220220,0x8021F720},
    Definition{352,0xD5D10,0x80220544,0x802202A8},
    Definition{353,0xD5D10,0x802207B8,0x802205A0},
    Definition{300,0x1561E0,0x8023E220,0x8023D8C4},
    Definition{301,0x1561E0,0x8023EFB0,0x8023ED54},
    Definition{302,0x1561E0,0x8023EFB0,0x8023ED54},
    Definition{303,0x1561E0,0x8023E438,0x8023E430},
    Definition{304,0x1561E0,0x8023E32C,0x8023E324},
    Definition{305,0x1561E0,0x8023ECF8,0x8023E764},
    Definition{306,0x1561E0,0x8023EFB0,0x8023ED54},
    Definition{307,0x1561E0,0x8023EFB0,0x8023ED54},
    Definition{399,0xF5200,0x8021EB4C,0x8021D7EC},
    Definition{400,0xF5200,0x8021EDF0,0x8021ED1C},
    Definition{401,0xF5200,0x8021F140,0x8021EE4C},
    Definition{402,0xF5200,0x8021F464,0x8021F2C8},
    Definition{403,0xF5200,0x8021FD84,0x8021F4C0},
    Definition{404,0xF5200,0x8021FE94,0x8021FE28},
    Definition{405,0xF5200,0x802203BC,0x8021FED0},
    Definition{406,0xF5200,0x80220654,0x80220460},
    Definition{407,0xF5200,0x802208D8,0x802206B0},
    Definition{408,0xF5200,0x80220A14,0x80220934},
    Definition{409,0xF5200,0x80220B28,0x80220AC8},
    Definition{410,0xF8A40,0x80228994,0x802229F0},
    Definition{411,0xF8A40,0x8021E814,0x8021D70C},
    Definition{412,0xF8A40,0x8021FE0C,0x8021E8C8},
    Definition{413,0xF8A40,0x8021FEC8,0x8021FEC0},
    Definition{414,0xF8A40,0x802204FC,0x8021FF3C},
    Definition{415,0xF8A40,0x802206B4,0x80220558},
    Definition{416,0xF8A40,0x80220718,0x80220710},
    Definition{417,0xF8A40,0x80220718,0x80220710},
    Definition{418,0xF8A40,0x80220988,0x8022080C},
    Definition{419,0xF8A40,0x80220B28,0x802209E4},
    Definition{420,0xF8A40,0x80220D48,0x80220B84},
    Definition{421,0xF8A40,0x80220FA8,0x80220F30},
    Definition{422,0xF8A40,0x802212E0,0x80221114},
    Definition{423,0xF8A40,0x80221FE4,0x8022133C},
    Definition{424,0xF8A40,0x8022223C,0x802220E8},
    Definition{425,0xF8A40,0x8022245C,0x80222298},
    Definition{426,0xF8A40,0x802225BC,0x802225B4},
    Definition{427,0xF8A40,0x80222674,0x8022266C},
    Definition{428,0x104AE0,0x80220EAC,0x8021E868},
    Definition{429,0x104AE0,0x8021DAD4,0x8021D6D8},
    Definition{430,0x104AE0,0x8021DDC8,0x8021DB30},
    Definition{431,0x104AE0,0x8021E078,0x8021DE24},
    Definition{432,0x104AE0,0x8021E120,0x8021E118},
    Definition{433,0x104AE0,0x8021E4A4,0x8021E240},
    Definition{434,0x104AE0,0x8021E534,0x8021E52C},
    Definition{435,0x108730,0x8022243C,0x8021F7CC},
    Definition{436,0x108730,0x8021DB50,0x8021D6D8},
    Definition{437,0x108730,0x8021E288,0x8021DBAC},
    Definition{438,0x108730,0x8021E808,0x8021E2E4},
    Definition{439,0x108730,0x8021E9E0,0x8021E914},
    Definition{440,0x108730,0x8021EDF0,0x8021EA3C},
    Definition{441,0x108730,0x8021EF6C,0x8021EE4C},
    Definition{442,0x108730,0x8021EFD0,0x8021EFC8},
    Definition{443,0x108730,0x8021F4DC,0x8021F344},
    Definition{444,0x108730,0x8021F540,0x8021F538},
    Definition{445,0x108730,0x8021F634,0x8021F62C},
    Definition{446,0x10DA30,0x80221674,0x8021ECBC},
    Definition{447,0x10DA30,0x8021D6E0,0x8021D6D8},
    Definition{448,0x10DA30,0x8021DA94,0x8021D770},
    Definition{449,0x10DA30,0x8021DD18,0x8021DAF0},
    Definition{450,0x10DA30,0x8021E100,0x8021DF38},
    Definition{451,0x10DA30,0x8021E59C,0x8021E15C},
    Definition{452,0x10DA30,0x8021E634,0x8021E624},
    Definition{453,0x10DA30,0x8021E9AC,0x8021E9A4},
    Definition{454,0x111E70,0x80228098,0x80222420},
    Definition{455,0x111E70,0x8021E738,0x8021D70C},
    Definition{456,0x111E70,0x8021F914,0x8021E7EC},
    Definition{457,0x111E70,0x8021F9D0,0x8021F9C8},
    Definition{458,0x111E70,0x80220004,0x8021FA44},
    Definition{459,0x111E70,0x802201BC,0x80220060},
    Definition{460,0x111E70,0x80220220,0x80220218},
    Definition{461,0x111E70,0x80220220,0x80220218},
    Definition{462,0x111E70,0x80220490,0x80220314},
    Definition{463,0x111E70,0x80220630,0x802204EC},
    Definition{464,0x111E70,0x80220850,0x8022068C},
    Definition{465,0x111E70,0x80220AB0,0x80220A38},
    Definition{466,0x111E70,0x80220DE8,0x80220C1C},
    Definition{467,0x111E70,0x80221A1C,0x80220E44},
    Definition{468,0x111E70,0x80221C54,0x80221B00},
    Definition{469,0x111E70,0x80221E74,0x80221CB0},
    Definition{470,0x111E70,0x80221FD4,0x80221FCC},
    Definition{471,0x111E70,0x8022208C,0x80222084},
    Definition{472,0x11D570,0x8021F048,0x8021DEB4},
    Definition{473,0x11D570,0x8021D6E0,0x8021D6D8},
    Definition{474,0x11D570,0x8021D7B4,0x8021D770},
    Definition{475,0x11D570,0x8021DA74,0x8021D854},
    Definition{476,0x11D570,0x8021DAD8,0x8021DAD0},
    Definition{477,0x11D570,0x8021DCE0,0x8021DCD8},
    Definition{478,0x15E440,0x8023D944,0x8023D93C},
    Definition{257,0x70A50,0x801E53D0,0x801E5D7C},
    Definition{258,0x70A50,0x801E5598,0x801E5D7C},
    Definition{259,0x70A50,0x801E57C0,0x801E5D7C}
};
// Immutable definition records are indexed by kind at compile time. Runtime
// overlay, callback/contact/constructor and lifetime validation remain live.
// Linked scene candidates retain their original order, including duplicate kinds.
constexpr size_t definition_kind_count=[] {
    int highest=-1;
    for(const auto& value:definitions)highest=std::max(highest,value.kind);
    for(const auto& value:dora3::scene_objects::definitions)highest=std::max(highest,value.kind);
    return size_t(highest)+1;
}();
struct DefinitionIndex {
    std::array<int,definition_kind_count> enemy{},scene{};
    std::array<int,dora3::scene_objects::definitions.size()> next{};
};
constexpr auto definition_index=[] {
    DefinitionIndex result;result.enemy.fill(-1);result.scene.fill(-1);result.next.fill(-1);
    for(size_t i=0;i<definitions.size();++i) {
        const auto kind=size_t(definitions[i].kind);
        if(result.enemy[kind]<0)result.enemy[kind]=int(i);
    }
    for(size_t i=dora3::scene_objects::definitions.size();i-->0;) {
        const auto kind=size_t(dora3::scene_objects::definitions[i].kind);
        result.next[i]=result.scene[kind];result.scene[kind]=int(i);
    }
    return result;
}();
const Definition* definition(int kind,uint8_t* rdram=nullptr) {
    if(kind<0||size_t(kind)>=definition_kind_count)return nullptr;
    if(const int selected=definition_index.enemy[size_t(kind)];selected>=0)
        return &definitions[size_t(selected)];
    for(int selected=definition_index.scene[size_t(kind)];selected>=0;selected=definition_index.next[size_t(selected)]) {
        const auto& v=dora3::scene_objects::definitions[size_t(selected)];
        if(!rdram)return &v;
        if(dora3_overlay_loaded(v.rom)&&uint32_t(MEM_W(0,int32_t(0x80202064u+unsigned(kind)*84)))==v.callback&&
           uint32_t(MEM_W(0,int32_t(0x80202068u+unsigned(kind)*84)))==v.contact&&
           uint32_t(MEM_W(0,int32_t(0x80202060u+unsigned(kind)*84)))==v.constructor)return &v;
    }
    return nullptr;
}
bool pointer(uint32_t p,unsigned size) {return !(p&3)&&p>=0x80000000u&&uint64_t(p)+size<=0x80800000ull;}
int32_t word(uint8_t* rdram,uint32_t p) {return MEM_W(0,int32_t(p));}
float real(uint8_t* rdram,uint32_t p) {return std::bit_cast<float>(word(rdram,p));}
void put(uint8_t* rdram,uint32_t p,float v) {MEM_W(0,int32_t(p))=std::bit_cast<int32_t>(v);}
double rom_double(uint8_t* rdram,uint32_t address) {
    return std::bit_cast<double>((uint64_t(uint32_t(word(rdram,address)))<<32)|uint32_t(word(rdram,address+4)));
}
unsigned index(uint32_t actor) {return (actor-pool)/stride;}
struct Identity {uint32_t actor,physical,model,callback,rom;uint64_t lifetime;int kind;};
std::optional<Identity> identity(uint8_t* rdram,uint32_t actor) {
    dora3::profile::Scope detailProfile(dora3::profile::Tag::EnemyIdentitySample,dora3::profile::sample_stage(dora3::profile::Tag::EnemyIdentitySample,16));
    if(!rdram||actor<pool||actor>=pool+dora3::distance::common_capacity*stride||(actor-pool)%stride||!word(rdram,actor+0x94))return std::nullopt;
    const auto physical=uint32_t(word(rdram,actor+0x98)),model=uint32_t(word(rdram,actor+0x74));
    if(!pointer(physical,0xDC)||!pointer(model,0x84))return std::nullopt;
    const auto kind=MEM_H(0x20,int32_t(physical));const auto* def=definition(kind,rdram);
    if(!def||!dora3_overlay_loaded(def->rom)||uint32_t(word(rdram,0x80202064u+unsigned(def->kind)*84))!=def->callback||
       uint32_t(word(rdram,0x80202068u+unsigned(def->kind)*84))!=def->contact)return std::nullopt;
    const auto lifetime=game_model_animation().lifetime_token(model);if(!lifetime)return std::nullopt;
    if(kind==272) {
        const int count=MEM_H(0x44,int32_t(physical));
        if(count<0||count>58||!pointer(physical,0x80+2*unsigned(count)))return std::nullopt;
    }
    if(kind==348) {
        const int count=MEM_H(0x42,int32_t(physical));
        if(count<0||count>23||!pointer(physical,0x80+4*unsigned(count)))return std::nullopt;
    }
    for(unsigned axis=0;axis<3;++axis)
        if(!std::isfinite(real(rdram,actor+0x9C+4*axis))||!std::isfinite(real(rdram,physical+0x10+4*axis)))return std::nullopt;
    return Identity{actor,physical,model,def->callback,def->rom,lifetime,kind};
}
bool same(const Identity& a,const Identity& b) {
    return a.actor==b.actor&&a.physical==b.physical&&a.model==b.model&&a.callback==b.callback&&a.rom==b.rom&&a.lifetime==b.lifetime&&a.kind==b.kind;
}
// Hob353 follows either its root or the preceding linked Hob. Validate the
// actual Source selector and link before reading its delayed parent motion.
int attachment_parent_slot(uint8_t* rdram,const Identity& child) {
    const int root=MEM_H(0x7C,int32_t(child.physical));
    if(child.rom==0x111E70&&((child.kind==454&&word(rdram,child.actor+0x94)==100)||(child.kind==469&&MEM_H(0x44,int32_t(child.physical))==1))) {
        if(root<30||root>=dora3::distance::common_end)return -1;
        const auto parent=identity(rdram,0x804B1CD0u+unsigned(root)*stride);
        if(!parent||parent->actor>=child.actor)return -1;
        const int selector=child.kind==469?1:MEM_H(0x42,int32_t(child.physical));
        if(selector==0)return root;
        if(selector<0||selector>9||!pointer(parent->physical,0x80+2*unsigned(selector)))return -1;
        const int selected=MEM_H(0x7E + 2*selector,int32_t(parent->physical));
        if(selected==-1)return root;
        const auto linked=selected>=30&&selected<dora3::distance::common_end?identity(rdram,0x804B1CD0u+unsigned(selected)*stride):std::nullopt;
        return linked&&linked->actor<child.actor?selected:-1;
    }
    if(child.rom==0xF8A40&&(child.kind==410||child.kind==414||child.kind==425)) {
        if(root<30||root>=dora3::distance::common_end)return -1;
        const auto parent=identity(rdram,0x804B1CD0u+unsigned(root)*stride);
        if(!parent||parent->actor>=child.actor)return -1;
        int selector=0;
        if(child.kind==410&&word(rdram,child.actor+0x94)==100)selector=MEM_H(0x42,int32_t(child.physical));
        if(child.kind==414||(child.kind==425&&MEM_H(0x44,int32_t(child.physical))==1))selector=1;
        if(selector==0)return root;
        if(selector<0||selector>8||!pointer(parent->physical,0x80+2*unsigned(selector)))return -1;
        const int selected=MEM_H(0x7E + 2*selector,int32_t(parent->physical));
        const auto linked=selected>=30&&selected<dora3::distance::common_end?identity(rdram,0x804B1CD0u+unsigned(selected)*stride):std::nullopt;
        if(selected==-1)return root; // Original retains its root fallback.
        return linked&&linked->actor<child.actor?selected:-1;
    }
    if(child.kind!=353&&child.kind!=406)return root;
    if(root<30||root>=dora3::distance::common_end)return -1;
    const auto parent=identity(rdram,0x804B1CD0u+unsigned(root)*stride);
    if(!parent||parent->actor>=child.actor)return -1;
    const int selector=MEM_H(0x44,int32_t(child.physical));
    if(selector==0)return root;
    if(selector<0||selector>(child.kind==406?2:46)||!pointer(parent->physical,0x80+2*unsigned(selector)))return -1;
    const int selected=MEM_H(0x7E + 2*selector,int32_t(parent->physical));
    if(selected<30||selected>=dora3::distance::common_end)return -1;
    const auto linked=identity(rdram,0x804B1CD0u+unsigned(selected)*stride);
    return linked&&linked->actor<child.actor?selected:-1;
}
// Boss2 detached phases use their own equations. Reattached phases copy only
// the actual parent fields; their model/health/counter commands remain Source.
bool enemy_parent_attachment_active(uint8_t* rdram,const Identity& v) {
    if(v.rom==0x11D570)return v.kind==473||(v.kind==475&&word(rdram,v.actor+0x94)==1);
    if(v.rom==0x111E70) {
        const auto state=word(rdram,v.actor+0x94);
        if(v.kind==455||v.kind==456)return state==1||state==11||state==12||(state==4&&MEM_H(0x34,int32_t(v.physical))==2);
        return (v.kind==459&&state==1)||v.kind==464||v.kind==465||v.kind==469||(v.kind==467&&state==4);
    }
    if(v.rom==0x10DA30) {
        const auto state=word(rdram,v.actor+0x94);
        return v.kind==447||(v.kind==448&&state==1)||v.kind==449||(v.kind==450&&state==1)||(v.kind==451&&state==2);
    }
    if(v.rom==0x108730)return v.kind==441&&word(rdram,v.actor+0x94)==1;
    if(v.rom!=0xF8A40)return false;
    const auto state=word(rdram,v.actor+0x94);
    if(v.kind==411||v.kind==412)return state==1||state==11||state==12||
        (state==4&&MEM_H(0x34,int32_t(v.physical))==2);
    return (v.kind==415&&state==1)||v.kind==420||v.kind==421||v.kind==425||(v.kind==423&&state==5);
}

bool scene_parent_attachment_active(uint8_t* rdram,const Identity& v) {
    if(v.kind==133&&v.rom==0x70A50)return word(rdram,v.actor+0x94)==1;
    if((v.kind==527||v.kind==528)&&v.rom==0x126BA0)return true;
    if(v.kind==564&&v.rom==0x14D4C0)return true;
    return false;
}

// Model time advances independently. State/latch/counter/callback commands
// invalidate a decision, while this module's own velocity/pose writes do not.
std::vector<uint64_t> commands(uint8_t* rdram,const Identity& v) {
    const auto kind=MEM_H(0x20,int32_t(v.physical));
    const std::array<int32_t,15> ordinary{word(rdram,v.actor+0x94),MEM_BU(0x28,int32_t(v.physical)),MEM_BU(0x29,int32_t(v.physical)),
        MEM_H(0x34,int32_t(v.physical)),MEM_H(0x36,int32_t(v.physical)),MEM_H(0x38,int32_t(v.physical)),
        MEM_H(0x3A,int32_t(v.physical)),MEM_H(0x3C,int32_t(v.physical)),MEM_H(0x3E,int32_t(v.physical)),
        MEM_H(0x40,int32_t(v.physical)),MEM_H(0x42,int32_t(v.physical)),MEM_H(0x44,int32_t(v.physical)),
        (kind==268||kind==361||kind==299||kind==273||kind==324||kind==355||kind==371||kind==373||kind==285||kind==286||kind==292||kind==383||kind==396||kind==329||kind==333||kind==338||kind==349||kind==353||kind==301||kind==302||kind==306||kind==307||kind==401||kind==403||kind==405||kind==406||(kind>=410&&kind<=477))?MEM_H(0x7C,int32_t(v.physical)):0,
        (kind==267||kind==359||kind==360||kind==297||kind==298)?MEM_H(0x80,int32_t(v.physical)):0,word(rdram,v.physical+0x48)};
    std::vector<uint64_t> result;
    for(const auto value:ordinary)result.push_back(uint64_t(int64_t(value)));
    const int child_count=kind==272?MEM_H(0x44,int32_t(v.physical)):kind==275?3:kind==288?5:kind==370||kind==373?9:kind==312||kind==316||kind==323||kind==354||kind==372||kind==283||kind==284||kind==285||kind==291||kind==382||kind==395||kind==327?1:kind==332?2:kind==337?1:kind==348?2*MEM_H(0x42,int32_t(v.physical)):kind==350||kind==351?1:kind==300||kind==305?8:kind==399?6:kind==405?3:kind==410?15:kind==428?2:kind==435?10:kind==446?5:kind==454?15:kind==472?8:0;
    // The complete link/lifetime vector also catches genuine external
    // changes. Source-owned child writes are finalized after the pool pass.
    for(int n=0;n<child_count;++n) {
        const int slot=MEM_H(int32_t(0x80+2*n),int32_t(v.physical));
        result.push_back(uint64_t(int64_t(slot)));
        uint64_t lifetime=0;
        if(slot>=30&&slot<dora3::distance::common_end) {
            const auto actor=0x804B1CD0u+unsigned(slot)*stride;
            const auto model=uint32_t(word(rdram,actor+0x74));
            if(pointer(model,0x84)&&word(rdram,actor+0x94))lifetime=game_model_animation().lifetime_token(model);
        }
        result.push_back(lifetime);
    }
    if(kind==301||kind==302||kind==306||kind==307) {
        const int slot=MEM_H(0x7C,int32_t(v.physical));
        const auto root=slot>=30&&slot<dora3::distance::common_end?identity(rdram,0x804B1CD0u+unsigned(slot)*stride):std::nullopt;
        result.push_back(root?root->lifetime:0);
        result.push_back(root?uint32_t(word(rdram,root->physical+0x48)):0);
    }
    if(kind==353||kind==406||(kind>=410&&kind<=477)) {
        for(const int slot:std::array<int,2>{MEM_H(0x7C,int32_t(v.physical)),attachment_parent_slot(rdram,v)}) {
            result.push_back(uint64_t(int64_t(slot)));
            const auto linked=slot>=30&&slot<dora3::distance::common_end?identity(rdram,0x804B1CD0u+unsigned(slot)*stride):std::nullopt;
            result.push_back(linked?linked->lifetime:0);
        }
    }
    if(kind==472) {
        for(const unsigned offset:{0xD0u,0xD8u})result.push_back(uint64_t(int64_t(MEM_H(int32_t(offset),int32_t(v.physical)))));
        result.push_back(uint32_t(word(rdram,0x8021F320)));
        result.push_back(uint16_t(MEM_H(0,int32_t(0x8021F324u))));
        for(unsigned n=0;n<8;++n) {
            const int slot=MEM_H(int32_t(0x80+2*n),int32_t(v.physical));
            const auto part=slot>=30&&slot<dora3::distance::common_end?identity(rdram,0x804B1CD0u+unsigned(slot)*stride):std::nullopt;
            result.push_back(part?uint32_t(word(rdram,part->actor+0x94)):0);
            for(const unsigned offset:{0xD0u,0xD6u,0xD8u})result.push_back(part?uint16_t(MEM_H(int32_t(offset),int32_t(part->physical))):0);
        }
    }
    // NPC contacts and damage callbacks publish the common D0/D6/D8
    // command words. A real contact can change the decision mid-period;
    // Source-pool finalization prevents its own writes from replaying the FSM.
    if((kind>=29&&kind<=70)||kind==78||kind==79)for(const unsigned offset:{0xD0u,0xD6u,0xD8u})
        result.push_back(uint16_t(MEM_H(int32_t(offset),int32_t(v.physical))));
    if(native_scene_object_kind(kind)) {
        for(const unsigned offset:{0xD0u,0xD6u,0xD8u})result.push_back(uint16_t(MEM_H(int32_t(offset),int32_t(v.physical))));
        for(const unsigned offset:{0x7Cu,0x80u,0x82u,0x84u,0x86u,0x88u,0x8Au,0x8Cu}) {
            const int slot=MEM_H(int32_t(offset),int32_t(v.physical));result.push_back(uint64_t(int64_t(slot)));
            const uint32_t linked=slot>=30&&slot<dora3::distance::common_end?0x804B1CD0u+unsigned(slot)*stride:0;
            const uint32_t model=linked?uint32_t(word(rdram,linked+0x74)):0;
            result.push_back(pointer(model,0x84)&&word(rdram,linked+0x94)?game_model_animation().lifetime_token(model):0);
        }
    }
    if(kind==262) {
        const uint32_t player=uint32_t(word(rdram,0x8021D49C));
        result.push_back(pointer(player,0xCC)?uint32_t(word(rdram,player+0x94)):0);
    }
    if(kind==477)result.push_back(MEM_BU(0,int32_t(0x801B361Au)));
    if(kind==454) {
        for(const unsigned offset:{0xD0u,0xD8u})result.push_back(uint64_t(int64_t(MEM_H(int32_t(offset),int32_t(v.physical)))));
        // Main parts carry independent damage/shell commands read by the head.
        for(unsigned n=0;n<2;++n) {
            const int slot=MEM_H(int32_t(0x80+2*n),int32_t(v.physical));
            const auto part=slot>=30&&slot<dora3::distance::common_end?identity(rdram,0x804B1CD0u+unsigned(slot)*stride):std::nullopt;
            result.push_back(part?uint32_t(word(rdram,part->actor+0x94)):0);
            result.push_back(part?uint16_t(MEM_H(0xD0,int32_t(part->physical))):0);
            result.push_back(part?uint16_t(MEM_H(0xD8,int32_t(part->physical))):0);
        }
    }
    if(kind==446) {
        for(const unsigned offset:{0xD0u,0xD6u,0xD8u})result.push_back(uint64_t(int64_t(MEM_H(int32_t(offset),int32_t(v.physical)))));
        if(word(rdram,v.actor+0x94)==6) {
            const int slot=MEM_H(0x88,int32_t(v.physical));
            const auto part=slot>=30&&slot<dora3::distance::common_end?identity(rdram,0x804B1CD0u+unsigned(slot)*stride):std::nullopt;
            result.push_back(part?game_model_animation().run_token(part->model):0);
            result.push_back(part?game_model_animation().time_command_token(part->model):0);
            result.push_back(part?uint32_t(word(rdram,part->physical+0xCC)):0);
        }
    }
    if(kind==435) {
        for(const unsigned offset:{0xD0u,0xD8u})result.push_back(uint64_t(int64_t(MEM_H(int32_t(offset),int32_t(v.physical)))));
        result.push_back(uint64_t(int64_t(MEM_H(0,int32_t(0x802182C8u)))));
    }
    if(kind==437||kind==336||kind==257) {
        // A real speed command/seek must invalidate the captured frame ramp.
        result.push_back(uint32_t(word(rdram,v.physical+0xCC)));
        result.push_back(game_model_animation().run_token(v.model));
        result.push_back(game_model_animation().time_command_token(v.model));
    }
    if(kind==428) {
        for(const unsigned offset:{0xD0u,0xD8u,0xE0u})result.push_back(uint64_t(int64_t(MEM_H(int32_t(offset),int32_t(v.physical)))));
        for(const unsigned offset:{0x74u,0x78u})result.push_back(uint32_t(word(rdram,v.physical+offset)));
    }
    if(kind>=410&&kind<=412) {
        for(const unsigned offset:{0xD0u,0xD6u,0xD8u})result.push_back(uint64_t(int64_t(MEM_H(int32_t(offset),int32_t(v.physical)))));
    }
    if(kind==399) {
        result.push_back(uint64_t(int64_t(MEM_H(0xD0,int32_t(v.physical)))));
        result.push_back(uint64_t(int64_t(MEM_H(0xD6,int32_t(v.physical)))));
        result.push_back(uint64_t(int64_t(MEM_H(0,int32_t(0x80220F10u)))));
    }
    if(kind==401) {
        const int slot=attachment_parent_slot(rdram,v);
        const auto parent=slot>=30&&slot<dora3::distance::common_end?identity(rdram,0x804B1CD0u+unsigned(slot)*stride):std::nullopt;
        result.push_back(parent?game_model_animation().run_token(parent->model):0);
    }
    return result;
}
struct Gravity {uint32_t actor=0,physical=0;float before=0,after=0;unsigned calls=0;int collision=0;};
struct Turn {uint32_t actor=0,physical=0;float before=0,after=0;unsigned calls=0;};
// Captured only at reviewed stores. A setter elsewhere cannot accidentally
// acquire a continuous equation just because its numeric delta is small.
std::complex<double> harmonic_sum(double phase,double omega,double age,double speed,double acceleration);
struct Period;
std::pair<float,float> sample_period_motion(const Period& v,double age,unsigned axis);
enum class EquationKind {parent_height_wave,cosine_angle,frame_triangle,spatial_heading,spatial_pitch,split_sine,path_heading,folded_pitch,sine_ramp,alternating_offset,radial_axis,harmonic_accumulation,harmonic_drift_angle,approach_drift_angle,affine,positive_phase,source_abs_sine,historical_axis,linear,angle,sine,cosine,sine_angle,absolute_sine,sine_drift_angle,approach,wrapped_approach,clamped,nested_sine,nested_cosine,exponential,composed_return,forced_sine_velocity,geometric_angle,phase_sine,phase_cosine,alternating_drift};
struct Equation {
    uint32_t address;EquationKind kind;float before,after;
    double phase=0,step=0,amplitude=0,radians=0,base=0,retention=0,acceleration=0;
    double initial_position=0;
    float expected=0;
    double inner_amplitude=0,outer_radians=0;
    double clamp_min=0,clamp_max=100;
    double pitch_direction=-1;
    double motion_start_age=0;
    std::shared_ptr<const Equation> upstream{},coefficient{},auxiliary{};
    std::shared_ptr<const Period> motion{};unsigned motion_axis=0;
    double forced_position(double age) const {
        const double omega=step*radians,phi=phase*radians;
        if(retention==0) {
            // Undamped Source target assignment. Retain its wave plus the
            // one-period correction from the actual starting height.
            return base+amplitude*std::sin(phi+omega*age)+
                (initial_position-base-amplitude*std::sin(phi))*(1-age);
        }
        const auto carrier=std::polar(1.,omega);
        const auto response=carrier*(std::pow(retention,age)-std::polar(1.,omega*age))/(retention-carrier);
        return base+(initial_position-base)*std::pow(retention,age)+
            (1-retention)*amplitude*(std::polar(1.,phi)*response).imag();
    }
    float evaluate(double age) const {
        if(age==1)return after;
        if(kind==EquationKind::parent_height_wave) {
            const double parent_y=sample_period_motion(*motion,motion_start_age+age,1).first;
            return float(parent_y+base+amplitude*std::sin((phase+step*age)*radians));
        }
        if(kind==EquationKind::frame_triangle) {
            const double frame=phase+step*age;
            return float((frame<24?frame:48-frame)/24);
        }

        if(kind==EquationKind::spatial_heading||kind==EquationKind::spatial_pitch) {
            const double x=base-upstream->evaluate(age),z=amplitude-coefficient->evaluate(age);
            const double heading=kind==EquationKind::spatial_heading?std::atan2(x,z):
                pitch_direction*std::atan2(initial_position-auxiliary->evaluate(age),std::hypot(x,z));
            const auto wrapped=std::fmod(float(heading/radians),360.f);return wrapped<0?wrapped+360.f:wrapped;
        }
        if(kind==EquationKind::split_sine) {
            const double p=phase+step*age;
            return float(p<=inner_amplitude?base+amplitude*std::sin(p*radians):
                initial_position+acceleration*std::sin((p-outer_radians)*radians));
        }
        if(kind==EquationKind::path_heading) {
            // Source aims from the position BEFORE its new curve store.
            const double x=upstream->evaluate(age-1),z=coefficient->evaluate(age-1);
            float heading=float(std::atan2(base-x,amplitude-z)/radians);
            heading=std::fmod(heading,360.f);return heading<0?heading+360.f:heading;
        }
        if(kind==EquationKind::folded_pitch) {
            const float pitch=upstream->evaluate(age);
            return pitch>90&&pitch<=180?180-pitch:pitch>180&&pitch<270?540-pitch:pitch;
        }
        if(kind==EquationKind::sine_ramp)
            return float(base+(amplitude+acceleration*age)*std::sin((phase+step*age)*radians));
        if(kind==EquationKind::alternating_offset) {
            const double anchor=upstream?upstream->evaluate(age):base;
            const double start=upstream?upstream->evaluate(0):base;
            const double natural=start+amplitude*std::cos(std::acos(-1.)*phase);
            return float(anchor+(amplitude+acceleration*age)*std::cos(std::acos(-1.)*(phase+age))+
                (double(before)-natural)*(1-age));
        }
        if(kind==EquationKind::radial_axis) {
            const double theta=double(upstream->evaluate(age))*radians;
            const double radius=double(coefficient->evaluate(age));
            return float(base+radius*(motion_axis==0?std::sin(theta):std::cos(theta)));
        }
        if(kind==EquationKind::harmonic_accumulation||kind==EquationKind::harmonic_drift_angle) {
            const float result=float(double(before)+harmonic_sum(phase*radians,step*radians,age,amplitude,0).imag());
            if(kind==EquationKind::harmonic_accumulation)return result;
            const auto wrapped=std::fmod(result,360.f);return wrapped<0?wrapped+360.f:wrapped;
        }
        if(kind==EquationKind::approach_drift_angle) {
            const double sum=retention==1?age:retention>0?
                retention*(-std::expm1(std::log(retention)*age))/(1-retention):0;
            const float result=float(double(before)+base*age+amplitude*sum);
            const auto wrapped=std::fmod(result,360.f);return wrapped<0?wrapped+360.f:wrapped;
        }
        if(kind==EquationKind::affine)return float(base+amplitude*double(upstream->evaluate(age)));
        if(kind==EquationKind::positive_phase)return float(base+amplitude*std::max(phase+step*age,0.));
        if(kind==EquationKind::source_abs_sine) {
            const double wave=double(float(std::sin((phase+step*age)*radians)))*amplitude;
            return float((wave<0?initial_position:base)-std::abs(wave));
        }
        if(kind==EquationKind::historical_axis)return sample_period_motion(*motion,age,motion_axis).first;
        if(kind==EquationKind::sine||kind==EquationKind::cosine||kind==EquationKind::sine_angle||kind==EquationKind::cosine_angle) {
            float value=float(base+amplitude*((kind==EquationKind::cosine||kind==EquationKind::cosine_angle)?
                std::cos((phase+step*age)*radians):std::sin((phase+step*age)*radians)));
            if(kind==EquationKind::sine_angle||kind==EquationKind::cosine_angle){value=std::fmod(value,360.f);if(value<0)value+=360.f;}
            return value;
        }
        if(kind==EquationKind::sine_drift_angle) {
            // Source bounded turn followed by a sinusoidal angular increment.
            // Sum the increments; evaluating just sin(current phase) would
            // erase the accumulated turn and make the model swing backwards.
            float value=float(double(upstream->evaluate(age))+
                harmonic_sum(phase*radians,step*radians,age,amplitude,0).imag());
            value=std::fmod(value,360.f);return value<0?value+360.f:value;
        }
        if(kind==EquationKind::absolute_sine) {
            // Derorin's death scale consumes the rounded float sine operand,
            // then abs and the ROM double baseline. Short phase stays Source.
            const float wave=float(amplitude*std::sin((phase+step*age)*radians));
            return float(base+std::abs(double(wave)));
        }
        if(kind==EquationKind::nested_sine||kind==EquationKind::nested_cosine) {
            const float inner=float(inner_amplitude*std::sin((phase+step*age)*radians));
            return float(base+amplitude*(kind==EquationKind::nested_cosine?std::cos(double(inner)*outer_radians):std::sin(double(inner)*outer_radians)));
        }
        if(kind==EquationKind::geometric_angle) {
            const double sum=retention==1?age:retention<=0?0:
                -std::expm1(std::log(retention)*age)/(1-retention);
            const float value=float(double(before)+step*sum);
            const auto wrapped=std::fmod(value,360.f);return wrapped<0?wrapped+360.f:wrapped;
        }
        if(kind==EquationKind::phase_sine||kind==EquationKind::phase_cosine) {
            const double argument=double(upstream->evaluate(age))*radians;
            return float(base+amplitude*(kind==EquationKind::phase_sine?std::sin(argument):std::cos(argument)));
        }
        if(kind==EquationKind::alternating_drift) {
            // Sum_j=1..h sign*(-1)^(j-1)*(N-j+1). This is the real
            // countdown recurrence, with the real branch of (-1)^h.
            const double sum=age/2+(phase/2+.25-age/2)*(1-std::cos(std::acos(-1.)*age));
            return float(double(before)+amplitude*sum);
        }
        if(kind==EquationKind::forced_sine_velocity)return float(forced_position(age)-forced_position(age-1));
        if(kind==EquationKind::exponential)return float(double(before)*std::pow(retention,age));
        if(kind==EquationKind::composed_return) {
            const double initial=upstream->evaluate(age),blend=coefficient->evaluate(age);
            return float(initial+(base-initial)*blend);
        }
        if(kind==EquationKind::clamped)return float(base+std::clamp(phase+step*age,clamp_min,clamp_max));
        if(kind==EquationKind::wrapped_approach) {
            const float value=float(base+(double(before)-base)*std::pow(retention,age));
            const float wrapped=std::fmod(value,360.f);return wrapped<0?wrapped+360.f:wrapped;
        }
        if(kind==EquationKind::approach)return float(base+(double(before)-base)*std::pow(retention,age));
        const float value=float(double(before)+step*age+acceleration*age*(age-1)/2);
        if(kind!=EquationKind::angle)return value;
        const auto wrapped=std::fmod(value,360.f);return wrapped<0?wrapped+360.f:wrapped;
    }
};
struct ProjectionPart {
    double speed,heading,pitch,radians;float proposal;bool xyz;
    std::optional<Equation> scalar,heading_equation,pitch_equation;
};
struct Projection {
    uint32_t physical=0;
    std::array<float,3> unit{},speed{},proposal{},first_factor{1,1,1};
    std::array<unsigned,3> calls{};
    std::array<uint32_t,3> scalar_offset{};
    std::array<bool,3> negate{};
    std::array<float,3> heading{},pitch{};
    std::array<double,3> radians{};
    std::array<bool,3> xyz{};
    float pitch_cosine=0;
    std::array<double,3> correction{};
    std::array<std::vector<ProjectionPart>,3> parts{};
    // Capture the mathematical operands actually consumed by this projection.
    // A later floor/wall query can replace the visible angle without changing
    // the earlier Source velocity proposal. Do not use that final setter as
    // proof of the equation used by E801C/E7F54/E7FC4.
    std::array<std::optional<Equation>,3> scalar_equation{},heading_equation{},pitch_equation{};
};

// ROM is part of a store's identity: e.g. 8021E030 is Brobo scalar speed in
// demo_slot0 and MiniCar visual roll in level1_slot0. VRAM alone is insufficient.
struct Store {
    uint32_t rom,site,offset;bool physical;EquationKind kind;unsigned operand;
    uint32_t coefficient=0;double target=0;
};
constexpr std::array additional_stores{
    Store{0x70A50,0x801E568C,0x18,false,EquationKind::approach,2,0x802113B0},
    Store{0x70A50,0x801E56BC,0x18,false,EquationKind::linear,0},
    Store{0x70A50,0x801E56D4,0x20,false,EquationKind::linear,0},
    Store{0x70A50,0x801E56D8,0x1C,false,EquationKind::linear,0},
    Store{0x11D570,0x8021E378,0x10,false,EquationKind::angle,0},
    Store{0x11D570,0x8021E4CC,0x4,true,EquationKind::angle,0},
    Store{0x11D570,0x8021E4EC,0xC,true,EquationKind::angle,0},
    Store{0x11D570,0x8021E5BC,0x0,true,EquationKind::linear,2},
    Store{0x11D570,0x8021E9A8,0xC,false,EquationKind::angle,0},
    Store{0x11D570,0x8021EAD8,0x18,false,EquationKind::linear,0},
    Store{0x11D570,0x8021EC18,0x4,true,EquationKind::angle,0},
    Store{0x11D570,0x8021ED28,0x0,true,EquationKind::linear,0},
    Store{0x11D570,0x8021EDF0,0x0,true,EquationKind::linear,0},
    Store{0x11D570,0x8021DA1C,0x58,true,EquationKind::linear,0},
    Store{0x111E70,0x80222C44,0x0,true,EquationKind::linear,0},
    Store{0x111E70,0x8022335C,0x0,true,EquationKind::linear,0},
    Store{0x111E70,0x802237BC,0x4,true,EquationKind::angle,0},
    Store{0x111E70,0x80223848,0x0,true,EquationKind::linear,0},
    Store{0x111E70,0x802239F0,0x0,true,EquationKind::linear,0},
    Store{0x111E70,0x80223DC0,0x4,true,EquationKind::angle,0},
    Store{0x111E70,0x802247AC,0xC,false,EquationKind::angle,0},
    Store{0x111E70,0x80224838,0x8,true,EquationKind::linear,0},
    Store{0x111E70,0x802254E0,0x10,false,EquationKind::angle,0},
    Store{0x111E70,0x80225F00,0x0,true,EquationKind::linear,0},
    Store{0x111E70,0x80226010,0x4,true,EquationKind::angle,0},
    Store{0x111E70,0x80226154,0x10,false,EquationKind::angle,0},
    Store{0x111E70,0x80226188,0x0,true,EquationKind::linear,0},
    Store{0x111E70,0x802265E8,0x60,true,EquationKind::exponential,0,0x80228BE0},
    Store{0x111E70,0x80226644,0x4,true,EquationKind::angle,0},
    Store{0x111E70,0x80226904,0x8,true,EquationKind::linear,0},
    Store{0x111E70,0x80226C2C,0x4,true,EquationKind::angle,0},
    Store{0x111E70,0x80226C60,0xC,false,EquationKind::angle,0},
    Store{0x111E70,0x80226E08,0x0,true,EquationKind::exponential,0,0x80228C18},
    Store{0x111E70,0x80226EA4,0x10,false,EquationKind::angle,0},
    Store{0x111E70,0x80226F74,0x10,false,EquationKind::angle,0},
    Store{0x111E70,0x80227B8C,0x58,true,EquationKind::linear,0},
    Store{0x111E70,0x80227C88,0xC,false,EquationKind::angle,0},
    Store{0x111E70,0x8021DB50,0x4,true,EquationKind::angle,0},
    Store{0x111E70,0x8021DDD0,0x8,true,EquationKind::linear,0},
    Store{0x111E70,0x8021E3B0,0x58,true,EquationKind::linear,0},
    Store{0x111E70,0x8021EBFC,0x4,true,EquationKind::angle,0},
    Store{0x111E70,0x8021EE5C,0x8,true,EquationKind::linear,0},
    Store{0x111E70,0x8021F58C,0x58,true,EquationKind::linear,0},
    Store{0x111E70,0x8021FC08,0xC,true,EquationKind::angle,0},
    Store{0x111E70,0x8021FEB0,0x58,true,EquationKind::linear,0},
    Store{0x111E70,0x80220290,0x58,true,EquationKind::linear,0},
    Store{0x111E70,0x802202B4,0x18,false,EquationKind::linear,0},
    Store{0x111E70,0x802203F4,0xC,false,EquationKind::angle,0},
    Store{0x111E70,0x80220D34,0x4,true,EquationKind::angle,0},
    Store{0x111E70,0x80221618,0x10,false,EquationKind::angle,0},
    Store{0x111E70,0x80221628,0xC,false,EquationKind::angle,0},
    Store{0x111E70,0x80221D88,0x58,true,EquationKind::linear,0},
    Store{0x10DA30,0x8021F2BC,0x0,true,EquationKind::linear,0},
    Store{0x10DA30,0x8021F5C4,0x0,true,EquationKind::linear,0},
    Store{0x10DA30,0x8021F688,0x0,true,EquationKind::linear,0},
    Store{0x10DA30,0x8021F7FC,0x4,true,EquationKind::angle,0},
    Store{0x10DA30,0x8021FE18,0x0,true,EquationKind::linear,0},
    Store{0x10DA30,0x8021FF30,0x4,true,EquationKind::angle,0},
    Store{0x10DA30,0x8021FFA8,0x10,false,EquationKind::angle,0},
    Store{0x10DA30,0x8022011C,0x4,true,EquationKind::angle,0},
    Store{0x10DA30,0x8022051C,0xA0,false,EquationKind::linear,0},
    Store{0x10DA30,0x80220714,0xA0,false,EquationKind::linear,0},
    Store{0x10DA30,0x802207A4,0x10,false,EquationKind::angle,0},
    Store{0x10DA30,0x8022091C,0x4,true,EquationKind::angle,0},
    Store{0x10DA30,0x80220B74,0x4,true,EquationKind::angle,0},
    Store{0x10DA30,0x80220CC8,0x4,true,EquationKind::angle,0},
    Store{0x10DA30,0x80220DE8,0xC,false,EquationKind::angle,0},
    Store{0x10DA30,0x80221338,0xC,false,EquationKind::angle,0},
    Store{0x10DA30,0x802214AC,0x4,true,EquationKind::angle,0},
    Store{0x10DA30,0x8021FB74,0x4,true,EquationKind::angle,0},
    Store{0x10DA30,0x8021D948,0x0,true,EquationKind::linear,0},
    Store{0x10DA30,0x8021D98C,0x4,true,EquationKind::angle,0},
    Store{0x10DA30,0x8021DA2C,0x58,true,EquationKind::exponential,0,0x80221990},
    Store{0x10DA30,0x8021DA54,0x14,false,EquationKind::angle,0},
    Store{0x10DA30,0x8021DEA4,0xC,false,EquationKind::angle,0},
    Store{0x10DA30,0x8021E544,0x58,true,EquationKind::linear,0},
    Store{0x10DA30,0x8021EAF0,0x18,false,EquationKind::linear,0},
    Store{0x10DA30,0x8021EBB4,0x5C,true,EquationKind::linear,0},
    Store{0x10DA30,0x8021EBE8,0x10,false,EquationKind::angle,0},
    Store{0x10DA30,0x8021EBFC,0xC,false,EquationKind::angle,0},
    Store{0x108730,0x80220634,0xC,true,EquationKind::angle,0},
    Store{0x108730,0x80220DC4,0xC,true,EquationKind::angle,0},
    Store{0x108730,0x80221424,0xC,true,EquationKind::angle,0},
    Store{0x108730,0x80221B10,0xC,true,EquationKind::angle,0},
    Store{0x108730,0x802223E8,0xC,false,EquationKind::angle,0},
    Store{0x108730,0x8021D828,0x18,false,EquationKind::linear,0},
    Store{0x108730,0x8021D82C,0x20,false,EquationKind::linear,0},
    Store{0x108730,0x8021D830,0x1C,false,EquationKind::linear,0},
    Store{0x108730,0x8021DED0,0x4,true,EquationKind::angle,0},
    Store{0x108730,0x8021DF2C,0x4,true,EquationKind::angle,0},
    Store{0x108730,0x8021DF38,0xC,true,EquationKind::angle,0},
    Store{0x108730,0x8021DF6C,0x4,true,EquationKind::angle,0},
    Store{0x108730,0x8021DF7C,0xC,true,EquationKind::angle,0},
    Store{0x108730,0x8021E260,0x18,false,EquationKind::linear,0},
    Store{0x108730,0x8021E264,0x20,false,EquationKind::linear,0},
    Store{0x108730,0x8021E26C,0x1C,false,EquationKind::linear,0},
    Store{0x108730,0x8021E418,0x18,false,EquationKind::linear,0},
    Store{0x108730,0x8021E41C,0x20,false,EquationKind::linear,0},
    Store{0x108730,0x8021E420,0x1C,false,EquationKind::linear,0},
    Store{0x108730,0x8021E4E0,0x0,true,EquationKind::linear,0},
    Store{0x108730,0x8021E554,0x4,true,EquationKind::angle,0},
    Store{0x108730,0x8021E5C8,0x0,true,EquationKind::linear,0},
    Store{0x108730,0x8021F130,0x18,false,EquationKind::linear,0},
    Store{0x108730,0x8021F134,0x1C,false,EquationKind::linear,0},
    Store{0x108730,0x8021F138,0x20,false,EquationKind::linear,0},
    Store{0x108730,0x8021F250,0x0,true,EquationKind::linear,0},
    Store{0x108730,0x8021F2B4,0x4,true,EquationKind::angle,0},
    Store{0x108730,0x8021F2EC,0xC,false,EquationKind::angle,0},
    Store{0x108730,0x8021F5B0,0x58,true,EquationKind::linear,0},
    Store{0x108730,0x8021F5CC,0x18,false,EquationKind::linear,0},
    Store{0x108730,0x8021F5D0,0x20,false,EquationKind::linear,0},
    Store{0x108730,0x8021F5D8,0x1C,false,EquationKind::linear,0},
    Store{0x108730,0x8021F6FC,0x18,false,EquationKind::linear,0},
    Store{0x108730,0x8021F700,0x1C,false,EquationKind::linear,0},
    Store{0x108730,0x8021F708,0x20,false,EquationKind::linear,0},
    Store{0x104AE0,0x8021F0F4,0x0,true,EquationKind::linear,0},
    Store{0x104AE0,0x8021F244,0x0,true,EquationKind::linear,0},
    Store{0x104AE0,0x8021F8F4,0x10,false,EquationKind::angle,0},
    Store{0x104AE0,0x8021F99C,0x4,true,EquationKind::angle,0},
    Store{0x104AE0,0x802200F0,0x10,false,EquationKind::angle,0},
    Store{0x104AE0,0x80220870,0x10,false,EquationKind::angle,0},
    Store{0x104AE0,0x8021D8D0,0x4,true,EquationKind::angle,0},
    Store{0x104AE0,0x8021D900,0xC,true,EquationKind::angle,0},
    Store{0x104AE0,0x8021D944,0x14,false,EquationKind::angle,0},
    Store{0x104AE0,0x8021DD18,0x18,false,EquationKind::linear,0},
    Store{0x104AE0,0x8021DD1C,0x20,false,EquationKind::linear,0},
    Store{0x104AE0,0x8021DD20,0x1C,false,EquationKind::linear,0},
    Store{0x104AE0,0x8021E384,0x58,true,EquationKind::linear,0},
    Store{0x104AE0,0x8021E464,0x58,true,EquationKind::linear,0},
    Store{0x104AE0,0x8021E61C,0x18,false,EquationKind::linear,0},
    Store{0x104AE0,0x8021E620,0x20,false,EquationKind::linear,0},
    Store{0x104AE0,0x8021E624,0x1C,false,EquationKind::linear,0},
    Store{0xF8A40,0x80228488,0x58,true,EquationKind::linear,0},
    Store{0xF8A40,0x8022309C,0x0,true,EquationKind::linear,0},
    Store{0xF8A40,0x802237B4,0x0,true,EquationKind::linear,0},
    Store{0xF8A40,0x80223CA0,0x0,true,EquationKind::linear,0},
    Store{0xF8A40,0x80223E48,0x0,true,EquationKind::linear,0},
    Store{0xF8A40,0x80224734,0x58,true,EquationKind::linear,0},
    Store{0xF8A40,0x80224790,0x5C,true,EquationKind::angle,0},
    Store{0xF8A40,0x80224E84,0x64,true,EquationKind::angle,0},
    Store{0xF8A40,0x80224F1C,0x64,true,EquationKind::angle,0},
    Store{0xF8A40,0x80224D78,0x10,false,EquationKind::angle,0},
    Store{0xF8A40,0x802252EC,0x8,true,EquationKind::linear,0},
    Store{0xF8A40,0x80226854,0x0,true,EquationKind::linear,0},
    Store{0xF8A40,0x80226ADC,0x0,true,EquationKind::linear,0},
    Store{0xF8A40,0x80227558,0xC,false,EquationKind::angle,0},
    Store{0xF8A40,0x8022779C,0x10,false,EquationKind::angle,0},
    Store{0xF8A40,0x8022786C,0x10,false,EquationKind::angle,0},
    Store{0xF8A40,0x80228584,0xC,false,EquationKind::angle,0},
    Store{0xF8A40,0x8021DB8C,0x58,true,EquationKind::linear,0},
    Store{0xF8A40,0x8021DBE8,0x5C,true,EquationKind::angle,0},
    Store{0xF8A40,0x8021DEAC,0x8,true,EquationKind::linear,0},
    Store{0xF8A40,0x8021E48C,0x58,true,EquationKind::linear,0},
    Store{0xF8A40,0x8021ED04,0x58,true,EquationKind::linear,0},
    Store{0xF8A40,0x8021ED60,0x5C,true,EquationKind::angle,0},
    Store{0xF8A40,0x8021F0D4,0x10,false,EquationKind::angle,0},
    Store{0xF8A40,0x8021F108,0x0,true,EquationKind::linear,0},
    Store{0xF8A40,0x8021F354,0x8,true,EquationKind::linear,0},
    Store{0xF8A40,0x8021FA84,0x58,true,EquationKind::linear,0},
    Store{0xF8A40,0x80220100,0xC,true,EquationKind::angle,0},
    Store{0xF8A40,0x802203A8,0x58,true,EquationKind::linear,0},
    Store{0xF8A40,0x80220498,0xC,false,EquationKind::angle,0},
    Store{0xF8A40,0x80220788,0x58,true,EquationKind::linear,0},
    Store{0xF8A40,0x802207AC,0x18,false,EquationKind::linear,0},
    Store{0xF8A40,0x802207B0,0x20,false,EquationKind::linear,0},
    Store{0xF8A40,0x802207B8,0x1C,false,EquationKind::linear,0},
    Store{0xF8A40,0x802208EC,0xC,false,EquationKind::angle,0},
    Store{0xF8A40,0x80221994,0x10,false,EquationKind::angle,0},
    Store{0xF8A40,0x802219A8,0xC,false,EquationKind::angle,0},
    Store{0xF8A40,0x80222370,0x58,true,EquationKind::linear,0},
    Store{0xF8A40,0x802223DC,0x58,true,EquationKind::linear,0},
    Store{0xF5200,0x8021DDE0,0x4,true,EquationKind::angle,0},
    Store{0xF5200,0x8021E168,0x4,true,EquationKind::angle,0},
    Store{0xF5200,0x8021E578,0x4,true,EquationKind::angle,0},
    Store{0xF5200,0x8021E79C,0x4,true,EquationKind::angle,0},
    Store{0xF5200,0x8021F5F4,0x58,true,EquationKind::linear,0},
    Store{0xF5200,0x8021F850,0x58,true,EquationKind::linear,0},
    Store{0xF5200,0x8021FD50,0x58,true,EquationKind::linear,0},
    Store{0xF5200,0x802202D0,0x78,true,EquationKind::linear,0},
    Store{0xF5200,0x802203E8,0x10,false,EquationKind::angle,0},
    Store{0xF5200,0x80220600,0x10,false,EquationKind::angle,0},
    Store{0x1561E0,0x8023DF18,0xC,true,EquationKind::angle,0},
    Store{0x1561E0,0x8023E9FC,0x4,true,EquationKind::angle,2},
    Store{0xD5D10,0x8021DBC4,0x4,true,EquationKind::angle,0},
    Store{0xD5D10,0x8021DBF4,0x0,true,EquationKind::linear,0},
    Store{0xD5D10,0x8021DCA0,0x10,false,EquationKind::angle,0},
    Store{0xD5D10,0x8021DF44,0x0,true,EquationKind::linear,0},
    Store{0xD5D10,0x8021E7B0,0x10,false,EquationKind::angle,0},
    Store{0xD5D10,0x8021EA4C,0x0,true,EquationKind::linear,0},
    Store{0xD5D10,0x8021EA80,0x0,true,EquationKind::linear,0},
    Store{0xD5D10,0x8021EB08,0x0,true,EquationKind::linear,0},
    Store{0xD5D10,0x8021EC80,0x10,false,EquationKind::angle,0},
    Store{0xD5D10,0x8021EE5C,0x4,true,EquationKind::angle,0},
    Store{0xD5D10,0x8021EE8C,0x0,true,EquationKind::linear,0},
    Store{0xD5D10,0x8021EFB8,0x10,false,EquationKind::angle,0},
    Store{0xD5D10,0x8021F210,0x0,true,EquationKind::linear,0},
    Store{0xD5D10,0x8021F548,0x10,false,EquationKind::angle,0},
    Store{0xD5D10,0x8021F948,0x4,true,EquationKind::angle,0},
    Store{0xD5D10,0x8021F978,0x0,true,EquationKind::linear,0},
    Store{0xD5D10,0x8021FAA4,0x10,false,EquationKind::angle,0},
    Store{0xD5D10,0x8021FD18,0x0,true,EquationKind::linear,0},
    Store{0xD5D10,0x8021FD7C,0x0,true,EquationKind::linear,0},
    Store{0xD5D10,0x8021FEFC,0x4,true,EquationKind::angle,0},
    Store{0xD5D10,0x8021FF38,0x0,true,EquationKind::linear,0},
    Store{0xD5D10,0x8021FFB8,0x0,true,EquationKind::linear,0},
    Store{0xD5D10,0x8022003C,0x10,false,EquationKind::angle,0},
    Store{0xD5D10,0x802201F0,0x10,false,EquationKind::angle,0},
    Store{0x129A30,0x8022E18C,0x4,true,EquationKind::angle,0},
    Store{0x129A30,0x8022E1EC,0x0,true,EquationKind::linear,0},
    Store{0x129A30,0x8022E21C,0x0,true,EquationKind::linear,0},
    Store{0x129A30,0x8022E3F4,0x4,true,EquationKind::angle,0},
    Store{0x129A30,0x8022E480,0x0,true,EquationKind::linear,0},
    Store{0x129A30,0x8022E4B0,0x0,true,EquationKind::linear,0},
    Store{0x129A30,0x8022E508,0x4,true,EquationKind::angle,0},
    Store{0x129A30,0x8022E678,0x4,true,EquationKind::angle,0},
    Store{0x129A30,0x8022E6D8,0x0,true,EquationKind::linear,0},
    Store{0x129A30,0x8022E884,0x0,true,EquationKind::linear,0},
    Store{0x129A30,0x8022F1C8,0x4,true,EquationKind::angle,0},
    Store{0x129A30,0x8022F228,0x0,true,EquationKind::linear,0},
    Store{0x129A30,0x8022F258,0x0,true,EquationKind::linear,0},
    Store{0x129A30,0x8022F424,0xC,true,EquationKind::angle,0},
    Store{0x129A30,0x8022F460,0x4,true,EquationKind::angle,0},
    Store{0x129A30,0x8022F504,0x0,true,EquationKind::linear,0},
    Store{0x129A30,0x8022F534,0x0,true,EquationKind::linear,0},
    Store{0x129A30,0x8022F58C,0x4,true,EquationKind::angle,0},
    Store{0x129A30,0x8022F7B0,0x4,true,EquationKind::angle,0},
    Store{0x129A30,0x8022F810,0x0,true,EquationKind::linear,0},
    Store{0x129A30,0x8022F9BC,0x0,true,EquationKind::linear,0},
    Store{0x129A30,0x8022FF50,0x4,true,EquationKind::angle,0},
    Store{0x129A30,0x8022FF68,0xC,true,EquationKind::angle,0},
    Store{0xE5480,0x802211B0,0x0,true,EquationKind::linear,0},
    Store{0xE5480,0x8022120C,0x0,true,EquationKind::linear,0},
    Store{0xE5480,0x80221244,0x8,true,EquationKind::linear,0},
    Store{0xE5480,0x80221278,0xC,true,EquationKind::angle,0},
    Store{0xE5480,0x802214CC,0x0,true,EquationKind::linear,0},
    Store{0xE5480,0x802215A8,0xC,true,EquationKind::angle,0},
    Store{0xE5480,0x802215D4,0xC,true,EquationKind::angle,0},
    Store{0xE5480,0x802218B4,0x4,true,EquationKind::angle,0},
    Store{0xE5480,0x80221AC0,0x10,false,EquationKind::angle,0},
    Store{0xE5480,0x80221B04,0xC,false,EquationKind::angle,0},
    Store{0xE5480,0x80221DC8,0x0,true,EquationKind::linear,0},
    Store{0xE5480,0x80221E24,0x0,true,EquationKind::linear,0},
    Store{0xE5480,0x80221E58,0xC,true,EquationKind::angle,0},
    Store{0xE5480,0x802220AC,0x0,true,EquationKind::linear,0},
    Store{0xE5480,0x8022210C,0xC,true,EquationKind::angle,0},
    Store{0xE5480,0x802223F8,0x4,true,EquationKind::angle,0},
    Store{0xE5480,0x802225D0,0x0,true,EquationKind::linear,0},
    Store{0xE5480,0x80222A0C,0x10,false,EquationKind::angle,0},
    Store{0xE5480,0x80222A50,0xC,false,EquationKind::angle,0},
    Store{0xE5480,0x8021EB88,0x0,true,EquationKind::linear,0},
    Store{0xE5480,0x8021ED50,0x4,true,EquationKind::angle,0},
    Store{0xE5480,0x8021EE18,0x4,true,EquationKind::angle,0},
    Store{0xE5480,0x8021EEF4,0x0,true,EquationKind::linear,0},
    Store{0xE5480,0x8021EF24,0x0,true,EquationKind::linear,2},
    Store{0xE5480,0x8021EF7C,0x4,true,EquationKind::angle,0},
    Store{0xE5480,0x8021EFDC,0x0,true,EquationKind::linear,0},
    Store{0xE5480,0x8021F1B4,0x4,true,EquationKind::angle,0},
    Store{0xE5480,0x8021F334,0x4,true,EquationKind::angle,0},
    Store{0xE5480,0x8021F3F0,0x0,true,EquationKind::linear,0},
    Store{0xE5480,0x8021F570,0x10,false,EquationKind::angle,0},
    Store{0xE5480,0x8021F58C,0x18,false,EquationKind::linear,0},
    Store{0xE5480,0x8021F5B4,0x30,true,EquationKind::linear,0},
    Store{0xE5480,0x8021F888,0x18,false,EquationKind::linear,0},
    Store{0xE5480,0x8021F9BC,0x18,false,EquationKind::linear,0},
    Store{0xE5480,0x8021FA18,0x18,false,EquationKind::linear,0},
    Store{0xE5480,0x8021FD54,0x0,true,EquationKind::linear,0},
    Store{0xE5480,0x80220014,0x0,true,EquationKind::linear,0},
    Store{0xE5480,0x80220124,0x0,true,EquationKind::linear,0},
    Store{0xE5480,0x802201E8,0xC,true,EquationKind::angle,0},
    Store{0xE5480,0x802202D0,0xC,true,EquationKind::angle,12},
    Store{0xE5480,0x802204D8,0x4,true,EquationKind::angle,0},
    Store{0xE5480,0x8022053C,0x4,true,EquationKind::angle,0},
    Store{0xE5480,0x80220588,0x14,false,EquationKind::angle,0},
    Store{0xE5480,0x802207C8,0x14,false,EquationKind::angle,0},
    Store{0xE5480,0x802207F8,0x0,true,EquationKind::linear,0},
    Store{0xE5480,0x802208BC,0xC,false,EquationKind::angle,0},
    Store{0xE5480,0x8022099C,0x18,false,EquationKind::linear,0},
    Store{0xE5480,0x80220C5C,0x0,true,EquationKind::linear,0},
    Store{0xE5480,0x80220C80,0x8,true,EquationKind::linear,2},
    Store{0xE5480,0x80220DC4,0x18,false,EquationKind::linear,0},
    Store{0xE5480,0x80220E20,0x18,false,EquationKind::linear,0},
    Store{0xE5480,0x8021D960,0x4,true,EquationKind::angle,0},
    Store{0xE5480,0x8021D974,0xC,true,EquationKind::angle,12},
    Store{0xE5480,0x8021DD6C,0x4,true,EquationKind::angle,0},
    Store{0xE5480,0x8021DD80,0xC,true,EquationKind::angle,12},
    Store{0xE5480,0x8021E070,0x4,true,EquationKind::angle,0},
    Store{0xE5480,0x80223124,0x4,true,EquationKind::angle,0},
    Store{0xE5480,0x80223480,0xC,false,EquationKind::angle,0},
    Store{0xE5480,0x802237BC,0xC,true,EquationKind::angle,0},
    Store{0xE5480,0x802237F4,0x18,false,EquationKind::linear,0},
    Store{0xE5480,0x80223848,0x18,false,EquationKind::linear,0},
    Store{0x125180,0x8022DDA4,0x10,false,EquationKind::angle,0},
    Store{0x125180,0x8022E178,0x10,false,EquationKind::angle,0},
    Store{0x125180,0x8022E6A8,0x18,false,EquationKind::linear,0},
    Store{0x125180,0x8022E6C8,0xC,true,EquationKind::angle,0},
    Store{0x125180,0x8022E954,0x58,true,EquationKind::linear,0},
    Store{0x125180,0x8022EB14,0x4,true,EquationKind::angle,0},
    Store{0x125180,0x8022EB28,0xC,true,EquationKind::angle,12},
    Store{0x12E230,0x8022EF48,0x18,false,EquationKind::linear,0},
    Store{0x12E230,0x8022EF84,0x0,true,EquationKind::linear,0},
    Store{0x129A30,0x8023031C,0x4,true,EquationKind::angle,0},
    Store{0x129A30,0x80230754,0x4,true,EquationKind::angle,0},
    Store{0x129A30,0x8023076C,0xC,true,EquationKind::angle,0},
    Store{0x129A30,0x80231830,0x58,true,EquationKind::angle,0},
    Store{0xDFC40,0x80221B78,0x58,true,EquationKind::linear,0},
    Store{0xDFC40,0x80221B8C,0x4,true,EquationKind::angle,0},
    Store{0xDFC40,0x80221E3C,0x4,true,EquationKind::angle,0},
    Store{0xDFC40,0x80222020,0x20,false,EquationKind::linear,0},
    Store{0xDFC40,0x80222440,0x58,true,EquationKind::linear,0},
    Store{0xDFC40,0x80222454,0x4,true,EquationKind::angle,0},
    Store{0xDFC40,0x80222734,0x4,true,EquationKind::angle,0},
    Store{0xDFC40,0x8022293C,0x20,false,EquationKind::linear,0},
    Store{0xDFC40,0x80222BE8,0x18,false,EquationKind::linear,6},
    Store{0xDFC40,0x80222BEC,0x1C,false,EquationKind::linear,4},
    Store{0xDFC40,0x80222BF4,0x20,false,EquationKind::linear,0},
    Store{0xDFC40,0x80222C3C,0x58,true,EquationKind::linear,0},
    Store{0xDFC40,0x80222C68,0x10,false,EquationKind::angle,0},
    Store{0x11F1C0,0x8022D910,0x4,true,EquationKind::angle,0},
    Store{0x11F1C0,0x8022D928,0xC,true,EquationKind::angle,0},
    Store{0x11F1C0,0x8022DD4C,0x4,true,EquationKind::angle,0},
    Store{0x11F1C0,0x8022DEC8,0x4,true,EquationKind::angle,0},
    Store{0x11F1C0,0x8022E02C,0x0,true,EquationKind::linear,0},
    Store{0x11F1C0,0x8022E2DC,0x0,true,EquationKind::linear,0},
    Store{0x11F1C0,0x8022E34C,0x4,true,EquationKind::angle,0},
    Store{0x11F1C0,0x8022E430,0x4,true,EquationKind::angle,0},
    Store{0x11F1C0,0x8022E460,0x0,true,EquationKind::linear,0},
    Store{0x11F1C0,0x8022E4B8,0x0,true,EquationKind::linear,0},
    Store{0x11F1C0,0x8022E808,0xC,false,EquationKind::angle,0},
    Store{0x11F1C0,0x8022E834,0xA0,false,EquationKind::linear,0},
    Store{0x11F1C0,0x8022EBC8,0x18,false,EquationKind::linear,0},
    Store{0xDFC40,0x8022042C,0x58,true,EquationKind::linear,0},
    Store{0xDFC40,0x80220440,0x4,true,EquationKind::angle,0},
    Store{0xDFC40,0x80220948,0x4,true,EquationKind::angle,0},
    Store{0xDFC40,0x80220C30,0x4,true,EquationKind::angle,0},
    Store{0xDFC40,0x80220D9C,0x4,true,EquationKind::angle,0},
    Store{0xDFC40,0x80220DA8,0x20,false,EquationKind::linear,0},
    Store{0xDFC40,0x8022102C,0x4,true,EquationKind::angle,0},
    Store{0xDFC40,0x80221258,0x4,true,EquationKind::angle,0},
    Store{0xDFC40,0x80221398,0x4,true,EquationKind::angle,0},
    Store{0xDFC40,0x802213B0,0x1C,false,EquationKind::linear,4},
    Store{0xD9910,0x80221CEC,0x4,true,EquationKind::angle,0},
    Store{0xD9910,0x80221F80,0x4,true,EquationKind::angle,0},
    Store{0xD9910,0x80222040,0x0,true,EquationKind::linear,0},
    Store{0xD9910,0x80222440,0x18,false,EquationKind::linear,0},
    Store{0xD9910,0x80222724,0x58,true,EquationKind::linear,0},
    Store{0xD9910,0x80222750,0x4,true,EquationKind::angle,0},
    Store{0xD9910,0x80222B50,0x4,true,EquationKind::angle,0},
    Store{0xD9910,0x80222D18,0x4,true,EquationKind::angle,0},
    Store{0xD9910,0x8022367C,0xA0,false,EquationKind::linear,0},
    Store{0xD9910,0x8022368C,0x10,false,EquationKind::angle,0},
    // Kinopper's roam timer adds ten degrees every eight Source periods.
    // Continue that heading and its literal model-yaw copy together; the
    // shared projection then consumes the same gradual heading. Wall/probe
    // reflection setters are separate instantaneous commands.
    Store{0x12FD90,0x8022D880,0x4,true,EquationKind::angle,0},
    Store{0x142670,0x8022D92C,0x10,false,EquationKind::angle,0},
    Store{0x142670,0x8022DA38,0x4,true,EquationKind::angle,0},
    Store{0x142670,0x8022DE40,0xC,false,EquationKind::angle,0},
    Store{0x142670,0x8022E28C,0x58,true,EquationKind::angle,0},
    Store{0x142670,0x8022E580,0x4,true,EquationKind::angle,0},
    Store{0x142670,0x8022EA54,0xC,false,EquationKind::angle,0},
    Store{0x121280,0x80230C1C,0x18,false,EquationKind::linear,0},
    Store{0x121280,0x80230CD0,0x18,false,EquationKind::linear,0},
    Store{0x121280,0x80230F10,0xA0,false,EquationKind::linear,0},
    Store{0x121280,0x80230F50,0xA0,false,EquationKind::linear,0},
    Store{0x126BA0,0x8022D938,0x0,true,EquationKind::linear,0},
    Store{0x126BA0,0x8022DA00,0x4,true,EquationKind::angle,0},
    Store{0x126BA0,0x8022DA14,0xC,true,EquationKind::angle,12},
    Store{0x126BA0,0x8022DCB8,0x10,false,EquationKind::angle,0},
    Store{0x126BA0,0x8022DD0C,0x0,true,EquationKind::linear,0},
    Store{0x126BA0,0x8022DD44,0x4,true,EquationKind::angle,0},
    Store{0x126BA0,0x8022DD5C,0x0,true,EquationKind::linear,0},
    Store{0x126BA0,0x8022E1B4,0x0,true,EquationKind::linear,0},
    Store{0x126BA0,0x8022E264,0x4,true,EquationKind::angle,0},
    Store{0x126BA0,0x8022E27C,0x0,true,EquationKind::linear,0},
    Store{0x126BA0,0x8022E2B4,0x0,true,EquationKind::linear,0},
    Store{0x1561E0,0x8023F27C,0x4,true,EquationKind::angle,0},
    Store{0x1561E0,0x8023F2AC,0x4,true,EquationKind::angle,0},
    Store{0x1561E0,0x8023F530,0x0,true,EquationKind::linear,0},
    Store{0x1561E0,0x8023F584,0x4,true,EquationKind::angle,0},
    Store{0x12FD90,0x8022F0B8,0x10,false,EquationKind::angle,0}, // Bironn death +30.
    Store{0x125180,0x8022D9F0,0xC,false,EquationKind::angle,0},
    Store{0x129A30,0x8022D8E8,0x10,false,EquationKind::angle,0},
    Store{0x12E230,0x8022DB80,0xA0,false,EquationKind::linear,6},
    Store{0x12E230,0x8022DBBC,0x18,false,EquationKind::linear,0},
    Store{0x12E230,0x8022E218,0x4,true,EquationKind::angle,0},
    Store{0x12E230,0x8022E264,0x4,true,EquationKind::angle,0},
    Store{0x11F1C0,0x8022EFCC,0x4,true,EquationKind::angle,0},
    Store{0x11F1C0,0x8022F238,0x4,true,EquationKind::angle,0},
    Store{0x11F1C0,0x8022F2CC,0x0,true,EquationKind::linear,0},
    Store{0xD5D10,0x80220AE4,0x8,true,EquationKind::linear,0},
    Store{0xD5D10,0x80220B5C,0x14,false,EquationKind::angle,0},
    Store{0xD5D10,0x80220B9C,0x14,false,EquationKind::angle,0},
    Store{0xD5D10,0x80220BD4,0x8,true,EquationKind::linear,0},
    Store{0xD5D10,0x80220C70,0x0,true,EquationKind::linear,0},
    Store{0xD5D10,0x80220CDC,0x0,true,EquationKind::linear,0},
    Store{0xD5D10,0x80220E84,0x0,true,EquationKind::linear,0},
    Store{0xD5D10,0x80220ED0,0x4,true,EquationKind::angle,0},
    Store{0xD5D10,0x80221070,0x14,false,EquationKind::angle,0},
    Store{0xEDDB0,0x8021F7B8,0x58,true,EquationKind::linear,0},
    Store{0x142670,0x8022F018,0x5C,true,EquationKind::linear,0},
    Store{0x142670,0x8022F0CC,0x5C,true,EquationKind::linear,0},
    Store{0x142670,0x8022F164,0x0,true,EquationKind::linear,0},
    Store{0x142670,0x8022F10C,0x4,true,EquationKind::angle,0},
    Store{0x142670,0x8022FAC8,0x4,true,EquationKind::angle,0},
    Store{0x142670,0x802300DC,0x4,true,EquationKind::angle,0},
    Store{0x142670,0x802301B4,0x4,true,EquationKind::angle,0},
    Store{0xDFC40,0x8021DE28,0x10,false,EquationKind::angle,0},
    Store{0xDFC40,0x8021E0C0,0x10,false,EquationKind::angle,0},
    Store{0xDFC40,0x8021E5C8,0x4,true,EquationKind::angle,0},
    Store{0xDFC40,0x8021E80C,0x18,false,EquationKind::linear,0},
    Store{0xDFC40,0x8021E870,0xC,false,EquationKind::angle,0},
    Store{0xDFC40,0x8021F2D4,0xA0,false,EquationKind::linear,4},
    Store{0xDFC40,0x8021F950,0x10,false,EquationKind::angle,0},
    Store{0xDFC40,0x8021FBE8,0x10,false,EquationKind::angle,0},
    Store{0xDFC40,0x802200D8,0x18,false,EquationKind::linear,0},
    Store{0xDFC40,0x8022013C,0xC,false,EquationKind::angle,0},
    Store{0x12FD90,0x8022FC90,0xC,true,EquationKind::wrapped_approach,0,0x802308A0,300},
    Store{0x12FD90,0x8022F870,0x4,true,EquationKind::angle,0},
    Store{0x12FD90,0x8022F8B8,0xC,true,EquationKind::angle,0},
    Store{0x12FD90,0x8022F8D8,0x8,true,EquationKind::linear,0},
    Store{0x12FD90,0x8022FB28,0x4,true,EquationKind::angle,0},
    Store{0x12FD90,0x8022FBFC,0x4,true,EquationKind::angle,0},
    Store{0x12FD90,0x8022FDC4,0x4,true,EquationKind::angle,0},
    Store{0x12FD90,0x8022FE30,0xC,false,EquationKind::angle,0},
    Store{0x12FD90,0x8022FE9C,0x58,true,EquationKind::linear,0},
    Store{0x12FD90,0x80230208,0x4,true,EquationKind::angle,0},
    Store{0x12FD90,0x8023031C,0x4,true,EquationKind::angle,0},
    Store{0xEBAC0,0x8021DB24,0x18,false,EquationKind::linear,0},
    Store{0xEBAC0,0x8021E730,0x1C,false,EquationKind::linear,0},
    Store{0xEBAC0,0x8021E834,0x58,true,EquationKind::linear,0},
    Store{0xEBAC0,0x8021EBE8,0x4,true,EquationKind::angle,0},
    Store{0xEBAC0,0x8021EC30,0x18,false,EquationKind::linear,0},
    Store{0xEBAC0,0x8021EDF8,0x0,true,EquationKind::linear,0},
    Store{0xEBAC0,0x8021E5E8,0x14,false,EquationKind::angle,0},
    Store{0xEBAC0,0x8021F364,0x10,false,EquationKind::angle,0},
    Store{0xEBAC0,0x8021F6EC,0xC,true,EquationKind::angle,0},
    Store{0x121280,0x8022DBBC,0,true,EquationKind::linear,0},
    Store{0x121280,0x8022DC18,0,true,EquationKind::linear,0},
    Store{0x121280,0x8022E3C0,0,true,EquationKind::linear,0},
    Store{0x121280,0x8022E434,0,true,EquationKind::linear,0},
    Store{0x121280,0x8022E4DC,0,true,EquationKind::linear,0},
    Store{0x121280,0x8022EC70,0,true,EquationKind::linear,0},
    Store{0x121280,0x8022EF68,0,true,EquationKind::linear,0},
    Store{0x121280,0x8022DD38,0xC,false,EquationKind::angle,0},
    Store{0x121280,0x8022E544,0xC,false,EquationKind::angle,0},
    Store{0x121280,0x8022ED58,0xC,false,EquationKind::angle,0},
    Store{0x121280,0x8022F090,0xC,false,EquationKind::angle,0},
    Store{0x121280,0x8022EC58,0x58,true,EquationKind::linear,0},
    Store{0xCEC60,0x80220F10,0x0,true,EquationKind::linear,0},
    Store{0xCEC60,0x8022167C,0x10,false,EquationKind::angle,0},
    Store{0xCEC60,0x80221690,0x18,false,EquationKind::linear,0},
    Store{0xCEC60,0x80221824,0x8,true,EquationKind::linear,0},
    Store{0xCEC60,0x80221838,0x10,false,EquationKind::angle,0},
    Store{0xCEC60,0x802221C8,0x10,false,EquationKind::angle,0},
    Store{0xCEC60,0x802221DC,0x18,false,EquationKind::linear,0},
    Store{0xCEC60,0x80222370,0x8,true,EquationKind::linear,0},
    Store{0xCEC60,0x80222384,0x10,false,EquationKind::angle,0},
    Store{0xCEC60,0x8022268C,0x18,false,EquationKind::linear,0},
    Store{0xCEC60,0x802226E0,0x60,true,EquationKind::linear,0},
    Store{0xD9910,0x8021D89C,0,true,EquationKind::linear,0},
    Store{0xD9910,0x8021D950,0,true,EquationKind::linear,0},
    Store{0xD9910,0x8021DC80,0,true,EquationKind::linear,0},
    Store{0xD9910,0x8021E1B4,0,true,EquationKind::linear,0},
    Store{0xD9910,0x8021E518,0,true,EquationKind::linear,0},
    Store{0xD9910,0x8021E618,0,true,EquationKind::linear,0},
    Store{0xD9910,0x8021ECAC,0,true,EquationKind::linear,0},
    Store{0xD9910,0x8021F290,0,true,EquationKind::linear,0},
    Store{0xD9910,0x8021D914,0x4,true,EquationKind::angle,0},
    Store{0xD9910,0x8021E5DC,0xC,true,EquationKind::angle,0},
    Store{0xD9910,0x8021E264,0x18,false,EquationKind::approach,0,0x802238F0,0},
    Store{0xD9910,0x8021F420,0x18,false,EquationKind::approach,0,0x80223908,0},
    Store{0xD9910,0x8021F680,0x1C,false,EquationKind::approach,0,0x80223920,4},
    Store{0xD9910,0x8021F6EC,0x1C,false,EquationKind::approach,0,0x80223938,1},
    Store{0xD9910,0x80220E90,4,true,EquationKind::angle,0},
    Store{0xD9910,0x80220EA8,0x10,false,EquationKind::angle,0},
    Store{0xD9910,0x80221048,0x10,false,EquationKind::angle,0},
    Store{0xD9910,0x8022131C,0x10,false,EquationKind::angle,0},
    Store{0xD9910,0x80221440,0x10,false,EquationKind::angle,0},
    Store{0xD9910,0x802215B0,0x10,false,EquationKind::angle,0},
    Store{0xD9910,0x802211BC,0,true,EquationKind::linear,0},
    Store{0xD9910,0x8022134C,0,true,EquationKind::linear,0},
    Store{0xD9910,0x80221404,0,true,EquationKind::linear,0},
    Store{0xD9910,0x80221324,0xC,false,EquationKind::approach,4,0x80223970,360},
    Store{0xD9910,0x80221624,0x18,false,EquationKind::approach,0,0x80223980,0},
    Store{0xD9910,0x80221628,0x20,false,EquationKind::approach,0,0x80223980,0},
    Store{0xD9910,0x80221634,0x1C,false,EquationKind::approach,4,0x80223978,3},
    Store{0xEDDB0,0x8021DB0C,8,true,EquationKind::linear,0},
    Store{0xEDDB0,0x8021E59C,8,true,EquationKind::linear,0},
    Store{0xEDDB0,0x8021EAF0,8,true,EquationKind::linear,0},
    Store{0xEDDB0,0x8021DC28,0,true,EquationKind::linear,0},
    Store{0xEDDB0,0x8021DFA0,0,true,EquationKind::linear,0},
    Store{0xEDDB0,0x8021E300,0,true,EquationKind::linear,0},
    Store{0xEDDB0,0x8021EB5C,0,true,EquationKind::linear,0},
    Store{0xEDDB0,0x8021EE9C,0,true,EquationKind::linear,0},
    Store{0xEDDB0,0x8021EF40,0,true,EquationKind::linear,0},
    Store{0xEDDB0,0x8021F0B0,0,true,EquationKind::linear,0},
    Store{0xEDDB0,0x8021F228,0,true,EquationKind::linear,0},
    Store{0xEDDB0,0x8021DFE8,0x10,false,EquationKind::angle,0},
    Store{0xEDDB0,0x8021EDE0,0x10,false,EquationKind::angle,0},
    Store{0xEDDB0,0x8021F0D4,0x10,false,EquationKind::angle,2},
    Store{0xEDDB0,0x8021E030,0x14,false,EquationKind::angle,0},
    Store{0xEDDB0,0x8021E07C,0x14,false,EquationKind::angle,0},
    Store{0xEDDB0,0x8021EE6C,0x14,false,EquationKind::angle,0},
    Store{0xEDDB0,0x8021FBD0,0,true,EquationKind::linear,0},
    Store{0xEDDB0,0x8021FF78,0,true,EquationKind::linear,0},
    Store{0xEDDB0,0x8022010C,0,true,EquationKind::linear,0},
    Store{0xEDDB0,0x802203A4,0,true,EquationKind::linear,0},
    Store{0xEDDB0,0x8021FF48,4,true,EquationKind::angle,0},
    Store{0xEDDB0,0x8021F5FC,0x58,true,EquationKind::linear,0},
    Store{0xEDDB0,0x80220BF4,4,true,EquationKind::angle,0},
    Store{0xEDDB0,0x8022129C,0x10,false,EquationKind::angle,12},
    Store{0xEDDB0,0x80221444,4,true,EquationKind::angle,0},
    Store{0xEDDB0,0x80221674,4,true,EquationKind::angle,0},
    Store{0xEDDB0,0x80221CC4,0x10,false,EquationKind::angle,0},
    Store{0xEDDB0,0x80221CE4,8,true,EquationKind::linear,0},
    Store{0xEDDB0,0x80221D34,0x58,true,EquationKind::linear,0},
    Store{0xEDDB0,0x80222318,4,true,EquationKind::angle,0},
    Store{0xEDDB0,0x80222508,4,true,EquationKind::angle,0},
    Store{0xEDDB0,0x80222C88,0x10,false,EquationKind::angle,0},
    Store{0xEDDB0,0x80222CA8,8,true,EquationKind::linear,0},
    Store{0xEDDB0,0x80222CF8,0x58,true,EquationKind::linear,0},
    Store{0xEDDB0,0x80222ED4,0x10,false,EquationKind::angle,0},
    Store{0xEDDB0,0x80222EF4,8,true,EquationKind::linear,0},
    Store{0xEDDB0,0x80222F48,0x58,true,EquationKind::linear,0},
    Store{0xEDDB0,0x80222FCC,0x58,true,EquationKind::linear,0},
    Store{0xEDDB0,0x80223004,0,true,EquationKind::linear,0},
    Store{0xEDDB0,0x802231DC,4,true,EquationKind::angle,0},
    Store{0xEDDB0,0x80223564,4,true,EquationKind::angle,0},
    Store{0xEDDB0,0x802236F0,4,true,EquationKind::angle,0},
    Store{0xEDDB0,0x802239B8,0x10,false,EquationKind::angle,0},
    Store{0xCEC60,0x80223054,4,true,EquationKind::angle,0},
    Store{0xCEC60,0x802231D4,4,true,EquationKind::angle,0},
    Store{0xCEC60,0x802235B4,4,true,EquationKind::angle,0},
    Store{0xCEC60,0x80223BB8,0x5C,true,EquationKind::linear,0},
    Store{0xCEC60,0x80223BD4,0x58,true,EquationKind::linear,0},
    Store{0xCEC60,0x80223BE8,4,true,EquationKind::angle,0},
    Store{0xCEC60,0x80223D3C,4,true,EquationKind::angle,0},
    Store{0xCEC60,0x80224258,4,true,EquationKind::angle,0}
};
static_assert([] {
    for(const auto& store:additional_stores)
        if(store.kind==EquationKind::exponential&&!store.coefficient)return false;
    return true;
}(), "Exponential stores require their ROM multiplier");


// EA5D4 writes another actor: an absolute pose equation would overwrite the
// player's own movement. Keep only the reviewed additive Source increment.
struct ForeignStep {
    uint32_t actor,physical,model,offset;uint64_t lifetime;
    float before,after,expected;double delta,applied=0,rebase=0;uint64_t start_phase=0,denominator=1;
};
bool foreign_identity(uint8_t* rdram,const ForeignStep& step) {
    return uint32_t(word(rdram,0x8021D49C))==step.actor&&pointer(step.actor,0xCC)&&
        uint32_t(word(rdram,step.actor+0x98))==step.physical&&
        uint32_t(word(rdram,step.actor+0x74))==step.model&&word(rdram,step.actor+0x94)&&
        game_model_animation().lifetime_token(step.model)==step.lifetime;
}

struct Period {
    Identity owner;uint64_t tick,denominator,start_phase,expected_phase,run;
    std::vector<uint64_t> command;
    std::array<float,3> initial_position{},velocity{},expected_position{},expected_velocity{};
    std::array<float,2> initial_yaw{},turn{},proposal_yaw{},expected_yaw{};
    // A reviewed Brobo child collision can undo an already visited actor's
    // Source X/Z step. Hold the corrected endpoint for the rest of this period.
    std::array<std::optional<float>,3> collision_hold{};
    // Source rail correction and its following drift form one movement step.
    // Split both over the same interval instead of showing a reset each tick.
    std::array<double,3> constraint_correction{};
    std::optional<Gravity> gravity{};
    std::vector<Equation> equations{};
    ModelEventCheckpoint model_events{};
    std::map<uint32_t,ModelEventCheckpoint> model_reads{};
    void (*update)(uint8_t*,recomp_context*)=nullptr;
    std::optional<Identity> parent{};
    std::array<float,3> attachment_source_position{},source_endpoint_position{};
    std::array<float,4> source_endpoint_angles{}; // pitch, yaw, roll, physical heading
    // A delayed pose must retain the mathematical parent path from the time
    // of the real Source sample. Live attachment writes cannot move its origin.
    std::shared_ptr<const Period> attachment_motion{};
    std::shared_ptr<const Period> scene_parent_motion{};
    uint64_t scene_parent_start_phase=0;
    std::array<bool,3> attachment_axes{};
    std::array<float,3> attachment_offsets{};
    double attachment_start_age=0;
    bool historical_attachment=false;
    int32_t acceleration=0;
    std::optional<Projection> projection{};
    std::optional<Equation> opacity{};
    std::vector<ForeignStep> foreign_steps{};
    int32_t callback_opacity_flag=0;
};
std::array<std::optional<Period>,dora3::distance::common_capacity> periods;
// Historical pose paths need only motion operands. Retaining every prior
// visual look-at equation would recursively keep unused old Period objects
// alive, even when the actor's physical path has no history dependency.
std::shared_ptr<const Period> immutable_motion(const Period& source) {
    auto motion=std::make_shared<Period>(source);
    motion->historical_attachment=bool(source.attachment_motion);
    if(source.parent)motion->initial_position=source.attachment_source_position;
    std::erase_if(motion->equations,[&](const auto& e){return e.address!=source.owner.actor+0x9C&&e.address!=source.owner.actor+0xA0&&e.address!=source.owner.actor+0xA4;});
    motion->scene_parent_motion.reset();motion->foreign_steps.clear();motion->command.clear();motion->model_reads.clear();motion->opacity.reset();motion->parent.reset();motion->update=nullptr;
    return motion;
}
#ifdef DORA3_CRASH_LOG
// Temporary, bounded diagnostics requested for Sentoutei's activation flight.
// Observe real guest state only; never run a probe or modify its controller.
void log_sentoutei_motion(uint8_t* rdram,const Identity& owner,const char* event) {
    if(owner.rom!=0x129A30||(owner.kind!=366&&owner.kind!=368))return;
    static unsigned lines=0;
    if(lines>=40000)return;
    const auto actor=owner.actor,ph=owner.physical;
    const auto* slice=active_simulation_slice();
    const auto& stored=periods[index(actor)];
    const auto* p=stored&&same(stored->owner,owner)?&*stored:nullptr;
    unsigned mismatch=0;
    if(p)for(unsigned a=0;a<3;++a) {
        if(std::bit_cast<int32_t>(p->expected_position[a])!=word(rdram,actor+0x9C+4*a))mismatch|=1u<<a;
        if(std::bit_cast<int32_t>(p->expected_velocity[a])!=word(rdram,ph+0x10+4*a))mismatch|=8u<<a;
    }
    std::fprintf(stderr,"SHIP event=%s actor=%08X kind=%d life=%llu native=%d tick=%llu phase=%llu/%llu dt=%llu state=%d latch=%u counters=%d,%d,%d xyz=%.9g,%.9g,%.9g velocity=%.9g,%.9g,%.9g accel=%.9g scalar=%.9g,%.9g heading=%.9g pitch=%.9g homeY=%d flags=%u,%u,%u clip=%.9g period=%llu,%llu,%llu mismatch=%u equations=%zu gravity=%d,%.9g,%.9g,%d projection=%d\n",
        event,actor,owner.kind,(unsigned long long)owner.lifetime,int(native_gameplay_preview_object_pass()),
        (unsigned long long)(slice?slice->tick:0),(unsigned long long)(slice?slice->phase:0),
        (unsigned long long)(slice?slice->denominator:0),(unsigned long long)(slice?slice->numerator:0),
        word(rdram,actor+0x94),unsigned(MEM_BU(0x28,int32_t(ph))),int(MEM_H(0x34,int32_t(ph))),int(MEM_H(0x36,int32_t(ph))),int(MEM_H(0x38,int32_t(ph))),
        double(real(rdram,actor+0x9C)),double(real(rdram,actor+0xA0)),double(real(rdram,actor+0xA4)),
        double(real(rdram,ph+0x10)),double(real(rdram,ph+0x14)),double(real(rdram,ph+0x18)),double(real(rdram,ph+0x1C)),
        double(real(rdram,ph)),double(real(rdram,ph+8)),double(real(rdram,ph+4)),double(real(rdram,ph+0xC)),int(MEM_H(0xC0,int32_t(ph))),
        unsigned(MEM_BU(0xC8,int32_t(ph))),unsigned(MEM_BU(0xC9,int32_t(ph))),unsigned(MEM_BU(0xCA,int32_t(ph))),double(real(rdram,owner.model+0x64)),
        (unsigned long long)(p?p->tick:0),(unsigned long long)(p?p->start_phase:0),(unsigned long long)(p?p->expected_phase:0),
        mismatch,p?p->equations.size():size_t(0),int(p&&p->gravity),p&&p->gravity?double(p->gravity->before):0.,p&&p->gravity?double(p->gravity->after):0.,p&&p->gravity?p->gravity->collision:0,int(p&&p->projection));
    if(++lines==40000)std::fprintf(stderr,"SHIP journal limit reached\n");
}
// User-requested test150 observation in both Original and Native callback entries.
void log_boss5_turn(uint8_t* rdram,const Identity& owner,const char* event) {
    if(owner.rom!=0x10DA30||(owner.kind!=446&&owner.kind!=447))return;
    const unsigned event_id=event[0]=='e'?0:event[0]=='c'?1:2;
    if(!dora3::manual_trace::gate(11,index(owner.actor)*3+event_id,16000,600))return;
    const auto actor=owner.actor,ph=owner.physical;
    const auto* slice=active_simulation_slice();const auto& stored=periods[index(actor)];
    const auto* p=stored&&same(stored->owner,owner)?&*stored:nullptr;
    const Equation* yaw=nullptr;const Equation* heading=nullptr;
    if(p)for(const auto& e:p->equations){if(e.address==actor+0x10)yaw=&e;if(e.address==ph+4)heading=&e;}
    dora3::manual_trace::log("BOSS5 event=%s actor=%08X kind=%d lifetime=%llu callback=%08X state=%d phase=%d turncounter=%d native_pass=%d tick=%llu phase_time=%llu/%llu dt=%llu yaw=%.9g heading=%.9g pitch=%.9g period=%d expected_phase=%llu yaw_initial=%.9g yaw_end=%.9g turn=%.9g expected_yaw=%.9g equations=%zu yawEq=%d,%.9g,%.9g headingEq=%d,%.9g,%.9g parent=%08X parent_motion=%d parent_phase=%llu",
        event,actor,owner.kind,(unsigned long long)owner.lifetime,owner.callback,word(rdram,actor+0x94),int(MEM_H(0x34,int32_t(ph))),int(MEM_H(0x38,int32_t(ph))),int(native_gameplay_preview_object_pass()),
        (unsigned long long)(slice?slice->tick:0),(unsigned long long)(slice?slice->phase:0),(unsigned long long)(slice?slice->denominator:0),(unsigned long long)(slice?slice->numerator:0),double(real(rdram,actor+0x10)),double(real(rdram,ph+4)),double(real(rdram,actor+0xC)),int(p!=nullptr),(unsigned long long)(p?p->expected_phase:0),p?double(p->initial_yaw[0]):0.,p?double(p->proposal_yaw[0]):0.,p?p->turn[0]:0.,p?double(p->expected_yaw[0]):0.,p?p->equations.size():size_t(0),yaw?int(yaw->kind):-1,yaw?double(yaw->before):0.,yaw?double(yaw->after):0.,heading?int(heading->kind):-1,heading?double(heading->before):0.,heading?double(heading->after):0.,p&&p->parent?p->parent->actor:0u,int(p&&p->scene_parent_motion),(unsigned long long)(p?p->scene_parent_start_phase:0));
}
// Owner-authorized test145 trace: observe the two-part refight form only.
// No controller calls, pose evaluation, writes or extra simulation steps.
void log_boss6_form(uint8_t* rdram,const Identity& owner,const char* event) {
    if(owner.rom!=0x111E70||owner.kind<454||owner.kind>456)return;
    auto root=std::optional<Identity>(owner);
    if(owner.kind!=454) {
        const int slot=MEM_H(0x7C,int32_t(owner.physical));
        if(slot<30||slot>=dora3::distance::common_end)return;
        root=identity(rdram,pool+unsigned(slot)*stride);
    }
    if(!root||root->rom!=owner.rom||root->kind!=454)return;
    const int root_state=word(rdram,root->actor+0x94);
    if(root_state!=7&&root_state!=8)return;
    static unsigned lines=0;
    if(lines>=600)return;
    const auto actor=owner.actor,ph=owner.physical;
    const auto* slice=active_simulation_slice();
    const auto& stored=periods[index(actor)];
    const auto* p=stored&&same(stored->owner,owner)?&*stored:nullptr;
    unsigned mismatch=0;
    if(p)for(unsigned a=0;a<3;++a) {
        if(std::bit_cast<int32_t>(p->expected_position[a])!=word(rdram,actor+0x9C+4*a))mismatch|=1u<<a;
        if(std::bit_cast<int32_t>(p->expected_velocity[a])!=word(rdram,ph+0x10+4*a))mismatch|=8u<<a;
    }
    const Equation* y=nullptr;
    if(p)for(const auto& e:p->equations)if(e.address==actor+0xA0)y=&e;
    std::fprintf(stderr,"BOSS6FORM event=%s actor=%08X kind=%d life=%llu root=%08X rootstate=%d rootphase=%d native=%d tick=%llu phase=%llu/%llu dt=%llu state=%d latch=%u counters=%d,%d,%d clipcmd=%d xyz=%.9g,%.9g,%.9g velocity=%.9g,%.9g,%.9g scalar=%.9g,%.9g angles=%.9g,%.9g,%.9g physicalangles=%.9g,%.9g model=%08X cliptime=%.9g clipflags=%u run=%llu flags=%u,%u,%u period=%llu,%llu,%llu expectedY=%.9g initialY=%.9g endpointY=%.9g storedVY=%.9g parent=%08X attachedY=%d offsetY=%.9g mismatch=%u yequation=%d,%.9g,%.9g\n",
        event,actor,owner.kind,(unsigned long long)owner.lifetime,root->actor,root_state,int(MEM_H(0x34,int32_t(root->physical))),int(native_gameplay_preview_object_pass()),
        (unsigned long long)(slice?slice->tick:0),(unsigned long long)(slice?slice->phase:0),(unsigned long long)(slice?slice->denominator:0),(unsigned long long)(slice?slice->numerator:0),
        word(rdram,actor+0x94),unsigned(MEM_BU(0x28,int32_t(ph))),int(MEM_H(0x34,int32_t(ph))),int(MEM_H(0x36,int32_t(ph))),int(MEM_H(0x38,int32_t(ph))),int(MEM_H(0x44,int32_t(ph))),
        double(real(rdram,actor+0x9C)),double(real(rdram,actor+0xA0)),double(real(rdram,actor+0xA4)),
        double(real(rdram,ph+0x10)),double(real(rdram,ph+0x14)),double(real(rdram,ph+0x18)),double(real(rdram,ph)),double(real(rdram,ph+8)),
        double(real(rdram,actor+0xC)),double(real(rdram,actor+0x10)),double(real(rdram,actor+0x14)),double(real(rdram,ph+4)),double(real(rdram,ph+0xC)),
        owner.model,double(real(rdram,owner.model+0x64)),unsigned(MEM_BU(0x7C,int32_t(owner.model))),(unsigned long long)game_model_animation().run_token(owner.model),
        unsigned(MEM_BU(0xC8,int32_t(ph))),unsigned(MEM_BU(0xC9,int32_t(ph))),unsigned(MEM_BU(0xCA,int32_t(ph))),
        (unsigned long long)(p?p->tick:0),(unsigned long long)(p?p->start_phase:0),(unsigned long long)(p?p->expected_phase:0),
        p?double(p->expected_position[1]):0.,p?double(p->initial_position[1]):0.,p?double(p->source_endpoint_position[1]):0.,p?double(p->velocity[1]):0.,
        p&&p->parent?p->parent->actor:0u,int(p&&p->attachment_axes[1]),p?double(p->attachment_offsets[1]):0.,mismatch,
        y?int(y->kind):-1,y?double(y->before):0.,y?double(y->after):0.);
    if(++lines==600)std::fprintf(stderr,"BOSS6FORM journal limit reached\n");
}
void log_boss_state(uint8_t* rdram,const Identity& owner) {
    if(!((owner.rom==0xF8A40&&owner.kind>=410&&owner.kind<=415)||
         (owner.rom==0x111E70&&owner.kind>=454&&owner.kind<=459)))return;
    struct Entry {uint64_t lifetime=0;std::array<int,12> key{};std::chrono::steady_clock::time_point time{};};
    static std::array<Entry,dora3::distance::common_capacity> entries;
    auto& old=entries[index(owner.actor)];
    const bool head=owner.kind==410||owner.kind==454;
    std::array<int,12> key{word(rdram,owner.actor+0x94),MEM_H(0x34,int32_t(owner.physical)),MEM_H(0x44,int32_t(owner.physical)),MEM_H(0x7C,int32_t(owner.physical)),MEM_BU(0x28,int32_t(owner.physical)),MEM_H(0xD0,int32_t(owner.physical))};
    if(head)for(unsigned n=0;n<6;++n)key[6+n]=MEM_H(int32_t(0x80+2*n),int32_t(owner.physical));
    const auto now=std::chrono::steady_clock::now();
    if(old.lifetime==owner.lifetime&&old.key==key&&now-old.time<std::chrono::seconds(2))return;
    old={owner.lifetime,key,now};
    std::fprintf(stderr,"BOSS actor=%08X kind=%d state=%d phase=%d clipcmd=%d parent=%d init=%d hp=%d model=%08X time=%.6g flags=%u links=%d,%d,%d,%d,%d,%d xyz=%.6g,%.6g,%.6g velocity=%.6g,%.6g,%.6g\n",owner.actor,owner.kind,key[0],key[1],key[2],key[3],key[4],key[5],owner.model,double(real(rdram,owner.model+0x64)),unsigned(MEM_BU(0x7C,int32_t(owner.model))),key[6],key[7],key[8],key[9],key[10],key[11],double(real(rdram,owner.actor+0x9C)),double(real(rdram,owner.actor+0xA0)),double(real(rdram,owner.actor+0xA4)),double(real(rdram,owner.physical+0x10)),double(real(rdram,owner.physical+0x14)),double(real(rdram,owner.physical+0x18)));
}
#endif
struct BossPartCommand {
    Identity child,parent;int32_t state;int16_t clip;
};
std::array<std::optional<BossPartCommand>,dora3::distance::common_capacity> boss_part_commands;
bool linked_boss_part(const Identity& owner) {
    return (owner.rom==0xF8A40&&(owner.kind==411||owner.kind==412))||
        (owner.rom==0x111E70&&(owner.kind==455||owner.kind==456));
}
std::optional<Identity> boss_part_parent(uint8_t* rdram,const Identity& child) {
    if(!linked_boss_part(child))return std::nullopt;
    const int slot=MEM_H(0x7C,int32_t(child.physical));
    if(slot<30||slot>=dora3::distance::common_end)return std::nullopt;
    const auto parent=identity(rdram,0x804B1CD0u+unsigned(slot)*stride);
    if(!parent||parent->rom!=child.rom||parent->kind!=(child.rom==0xF8A40?410:454))return std::nullopt;
    return parent;
}
bool pending_boss_part_command(uint8_t* rdram,const Identity& child) {
    auto& pending=boss_part_commands[index(child.actor)];
    if(!pending)return false;
    const auto parent=boss_part_parent(rdram,child);
    if(!same(pending->child,child)||!parent||!same(pending->parent,*parent)||
       pending->state!=word(rdram,child.actor+0x94)) {
        pending.reset();return false;
    }
    if(pending->state==1)return true;
    const int waiting_phase=(child.kind==411||child.kind==455)?3:2;
    return MEM_BU(0x28,int32_t(child.physical))&&MEM_H(0x34,int32_t(child.physical))==waiting_phase;
}
void publish_boss_part_command(uint8_t* rdram,const Identity& head) {
    if(!((head.rom==0xF8A40&&head.kind==410)||(head.rom==0x111E70&&head.kind==454)))return;
    const int16_t clip=MEM_H(0x44,int32_t(head.physical));
    if(clip<0)return;
    for(unsigned n=0;n<2;++n) {
        const int slot=MEM_H(int32_t(0x80+2*n),int32_t(head.physical));
        if(slot<30||slot>=dora3::distance::common_end)continue;
        const auto child=identity(rdram,0x804B1CD0u+unsigned(slot)*stride);
        if(!child)continue;
        const auto parent=boss_part_parent(rdram,*child);
        if(!parent||!same(*parent,head))continue;
        auto& pending=boss_part_commands[index(child->actor)];
        const auto state=word(rdram,child->actor+0x94);
        // A later head clip is not a cancellation of reassembly. The head
        // can finish its own return before the slower body/leg reaches its
        // waiting phase. Preserve clip13 until that same live part consumes it.
        if(pending&&state==3&&pending->state==3&&pending->clip==13&&
           same(pending->child,*child)&&same(pending->parent,head))continue;
        pending.reset();
        // Follow consumes every clip. Reassembly waits for clip13 in body
        // phase3 / leg phase2. Retain that pulse while the independent part
        // completes its descent; a state/lifetime change cancels it.
        if(state==1||(state==3&&clip==13))
            pending=BossPartCommand{*child,head,state,clip};
    }
}
class BossPartCommandView {
    uint8_t* ram;std::optional<Identity> parent;int16_t saved=-1;
public:
    BossPartCommandView(uint8_t* rdram,const Identity& child):ram(rdram),parent(boss_part_parent(rdram,child)) {
        if(!parent)return;
        const bool pending=pending_boss_part_command(rdram,child);
        saved=MEM_H(0x44,int32_t(parent->physical));
        MEM_H(0x44,int32_t(parent->physical))=pending?boss_part_commands[index(child.actor)]->clip:-1;
        if(pending)boss_part_commands[index(child.actor)].reset();
    }
    ~BossPartCommandView() {
        if(!parent)return;
        const auto current=identity(ram,parent->actor);
        if(current&&same(*current,*parent)) {uint8_t* rdram=ram;MEM_H(0x44,int32_t(parent->physical))=saved;}
    }
};
struct ScenePhaseSample {Identity owner;uint64_t epoch;int32_t state;double phase;};
std::map<std::pair<uint32_t,uint32_t>,ScenePhaseSample> scene_phase_samples;
uint8_t scene_entry_latch=0;
bool executing=false;
bool attachment_only=false;
std::vector<Equation> equations;
std::vector<ForeignStep> foreign_steps;
// Source scratch fields may overwrite the old visual angle with its target
// delta. Capture that actual operand before the reviewed intermediate store.
std::map<uint32_t,float> equation_operands;
// Six real Source history cells retain the parent's actual equation from
// the time it was sampled. Continue that historical equation; do not lerp
// the two completed tail poses or shift Source history on Native slices.
struct DelayedHeadingHistory {
    Identity owner;uint64_t epoch;std::array<Equation,6> samples;
};
std::array<std::optional<DelayedHeadingHistory>,dora3::distance::common_capacity> delayed_heading_history;
struct SoundoragoHistory {
    Identity owner;uint64_t epoch;uint32_t buffer;unsigned count;
    std::array<std::array<Equation,5>,60> samples{};
};
std::map<uint32_t,SoundoragoHistory> soundorago_history;
struct VortexTrailHistory {
    Identity owner;uint64_t epoch;std::array<std::array<Equation,3>,2> samples;
};
std::map<uint32_t,VortexTrailHistory> vortex_trail_history;
struct Boss2GhostHistory {Identity owner;uint64_t epoch;std::array<Equation,6> fields;};
std::map<uint32_t,Boss2GhostHistory> boss2_ghost_history;
struct IntegerMotionOperand {double before,step;};
std::map<uint32_t,IntegerMotionOperand> integer_motion_operands;
std::map<uint32_t,ModelEventCheckpoint> model_reads;
// Per allocation/run/site, not per AI command: a hit can run the same command
// twice while its clip still projects to the same authored frame.
struct BossCueDelivery {uint64_t lifetime,run;long double distance;};
std::map<std::tuple<uint32_t,uint64_t,uint32_t>,BossCueDelivery> boss_cue_deliveries;
std::optional<SimulationSlice> command_slice;
struct ModelCommandShadow {
    uint64_t lifetime,run,time_command;
    int32_t saved,published;
    SourceModelCommandSample sample;
};
std::map<uint32_t,ModelCommandShadow> model_command_shadows;
void restore_model_command_shadows(uint8_t* rdram) {
    auto& models=game_model_animation();
    for(const auto& [model,shadow]:model_command_shadows)
        if(models.lifetime_token(model)==shadow.lifetime&&models.run_token(model)==shadow.run&&
           models.time_command_token(model)==shadow.time_command&&word(rdram,model+0x64)==shadow.published)
            MEM_W(0,int32_t(model+0x64))=shadow.saved;
    model_command_shadows.clear();
}
SourceModelCommandSample model_command_sample(uint8_t* rdram,uint32_t model) {
    auto& models=game_model_animation();
    if(const auto shadow=model_command_shadows.find(model);shadow!=model_command_shadows.end()&&
       models.lifetime_token(model)==shadow->second.lifetime&&models.run_token(model)==shadow->second.run&&
       models.time_command_token(model)==shadow->second.time_command&&word(rdram,model+0x64)==shadow->second.published)
    {
        auto result=shadow->second.sample;
        result.flags=uint8_t((result.flags&~4u)|(MEM_BU(0x7C,int32_t(model))&4u));
        return result;
    }
    return command_slice?models.source_command_sample(rdram,model,*command_slice):
        SourceModelCommandSample{models.command_event_checkpoint(rdram,model),uint8_t(MEM_BU(0x7C,int32_t(model)))};
}
std::optional<Gravity> gravity;
std::optional<Turn> turn;
std::optional<Projection> projection;
uint64_t epoch=0;
bool unchanged(uint8_t* rdram,const Period& v) {
    const auto current=identity(rdram,v.owner.actor);
    if(!current||!same(*current,v.owner)||commands(rdram,v.owner)!=v.command||word(rdram,v.owner.physical+0x1C)!=v.acceleration)return false;
    for(unsigned axis=0;axis<3;++axis) {
        if(std::bit_cast<int32_t>(v.expected_position[axis])!=word(rdram,v.owner.actor+0x9C+4*axis)||
           std::bit_cast<int32_t>(v.expected_velocity[axis])!=word(rdram,v.owner.physical+0x10+4*axis))return false;
    }
    for(const auto& equation:v.equations)
        if(std::bit_cast<int32_t>(equation.expected)!=word(rdram,equation.address))return false;
    if(v.opacity&&(std::bit_cast<int32_t>(v.opacity->expected)!=word(rdram,v.owner.model+0x78)||
        word(rdram,v.owner.model+0x74)!=(v.opacity->expected==1.f?0:1)||
        MEM_BU(0x2A,int32_t(v.owner.physical))!=(v.opacity->expected>=1.f?0:1)))return false;
    return std::bit_cast<int32_t>(v.expected_yaw[0])==word(rdram,v.owner.actor+0x10)&&
        std::bit_cast<int32_t>(v.expected_yaw[1])==word(rdram,v.owner.physical+4);
}
float angle(float value) {value=std::fmod(value,360.f);return value<0?value+360.f:value;}

// Sum the actual kick-before-drift recurrence at fractional h. At integer h,
// S is sum exp(i*w*j), T is sum j*exp(i*w*j). The h==1 caller keeps the real
// ROM proposal bits. This also pairs a speed ramp with a changing heading.
std::complex<double> harmonic_sum(double phase,double omega,double age,double speed,double acceleration) {
    if(age==0)return {};
    // Integer Source samples cannot distinguish frequencies separated by
    // 2*pi. Use their principal alias before fractional continuation. The
    // 12th sideband of Boss2/Boss6's 30-degree wave is exactly a full turn:
    // dividing by sin(pi) amplified its tiny coefficient into arena-sized
    // (or larger) displacements, especially during the speed ramp.
    omega=std::remainder(omega,2*std::acos(-1.));
    const auto rotation=std::polar(1.,phase+omega*(age+1)/2);
    double ratio,derivative;
    if(std::abs(omega)<1e-4) {
        // sin(h*w/2)/sin(w/2) and its derivative have removable limits.
        // Retain the acceleration term without cancellation near zero.
        const double h2=age*age,w2=omega*omega;
        const double fourth=7-10*h2+3*h2*h2;
        ratio=age*(1+(1-h2)*w2/24+fourth*w2*w2/5760);
        derivative=age*((1-h2)*omega/12+fourth*omega*w2/1440);
    } else {
        const double half=omega/2,extent=age*half;
        ratio=std::sin(extent)/std::sin(half);
        derivative=(age*std::cos(extent)*std::sin(half)-std::sin(extent)*std::cos(half))/(2*std::sin(half)*std::sin(half));
    }
    return rotation*std::complex<double>(ratio*(speed+acceleration*(age+1)/2),-acceleration*derivative);
}

// The Original common pool finishes an earlier parent's whole step before
// entering its child callback. Native visible motion has only finished this
// slice. Publish that parent's actual Source endpoint for the child's discrete
// decision, then restore visible fields; the pose-only path uses visible motion.
class SourceParentView {
    uint8_t* ram;
    std::optional<Identity> owner;
    struct Field {Identity target;uint32_t address;int32_t saved,published;};
    bool brobo_collision=false;
    std::vector<Field> fields;
    void publish_target(const Identity& target,uint32_t address,float value) {
        uint8_t* rdram=ram;
        const auto bits=std::bit_cast<int32_t>(value);
        fields.push_back({target,address,word(rdram,address),bits});MEM_W(0,int32_t(address))=bits;
    }
    void publish(uint32_t address,float value) {publish_target(*owner,address,value);}
public:
    SourceParentView(uint8_t* rdram,const Identity& child):ram(rdram) {
        const auto kind=MEM_H(0x20,int32_t(child.physical));
        if(!scene_parent_attachment_active(rdram,child)&&kind!=268&&kind!=361&&kind!=299&&kind!=273&&kind!=324&&kind!=285&&kind!=286&&kind!=292&&kind!=329&&kind!=333&&kind!=338&&kind!=349&&kind!=353&&kind!=401&&kind!=403&&kind!=405&&kind!=406&&!(child.rom==0xF8A40||child.rom==0x10DA30||child.rom==0x111E70||child.rom==0x11D570||(child.rom==0x108730&&child.kind==441)))return;
        const int parent_slot=attachment_parent_slot(rdram,child);
        if(parent_slot<30||parent_slot>=dora3::distance::common_end)return;
        owner=identity(rdram,0x804B1CD0u+unsigned(parent_slot)*stride);
        if(!owner||owner->actor>=child.actor)return;
        const auto& stored=periods[index(owner->actor)];
        if(!stored||!same(stored->owner,*owner))return;
        const auto& parent=*stored;
        // A real command/write after the earlier pool entry owns its fields.
        if(!unchanged(rdram,parent))return;
        brobo_collision=child.rom==0xD5D10&&child.kind==349&&word(rdram,child.actor+0x94)==1&&
            owner->rom==0xD5D10&&owner->kind==348;
        if(brobo_collision) {
            // E5A0..E65C also rewinds earlier linked parts. Their visible
            // fractional positions are not the operands the Source pool read.
            const int count=2*MEM_H(0x42,int32_t(owner->physical));
            for(int n=0;n<count;++n) {
                const int slot=MEM_H(0x80+2*n,int32_t(owner->physical));
                if(slot<30||slot>=dora3::distance::common_end)continue;
                const auto part=identity(rdram,0x804B1CD0u+unsigned(slot)*stride);
                if(!part||part->actor>=child.actor||part->actor==owner->actor||part->rom!=0xD5D10||
                   (part->kind!=349&&part->kind!=353))continue;
                const auto& motion=periods[index(part->actor)];
                if(!motion||!same(motion->owner,*part)||!unchanged(rdram,*motion))continue;
                for(const unsigned axis:{0u,2u})
                    publish_target(*part,part->actor+0x9C+4*axis,motion->source_endpoint_position[axis]);
            }
        }
        for(unsigned axis=0;axis<3;++axis)
            publish(owner->actor+0x9C+4*axis,parent.source_endpoint_position[axis]);
        if(kind==406)for(const auto& equation:parent.equations)
            if(equation.address==owner->physical+0x78)publish(equation.address,equation.after);
        if(kind==349)for(unsigned axis=0;axis<3;++axis)
            publish(owner->physical+0x10+4*axis,parent.velocity[axis]);
        if(scene_parent_attachment_active(rdram,child)||child.rom==0xF8A40||child.rom==0x10DA30||child.rom==0x111E70||child.rom==0x11D570||(child.rom==0x108730&&child.kind==441))for(const unsigned offset:{0xCu,0x14u}) {
            publish(owner->actor+offset,parent.source_endpoint_angles[offset==0xC?0:2]);
        }
        for(unsigned axis=0;axis<2;++axis) {
            publish(axis?owner->physical+4:owner->actor+0x10,parent.source_endpoint_angles[axis?3:1]);
        }
    }
    ~SourceParentView() {
        if(!owner)return;
        const auto current=identity(ram,owner->actor);
        if(!current||!same(*current,*owner))return;
        uint8_t* rdram=ram;
        for(const auto& field:fields) {
            const auto target=identity(rdram,field.target.actor);
            if(!target||!same(*target,field.target))continue;
            if(word(rdram,field.address)==field.published) {
                MEM_W(0,int32_t(field.address))=field.saved;continue;
            }
            if(!brobo_collision)continue;
            auto& motion=periods[index(target->actor)];
            if(!motion||!same(motion->owner,*target))continue;
            const float corrected=real(rdram,field.address);
            // This Source branch cancels movement, not velocity. Keep the
            // original velocity for the next recoil command, but do not drift
            // into the obstacle again on the remaining Native slices.
            for(const unsigned axis:{0u,2u})if(field.address==target->actor+0x9C+4*axis) {
                motion->collision_hold[axis]=corrected;
                motion->initial_position[axis]=motion->expected_position[axis]=
                    motion->source_endpoint_position[axis]=corrected;
            }
            if(field.address==target->physical+4) {
                motion->initial_yaw[1]=motion->proposal_yaw[1]=motion->expected_yaw[1]=corrected;
                motion->source_endpoint_angles[3]=corrected;motion->turn[1]=0;
                std::erase_if(motion->equations,[&](const auto& e){return e.address==field.address;});
            }
        }
    }
};
}
bool native_enemy_kind(int kind) {
    return kind>=0&&size_t(kind)<definition_kind_count&&definition_index.enemy[size_t(kind)]>=0;
}
bool native_scene_object_kind(int kind) {
    return kind>=0&&size_t(kind)<definition_kind_count&&definition_index.scene[size_t(kind)]>=0;
}
bool native_scene_object_owned(uint8_t* rdram,uint32_t actor) {
    const auto owner=identity(rdram,actor);return owner&&native_scene_object_kind(owner->kind);
}
bool native_enemy_owned(uint8_t* rdram,uint32_t actor) {const auto owner=identity(rdram,actor);return owner&&native_enemy_kind(owner->kind);}
void native_enemy_reset() {
    ++epoch;
    scene_phase_samples.clear();
    boss_part_commands.fill(std::nullopt);
    boss_cue_deliveries.clear();
    foreign_steps.clear();
    reset_slots(periods);
    reset_slots(delayed_heading_history);
    soundorago_history.clear();
    vortex_trail_history.clear();
    boss2_ghost_history.clear();
    gravity.reset();
    turn.reset();
    projection.reset();
    equations.clear();
    equation_operands.clear();
    integer_motion_operands.clear();
    model_reads.clear();
}
extern "C" int dora3_native_enemy_callback(uint8_t* rdram,recomp_context* ctx,uint32_t callback,uint32_t rom,void (*update)(uint8_t*,recomp_context*)) {
    dora3::graphics_profile::SampleScope graphics_sample(dora3::graphics_profile::Stage::EnemyCallbackSample); dora3::profile::Scope profile(dora3::profile::Tag::EnemyCallback);
#ifdef DORA3_CRASH_LOG
    if(!executing&&!attachment_only&&rom==0x10DA30) {
        const auto observed=identity(rdram,uint32_t(word(rdram,0x8021BDEC)));
        if(observed&&observed->callback==callback&&observed->rom==rom)log_boss5_turn(rdram,*observed,"entry");
    }
    if(!executing&&!attachment_only&&rom==0x111E70) {
        const auto observed=identity(rdram,uint32_t(word(rdram,0x8021BDEC)));
        if(observed&&observed->callback==callback&&observed->rom==rom)log_boss6_form(rdram,*observed,"entry");
    }
    if(!executing&&!attachment_only&&rom==0x129A30) {
        const auto observed=identity(rdram,uint32_t(word(rdram,0x8021BDEC)));
        if(observed&&observed->callback==callback&&observed->rom==rom)log_sentoutei_motion(rdram,*observed,"entry");
    }
#endif
    if(executing||attachment_only||!native_gameplay_preview_object_pass()||!active_simulation_slice())return 0;
    const auto actor=uint32_t(word(rdram,0x8021BDEC));const auto owner=identity(rdram,actor);
    if(!owner||owner->callback!=callback||owner->rom!=rom)return 0;
#ifdef DORA3_CRASH_LOG
    dora3::crash::enemy={actor,owner->model,rom,callback,0,word(rdram,actor+0x94),MEM_H(0x34,int32_t(owner->physical)),owner->kind};
    log_boss_state(rdram,*owner);
#endif
    // Every registered Source-command owner can be interrupted independently
    // of its animation. Ordinary enemies also wait for exact clip cues (e.g.
    // Bironn, Brobo and Arizinogu); restricting clip-local sampling to bosses
    // and Powan leaves their cues on a shifted fractional AI timeline.
    // Preserve authored equality and skipped frames, using the clip's clock.
    // Source callbacks also query secondary model groups (shield, warning,
    // linked NPC channels). Each group has its own clip origin and can be
    // rebound independently of the actor's main model.
    for(unsigned group=0;group<8;++group)
        game_model_animation().source_clip_period(uint32_t(word(rdram,actor+0x74+4*group)),true);
    auto slice=*active_simulation_slice();auto& previous=periods[index(actor)];

    if(previous&&same(previous->owner,*owner)&&previous->tick==slice.tick&&previous->denominator==slice.denominator&&previous->expected_phase==slice.phase) {
        if(!pending_boss_part_command(rdram,*owner)&&previous->command==commands(rdram,*owner)) {
            // E8D44 clears physical2A before every callback. Original's
            // callback can then set it via E89CC; an unchanged Native period
            // skips that callback, so preserve its result after this prelude.
            // Otherwise transparent projectiles lose their flag every slice,
            // and fading actors spuriously cancel their opacity equation.
            if(MEM_BU(0xC8,int32_t(owner->physical))==0&&MEM_BU(0x29,int32_t(owner->physical))==0)
                MEM_B(0x2A,int32_t(owner->physical))=int8_t(previous->callback_opacity_flag);
            // Camera-facing geometry below is our own write, not an external
            // command. Check before it so it cannot cancel scale/growth/fade.
            const bool motion_unchanged=unchanged(rdram,*previous);
            if(owner->kind==373&&word(rdram,actor+0x94)==1) {
                // This family's Source geometry before its entry/FSM reads
                // the current camera. Repeat only that proven pure prefix;
                // its counters, chain births and opacity commands stay once.
                recomp_context pose;dora3::copy_recomp_context(pose,*ctx);
                struct VisualOnly {VisualOnly(){attachment_only=true;}~VisualOnly(){attachment_only=false;}} visual_only;
                {SimulationSuspension source;update(rdram,&pose);}
                if(motion_unchanged) {
                    previous->initial_yaw[0]=previous->proposal_yaw[0]=real(rdram,actor+0x10);
                    previous->turn[0]=0;
                }
            }
            if(motion_unchanged)return 1;
            // A displacement/impulse/seek is a motion command. It must not
            // consume another wait counter, RNG draw or attack command.
            auto& v=*previous;v.start_phase=slice.phase;v.gravity.reset();v.projection.reset();v.equations.clear();v.opacity.reset();
            for(unsigned j=0;j<3;++j) {
                v.initial_position[j]=v.expected_position[j]=real(rdram,actor+0x9C+4*j);
                v.velocity[j]=v.expected_velocity[j]=real(rdram,owner->physical+0x10+4*j);
            }
            v.initial_yaw=v.proposal_yaw=v.expected_yaw={real(rdram,actor+0x10),real(rdram,owner->physical+4)};v.turn={};
            v.acceleration=word(rdram,owner->physical+0x1C);
            v.attachment_motion.reset();v.attachment_axes={};v.historical_attachment=false;v.collision_hold={};v.constraint_correction={};
            v.attachment_source_position=v.initial_position;
            for(unsigned j=0;j<3;++j)v.source_endpoint_position[j]=float(double(v.initial_position[j])+double(v.velocity[j])*(1-double(slice.phase)/slice.denominator));
            v.source_endpoint_angles={real(rdram,actor+0xC),real(rdram,actor+0x10),real(rdram,actor+0x14),real(rdram,owner->physical+4)};
            return 1;
        }
    }
    if(slice.phase) {
        // A genuinely new state/type/lifetime or newly acquired domain begins
        // at this command's timestamp. Its first counter period is full length.
        // The motion-only rebase above returns without repeating AI commands.
        native_gameplay_preview_enemy_command_clock(rdram,actor);
        slice=*active_simulation_slice();
    }
    const auto entered_state=word(rdram,actor+0x94);
    const auto entered_phase=MEM_H(0x34,int32_t(owner->physical));
    const std::array<float,3> command_position{
        real(rdram,actor+0x9C),real(rdram,actor+0xA0),real(rdram,actor+0xA4)};
    const std::array<float,2> before_yaw{real(rdram,actor+0x10),real(rdram,owner->physical+4)};
    const auto entered_epoch=epoch;
    // A producer can reuse an earlier slot that Source has already visited.
    // Capture allocation identity before its command, never just the address.
    std::array<uint64_t,dora3::distance::common_capacity> earlier_lifetimes{};
    for(unsigned slot=0;slot<index(actor);++slot) {
        const uint32_t prior=pool+slot*stride,model=uint32_t(word(rdram,prior+0x74));
        if(word(rdram,prior+0x94)&&pointer(model,0x84))earlier_lifetimes[slot]=game_model_animation().lifetime_token(model);
    }
    struct CommandScope {
        bool prior=executing;uint32_t prior_model_actor;
        CommandScope(uint32_t actor,uint8_t* rdram):prior_model_actor(game_model_animation().source_command_actor(actor)){scene_entry_latch=MEM_BU(0x28,word(rdram,actor+0x98));executing=true;foreign_steps.clear();gravity.reset();turn.reset();projection.reset();equations.clear();equation_operands.clear();integer_motion_operands.clear();model_reads.clear();}
        ~CommandScope(){game_model_animation().source_command_actor(prior_model_actor);executing=prior;command_slice.reset();}
    } command_scope(actor,rdram);
    command_slice=slice;
    // The complete real FSM runs once, preserving wait/RNG/clip/sound/spawn/
    // damage/retirement order. Recursion re-enters the untouched Original body.
    // Suspension keeps its full Source operands; Native motion is below.
    {
        struct RestoreModelReads {uint8_t* ram;~RestoreModelReads(){restore_model_command_shadows(ram);}} restore_reads{rdram};
        SourceParentView parent_view(rdram,*owner);BossPartCommandView clip_view(rdram,*owner);
        SimulationSuspension source;update(rdram,ctx);
    }
    if(epoch==entered_epoch)for(unsigned slot=0;slot<index(actor);++slot) {
        const uint32_t born_actor=pool+slot*stride;const auto born=identity(rdram,born_actor);
        if(born&&born->lifetime!=earlier_lifetimes[slot])native_gameplay_preview_enemy_defer_birth(rdram,born_actor);
    }
    const auto after=identity(rdram,actor);
    if(epoch!=entered_epoch||!after||!same(*owner,*after)||!active_simulation_slice())return 1;
    publish_boss_part_command(rdram,*after);
    const auto kind=MEM_H(0x20,int32_t(after->physical));
    Period value{*after,slice.tick,slice.denominator,slice.phase,slice.phase,game_model_animation().run_token(after->model),commands(rdram,*after)};
    for(unsigned axis=0;axis<3;++axis) {
        value.initial_position[axis]=value.expected_position[axis]=real(rdram,actor+0x9C+4*axis);
        value.velocity[axis]=value.expected_velocity[axis]=real(rdram,after->physical+0x10+4*axis);
        // Heading/projection and speed commands act at this timestamp. Only
        // a reviewed gravity kick becomes a continuous velocity recurrence.
    }
    if(gravity&&gravity->actor==actor&&gravity->physical==after->physical&&gravity->calls==1&&
       std::bit_cast<int32_t>(gravity->after)==std::bit_cast<int32_t>(value.velocity[1]))value.gravity=gravity;
    // Judge the final Original stores before the generic heading rewind:
    // that rewind can touch the same yaw and hide an accepted rate equation.
    std::vector<Equation> accepted_equations;
    for(const auto& equation:equations)
        if(std::bit_cast<int32_t>(equation.after)==word(rdram,equation.address))
            accepted_equations.push_back(equation);
    for(unsigned axis=0;axis<2;++axis) {
        const auto address=axis?after->physical+4:actor+0x10;const auto proposal=real(rdram,address);
        // Only the actual bounded-turn helper E88C4 establishes a continuous
        // angular equation. Init, wall reflections and other direct setters
        // keep their exact instantaneous Source values, regardless of size.
        const auto step=float(std::remainder(double(proposal)-before_yaw[axis],360.));
        const bool continuous=turn&&turn->actor==actor&&turn->physical==after->physical&&turn->calls==1&&
            std::bit_cast<int32_t>(turn->before)==std::bit_cast<int32_t>(before_yaw[axis])&&
            std::bit_cast<int32_t>(turn->after)==std::bit_cast<int32_t>(proposal)&&std::isfinite(step);
        value.initial_yaw[axis]=continuous?before_yaw[axis]:proposal;value.turn[axis]=continuous?step:0;value.proposal_yaw[axis]=proposal;
        value.expected_yaw[axis]=value.initial_yaw[axis];put(rdram,address,value.expected_yaw[axis]);
    }

    for(const auto& step:foreign_steps)if(foreign_identity(rdram,step)&&
        std::bit_cast<int32_t>(step.after)==word(rdram,step.actor+step.offset)) {
        value.foreign_steps.push_back(step);value.foreign_steps.back().rebase=step.before;value.foreign_steps.back().start_phase=slice.phase;value.foreign_steps.back().denominator=slice.denominator;put(rdram,step.actor+step.offset,step.before);
    }
    for(const auto& equation:accepted_equations) {
        // Direct floor/initialization writes have already run. Only the final
        // actual mathematical store receives its fractional continuation.
        value.equations.push_back(equation);
        value.equations.back().expected=equation.evaluate(0);
        put(rdram,equation.address,value.equations.back().expected);
    }
    for(const auto& equation:value.equations)if((((kind==475||kind==476||kind==458||kind==460||kind==461||kind==469||kind==471||kind==449||kind==451||kind==453||kind==444||kind==429||kind==433||kind==434||kind==414||kind==416||kind==417||kind==425||kind==427||kind==401||kind==403||kind==311||kind==313||kind==316||kind==327||kind==329||kind==338||kind==296||kind==397||kind==361||kind==381||kind==389||kind==390||kind==391)&&equation.address==after->physical+0x58)||((kind==405||kind==406)&&equation.address==after->physical+0x78)||(kind==299&&equation.address==after->physical+0x60)||((kind==373||kind==335)&&equation.address==after->physical+0x68)||((kind==281||kind==291||(kind>=308&&kind<=310))&&equation.address==after->physical+0x5C))) {
        const auto published=real(rdram,after->model+0x78);
        if(std::bit_cast<int32_t>(published)==std::bit_cast<int32_t>(equation.after)) {
            // Powan fade/effect publishes the new counter. Reproduce the real
            // 3B168 model alpha/flag and E89CC physics flag along that equation.
            value.opacity=equation;value.opacity->address=after->model+0x78;
        } else if((kind==403||kind==281||kind==311||kind==313||kind==361)&&std::bit_cast<int32_t>(published)==std::bit_cast<int32_t>(equation.before)&&
                  previous&&same(previous->owner,*after)&&previous->opacity) {
            // Supaizan web and MiniCar smoke publish BEFORE decrement. Its model alpha follows the
            // previous published value to this proposal; the first bind is an
            // instantaneous command, not an invented pre-birth fade sample.
            value.opacity=Equation{after->model+0x78,EquationKind::linear,previous->opacity->after,published};
            value.opacity->step=double(published)-previous->opacity->after;
        } else if((kind==403||kind==281||kind==311||kind==313||kind==361)&&std::bit_cast<int32_t>(published)==std::bit_cast<int32_t>(equation.before)) {
            value.opacity=Equation{after->model+0x78,EquationKind::linear,published,published};
        }
    }
    if(kind==260||kind==582)for(const auto& equation:value.equations)if(equation.address==after->model+0x78) {
        value.opacity=equation;
    }
    if(kind==66||kind==67||kind==493||kind==494||kind==515||kind==516||kind==517||kind==520||kind==540||kind==581||kind==583) {
        for(const auto& equation:value.equations)if(equation.address==after->physical+0x58&&
            std::bit_cast<int32_t>(equation.after)==word(rdram,after->model+0x78)) {
            value.opacity=equation;value.opacity->address=after->model+0x78;
        }
    }
    if(kind==257||kind==258) {
        const auto alpha=real(rdram,after->model+0x78);
        value.opacity=Equation{after->model+0x78,EquationKind::linear,alpha,alpha};
        if(kind==258) {
            value.opacity->kind=EquationKind::sine;
            value.opacity->phase=double(MEM_H(0x34,int32_t(after->physical)))-5;
            value.opacity->step=5;value.opacity->amplitude=1;
            value.opacity->radians=rom_double(rdram,0x802113A8);
            value.opacity->before=value.opacity->evaluate(0);
        } else if(previous&&same(previous->owner,*after)&&previous->run==value.run&&model_reads.contains(after->model)) {
            // Source publishes the profile at the command's observed frame.
            // Continue that real profile from its preceding frame sample;
            // first bind/run remains an instantaneous opacity command.
            value.opacity->kind=EquationKind::frame_triangle;
            const float rate=MEM_BU(0xC9,int32_t(after->physical))==1?0:
                real(rdram,after->physical+0xCC)==0?3:real(rdram,after->physical+0xCC);
            value.opacity->phase=double(model_reads.at(after->model).time)-rate;
            value.opacity->step=rate;value.opacity->before=value.opacity->evaluate(0);
        }
    }
    value.update=update;
    value.acceleration=word(rdram,after->physical+0x1C);
    value.callback_opacity_flag=MEM_BU(0x2A,int32_t(after->physical));
    value.projection=projection;
    const auto source_clock=game_model_animation().source_command_sample(rdram,after->model,slice);
    value.model_events=source_clock.checkpoint;
    game_model_animation().remember_source_clock(after->model,source_clock);
    value.model_reads=model_reads;
    if((((kind==268||kind==299||kind==292||kind==333)&&MEM_H(0x36,int32_t(after->physical))==0)||((kind==273||kind==324||kind==285||kind==286)&&word(rdram,actor+0x94)==1)||(kind==329&&word(rdram,actor+0x94)==1&&MEM_H(0x34,int32_t(after->physical))==0))||((kind==361||kind==338||kind==349||kind==353||kind==405)&&word(rdram,actor+0x94)==1)||(kind==401&&word(rdram,actor+0x94)!=10)||(kind==403&&(word(rdram,actor+0x94)==1||
        (word(rdram,actor+0x94)==10&&MEM_H(0x34,int32_t(after->physical))==0)))||enemy_parent_attachment_active(rdram,*after)) {
        // Fannel's state1 can choose state4 after publishing its own aim.
        // The orbit body and its parent yaw copy have not executed yet.
        // Start continuous attachment only after an actual state4 command.
        const bool attachment_executed=!(after->rom==0x111E70&&kind==467&&entered_state!=4);
        const int parent_slot=attachment_parent_slot(rdram,*after);
        if(attachment_executed&&parent_slot>=30&&parent_slot<dora3::distance::common_end)value.parent=identity(rdram,0x804B1CD0u+unsigned(parent_slot)*stride);
    }

    if(scene_parent_attachment_active(rdram,*after)) {
        const int parent_slot=MEM_H(0x7C,int32_t(after->physical));
        if(parent_slot>=30&&parent_slot<dora3::distance::common_end)value.parent=identity(rdram,0x804B1CD0u+unsigned(parent_slot)*stride);
    }
    value.attachment_source_position=value.initial_position;
    for(unsigned axis=0;axis<3;++axis) {
        float initial=value.initial_position[axis];
        for(const auto& equation:value.equations)if(equation.address==actor+0x9C+4*axis)initial=equation.after;
        value.source_endpoint_position[axis]=float(initial+value.velocity[axis]);
    }
    if(after->rom==0x111E70&&kind==454&&entered_state==7&&entered_phase==7&&
       word(rdram,actor+0x94)==7&&MEM_H(0x34,int32_t(after->physical))==7) {
        // 25794..258BC projects X/Z onto the rear rail, then 258D0 adds
        // the newly aimed velocity. At a stationary edge these cancel.
        // Applying the projection instantly, then only splitting velocity,
        // showed a 20-unit sawtooth in test145 despite stable Y/clip time.
        // Continue the actual correction with the drift; keep the ROM's
        // endpoint, heading, velocity, attack counters and collision data.
        const float rail_x=std::clamp(command_position[0],-700.f,700.f);
        const float rail_z=std::max(std::abs(rail_x)-500.f,0.f)-850.f;
        if(value.initial_position[0]==rail_x&&value.initial_position[2]==rail_z)
            for(const unsigned axis:{0u,2u})
                value.constraint_correction[axis]=double(value.initial_position[axis])-command_position[axis];
    }
    if(after->rom==0xD9910&&(kind==269||kind==270)&&entered_state==2&&
       word(rdram,actor+0x94)==2&&MEM_H(0x34,int32_t(after->physical))==1) {
        // Both ground-phase tails assign floor+80, restore the landing Y
        // velocity and let the common pass add it once. The resulting height
        // is the Source pose. Splitting that final add across Native slices
        // repeatedly drifts down from floor+80, then snaps back next period.
        // Hold the complete grounded result; keep the original velocity and
        // FSM untouched for takeoff, death and external motion commands.
        value.collision_hold[1]=value.source_endpoint_position[1];
    }
    for(unsigned field=0;field<4;++field) {
        const auto address=field==3?after->physical+4:actor+0xC+4*field;
        float proposed=field==1?value.proposal_yaw[0]:field==3?value.proposal_yaw[1]:real(rdram,address);
        for(const auto& equation:value.equations)if(equation.address==address)proposed=equation.after;
        value.source_endpoint_angles[field]=proposed;
    }

    if(value.parent&&(scene_parent_attachment_active(rdram,*after)||
       (after->rom==0x10DA30&&kind==447))) {
        // Slime copies pitch/yaw as well as XYZ. Keep the same mathematical
        // parent interval for all five copies, including an earlier reused
        // child slot; a live parent field may still be its Source endpoint.
        const auto& parent=periods[index(value.parent->actor)];
        if(parent&&same(parent->owner,*value.parent)) {
            // Earlier parents have this period's equation; later parents
            // retain the preceding Source period, exactly as the pool reads.
            // Preserve operands (including roll), never completed pose pairs.
            auto motion=std::make_shared<Period>(*parent);
            motion->command.clear();motion->model_reads.clear();motion->foreign_steps.clear();
            motion->opacity.reset();motion->parent.reset();motion->update=nullptr;
            value.scene_parent_motion=std::move(motion);
        } else {
            // The first Source visit of a child before its parent reads an
            // unmoved pose. It cannot follow that parent's future first step.
            auto motion=std::make_shared<Period>(Period{*value.parent,slice.tick,slice.denominator,slice.phase,slice.phase,0,{}});
            for(unsigned axis=0;axis<3;++axis)motion->initial_position[axis]=real(rdram,value.parent->actor+0x9C+4*axis);
            motion->initial_yaw=motion->proposal_yaw={real(rdram,value.parent->actor+0x10),real(rdram,value.parent->physical+4)};
            motion->source_endpoint_angles={real(rdram,value.parent->actor+0xC),real(rdram,value.parent->actor+0x10),real(rdram,value.parent->actor+0x14),real(rdram,value.parent->physical+4)};
            value.scene_parent_motion=std::move(motion);
        }
        value.scene_parent_start_phase=slice.phase;
        if(kind==527) {
            const bool earlier=value.parent->actor<actor;
            const float source_y=earlier&&parent&&same(parent->owner,*value.parent)?
                parent->source_endpoint_position[1]:real(rdram,value.parent->actor+0xA0);
            value.attachment_offsets[1]=value.attachment_source_position[1]-source_y;
        }
    }
    if(value.parent&&value.parent->actor<actor) {
        const auto state=word(rdram,actor+0x94);
        bool xyz=(after->rom==0xF8A40&&(kind==411||kind==412||kind==425||(kind==415&&MEM_H(0x44,int32_t(after->physical))>=3)))||
            (after->rom==0x111E70&&(kind==455||kind==456||kind==469||(kind==459&&MEM_H(0x44,int32_t(after->physical))>=3)))||
            (after->rom==0x10DA30&&kind==447)||(after->rom==0x108730&&kind==441)||(after->rom==0x11D570&&(kind==473||(kind==475&&state==1)));
        const bool brobo_part=after->rom==0xD5D10&&kind==349&&state==1&&
            value.parent->rom==0xD5D10&&value.parent->kind==348;
        xyz=xyz||brobo_part;
        value.attachment_axes={xyz,xyz,xyz};
        if(((kind==415&&after->rom==0xF8A40)||(kind==459&&after->rom==0x111E70))&&MEM_H(0x44,int32_t(after->physical))==2)value.attachment_axes[1]=true;
        const auto& parent=periods[index(value.parent->actor)];
        if(std::any_of(value.attachment_axes.begin(),value.attachment_axes.end(),[](bool x){return x;})&&parent&&same(parent->owner,*value.parent)) {
            value.attachment_motion=immutable_motion(*parent);
            value.attachment_start_age=double(parent->expected_phase-parent->start_phase)/parent->denominator-double(slice.numerator)/slice.denominator;
            for(unsigned axis=0;axis<3;++axis)value.attachment_offsets[axis]=value.attachment_source_position[axis]-parent->source_endpoint_position[axis];
        }
    }
    previous=value;
#ifdef DORA3_CRASH_LOG
    log_boss5_turn(rdram,*after,"command");
    log_sentoutei_motion(rdram,*after,"command");
    log_boss6_form(rdram,*after,"command");
#endif
    return 1;
}
// Child FSMs can release links or write a parent's handshake after that
// parent's entry. Those are Source-pool results, consumed next Source period;
// they must not rerun its wait/attack commands on the next Native slice.
void native_enemy_source_pool_end(uint8_t* rdram) {
    dora3::graphics_profile::SampleScope graphics_sample(dora3::graphics_profile::Stage::EnemyFinalizeSample); dora3::profile::Scope detailProfile(dora3::profile::Tag::EnemyPoolFinalize);
    for(auto& period:periods)if(period) {
        const auto owner=identity(rdram,period->owner.actor);
        if(owner&&same(*owner,period->owner))period->command=commands(rdram,*owner);
    }
}
extern "C" int dora3_native_boss_detach_empty(uint8_t* rdram,recomp_context* ctx) {
    if(!executing&&!active_simulation_slice())return 0;
    const auto parent=identity(rdram,uint32_t(ctx->r4));
    if(!parent||!((parent->rom==0xF8A40&&parent->kind==410)||
                 (parent->rom==0x111E70&&parent->kind==454)))return 0;
    const auto slot=int32_t(ctx->r5);
    if(slot<0||slot>=15)return 0;
    // 801E7528 assumes a live child. In head state4 the finished effect can
    // be detached at the top of the callback and again by the phase2 exit.
    // An already empty link needs no second reciprocal write through slot-1.
    if(MEM_H(0x80+2*slot,int32_t(parent->physical))!=-1)return 0;
#ifdef DORA3_CRASH_LOG
    std::fprintf(stderr,"BOSS EMPTY DETACH rom=%08X actor=%08X state=%d phase=%d slot=%d\n",parent->rom,parent->actor,word(rdram,parent->actor+0x94),int(MEM_H(0x34,int32_t(parent->physical))),slot);
#endif
    return 1;
}
extern "C" int dora3_native_enemy_attachment_only() {return attachment_only;}
extern "C" int dora3_native_enemy_commands_active() {return executing;}
extern "C" void dora3_native_enemy_clip_end(uint8_t* rdram,recomp_context* ctx) {
    const auto model=uint32_t(ctx->r4);
    if(!executing) {
        if(const auto flags=native_gameplay_preview_npc_model_flags(rdram,model))ctx->r2=(ctx->r2&~2ull)|(*flags&2u);
        else if(const auto flags=game_model_animation().handoff_source_flags(rdram,model))ctx->r2=(ctx->r2&~2ull)|(*flags&2u);
        return;
    }
    ctx->r2=(ctx->r2&~2ull)|(model_command_sample(rdram,model).flags&2u);
    for(const auto& period:periods)if(period&&period->owner.model==model&&
        identity(rdram,period->owner.actor)&&same(*identity(rdram,period->owner.actor),period->owner)&&
        period->owner.lifetime==game_model_animation().lifetime_token(model)&&
        period->run==game_model_animation().run_token(model)&&
        model_command_sample(rdram,model).checkpoint.ends>period->model_events.ends) {
        ctx->r2|=2;return;
    }
    (void)rdram;
}
extern "C" int dora3_native_enemy_model_event(uint8_t* rdram,uint32_t model,float observed,float target,uint64_t site) {
    // During the discrete FSM, SDK3955C already returned the complete
    // Source-step sample. Preserve the actual c.eq.s: Source can skip a cue
    // (e.g. frames18->21 never equals20), so crossing detection here would
    // invent an attack/sound. Continuous Native consumers use ModelAnimation's
    // separate crossing interface outside this Source-command scope.
    if(!executing)return game_model_animation().handoff_source_event(rdram,model,observed,target);
    if(observed!=target)return 0;
    const auto owner=identity(rdram,uint32_t(word(rdram,0x8021BDEC)));
    if(!owner)return 1;
    auto& models=game_model_animation();
    const auto sample=model_command_sample(rdram,model).checkpoint;
    const auto key=std::make_tuple(model,site,std::bit_cast<uint32_t>(target));
    const BossCueDelivery next{models.lifetime_token(model),sample.run,sample.distance};
    const auto prior=boss_cue_deliveries.find(key);
    // Projected distances are calculated from rational time with one float
    // read. Use a tiny distance tolerance solely for deduplication, never to
    // make an unreachable Source frame into a new event.
    if(prior!=boss_cue_deliveries.end()&&prior->second.lifetime==next.lifetime&&
       prior->second.run==next.run&&std::abs(prior->second.distance-next.distance)<0.00001L)return 0;
    boss_cue_deliveries.insert_or_assign(key,next);
#ifdef DORA3_CRASH_LOG
    std::fprintf(stderr,"BOSS CUE actor=%08X kind=%d site=%llX frame=%.6g run=%llu\n",owner->actor,owner->kind,(unsigned long long)site,double(target),(unsigned long long)next.run);
#endif
    return 1;
}
extern "C" void dora3_native_enemy_model_read(uint8_t* rdram,uint32_t model) {
    if(!executing)return;
    auto& models=game_model_animation();
    if(!models.lifetime_token(model))return;
    const auto sample=model_command_sample(rdram,model);
    if(!model_command_shadows.contains(model)) {
        const auto saved=word(rdram,model+0x64),published=std::bit_cast<int32_t>(sample.checkpoint.time);
        model_command_shadows.emplace(model,ModelCommandShadow{models.lifetime_token(model),models.run_token(model),
            models.time_command_token(model),saved,published,sample});
        MEM_W(0,int32_t(model+0x64))=published;
    }
    model_reads.insert_or_assign(model,sample.checkpoint);
}
extern "C" int dora3_native_enemy_model_zero(uint8_t* rdram,uint32_t model,float observed,uint64_t site) {
    return observed<=0||dora3_native_enemy_model_event(rdram,model,observed,0,site);
}
extern "C" void dora3_native_enemy_projection(uint8_t* rdram,recomp_context* ctx,uint32_t site) {
    if(!executing)return;
    const auto actor=uint32_t(word(rdram,0x8021BDEC));const auto owner=identity(rdram,actor);
    if(!owner||uint32_t(ctx->r16)!=owner->physical)return;
    if(!projection)projection=Projection{owner->physical};
    auto& value=*projection;
    if(site==0x801E8064||site==0x801E80A0){value.pitch_cosine=ctx->f0.fl;return;}
    const bool xyz=site==0x801E8074||site==0x801E80B0||site==0x801E80D8;
    const unsigned axis=(site==0x801E7F88||site==0x801E8074)?0:
        (site==0x801E7FA8||site==0x801E80B0)?2:1;
    ++value.calls[axis];value.unit[axis]=ctx->f0.fl;
    value.scalar_offset[axis]=!xyz&&axis==1?8:0;
    value.negate[axis]=axis==1;
    value.first_factor[axis]=xyz&&axis!=1?value.pitch_cosine:1;
    value.xyz[axis]=xyz;
    value.heading[axis]=real(rdram,owner->physical+4);
    value.pitch[axis]=real(rdram,owner->physical+0xC);
    const uint32_t constant=xyz?0x802115A0u:axis==1?0x80211598u:0x80211590u;
    const uint64_t bits=(uint64_t(uint32_t(word(rdram,constant)))<<32)|uint32_t(word(rdram,constant+4));
    value.radians[axis]=std::bit_cast<double>(bits);
    value.speed[axis]=real(rdram,owner->physical+value.scalar_offset[axis]);
    const auto capture=[&](uint32_t address) -> std::optional<Equation> {
        const auto found=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
            return e.address==address&&std::bit_cast<int32_t>(e.after)==word(rdram,address);
        });
        if(found==equations.end())return std::nullopt;
        return *found;
    };
    value.scalar_equation[axis]=capture(owner->physical+value.scalar_offset[axis]);
    value.heading_equation[axis]=capture(owner->physical+4);
    value.pitch_equation[axis]=capture(owner->physical+0xC);
    const float first=axis==1?-value.speed[axis]:float(value.speed[axis]*value.first_factor[axis]);
    value.proposal[axis]=site==0x801E8004?ctx->f20.fl:
        float((site==0x801E8074?ctx->f20.fl:site==0x801E80B0?ctx->f22.fl:xyz?ctx->f20.fl:first)*value.unit[axis]);
    if(owner->rom==0xE5480&&owner->kind==335)value.parts[axis].push_back({value.speed[axis],value.heading[axis],
        value.pitch[axis],value.radians[axis],value.proposal[axis],xyz,value.scalar_equation[axis],
        value.heading_equation[axis],value.pitch_equation[axis]});
}

// NPC motion stores are separate from enemy AI. Every row corresponds to a
// reviewed ROM instruction, never to an observed small numerical delta.
struct SceneStore {
    uint32_t rom,site,offset;bool physical;EquationKind kind;unsigned operand;
    uint32_t coefficient=0;double target=0;uint32_t copy_from=0;
};
constexpr std::array scene_stores{
    SceneStore{0x1456C0,0x8022DDA0,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x1456C0,0x8022E6BC,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x1456C0,0x8022F1A0,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x1456C0,0x8022FC08,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x1456C0,0x80230400,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x1456C0,0x80230AE0,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x1456C0,0x802310F0,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x1456C0,0x80231688,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x1456C0,0x8023215C,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x1456C0,0x8022E430,0x4,true,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x1456C0,0x8022F97C,0x4,true,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x1456C0,0x802325F0,0x4,true,EquationKind::angle,0,0x0,0,0x0},
    // Doragonia guard state100: visible body follows the same +10 turn.
    SceneStore{0x1456C0,0x8023267C,0x10,false,EquationKind::angle,0,0x0,0,0x4},
    SceneStore{0x1456C0,0x8022E434,0x10,false,EquationKind::angle,0,0x0,0,0x4},
    SceneStore{0x1456C0,0x8022F980,0x10,false,EquationKind::angle,0,0x0,0,0x4},
    SceneStore{0x1398A0,0x80231190,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x1398A0,0x80232070,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x1398A0,0x802327FC,0x4,true,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x1398A0,0x802328B8,0x4,true,EquationKind::angle,0,0x0,0,0x0},
    // Keeper pass sequence copies its captured movement yaw to the visible body.
    SceneStore{0x1398A0,0x802328F4,0x10,false,EquationKind::angle,0,0x0,0,0x4},
    SceneStore{0x154A90,0x8023E18C,0x4,true,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x154A90,0x8023E5C0,0x4,true,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x154A90,0x8023E420,0x0,true,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x1561E0,0x8023FEEC,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x1561E0,0x80240144,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x154A90,0x8023E780,0x18,false,EquationKind::approach,4,0x8023EDE0,0.5,0x0},
    SceneStore{0x154A90,0x8023E784,0x20,false,EquationKind::approach,4,0x8023EDE0,0.5,0x18},
    SceneStore{0x154A90,0x8023E788,0x1C,false,EquationKind::approach,2,0x8023EDD8,3,0x0},
    SceneStore{0x154A90,0x8023EBC4,0x18,false,EquationKind::approach,4,0x8023EE18,0.5,0x0},
    SceneStore{0x154A90,0x8023EBC8,0x20,false,EquationKind::approach,4,0x8023EE18,0.5,0x18},
    SceneStore{0x154A90,0x8023EBCC,0x1C,false,EquationKind::approach,2,0x8023EE10,3,0x0},
    SceneStore{0x154A90,0x8023E7A8,0x58,true,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x154A90,0x8023EBEC,0x58,true,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x14D4C0,0x8023DA50,0x18,false,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x14D4C0,0x8023DA58,0x20,false,EquationKind::linear,0,0x0,0,0x18},
    SceneStore{0x14D4C0,0x8023DA5C,0x1C,false,EquationKind::linear,0,0x0,0,0x18},
    SceneStore{0x14D4C0,0x8023DC60,0x18,false,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x14D4C0,0x8023DC68,0x20,false,EquationKind::linear,0,0x0,0,0x18},
    SceneStore{0x14D4C0,0x8023DC6C,0x1C,false,EquationKind::linear,0,0x0,0,0x18},
    SceneStore{0x70A50,0x801EACC8,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x70A50,0x801E9654,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x70A50,0x801E97F0,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x129A30,0x8022DD34,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x129A30,0x8022DEF0,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x14D4C0,0x8023DE64,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    // Both Park Dez wheels share this +2-degree roll; gondolas derive their pose from it.
    SceneStore{0x14D4C0,0x8023ECCC,0x14,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x14FD20,0x8023D884,0x14,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x126BA0,0x8022EB1C,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x154A90,0x8023DE30,0x14,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x150C60,0x8023F658,0xC,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x15A960,0x8023EDA8,0x10,false,EquationKind::angle,0,0x0,0,0x0},
    SceneStore{0x70A50,0x801EAC2C,0x18,false,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x70A50,0x801EAC30,0x20,false,EquationKind::linear,0,0x0,0,0x18},
    SceneStore{0x14FD20,0x8023D8C8,0x18,false,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x14FD20,0x8023D8CC,0x20,false,EquationKind::linear,0,0x0,0,0x18},
    SceneStore{0x14FD20,0x8023D8D4,0x1C,false,EquationKind::linear,0,0x0,0,0x18},
    SceneStore{0x15A960,0x8023DFCC,0x18,false,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x15A960,0x8023DFD0,0x20,false,EquationKind::linear,0,0x0,0,0x18},
    SceneStore{0x15A960,0x8023DFD8,0x1C,false,EquationKind::linear,0,0x0,0,0x18},
    SceneStore{0x15A960,0x8023E230,0x18,false,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x15A960,0x8023E238,0x20,false,EquationKind::linear,0,0x0,0,0x18},
    SceneStore{0x15A960,0x8023E23C,0x1C,false,EquationKind::linear,0,0x0,0,0x18},
    SceneStore{0x15A960,0x8023F544,0x18,false,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x15A960,0x8023F548,0x20,false,EquationKind::linear,0,0x0,0,0x18},
    SceneStore{0x15A960,0x8023F54C,0x1C,false,EquationKind::linear,0,0x0,0,0x18},
    SceneStore{0x125180,0x8022ED94,0x58,true,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x125180,0x8022EF48,0x58,true,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x14FD20,0x8023DE88,0x58,true,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x121280,0x802312CC,0x58,true,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x150C60,0x8023F98C,0x58,true,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x15A960,0x8023EA98,0x58,true,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x15A960,0x8023E294,0x58,true,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x15E440,0x8023E138,0x0,true,EquationKind::linear,0,0x0,0,0x0},
    SceneStore{0x70A50,0x801E98A8,0x18,false,EquationKind::clamped,0,0x0,.1,0x0},
    SceneStore{0x70A50,0x801E98BC,0x18,false,EquationKind::clamped,0,0x0,.1,0x0},
    SceneStore{0x70A50,0x801E98CC,0x20,false,EquationKind::clamped,0,0x0,.1,0x18},
    SceneStore{0x70A50,0x801E98D0,0x1C,false,EquationKind::clamped,0,0x0,.1,0x18},
    SceneStore{0x70A50,0x801E9E90,0x18,false,EquationKind::clamped,0,0x0,.1,0x0},
    SceneStore{0x70A50,0x801E9EA4,0x18,false,EquationKind::clamped,0,0x0,.1,0x0},
    SceneStore{0x70A50,0x801E9EB4,0x20,false,EquationKind::clamped,0,0x0,.1,0x18},
    SceneStore{0x70A50,0x801E9EB8,0x1C,false,EquationKind::clamped,0,0x0,.1,0x18},
    SceneStore{0x121280,0x8022FCD0,0x18,false,EquationKind::approach,0,0x802315B0,1.8,0x0},
    SceneStore{0x121280,0x8022FCD4,0x20,false,EquationKind::approach,0,0x802315B0,1.8,0x18},
    SceneStore{0x121280,0x8022FCD8,0x1C,false,EquationKind::approach,0,0x802315B0,1.8,0x18},
};
std::optional<Equation> scene_equation(uint8_t* rdram,recomp_context* ctx,const Identity& owner,uint32_t site) {
    const auto actor=owner.actor,physical=owner.physical;
    const auto captured=[&](uint32_t address,float proposal)->const Equation* {
        const auto found=std::find_if(equations.rbegin(),equations.rend(),[&](const auto& e){
            return e.address==address&&std::bit_cast<int32_t>(e.after)==std::bit_cast<int32_t>(proposal);
        });
        return found==equations.rend()?nullptr:&*found;
    };
    if(owner.rom==0x70A50&&owner.kind==260&&site==0x801E596C) {
        const float proposal=std::bit_cast<float>(uint32_t(ctx->r6));
        Equation value{owner.model+0x78,EquationKind::linear,std::min(proposal+.1f,1.f),proposal};
        value.step=double(value.after)-value.before;return value;
    }
    if(owner.rom==0x126BA0&&owner.kind==525&&site==0x8022EB1C) {
        // Original EB1C takes exactly +/-3 degrees and E990 waits for exact
        // target equality. A button command can rebase at a fractional yaw;
        // cap the final directed step so it cannot oscillate across the stop.
        const int selected=MEM_H(0x34,int32_t(physical));
        if(selected>=0&&selected<4) {
            const float before=real(rdram,actor+0x10);
            const float target=float(MEM_H(selected*2,int32_t(0x80230048u)));
            const double step=std::remainder(double(ctx->f0.fl)-before,360.);
            const double remaining=std::remainder(double(target)-before,360.);
            if(remaining*step>=0&&std::abs(remaining)<=std::abs(step))ctx->f0.fl=target;
            Equation value{actor+0x10,EquationKind::angle,before,ctx->f0.fl};
            value.step=std::remainder(double(value.after)-before,360.);return value;
        }
    }
    if(owner.rom==0x1456C0&&site==0x802310F0) {
        const float before=angle(float(MEM_H(0x34,int32_t(physical))-1+90));
        Equation value{actor+0x10,EquationKind::angle,before,ctx->f0.fl};
        value.step=1;return value;
    }

    const auto phase_step=[&](uint32_t key,double current,double stride)->double {
        const auto identity_key=std::pair{actor,key};
        const auto found=scene_phase_samples.find(identity_key);
        double step=!scene_entry_latch&&current==stride?stride:0;
        if(found!=scene_phase_samples.end()&&same(found->second.owner,owner)&&found->second.epoch==epoch&&
           found->second.state==word(rdram,actor+0x94)&&scene_entry_latch) {
            const double delta=std::remainder(current-found->second.phase,360.);
            // A Source wait holds its phase. A reset/seek starts immediately,
            // while the reviewed stride alone receives continuation.
            step=delta==0?0:std::abs(delta-stride)<1e-8?stride:0;
        } else if(!scene_entry_latch&&current==0)step=0;
        scene_phase_samples.insert_or_assign(identity_key,ScenePhaseSample{owner,epoch,word(rdram,actor+0x94),current});
        return step;
    };
    const auto wave=[&](uint32_t address,EquationKind kind,float proposal,unsigned phase_offset,double stride,
                        double base,double amplitude,uint32_t radians,uint32_t key)->Equation {
        const double current=MEM_H(int32_t(phase_offset),int32_t(physical));
        Equation value{address,kind,0,proposal};value.step=phase_step(key,current,stride);
        value.phase=current-value.step;value.base=base;value.amplitude=amplitude;value.radians=rom_double(rdram,radians);
        value.before=value.evaluate(0);return value;
    };
    if(owner.rom==0x70A50&&site==0x801EAE30)
        return wave(actor+0x1C,EquationKind::sine,ctx->f0.fl,0x34,5,0,1,0x80211688,site);
    if(owner.rom==0x14D4C0&&site==0x8023E640)
        return wave(actor+0xA0,EquationKind::cosine,ctx->f2.fl,0x34,3,725,-475,0x8023FF18,site);
    if(owner.rom==0x14D4C0&&site==0x8023F588) {
        const double radius=real(rdram,physical+0x58);
        return wave(actor+0xA0,EquationKind::sine,ctx->f2.fl,0x34,4,
            float(MEM_H(0xC0,int32_t(physical)))+float(radius),radius,0x8023FF28,site);
    }
    if(owner.rom==0x14FD20&&site==0x8023D784)
        return wave(actor+0xA0,EquationKind::sine,ctx->f0.fl,0x34,3,real(rdram,physical+0x58),770,0x8023E5D0,site);
    if(owner.rom==0x14FD20&&site==0x8023DDDC)
        return wave(physical+0x58,EquationKind::absolute_sine,ctx->f0.fl,0x34,5,
            rom_double(rdram,0x8023E5F0),rom_double(rdram,0x8023E5E8),0x8023E5E0,site);
    if(owner.rom==0x159780&&site==0x8023E184) {
        auto value=wave(actor+0x9C,EquationKind::harmonic_accumulation,ctx->f2.fl,0x34,10,
            0,real(rdram,physical+0x58),0x8023E898,site);
        value.before=real(rdram,actor+0x9C);return value;
    }
    if(owner.rom==0x159780&&(site==0x8023E74C||site==0x8023E78C)) {
        const bool x=site==0x8023E74C;
        return wave(actor+(x?0x9C:0xA4),EquationKind::sine,ctx->f2.fl,0x34,2,
            real(rdram,physical+(x?0x60:0x64)),real(rdram,physical+(x?0x58:0x5C)),0x8023E8A8,site);
    }
    if(owner.rom==0x15D430&&(site==0x8023D848||site==0x8023D9F8||site==0x8023DD60||site==0x8023E314)) {
        const bool pause=site==0x8023DD60,third=site==0x8023E314;
        const uint32_t radians=site==0x8023D848?0x8023E6C0:site==0x8023D9F8?0x8023E6C8:pause?0x8023E6D0:0x8023E6D8;
        return wave(actor+0xA0,EquationKind::sine,ctx->f2.fl,0x34,pause||third?3:5,
            pause?-400:third?-650:-550,pause?350:third?400:150,radians,site);
    }
    if(owner.rom==0x150C60&&site==0x8023ED88) {
        const auto anchor=MEM_H(0xC0,int32_t(physical));const double factor=rom_double(rdram,0x8023FBA0);
        const float rounded_base=float(double(anchor)*factor+rom_double(rdram,0x8023FBA8));
        return wave(actor+0xA0,EquationKind::sine,ctx->f0.fl,0x34,3,double(rounded_base)-50,
            double(anchor+50)*factor,0x8023FBB0,site);
    }
    if(owner.rom==0x150C60&&(site==0x8023F520||site==0x8023F528||site==0x8023F52C||site==0x8023F55C)) {
        if(site==0x8023F520)return wave(actor+0x18,EquationKind::sine,ctx->f0.fl,0x36,5,.5,.5,0x8023FBB8,site);
        if(site==0x8023F55C)return wave(owner.model+0x78,EquationKind::cosine,
            std::bit_cast<float>(uint32_t(ctx->r6)),0x36,5,0,1,0x8023FBB8,site);
        if(const auto scale=captured(actor+0x18,ctx->f0.fl)){auto value=*scale;value.address=actor+(site==0x8023F528?0x20:0x1C);return value;}
    }
    if(owner.rom==0x150C60&&(site==0x8023F7D4||site==0x8023F878||site==0x8023F88C||site==0x8023F8BC)) {
        if(site==0x8023F7D4)return wave(actor+0x1C,EquationKind::sine,ctx->f0.fl,0x36,10,0,
            rom_double(rdram,0x8023FBC8),0x8023FBC0,site);
        if(site==0x8023F878)return wave(actor+0x18,EquationKind::sine,ctx->f0.fl,0x36,5,0,2,0x8023FBD0,site);
        if(site==0x8023F8BC)return wave(actor+0x1C,EquationKind::cosine,ctx->f2.fl,0x36,5,.5,1,0x8023FBD0,site);
        if(const auto scale=captured(actor+0x18,ctx->f2.fl)){auto value=*scale;value.address=actor+0x20;return value;}
    }
    if(owner.rom==0x15A960&&(site==0x8023E7B0||site==0x8023E7D4))
        return wave(site==0x8023E7B0?physical+0x58:actor+0xC,
            site==0x8023E7B0?EquationKind::cosine:EquationKind::cosine_angle,ctx->f0.fl,0x36,9,
            site==0x8023E7B0?0:315,site==0x8023E7B0?45:-45,0x8023FF58,site);
    if(owner.rom==0x15A960&&site==0x8023F518) {
        const double current=MEM_H(0x34,int32_t(physical));
        Equation value{physical+4,EquationKind::sine_angle,0,ctx->f4.fl};value.step=phase_step(site,current,25);
        value.phase=current-value.step;value.radians=rom_double(rdram,0x8023FF68);
        value.base=real(rdram,physical+0x58);value.amplitude=40;value.before=value.evaluate(0);return value;
    }
    if(owner.rom==0x15A960&&site==0x8023EA70) {
        const double decrease=rom_double(rdram,0x8023FF60),phase=MEM_H(0x38,int32_t(physical));
        Equation value{actor+0x20,EquationKind::alternating_offset,real(rdram,actor+0x20),ctx->f0.fl};
        value.phase=phase-1;value.base=1;value.amplitude=-(double(real(rdram,physical+0x58))+decrease);value.acceleration=decrease;
        return value;
    }
    if(owner.rom==0x15A960&&(site==0x8023DFF8||site==0x8023E264)) {
        const auto scale=std::find_if(equations.rbegin(),equations.rend(),[&](const auto& e){return e.address==actor+0x18;});
        if(scale!=equations.rend()) {Equation value{physical+0x30,EquationKind::affine,0,ctx->f0.fl};
            value.upstream=std::make_shared<Equation>(*scale);value.amplitude=2;value.before=value.evaluate(0);
            value.after=site==0x8023E264?ctx->f2.fl:ctx->f0.fl;return value;}
    }

    const auto table=std::find_if(scene_stores.begin(),scene_stores.end(),[&](const auto& s){return s.rom==owner.rom&&s.site==site;});
    if(table!=scene_stores.end()) {
        const auto& s=*table;const uint32_t address=(s.physical?physical:actor)+s.offset;
        const float proposal=s.operand==4?ctx->f4.fl:s.operand==2?ctx->f2.fl:ctx->f0.fl;
        if(s.copy_from) {
            const auto source=captured((s.kind==EquationKind::angle?physical:actor)+s.copy_from,proposal);
            if(!source)return std::nullopt;
            auto value=*source;value.address=address;return value;
        }
        Equation value{address,s.kind,real(rdram,address),proposal};
        if(s.kind==EquationKind::clamped) {
            const auto prior=std::find_if(equations.rbegin(),equations.rend(),[&](const auto& e){return e.address==address&&e.kind==EquationKind::clamped;});
            if(prior!=equations.rend()) {value=*prior;value.after=proposal;return value;}
            value.phase=value.before;value.step=double(proposal)-value.before;
            value.clamp_min=s.target;value.clamp_max=std::max(double(value.before),s.target);return value;
        }
        if(s.kind==EquationKind::approach) {value.base=s.target;value.retention=1-rom_double(rdram,s.coefficient);}
        else value.step=s.kind==EquationKind::angle?std::remainder(double(proposal)-value.before,360.):double(proposal)-value.before;
        return value;
    }
    if(owner.rom==0x1456C0&&(site==0x80231080||site==0x802310C0||site==0x80231168||site==0x80231974)) {
        // Fly resets XYZ to the spawn anchor before its radius800 orbit and
        // independent amplitude30 bob. Fuyuu's takeoff uses absolute400*sin.
        const bool x=site==0x80231080,z=site==0x802310C0,takeoff=site==0x80231974;
        const unsigned axis=x?0:z?2:1;
        Equation value{actor+0x9C+4*axis,z?EquationKind::cosine:EquationKind::sine,0,takeoff?ctx->f0.fl:ctx->f2.fl};
        value.step=x||z?1:takeoff?3:15;
        value.phase=double(MEM_H(x||z?0x34:0x36,int32_t(physical)))-value.step;
        value.radians=rom_double(rdram,takeoff?0x80234E38:0x802347C8);
        value.amplitude=x||z?800:takeoff?400:30;
        value.base=takeoff?0:double(MEM_H(int32_t(0xBE + 2*axis),int32_t(physical)));
        value.before=value.evaluate(0);return value;
    }
    if(owner.rom==0x1561E0&&site==0x802402D0) {
        Equation value{actor+0xA0,EquationKind::sine,0,ctx->f2.fl};
        value.phase=double(MEM_H(0x42,int32_t(physical)))-10;value.step=10;
        value.base=MEM_H(0xC0,int32_t(physical));value.amplitude=10;value.radians=rom_double(rdram,0x80240C60);
        value.before=value.evaluate(0);return value;
    }
    if(owner.rom==0x14D4C0&&(site==0x8023DA68||site==0x8023DC70)) {
        if(const auto scale=captured(actor+0x18,ctx->f2.fl)) {
            auto value=*scale;value.address=physical+0x30;return value;
        }
    }
    return std::nullopt;
}

extern "C" void dora3_native_enemy_equation(uint8_t* rdram,recomp_context* ctx,uint32_t site) {
#ifdef DORA3_CRASH_LOG
    dora3::crash::enemy.site=site;
#endif
    if(!executing)return;
    const auto actor=uint32_t(word(rdram,0x8021BDEC));const auto owner=identity(rdram,actor);if(!owner)return;

    if(owner->rom==0x70A50&&owner->kind==109&&(site==0x801EA7DC||site==0x801EA7F4||site==0x801EA820)) {
        const uint32_t player=uint32_t(word(rdram,0x8021D49C));
        if(!pointer(player,0xCC)||!word(rdram,player+0x94))return;
        const uint32_t physical=uint32_t(word(rdram,player+0x98)),model=uint32_t(word(rdram,player+0x74));
        if(!pointer(physical,0xDC)||!pointer(model,0x84))return;
        const auto lifetime=game_model_animation().lifetime_token(model);if(!lifetime)return;
        const uint32_t offset=site==0x801EA7DC?0x9C:site==0x801EA7F4?0xA4:0x10;
        const float before=real(rdram,player+offset),proposal=ctx->f0.fl;
        if(!std::isfinite(before)||!std::isfinite(proposal))return;
        foreign_steps.push_back({player,physical,model,offset,lifetime,before,proposal,before,double(proposal)-before});
        return;
    }
    const auto physical=owner->physical;Equation value{};
    if((owner->rom==0xF8A40&&owner->kind==410&&site==0x802271FC)||
       (owner->rom==0x111E70&&owner->kind==454&&site==0x80226904)) {
        const float before=real(rdram,physical+8);
        const double height=real(rdram,actor+0xA0);
        const double brake=rom_double(rdram,owner->rom==0xF8A40?0x802295A8u:0x80228C08u);
        // Both ROM state9/phase1 paths brake above Y300 and only leave at
        // Y400. Fractional/rebased heights can enter the brake one Source
        // period early: from Y300+epsilon, 20 with -1.85 peaks at 398.25.
        // Below Y300 the ROM then stops braking and falls forever. Retain
        // this period's positive speed only when the proposed braking
        // sequence cannot reach the existing transition. Never move the
        // actor, force a phase, or change health; the real Y400 branch wins.
        if(word(rdram,actor+0x94)==9&&MEM_H(0x34,int32_t(physical))==1&&
           real(rdram,physical+0xC)==270.f&&height>300&&height<400&&
           before>0&&brake>0&&ctx->f0.fl<before) {
            const double proposed=std::max(0.,double(ctx->f0.fl));
            const double steps=std::ceil(proposed/brake);
            const double peak=height+steps*proposed-brake*steps*(steps-1)/2;
            if(peak<400) {
#ifdef DORA3_CRASH_LOG
                std::fprintf(stderr,"BOSS ASCENT BRAKE HOLD actor=%08X kind=%d y=%.9g speed=%.9g proposed=%.9g peak=%.9g target=400\n",actor,owner->kind,height,double(before),double(ctx->f0.fl),peak);
#endif
                ctx->f0.fl=before;
            }
        }
        value={physical+8,EquationKind::linear,before,ctx->f0.fl};
        value.step=double(value.after)-before;
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});
        equations.push_back(value);return;
    }
    if(native_scene_object_kind(owner->kind)) {
        if(const auto captured=scene_equation(rdram,ctx,*owner,site)) {
            std::erase_if(equations,[&](const auto& old){return old.address==captured->address;});
            equations.push_back(*captured);
        }
        return;
    }
    if(owner->rom==0xDFC40&&(owner->kind==287||owner->kind==289)) {
        // Only these exact SH3E stores are arithmetic height operands. The
        // short stays Source-owned; Native evaluates its dependent Y geometry.
        switch(site) {
        case 0x8021DBF8:integer_motion_operands[physical+0x3E]={double(MEM_H(0x3E,int32_t(physical))),-10};return;
        case 0x8021DEB8:integer_motion_operands[physical+0x3E]={double(MEM_H(0x3E,int32_t(physical))),-10};return;
        case 0x8021E030:integer_motion_operands[physical+0x3E]={double(MEM_H(0x3E,int32_t(physical))),5};return;
        case 0x8021E1B8:integer_motion_operands[physical+0x3E]={double(MEM_H(0x3E,int32_t(physical))),5};return;
        case 0x8021E27C:integer_motion_operands[physical+0x3E]={double(MEM_H(0x3E,int32_t(physical))),5};return;
        case 0x8021E338:integer_motion_operands[physical+0x3E]={double(MEM_H(0x3E,int32_t(physical))),-15};return;
        case 0x8021E358:integer_motion_operands[physical+0x3E]={double(MEM_H(0x3E,int32_t(physical))),5};return;
        case 0x8021E6D4:integer_motion_operands[physical+0x3E]={double(MEM_H(0x3E,int32_t(physical))),-10};return;
        case 0x8021F724:integer_motion_operands[physical+0x3E]={double(MEM_H(0x3E,int32_t(physical))),-10};return;
        case 0x8021F9E0:integer_motion_operands[physical+0x3E]={double(MEM_H(0x3E,int32_t(physical))),-10};return;
        case 0x8021FB58:integer_motion_operands[physical+0x3E]={double(MEM_H(0x3E,int32_t(physical))),5};return;
        case 0x8021FCC8:integer_motion_operands[physical+0x3E]={double(MEM_H(0x3E,int32_t(physical))),5};return;
        case 0x8021FD8C:integer_motion_operands[physical+0x3E]={double(MEM_H(0x3E,int32_t(physical))),5};return;
        case 0x8021FE48:integer_motion_operands[physical+0x3E]={double(MEM_H(0x3E,int32_t(physical))),-15};return;
        case 0x8021FE68:integer_motion_operands[physical+0x3E]={double(MEM_H(0x3E,int32_t(physical))),5};return;
        case 0x8021DC68:integer_motion_operands.erase(physical+0x3E);return;
        case 0x8021DF28:integer_motion_operands.erase(physical+0x3E);return;
        case 0x8021E4FC:integer_motion_operands.erase(physical+0x3E);return;
        case 0x8021E744:integer_motion_operands.erase(physical+0x3E);return;
        case 0x8021F794:integer_motion_operands.erase(physical+0x3E);return;
        case 0x8021FA50:integer_motion_operands.erase(physical+0x3E);return;
        case 0x8022000C:integer_motion_operands.erase(physical+0x3E);return;
        default:break;
        }
        const bool height=site==0x8021DCA4||site==0x8021DF64||site==0x8021E538||
            site==0x8021E784||site==0x8021F7D4||site==0x8021FA8C||site==0x80220048;
        if(height) {
            const auto operand=integer_motion_operands.find(physical+0x3E);
            if(operand==integer_motion_operands.end())return;
            value={actor+0xA0,EquationKind::clamped,0,ctx->f0.fl};
            value.base=double(real(rdram,uint32_t(ctx->r29)+0x18))+80.;
            value.phase=operand->second.before;value.step=operand->second.step;
            value.before=value.evaluate(0);
            std::erase_if(equations,[&](const auto& old){return old.address==value.address;});
            equations.push_back(value);return;
        }
        if(site==0x8021E810||site==0x802200DC) {
            const auto scale=std::find_if(equations.begin(),equations.end(),[&](const auto& e){return e.address==actor+0x18;});
            if(scale==equations.end())return;
            value=*scale;value.kind=EquationKind::clamped;value.phase=value.before;
            value.base=0;value.clamp_min=double(ctx->f2.fl);value.clamp_max=std::max(double(value.before),value.clamp_min);
            value.after=ctx->f2.fl;
            std::erase_if(equations,[&](const auto& old){return old.address==value.address;});
            equations.push_back(value);return;
        }
    }
    if(owner->rom==0xEBAC0&&(site==0x8021DF08||site==0x8021E438)) {
        const uint32_t address=physical+(site==0x8021DF08?0x60u:0x58u);
        equation_operands[address]=real(rdram,address);return;
    }
    if(owner->rom==0xEBAC0&&(site==0x8021E044||site==0x8021E520)) {
        const uint32_t address=physical+(site==0x8021E044?0x60u:0x58u);
        const auto before=equation_operands.find(address);
        if(before==equation_operands.end())return;
        value={actor+0x14,EquationKind::angle,before->second,ctx->f0.fl};
        value.step=std::remainder(double(value.after)-value.before,360.);
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});
        equations.push_back(value);return;
    }
    if(owner->rom==0xEBAC0&&(site==0x8021E048||site==0x8021E524)) {
        const auto visual=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
            return e.address==actor+0x14&&std::bit_cast<int32_t>(e.after)==word(rdram,actor+0x14);
        });
        if(visual==equations.end())return;
        value=*visual;value.address=physical+(site==0x8021E048?0x60u:0x58u);
        value.after=ctx->f0.fl;
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});
        equations.push_back(value);return;
    }
    if(owner->rom==0x12FD90&&owner->kind==316&&
       (site==0x8022F8C0||site==0x8022FCE8)) {
        const auto pitch=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
            return e.address==physical+0xC&&std::bit_cast<int32_t>(e.after)==word(rdram,physical+0xC);
        });
        if(pitch==equations.end())return;
        value=*pitch;value.address=actor+0xC;value.after=ctx->f0.fl;
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});
        equations.push_back(value);return;
    }
    if(owner->rom==0x142670&&owner->kind>=308&&owner->kind<=310) {
        if(site==0x8022F9F0) {
            // Source mfc1 a2,f0 followed by SW in the E89CC call delay slot.
            // Capture the bits actually stored, before E89CC clobbers f0.
            value={physical+0x5C,EquationKind::linear,real(rdram,physical+0x5C),
                   std::bit_cast<float>(uint32_t(ctx->r6))};
            value.step=double(value.after)-value.before;
        } else if(site==0x8022EFB8||site==0x8022FA74||site==0x8022F528) {
            const bool death=site==0x8022F528;
            value={physical+0x58,EquationKind::sine,0,ctx->f0.fl};
            value.step=death?15:5;
            value.phase=double(MEM_H(0x34,int32_t(physical)))-value.step;
            value.radians=rom_double(rdram,death?0x802306E0:site==0x8022FA74?0x80230708:0x802306A8);
            value.amplitude=death?rom_double(rdram,0x802306E8):1;
            value.before=value.evaluate(0);
        } else if(site==0x8022EFC0||site==0x8022EFC4||site==0x8022EFC8||
                  site==0x8022FA7C||site==0x8022FA80||site==0x8022FA84||
                  site==0x8022F564||site==0x8022F568||site==0x8022F56C) {
            const auto wave=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==physical+0x58&&std::bit_cast<int32_t>(e.after)==word(rdram,physical+0x58);
            });
            if(wave==equations.end())return;
            const bool death=site==0x8022F564||site==0x8022F568||site==0x8022F56C;
            const unsigned offset=site==0x8022EFC0||site==0x8022FA7C||site==0x8022F564?0x20:
                site==0x8022EFC4||site==0x8022FA80||site==0x8022F568?0x1C:0x18;
            value=*wave;value.address=actor+offset;value.after=death?ctx->f2.fl:ctx->f0.fl;
            if(death){value.kind=EquationKind::absolute_sine;value.base=rom_double(rdram,0x802306F0);}
            value.before=value.evaluate(0);
        } else if(site==0x8023004C||site==0x80230068||site==0x80230084) {
            // The orbit consumes short34 BEFORE its +2 store at300C8. Its
            // published sample belongs to the interval ending at that phase.
            const unsigned axis=site==0x8023004C?0:site==0x80230068?1:2;
            // Original: X=cos(phase)*sin(heading), Y=sin(phase), Z=cos(phase)*cos(heading).
            value={actor+0x9C+4*axis,axis==1?EquationKind::sine:EquationKind::cosine,0,ctx->f0.fl};
            value.phase=double(MEM_H(0x34,int32_t(physical)))-2;value.step=2;
            value.radians=rom_double(rdram,0x80230718);value.amplitude=300;
            const double heading=double(real(rdram,physical+0x58))*value.radians;
            if(axis!=1)value.amplitude*=axis==0?std::sin(heading):std::cos(heading);
            value.base=double(MEM_H(int32_t(0xBE + 2*axis),int32_t(physical)));
            value.before=value.evaluate(0);
        } else if(site==0x8022F274||site==0x8022FB30||site==0x802300E0||site==0x8023021C) {
            const auto heading=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==physical+4&&std::bit_cast<int32_t>(e.after)==word(rdram,physical+4);
            });
            if(heading==equations.end())return;
            value=*heading;value.address=actor+0x10;value.after=ctx->f0.fl;
        } else goto derorin_other_store;
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});
        equations.push_back(value);return;
    }
derorin_other_store:
    if(owner->rom==0xD5D10&&owner->kind==320&&(site==0x80220F54||site==0x80220F6C)) {
        const auto heading=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
            return e.address==physical+4&&std::bit_cast<int32_t>(e.after)==word(rdram,physical+4);
        });
        if(site==0x80220F54) {
            const float before=real(rdram,physical+4);
            Equation fixed{physical+4,EquationKind::angle,before,before};
            value={physical+4,EquationKind::sine_drift_angle,0,ctx->f0.fl};
            value.upstream=std::make_shared<Equation>(heading==equations.end()?fixed:*heading);
            value.phase=double(MEM_H(0x38,int32_t(physical)))-20;value.step=20;
            value.radians=rom_double(rdram,0x802212C8);value.amplitude=15;
            value.before=value.evaluate(0);
        } else {
            if(heading==equations.end())return;
            value=*heading;value.address=actor+0x10;value.after=ctx->f0.fl;
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});
        equations.push_back(value);return;
    }
    if(owner->rom==0x11F1C0&&owner->kind==321&&site==0x8022F29C) {
        value={physical,EquationKind::sine,0,ctx->f2.fl};
        value.phase=double(MEM_H(0x34,int32_t(physical)))-10;value.step=10;
        value.radians=rom_double(rdram,0x8022F780);value.amplitude=10;value.base=15;
        value.before=value.evaluate(0);
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});
        equations.push_back(value);return;
    }
    if(owner->rom==0x12E230) {
        if(owner->kind==322&&(site==0x8022DB18||site==0x8022DB40||site==0x8022DB8C||site==0x8022DBE0)) {
            const bool outbound=site==0x8022DB18||site==0x8022DB40;
            const bool x=site==0x8022DB18||site==0x8022DB8C;
            const uint32_t address=actor+(x?0x9Cu:0xA4u);
            value={address,EquationKind::approach,real(rdram,address),site==0x8022DBE0?ctx->f6.fl:ctx->f2.fl};
            value.base=real(rdram,physical+(outbound?(x?0x58u:0x5Cu):(x?0x60u:0x64u)));
            value.retention=1-rom_double(rdram,outbound?0x8022F1E8:0x8022F1F0);
        } else if(owner->kind==322&&site==0x8022DBC8) {
            const auto x=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==actor+0x18&&std::bit_cast<int32_t>(e.after)==word(rdram,actor+0x18);
            });
            if(x==equations.end())return;
            value=*x;value.address=actor+0x1C;value.after=ctx->f0.fl;
        } else if(owner->kind==323&&site==0x8022E498) {
            const auto yaw=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==physical+4&&std::bit_cast<int32_t>(e.after)==word(rdram,physical+4);
            });
            if(yaw==equations.end())return;
            value=*yaw;value.address=actor+0x10;value.after=ctx->f0.fl;
        } else if(owner->kind==324&&(site==0x8022E758||site==0x8022E7AC)) {
            const bool extend=site==0x8022E758;
            value={actor+0x18,EquationKind::approach,real(rdram,actor+0x18),extend?ctx->f2.fl:ctx->f0.fl};
            value.base=extend?2:rom_double(rdram,0x8022F208);
            value.retention=1-rom_double(rdram,extend?0x8022F200:0x8022F210);
        } else if(owner->kind==324&&(site==0x8022E75C||site==0x8022E764||site==0x8022E7B0||site==0x8022E7B4)) {
            const auto x=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==actor+0x18&&std::bit_cast<int32_t>(e.after)==word(rdram,actor+0x18);
            });
            if(x==equations.end())return;
            value=*x;value.address=actor+(site==0x8022E75C||site==0x8022E7B0?0x20u:0x1Cu);
            value.after=site==0x8022E75C||site==0x8022E764?ctx->f2.fl:ctx->f0.fl;
        } else goto metabo_gabugon_other_store;
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});
        equations.push_back(value);return;
    }
metabo_gabugon_other_store:
    if(owner->rom==0x125180&&owner->kind==326&&
       (site==0x8022DA04||site==0x8022DA08||site==0x8022DA10)) {
        if(site==0x8022DA04) {
            value={actor+0x18,EquationKind::exponential,real(rdram,actor+0x18),ctx->f0.fl};
            value.retention=1-rom_double(rdram,0x8022F090);
        } else {
            const auto x=std::find_if(equations.begin(),equations.end(),[&](const auto& e){return e.address==actor+0x18;});
            if(x==equations.end()||std::bit_cast<int32_t>(x->after)!=word(rdram,actor+0x18))return;
            value=*x;value.address=actor+(site==0x8022DA08?0x20u:0x1Cu);value.after=ctx->f0.fl;
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
    if(owner->rom==0x12FD90&&owner->kind>=374&&owner->kind<=376) {
        if(site==0x802305F0||site==0x80230610) {
            integer_motion_operands[physical+0x34]={double(MEM_H(0x34,int32_t(physical))),site==0x802305F0?-5.:5.};return;
        }
        if(site==0x802306C8) {
            if(ctx->r2!=0)return; // Real ceiling selection is an instantaneous setter.
            const auto operand=integer_motion_operands.find(physical+0x34);if(operand==integer_motion_operands.end())return;
            value={actor+0xA0,EquationKind::clamped,0,ctx->f2.fl};
            value.base=real(rdram,uint32_t(ctx->r29)+0x18);value.phase=operand->second.before;value.step=operand->second.step;
            value.clamp_min=50;value.clamp_max=150;value.before=value.evaluate(0);
            std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
        }
    }
    if(owner->rom==0x1561E0&&(owner->kind==377||owner->kind==378)) {
        if(site==0x8023F5B4) {
            // Shared store also receives instantaneous home atan in phase1.
            // Admit only the actual phase0 shortest-angle recurrence.
            if(MEM_H(0x34,int32_t(physical))!=0)return;
            value={physical+4,EquationKind::wrapped_approach,real(rdram,physical+4),ctx->f0.fl};
            value.base=double(value.before)+std::remainder(double(MEM_H(0x36,int32_t(physical)))-value.before,360.);
            value.retention=1-rom_double(rdram,0x80240838);
        } else if(site==0x8023F310||site==0x8023F60C) {
            const auto yaw=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==physical+4&&std::bit_cast<int32_t>(e.after)==word(rdram,physical+4);
            });
            if(yaw==equations.end())return;
            value=*yaw;value.address=actor+0x10;value.after=ctx->f0.fl;
        } else goto small_family_other_store;
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
    if(owner->rom==0x126BA0&&owner->kind==381&&site==0x8022FCCC) {
        value={physical+0x58,EquationKind::linear,real(rdram,physical+0x58),std::bit_cast<float>(uint32_t(ctx->r6))};
        value.step=double(value.after)-value.before;
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
small_family_other_store:
    if(owner->rom==0x142670&&(owner->kind==318||owner->kind==319)) {
        const auto captured=[&](uint32_t address)->const Equation* {
            const auto item=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==address&&std::bit_cast<int32_t>(e.after)==word(rdram,address);
            });return item==equations.end()?nullptr:&*item;
        };
        if(site==0x8022DC44) {
            // This store is also fed by E8404's instantaneous reflection.
            if(!turn||turn->calls!=1||turn->actor!=actor||turn->physical!=physical||
               std::bit_cast<int32_t>(turn->after)!=std::bit_cast<int32_t>(ctx->f0.fl))return;
            value={physical+4,EquationKind::angle,turn->before,turn->after};
            value.step=std::remainder(double(value.after)-value.before,360.);
        } else if(site==0x8022E23C||site==0x8022E7CC) {
            const double gain=rom_double(rdram,site==0x8022E23C?0x80230670u:0x80230690u);
            value={physical+4,EquationKind::wrapped_approach,real(rdram,physical+4),site==0x8022E23C?ctx->f12.fl:ctx->f2.fl};
            value.base=double(value.before)+real(rdram,physical+(site==0x8022E23C?0x60u:0x5Cu))/gain;
            value.retention=1-gain;
        } else if(site==0x8022E250||site==0x8022E7F8||site==0x8022E81C||site==0x8022E850) {
            const bool pitch=site==0x8022E7F8||site==0x8022E81C;
            const uint32_t address=physical+(pitch?0xCu:4u);
            const float result=site==0x8022E7F8?ctx->f12.fl:ctx->f0.fl;
            if(const auto* input=captured(address))value=*input;
            else if(site==0x8022E250&&MEM_H(0x38,int32_t(physical))==0&&MEM_H(0x34,int32_t(physical))==0) {
                value={address,EquationKind::angle,real(rdram,address),result};
                value.step=std::remainder(double(result)-value.before,360.);
            } else return;
            value.after=(site==0x8022E81C||site==0x8022E850)&&result==360.f?0.f:result;
        } else if(site==0x8022E634) {
            value={physical+0xC,EquationKind::sine_angle,0,ctx->f0.fl};
            value.phase=MEM_H(0x36,int32_t(physical))-10;value.step=10;value.amplitude=40;
            value.radians=rom_double(rdram,0x80230688);value.before=value.evaluate(0);
        } else if(site==0x8022E310) {
            value={physical+8,EquationKind::forced_sine_velocity,0,ctx->f0.fl};
            value.phase=real(rdram,physical+0x58)-13.5;value.step=13.5;value.amplitude=50;
            value.radians=rom_double(rdram,0x80230678);value.base=ctx->f20.fl;
            value.initial_position=real(rdram,actor+0xA0);value.retention=0;value.before=value.evaluate(0);
        } else if(site==0x8022E3E0) {
            const auto* input=captured(physical+8);if(!input||input->kind!=EquationKind::forced_sine_velocity)return;
            value=*input;value.retention=1-rom_double(rdram,0x80230680);value.after=ctx->f0.fl;value.before=value.evaluate(0);
        } else if(site==0x8022DA3C||site==0x8022DCDC||site==0x8022E424||site==0x8022E880||site==0x8022E894) {
            const bool pitch=site==0x8022E894;const auto* input=captured(physical+(pitch?0xCu:4u));
            if(!input)return;
            value=*input;value.address=actor+(pitch?0xCu:0x10u);value.after=ctx->f0.fl;
        } else goto churatto_other_store;
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
churatto_other_store:
    if(owner->rom==0xF5200) {
        if(site==0x8021DDE8||site==0x8021E170||site==0x8021E580||site==0x8021E7A0) {
            const auto source=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==physical+4&&std::bit_cast<int32_t>(e.after)==word(rdram,physical+4);
            });
            if(source==equations.end())return;
            value=*source;value.address=actor+0x10;value.after=ctx->f0.fl;
        } else if(site==0x8021EF2C) {
            const int slot=attachment_parent_slot(rdram,*owner);
            const auto parent=slot>=30&&slot<dora3::distance::common_end?identity(rdram,0x804B1CD0u+unsigned(slot)*stride):std::nullopt;
            const auto& previous=periods[index(actor)];
            if(!parent||!previous||!same(previous->owner,*owner)||!model_reads.contains(parent->model)||
               !previous->model_reads.contains(parent->model)||ctx->f0.fl<=0)return;
            const auto now=model_reads.at(parent->model),old=previous->model_reads.at(parent->model);
            if(now.run!=old.run||now.distance<old.distance)return;
            const double advance=double(now.distance-old.distance);
            value={actor+0x1C,EquationKind::positive_phase,0,ctx->f20.fl};
            value.phase=double(ctx->f20.fl)*ctx->f0.fl-advance;value.step=advance;
            value.amplitude=1./ctx->f0.fl;value.before=value.evaluate(0);
        } else if(site==0x8021EF30||site==0x8021F0B8||site==0x8021F0BC||site==0x8021F0D0||
                  site==0x802207F4||site==0x802207FC) {
            const bool shield=site==0x8021F0B8||site==0x8021F0BC||site==0x8021F0D0;
            const uint32_t input=shield?physical+0x58:site==0x8021EF30?actor+0x1C:actor+0x18;
            const auto source=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==input&&std::bit_cast<int32_t>(e.after)==word(rdram,input);
            });
            if(source==equations.end())return;
            if(shield) {
                value={actor+(site==0x8021F0B8?0x1Cu:0x18u),EquationKind::affine,0,ctx->f0.fl};
                value.upstream=std::make_shared<const Equation>(*source);value.amplitude=1;
                value.base=rom_double(rdram,0x80220E90);value.before=value.evaluate(0);
            } else {
                value=*source;value.address=actor+(site==0x8021EF30?0x18u:site==0x802207F4?0x20u:0x1Cu);
                value.after=site==0x8021EF30?ctx->f20.fl:ctx->f2.fl;
            }
        } else if(site==0x8021F080) {
            value={physical+0x58,EquationKind::source_abs_sine,0,ctx->f0.fl};
            value.phase=double(MEM_H(0x34,int32_t(physical)))-12;value.step=12;
            value.radians=rom_double(rdram,0x80220E70);value.amplitude=rom_double(rdram,0x80220E78);
            value.base=rom_double(rdram,0x80220E88);value.initial_position=rom_double(rdram,0x80220E80);
            value.before=value.evaluate(0);
        } else if(site==0x802200C4||site==0x802207F0) {
            const bool vortex=site==0x802200C4;
            value={actor+(vortex?0x1Cu:0x18u),EquationKind::approach,real(rdram,actor+(vortex?0x1C:0x18)),
                vortex?ctx->f0.fl:ctx->f2.fl};
            const bool grow=vortex&&MEM_H(0x36,int32_t(physical))<8;
            value.base=vortex?(grow?6:0):real(rdram,physical+0x78);
            value.retention=1-rom_double(rdram,vortex?(grow?0x80220EC0:0x80220EC8):0x80220F00);
        } else if(owner->kind==406&&(site==0x8022058C||site==0x802205B4||site==0x802205C0||
                  site==0x802205D0||site==0x802205D4||site==0x802205E0||site==0x802205EC||site==0x8022060C)) {
            const int slot=attachment_parent_slot(rdram,*owner);
            const auto parent=slot>=30&&slot<dora3::distance::common_end?identity(rdram,0x804B1CD0u+unsigned(slot)*stride):std::nullopt;
            if(!parent)return;
            const auto& source_period=periods[index(parent->actor)];
            if(site==0x8022058C) {
                if(!source_period||!same(source_period->owner,*parent))return;
                const auto source=std::find_if(source_period->equations.begin(),source_period->equations.end(),[&](const auto& e){return e.address==parent->physical+0x78;});
                if(source==source_period->equations.end())return;
                value={physical+0x78,EquationKind::affine,0,ctx->f0.fl};
                value.upstream=std::make_shared<const Equation>(*source);value.amplitude=rom_double(rdram,0x80220EE0);
                if(std::bit_cast<int32_t>(float(source->after*value.amplitude))!=std::bit_cast<int32_t>(value.after))return;
                value.before=value.evaluate(0);
            } else if(site==0x8022060C) {
                if(!source_period||!same(source_period->owner,*parent)||
                   std::bit_cast<int32_t>(sample_period_motion(*source_period,1,1).first)!=std::bit_cast<int32_t>(ctx->f0.fl))return;
                value={actor+0xA0,EquationKind::historical_axis,sample_period_motion(*source_period,0,1).first,ctx->f0.fl};
                value.motion=immutable_motion(*source_period);value.motion_axis=1;
            } else {
                const unsigned axis=site==0x802205B4||site==0x802205C0||site==0x802205D0?0:1;
                auto history=vortex_trail_history.find(actor);
                const uint32_t base=physical+(axis==0?0x58:0x64);
                if(history==vortex_trail_history.end()||!same(history->second.owner,*owner)||history->second.epoch!=epoch||
                   ((site==0x802205B4||site==0x802205D4)&&
                    std::bit_cast<int32_t>(history->second.samples[axis][0].after)!=word(rdram,base))) {
                    VortexTrailHistory initial{*owner,epoch,{}};
                    for(unsigned a=0;a<2;++a)for(unsigned n=0;n<3;++n) {
                        const float sample=real(rdram,physical+(a==0?0x58:0x64)+4*n);
                        initial.samples[a][n]=Equation{0,EquationKind::linear,sample,sample};
                    }
                    history=vortex_trail_history.insert_or_assign(actor,initial).first;
                }
                if(site==0x802205C0||site==0x802205E0) {
                    for(unsigned n=0;n<2;++n)history->second.samples[axis][n]=history->second.samples[axis][n+1];
                    return;
                }
                if(site==0x802205D0||site==0x802205EC) {
                    Equation sample{0,EquationKind::linear,ctx->f0.fl,ctx->f0.fl};const unsigned movement_axis=axis==0?0:2;
                    if(source_period&&same(source_period->owner,*parent)&&
                       std::bit_cast<int32_t>(sample_period_motion(*source_period,1,movement_axis).first)==std::bit_cast<int32_t>(ctx->f0.fl)) {
                        sample.kind=EquationKind::historical_axis;sample.motion=immutable_motion(*source_period);
                        sample.motion_axis=movement_axis;sample.before=sample_period_motion(*source_period,0,movement_axis).first;
                    }
                    history->second.samples[axis][2]=sample;return;
                }
                value=history->second.samples[axis][0];value.address=actor+(axis==0?0x9Cu:0xA4u);
                value.after=axis==0?ctx->f0.fl:ctx->f2.fl;
            }
        } else goto boss1_other_store;
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
boss1_other_store:
    if(owner->rom==0xF8A40) {
        const auto captured=[&](uint32_t address)->std::optional<Equation> {
            const auto found=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==address&&std::bit_cast<int32_t>(e.after)==word(rdram,address);
            });
            return found==equations.end()?std::nullopt:std::optional<Equation>(*found);
        };
        const auto scalar=[&](uint32_t address) {
            if(auto source=captured(address))return *source;
            const auto literal=real(rdram,address);return Equation{address,EquationKind::linear,literal,literal};
        };
        switch(site) {
        case 0x80225A48: {
            // The head is earlier in the real pool than its body. Source
            // reads the body's preceding period, not its future updated pose.
            const int slot=MEM_H(0x80,int32_t(physical));
            const auto body=slot>=30&&slot<dora3::distance::common_end?identity(rdram,0x804B1CD0u+unsigned(slot)*stride):std::nullopt;
            if(!body||body->rom!=owner->rom||body->kind!=411)return;
            const auto& old=periods[index(body->actor)];
            if(!old||!same(old->owner,*body)||
                std::bit_cast<int32_t>(sample_period_motion(*old,1,1).first)!=std::bit_cast<int32_t>(ctx->f0.fl))return;
            value={actor+0xA0,EquationKind::historical_axis,sample_period_motion(*old,0,1).first,ctx->f0.fl};
            value.motion=immutable_motion(*old);value.motion_axis=1;break;
        }
        case 0x802255DC:case 0x80225B78:case 0x802266CC:
        case 0x8021E4AC:case 0x8021FAA4:case 0x8022843C: {
            const bool countdown=site==0x802255DC||site==0x80225B78||site==0x802266CC;
            const bool head_death=site==0x8022843C;
            const int count=MEM_H(head_death?0x34:0x36,int32_t(physical));
            value={actor+0xA0,EquationKind::alternating_offset,real(rdram,actor+0xA0),
                site==0x80225B78||site==0x802266CC||head_death?ctx->f2.fl:ctx->f0.fl};
            value.phase=count-(countdown?-1:1);
            value.amplitude=countdown?-(count+1):head_death?4:5;
            value.acceleration=countdown?1:0;
            value.base=site==0x802255DC?430:real(rdram,physical+0x58);
            if(!countdown&&!head_death)if(auto anchor=captured(physical+0x58))
                value.upstream=std::make_shared<const Equation>(*anchor);
            if(head_death)if(const auto& old=periods[index(actor)];old&&same(old->owner,*owner)) {
                const auto anchor=std::find_if(old->equations.begin(),old->equations.end(),[&](const auto& e){
                    return e.address==physical+0x58&&std::bit_cast<int32_t>(e.after)==word(rdram,physical+0x58);
                });
                if(anchor!=old->equations.end())value.upstream=std::make_shared<const Equation>(*anchor);
            }
            break;
        }
        case 0x80224540:case 0x80225428:case 0x8021DF6C:case 0x8021F40C:
        case 0x802276C4:case 0x80226D98: {
            const bool turn_step=site==0x80226D98;
            const uint32_t address=turn_step?physical+0x60:actor+0xA0;
            // DF6C/F40C store f2; f0 is only the landing threshold.
            value={address,EquationKind::approach,real(rdram,address),site==0x80224540||site==0x80225428||site==0x8021DF6C||site==0x8021F40C?ctx->f2.fl:ctx->f0.fl};
            value.base=site==0x80224540?320:site==0x80225428?480:site==0x8021DF6C?307:site==0x8021F40C?122:0;
            if(turn_step&&MEM_H(0x36,int32_t(physical))>=10)
                value.base=MEM_H(0x38,int32_t(physical))&1?15:-15;
            const uint32_t coefficient=site==0x80224540?0x80229490u:site==0x80225428?0x80229520u:
                site==0x8021DF6C?0x802292F0u:site==0x8021F40C?0x80229380u:
                site==0x802276C4?0x802295B0u:
                MEM_H(0x36,int32_t(physical))<10?0x80229578u:0x80229580u;
            value.retention=1-rom_double(rdram,coefficient);break;
        }
        case 0x80226F0C: {
            value={actor+0x10,EquationKind::angle,real(rdram,actor+0x10),ctx->f0.fl};
            if(const auto input=captured(physical+0x60);input&&input->kind==EquationKind::approach) {
                value.kind=EquationKind::approach_drift_angle;value.base=input->base;
                value.retention=input->retention;value.amplitude=double(input->before)-input->base;
            } else value.step=real(rdram,physical+0x60);
            break;
        }
        case 0x80227700: {
            value={physical,EquationKind::exponential,real(rdram,physical),ctx->f0.fl};
            value.retention=rom_double(rdram,0x802295B8);break;
        }
        case 0x802247AC:case 0x802247DC:case 0x8021DC04:case 0x8021DC34:
        case 0x8021ED7C:case 0x8021EDAC:case 0x80224FB4:case 0x80224FE4: {
            const bool sweep=site==0x80224FB4||site==0x80224FE4;
            const bool x=site==0x802247AC||site==0x8021DC04||site==0x8021ED7C||site==0x80224FB4;
            value={actor+(x?0x9Cu:0xA4u),EquationKind::radial_axis,0,ctx->f0.fl};
            value.upstream=std::make_shared<const Equation>(scalar(physical+(sweep?0x64:0x5C)));
            value.coefficient=std::make_shared<const Equation>(scalar(physical+(sweep?0x60:0x58)));
            value.radians=rom_double(rdram,site==0x8021DC04||site==0x8021DC34?0x802292D0u:
                site==0x8021ED7C||site==0x8021EDAC?0x80229350u:sweep?0x802294E8u:0x80229498u);
            value.motion_axis=x?0:2;value.before=value.evaluate(0);break;
        }
        case 0x8022501C:case 0x8022507C: {
            const auto z=captured(actor+0xA4);if(!z)return;value=*z;
            value.base=site==0x8022501C?500:-500;value.after=site==0x8022501C?ctx->f0.fl:ctx->f2.fl;
            value.before=value.evaluate(0);break;
        }
        case 0x80224EBC:case 0x80224F60:case 0x80224F74:case 0x80225090: {
            const bool height=site==0x80224EBC||site==0x80224F60;
            const auto theta=scalar(physical+0x64);
            const double direction=double(MEM_H(0x36,int32_t(physical)))*
                (MEM_H(0x3A,int32_t(physical))?-1:1);
            value={actor+(height?0xA0u:site==0x80224F74?0xCu:0x14u),
                height?EquationKind::harmonic_accumulation:EquationKind::harmonic_drift_angle,0,ctx->f0.fl};
            value.before=real(rdram,value.address);value.step=direction;
            // Y and pitch read X before the phase kick, roll reads it after.
            value.phase=double(theta.after)-(site==0x80225090?direction:2*direction);
            value.radians=rom_double(rdram,0x802294E8);
            const double coefficient=height?(site==0x80224EBC?-1:1)*rom_double(rdram,0x802294C8):
                site==0x80224F74?(MEM_H(0x3A,int32_t(physical))?1:-1)*rom_double(rdram,0x802294D0):
                (MEM_H(0x36,int32_t(physical))>0?-1:1)*rom_double(rdram,0x802294F0);
            value.amplitude=coefficient*real(rdram,physical+0x60);break;
        }
        case 0x80224E4C:case 0x80224EE4:case 0x80225048: {
            value={actor+0x10,EquationKind::angle,0,ctx->f0.fl};
            value.step=double(MEM_H(0x36,int32_t(physical)))*(MEM_H(0x3A,int32_t(physical))?-1:1);
            value.before=angle(float(double(value.after)-value.step));break;
        }
        case 0x8022690C:case 0x8022692C:case 0x8021EF78:case 0x80227A90: {
            const bool leg=site==0x8021EF78,final_spin=site==0x80227A90;
            value={final_spin||site==0x8022690C?actor+0x10:physical+4,EquationKind::sine_angle,0,
                site==0x8022690C?ctx->f2.fl:ctx->f0.fl};
            value.step=final_spin?5:30;
            value.phase=double(MEM_H(leg?0x3A:0x36,int32_t(physical)))-value.step;
            value.base=real(rdram,physical+(leg?0x64:final_spin?0x58:0x5C));
            value.amplitude=final_spin?45:25;
            value.radians=rom_double(rdram,leg?0x80229378u:final_spin?0x802295C0u:0x80229570u);
            value.before=value.evaluate(0);break;
        }
        case 0x8021F178: {
            const auto source=captured(physical+4);if(!source)return;
            value=*source;value.address=actor+0x10;value.after=ctx->f0.fl;break;
        }
        case 0x8022122C:case 0x80221230: {
            // The homing helper's actual bounded-turn operands also own the
            // model yaw; copy its captured physical-heading recurrence.
            if(!turn||turn->actor!=actor||turn->physical!=physical||turn->calls!=1)return;
            value={site==0x8022122C?physical+4:actor+0x10,EquationKind::angle,turn->before,ctx->f0.fl};
            value.step=std::remainder(double(turn->after)-turn->before,360.);break;
        }
        case 0x80221F04:case 0x80221F14: {
            value={actor+0xA0,EquationKind::sine,0,ctx->f0.fl};
            value.step=MEM_H(0x44,int32_t(physical))&1?10:-10;
            value.phase=double(MEM_H(0x34,int32_t(physical)))-value.step;
            value.radians=rom_double(rdram,0x802293F8);value.amplitude=10;
            value.base=site==0x80221F14?real(rdram,physical+0x60):0;
            value.before=value.evaluate(0);break;
        }
        case 0x80220264:case 0x80222798: {
            value={physical+0x58,EquationKind::linear,real(rdram,physical+0x58),std::bit_cast<float>(uint32_t(ctx->r6))};
            value.step=-rom_double(rdram,site==0x80220264?0x802293C8u:0x80229440u);break;
        }
        default: {
            if(site<0x80228908||site>0x80228964)goto boss2_other_store;
            const bool write=site>=0x80228938;
            const unsigned field=write?(site-0x80228938)/8:(site-0x80228908)/8;
            if(field>=6)return;
            const unsigned offset=field<3?0x9C+4*field:0xC+4*(field-3);
            auto history=boss2_ghost_history.find(actor);
            if(history==boss2_ghost_history.end()||!same(history->second.owner,*owner)||history->second.epoch!=epoch||
               (!write&&std::bit_cast<int32_t>(history->second.fields[field].after)!=word(rdram,physical+0x58+4*field))) {
                Boss2GhostHistory initial{*owner,epoch,{}};
                for(unsigned n=0;n<6;++n) {
                    const auto literal=real(rdram,physical+0x58+4*n);
                    initial.fields[n]=Equation{0,EquationKind::linear,literal,literal};
                }
                history=boss2_ghost_history.insert_or_assign(actor,initial).first;
            }
            if(!write) {value=history->second.fields[field];value.address=actor+offset;value.after=ctx->f0.fl;break;}
            const int slot=attachment_parent_slot(rdram,*owner);
            const auto parent=slot>=30&&slot<dora3::distance::common_end?identity(rdram,0x804B1CD0u+unsigned(slot)*stride):std::nullopt;
            Equation sample{0,EquationKind::linear,ctx->f0.fl,ctx->f0.fl};
            if(parent)if(const auto& period=periods[index(parent->actor)];period&&same(period->owner,*parent)) {
                if(field<3&&std::bit_cast<int32_t>(sample_period_motion(*period,1,field).first)==std::bit_cast<int32_t>(ctx->f0.fl)) {
                    sample.kind=EquationKind::historical_axis;sample.motion=immutable_motion(*period);
                    sample.motion_axis=field;sample.before=sample_period_motion(*period,0,field).first;
                } else if(field>=3) {
                    const auto input=std::find_if(period->equations.begin(),period->equations.end(),[&](const auto& e){return e.address==parent->actor+offset;});
                    if(input!=period->equations.end()&&std::bit_cast<int32_t>(input->after)==std::bit_cast<int32_t>(ctx->f0.fl))sample=*input;
                    else if(field==4&&period->proposal_yaw[0]==ctx->f0.fl) {
                        sample.kind=EquationKind::angle;sample.before=period->initial_yaw[0];sample.step=period->turn[0];
                    }
                }
            }
            history->second.fields[field]=sample;return;
        }
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
boss2_other_store:
    if(owner->rom==0x104AE0) {
        const auto captured=[&](uint32_t address)->std::optional<Equation> {
            const auto input=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==address&&std::bit_cast<int32_t>(e.after)==word(rdram,address);
            });
            return input==equations.end()?std::nullopt:std::optional<Equation>(*input);
        };
        switch(site) {
        case 0x8021FD10:case 0x8021D928:case 0x8021D934: {
            const auto source=captured(physical+(site==0x8021D934?0xCu:4u));if(!source)return;
            value=*source;value.address=actor+(site==0x8021D934?0xCu:0x10u);value.after=ctx->f0.fl;break;
        }
        case 0x8021DA40:case 0x8021E67C: {
            value={physical+0x58,EquationKind::linear,real(rdram,physical+0x58),std::bit_cast<float>(uint32_t(ctx->r6))};
            value.step=-rom_double(rdram,site==0x8021DA40?0x80221250u:0x80221290u);break;
        }
        case 0x8021DCE0: {
            value={physical+4,EquationKind::sine_angle,0,ctx->f0.fl};
            value.step=MEM_H(0x36,int32_t(physical));value.phase=MEM_H(0x34,int32_t(physical))-value.step;
            value.base=real(rdram,physical+0x58);value.amplitude=40;value.radians=rom_double(rdram,0x80221258);
            value.before=value.evaluate(0);break;
        }
        case 0x8021DF20:case 0x8021E020:case 0x8021E354: {
            const bool rising=site==0x8021DF20,plate=site==0x8021E354;
            value={actor+0xA0,plate?EquationKind::sine_ramp:EquationKind::sine,0,plate?ctx->f2.fl:ctx->f0.fl};
            value.step=plate?30:rising?9:5;value.phase=MEM_H(0x34,int32_t(physical))-value.step;
            value.base=plate?-50:rising?-250:0;value.amplitude=plate?real(rdram,physical+0x58)+1:rising?250:-250;
            value.acceleration=plate?-1:0;
            value.radians=rom_double(rdram,plate?0x80221278u:rising?0x80221268u:0x80221270u);
            value.before=value.evaluate(0);break;
        }
        case 0x8021E418:
            // This Source subtracts2 directly instead of calling E810C.
            // Its actual velocity kick still owns the falling displacement.
            gravity=Gravity{actor,physical,real(rdram,physical+0x14),ctx->f0.fl,1,0};return;
        default:goto boss3_other_store;
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
boss3_other_store:
    if(owner->rom==0x108730) {
        const auto captured=[&](uint32_t address)->std::optional<Equation> {
            const auto input=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==address&&std::bit_cast<int32_t>(e.after)==word(rdram,address);
            });
            return input==equations.end()?std::nullopt:std::optional<Equation>(*input);
        };
        const auto sine=[&](uint32_t address,float proposal,unsigned counter,double step,
                            double base,double amplitude,uint32_t constant,EquationKind kind=EquationKind::sine) {
            Equation wave{address,kind,0,proposal};wave.step=step;
            wave.phase=MEM_H(int32_t(counter),int32_t(physical))-step;
            wave.base=base;wave.amplitude=amplitude;wave.radians=rom_double(rdram,constant);
            wave.before=wave.evaluate(0);return wave;
        };
        switch(site) {
        case 0x8021FB70:
            value=sine(physical+4,ctx->f0.fl,0x34,10,real(rdram,physical+0x58),45,0x80222978,EquationKind::sine_angle);break;
        case 0x8021FB78:case 0x8021E168:case 0x8021E790:case 0x8021F2DC: {
            const auto source=captured(physical+4);if(!source)return;
            value=*source;value.address=actor+0x10;value.after=ctx->f0.fl;break;
        }
        case 0x8021FDD4:
            gravity=Gravity{actor,physical,real(rdram,physical+0x14),ctx->f0.fl,1,0};return;
        case 0x802201C8:case 0x80220200:case 0x80220824:case 0x8022085C:
        case 0x80220FC0:case 0x80220FF8:case 0x80221564:case 0x8022159C: {
            const bool x=site==0x802201C8||site==0x80220824||site==0x80220FC0||site==0x80221564;
            const int state=word(rdram,actor+0x94);
            const double step=MEM_H(state==7?0x3E:0x38,int32_t(physical));
            const uint32_t constant=state==5?0x80222980u:state==6?0x80222988u:state==7?0x80222990u:0x80222998u;
            value=sine(actor+(x?0x9Cu:0xA4u),ctx->f0.fl,0x36,step,0,600,constant,
                       x?EquationKind::sine:EquationKind::cosine);break;
        }
        case 0x802205F8:case 0x80220D78:case 0x802213E8:case 0x80221AD4: {
            // Attack logic may reverse the direction after X/Z were written.
            // Yaw belongs to that earlier captured phase, not the new command.
            const auto source=captured(actor+0x9C);if(!source)return;
            value={actor+0x10,EquationKind::angle,angle(float(source->phase+180)),ctx->f0.fl};
            value.step=source->step;break;
        }
        case 0x80222190:
            value={actor+0xA0,EquationKind::alternating_offset,real(rdram,actor+0xA0),ctx->f2.fl};
            value.base=real(rdram,physical+0x58);value.amplitude=5;
            value.phase=MEM_H(0x34,int32_t(physical))-1;break;
        case 0x8021D934:case 0x8021D97C:
            value=sine(actor+(site==0x8021D934?0x18u:0x1Cu),ctx->f0.fl,0x36,20,
                rom_double(rdram,0x80222898),rom_double(rdram,site==0x8021D934?0x80222890u:0x802228A0u),0x80222888);break;
        case 0x8021D93C: {
            const auto source=captured(actor+0x18);if(!source)return;
            value=*source;value.address=actor+0x20;value.after=ctx->f0.fl;break;
        }
        case 0x8021DB18:case 0x8021DB1C:case 0x8021DB28:
            value={actor+(site==0x8021DB18?0x18u:site==0x8021DB1C?0x20u:0x1Cu),EquationKind::approach,
                real(rdram,actor+(site==0x8021DB28?0x1Cu:0x18u)),site==0x8021DB28?ctx->f4.fl:ctx->f2.fl};
            // The second store copies the already updated X operand.
            if(site==0x8021DB1C) {
                const auto source=captured(actor+0x18);if(!source)return;
                value=*source;value.address=actor+0x20;value.after=ctx->f2.fl;
            } else {value.base=site==0x8021DB28?3:0;value.retention=1-rom_double(rdram,0x802228B0);}
            break;
        case 0x8021DFC8: {
            // Source chooses the rising (<40) or falling half-frame branch.
            // That branch change is a discrete command. Within it, the rate
            // is the actual CC advance (zero means3 in the common pool), not
            // a difference between two already computed speeds or poses.
            const double half_frame=real(rdram,physical+0x58);
            const float stored_rate=real(rdram,physical+0xCC);
            const double rate=(stored_rate==0?3.:double(stored_rate))/2;
            value={physical,EquationKind::linear,0,ctx->f0.fl};
            const auto profile=[&](double frame){return rom_double(rdram,0x802228B8)*(frame<40?frame-24:48-frame);};
            value.before=float(profile(half_frame-rate));value.step=double(value.after)-value.before;break;
        }
        case 0x8021E184:case 0x8021E188: {
            const auto source=captured(physical+0xC);if(!source)return;
            if(site==0x8021E184) {
                value={physical+0xC,EquationKind::folded_pitch,0,ctx->f0.fl};
                value.upstream=std::make_shared<const Equation>(*source);value.before=value.evaluate(0);
            } else {value=*source;value.address=actor+0xC;value.after=ctx->f0.fl;}
            break;
        }
        case 0x8021EBCC:case 0x8021EBD4:case 0x8021EBD8:
        case 0x8021ED24:case 0x8021ED2C:case 0x8021ED30: {
            const bool closing=site==0x8021ED24||site==0x8021ED2C||site==0x8021ED30;
            const unsigned offset=site==0x8021EBCC||site==0x8021ED24?0x18u:
                site==0x8021EBD4||site==0x8021ED2C?0x20u:0x1Cu;
            value=sine(actor+offset,ctx->f0.fl,0x36,6,0,closing?2:1,closing?0x802228F8u:0x802228F0u);break;
        }
        case 0x8021F1D0:
            value=sine(actor+0xA0,ctx->f0.fl,0x36,5,-100,300,0x80222908);break;
        case 0x8021F284:
            value={actor+0xA0,EquationKind::approach,real(rdram,actor+0xA0),ctx->f2.fl};
            value.base=50;value.retention=1-rom_double(rdram,0x80222910);break;
        default:goto boss4_other_store;
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
boss4_other_store:
    if(owner->rom==0x10DA30) {
        const auto captured=[&](uint32_t address)->std::optional<Equation> {
            const auto found=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==address&&std::bit_cast<int32_t>(e.after)==word(rdram,address);
            });
            return found==equations.end()?std::nullopt:std::optional<Equation>(*found);
        };
        const auto save=[&](const Equation& equation) {
            std::erase_if(equations,[&](const auto& old){return old.address==equation.address;});equations.push_back(equation);
        };
        const auto split=[&](uint32_t address,float proposal,bool heading) {
            Equation sample{address,EquationKind::split_sine,0,proposal};
            sample.phase=MEM_H(0x3C,int32_t(physical))-4;sample.step=4;
            sample.radians=rom_double(rdram,owner->kind==452?0x802219E8u:0x80221AD0u);
            sample.inner_amplitude=90;sample.outer_radians=90;
            sample.base=heading?0:30;sample.amplitude=heading?90:-30;
            sample.initial_position=heading?90:0;sample.acceleration=heading?90:30;
            sample.before=sample.evaluate(0);return sample;
        };
        switch(site) {
        case 0x8021FA20:case 0x8021FA78:case 0x80220704: {
            const uint32_t offset=site==0x80220704?0x36:0x38;
            const double before=MEM_H(int32_t(offset),int32_t(physical));
            integer_motion_operands[physical+offset]={before,double(int16_t(ctx->r2))-before};return;
        }
        case 0x8021F1D8:case 0x8021F4C0:case 0x8021F714:case 0x8021FCF0:
        case 0x8021FEC0:case 0x80220B1C:case 0x80220D64:case 0x80220D70: {
            const bool lower=site==0x8021F4C0||site==0x8021FEC0||site==0x80220D64||site==0x80220D70;
            value={actor+0xA0,lower?EquationKind::cosine:EquationKind::sine,0,ctx->f0.fl};
            value.step=site==0x8021F1D8?6:site==0x8021F4C0?7:site==0x8021F714||site==0x80220B1C?9:site==0x8021FCF0||site==0x8021FEC0?18:7;
            value.phase=MEM_H(0x36,int32_t(physical))-value.step;
            value.amplitude=site==0x8021FCF0||site==0x8021FEC0||site==0x80220B1C?50:site==0x80220D64||site==0x80220D70?40:200;
            value.radians=rom_double(rdram,site==0x8021F1D8?0x80221A40u:site==0x8021F4C0?0x80221A48u:
                site==0x8021F714?0x80221A50u:site==0x8021FCF0?0x80221A70u:site==0x8021FEC0?0x80221A78u:site==0x80220B1C?0x80221AC0u:0x80221AC8u);
            value.base=site==0x80220D70?10:0;value.before=value.evaluate(0);break;
        }
        case 0x8021F800:case 0x8021FB78:case 0x80220124:case 0x8021D990: {
            const auto source=captured(physical+4);if(!source)return;
            value=*source;value.address=actor+0x10;value.after=ctx->f0.fl;break;
        }
        case 0x80220474: {
            // Shield-state6 copies the PREVIOUS physical heading before
            // E88C4 computes this callback's new heading at2091C. Keep that
            // Source ordering by borrowing the completed preceding path.
            const auto& old=periods[index(actor)];
            if(!old||!same(old->owner,*owner))return;
            const auto source=std::find_if(old->equations.begin(),old->equations.end(),[&](const auto& e){
                return e.address==physical+4&&std::bit_cast<uint32_t>(e.after)==ctx->f2.u32l;
            });
            if(source!=old->equations.end())value=*source;
            else {
                if(std::bit_cast<uint32_t>(old->proposal_yaw[1])!=ctx->f2.u32l)return;
                value={actor+0x10,EquationKind::angle,old->initial_yaw[1],ctx->f2.fl};value.step=old->turn[1];
            }
            if(std::bit_cast<int32_t>(value.before)!=word(rdram,actor+0x10))return;
            value.address=actor+0x10;value.after=ctx->f2.fl;break;
        }
        case 0x8022046C: {
            // This body's speed consumes the SHIELD's actual clip frame /3.
            const auto part=identity(rdram,uint32_t(ctx->r16));if(!part||part->kind!=451)return;
            const double frame=real(rdram,part->model+0x64)/3;
            const float command=real(rdram,part->physical+0xCC);
            const double rate=(command==0?3.:double(command))/3;
            value={physical,EquationKind::linear,float(std::abs(frame-rate-7)*rom_double(rdram,0x80221AA0)),ctx->f0.fl};
            value.step=double(value.after)-value.before;break;
        }
        case 0x80220680:
            value={physical+0x60,EquationKind::approach,real(rdram,physical+0x60),ctx->f2.fl};
            value.base=real(rdram,physical+0x68);value.retention=1-rom_double(rdram,0x80221AB0);break;
        case 0x802206A4:case 0x802206E0: {
            const auto phase=captured(physical+0x60);if(!phase)return;
            value={actor+(site==0x802206A4?0x9Cu:0xA4u),site==0x802206A4?EquationKind::phase_sine:EquationKind::phase_cosine,0,ctx->f2.fl};
            value.upstream=std::make_shared<const Equation>(*phase);value.amplitude=real(rdram,physical+0x64);
            value.base=real(rdram,physical+(site==0x802206A4?0x58u:0x5Cu));value.radians=rom_double(rdram,0x80221AB8);
            value.before=value.evaluate(0);break;
        }
        case 0x802210C8:case 0x8022118C:case 0x802211A8:
        case 0x8021E73C:case 0x8021E800:case 0x8021E81C:
            value=split(physical+0x5C,ctx->f0.fl,true);break;
        case 0x8022113C:case 0x80221238:case 0x8021E7B0:case 0x8021E8AC:
            value=split(actor+0xC,site==0x8022113C||site==0x8021E7B0?ctx->f2.fl:ctx->f0.fl,false);break;
        case 0x80221278:case 0x802212C0:case 0x8021E8EC:case 0x8021E934: {
            const auto phase=captured(physical+0x5C);if(!phase)return;
            const bool x=site==0x80221278||site==0x8021E8EC;
            value={actor+(x?0x9Cu:0xA4u),x?EquationKind::phase_sine:EquationKind::phase_cosine,0,ctx->f0.fl};
            value.upstream=std::make_shared<const Equation>(*phase);value.radians=rom_double(rdram,owner->kind==452?0x802219F8u:0x80221AE0u);
            value.amplitude=x?real(rdram,physical+0x58):700.*MEM_H(0x3E,int32_t(physical));value.before=value.evaluate(0);break;
        }
        case 0x8022151C:case 0x8021E980: {
            const auto x=captured(actor+0x9C),z=captured(actor+0xA4);
            if(x&&z) {
                Equation heading{physical+4,EquationKind::path_heading,0,ctx->f0.fl};
                heading.upstream=std::make_shared<const Equation>(*x);heading.coefficient=std::make_shared<const Equation>(*z);
                heading.base=MEM_H(0x3C,int32_t(physical))<=90?real(rdram,physical+0x58):0;
                heading.amplitude=MEM_H(0x3C,int32_t(physical))<=90?0:-600.*MEM_H(0x3E,int32_t(physical));
                heading.radians=rom_double(rdram,0x80221AE0);heading.before=heading.evaluate(0);save(heading);
            }
            const auto source=captured(physical+4);if(!source)return;
            value=*source;value.address=actor+0x10;value.after=ctx->f0.fl;break;
        }
        case 0x8021DBC4:case 0x8021DBC8:case 0x8021DBCC:
        case 0x8021DCAC:case 0x8021DCB0:case 0x8021DCB8:
        case 0x8021E248:case 0x8021E24C:case 0x8021E254:
        case 0x8021E570:case 0x8021E574:case 0x8021E57C: {
            const bool laser_up=site==0x8021DBC4||site==0x8021DBC8||site==0x8021DBCC;
            const bool laser_down=site==0x8021DCAC||site==0x8021DCB0||site==0x8021DCB8;
            const bool shield_up=site==0x8021E248||site==0x8021E24C||site==0x8021E254;
            const bool first=site==0x8021DBC4||site==0x8021DCAC||site==0x8021E248||site==0x8021E570;
            const bool z=site==0x8021DBC8||site==0x8021DCB0||site==0x8021E24C||site==0x8021E574;
            if(first) {
                value={actor+0x18,EquationKind::approach,real(rdram,actor+0x18),laser_up||laser_down||shield_up?ctx->f2.fl:ctx->f0.fl};
                value.base=laser_up?1:laser_down?0:shield_up?rom_double(rdram,0x802219B8):rom_double(rdram,0x802219E0);
                value.retention=1-rom_double(rdram,laser_up?0x80221998u:laser_down?0x802219B0u:shield_up?0x802219C0u:0x802219D0u);
            } else {
                const auto source=captured(actor+0x18);if(!source)return;value=*source;value.address=actor+(z?0x20u:0x1Cu);
                value.after=laser_up||laser_down||shield_up?ctx->f2.fl:ctx->f0.fl;
            }
            break;
        }
        case 0x8021DC08:case 0x8021DC78:case 0x8021EB18:
            value={physical+0x58,EquationKind::linear,real(rdram,physical+0x58),std::bit_cast<float>(uint32_t(ctx->r6))};
            value.step=rom_double(rdram,site==0x8021DC08?0x80221998u:site==0x8021DC78?0x802219A8u:0x80221A10u)*(site==0x8021DC08?1:-1);break;
        case 0x8021EAC4:
            value={actor+0x18,EquationKind::approach,real(rdram,actor+0x18),ctx->f2.fl};
            value.base=7;value.retention=1-rom_double(rdram,0x80221A00);break;
        case 0x8021EB7C:case 0x8021EB80: {
            const auto source=captured(actor+0x18);if(!source)return;
            value=*source;value.address=actor+(site==0x8021EB7C?0x20u:0x1Cu);value.after=ctx->f0.fl;break;
        }
        default:goto boss5_other_store;
        }
        save(value);return;
    }
boss5_other_store:
    if(owner->rom==0x111E70) {
        const auto captured=[&](uint32_t address)->std::optional<Equation> {
            const auto found=std::find_if(equations.begin(),equations.end(),[&](const auto& e){return e.address==address&&std::bit_cast<int32_t>(e.after)==word(rdram,address);});
            return found==equations.end()?std::nullopt:std::optional<Equation>(*found);
        };
        const auto scalar=[&](uint32_t address) {
            if(auto e=captured(address))return *e;
            const float literal=real(rdram,address);return Equation{address,EquationKind::linear,literal,literal};
        };
        const auto prior_position=[&](unsigned axis) {
            const uint32_t address=actor+0x9C+4*axis;
            if(auto e=captured(address))return *e;
            const auto& old=periods[index(actor)];const float literal=real(rdram,address);
            Equation sample{address,EquationKind::linear,literal,literal};
            if(old&&same(old->owner,*owner)&&std::bit_cast<int32_t>(sample_period_motion(*old,1,axis).first)==word(rdram,address)) {
                sample.kind=EquationKind::historical_axis;sample.motion=immutable_motion(*old);sample.motion_axis=axis;sample.before=sample_period_motion(*old,0,axis).first;
            }
            return sample;
        };
        const auto borrow_heading=[&](uint32_t address,float proposal) {
            auto e=scalar(physical+4);e.address=address;e.after=proposal;return e;
        };
        switch(site) {
        case 0x80223C00:case 0x80223DC8:case 0x80226014:case 0x8022664C:
        case 0x80226C44:case 0x8021DB54:case 0x8021EC00:case 0x8021FFA0: {
            if(site==0x8021FFA0) {value=scalar(physical+0xC);value.address=actor+0xC;value.after=ctx->f0.fl;}
            else value=borrow_heading(actor+0x10,ctx->f0.fl);
            break;
        }
        case 0x802241D8:case 0x80224974:case 0x80226DCC:case 0x8021DE90:case 0x8021EF14: {
            value={actor+0xA0,EquationKind::approach,real(rdram,actor+0xA0),site==0x80226DCC?ctx->f0.fl:ctx->f2.fl};
            value.base=site==0x802241D8?320:site==0x80224974?480:site==0x80226DCC?0:site==0x8021DE90?307:122;
            value.retention=1-rom_double(rdram,site==0x802241D8?0x80228B40u:site==0x80224974?0x80228B70u:site==0x80226DCC?0x80228C10u:site==0x8021DE90?0x802289E8u:0x80228A30u);break;
        }
        case 0x802244CC:case 0x8022450C:
            value={physical+0x58,EquationKind::linear,real(rdram,physical+0x58),ctx->f0.fl};value.step=site==0x802244CC?-3:3;break;
        case 0x80224548:
            // SWC1 stores f0; f20 already holds the radians double here.
            value={physical+0x5C,EquationKind::linear,real(rdram,physical+0x5C),ctx->f0.fl};value.step=3;break;
        case 0x80224564:case 0x8022459C: {
            value={actor+(site==0x80224564?0x9Cu:0xA4u),EquationKind::radial_axis,0,ctx->f0.fl};
            value.motion_axis=site==0x80224564?0:2;value.radians=rom_double(rdram,0x80228B48);
            value.upstream=std::make_shared<const Equation>(scalar(physical+0x5C));value.coefficient=std::make_shared<const Equation>(scalar(physical+0x58));value.before=value.evaluate(0);break;
        }
        case 0x802245B8: {
            value={actor+0x10,EquationKind::affine,0,ctx->f0.fl};value.upstream=std::make_shared<const Equation>(scalar(physical+0x5C));value.amplitude=1;value.base=90;value.before=value.evaluate(0);break;
        }
        case 0x80225000: {
            // The reattached body copied this head on the preceding pass.
            // An equal Y assignment is a no-op, not a replay of that body's
            // preceding descent. Replaying it jumps up each period before
            // adding the head's own downward velocity again.
            if(word(rdram,actor+0xA0)==std::bit_cast<int32_t>(ctx->f0.fl))return;
            // Source head precedes its detached body in the pool; consume its
            // completed preceding period, never the body's future callback.
            const int slot=MEM_H(0x80,int32_t(physical));const auto part=slot>=30&&slot<dora3::distance::common_end?identity(rdram,0x804B1CD0u+unsigned(slot)*stride):std::nullopt;
            if(!part||part->rom!=owner->rom||part->kind!=455)return;
            const auto& old=periods[index(part->actor)];
            if(!old||!same(old->owner,*part)||std::bit_cast<int32_t>(sample_period_motion(*old,1,1).first)!=std::bit_cast<int32_t>(ctx->f0.fl))return;
            value={actor+0xA0,EquationKind::historical_axis,sample_period_motion(*old,0,1).first,ctx->f0.fl};value.motion=immutable_motion(*old);value.motion_axis=1;break;
        }
        case 0x80223774:case 0x80223788:case 0x80224B28:case 0x80225130:case 0x80225D78:
        case 0x802275A0:case 0x802275F4:case 0x80227B40:case 0x8021E3D0:case 0x8021F5AC: {
            const bool countdown=site==0x80224B28||site==0x80225130||site==0x80225D78;
            const bool dying=site==0x80227B40,hit=site==0x802275A0||site==0x802275F4,charge=site==0x80223774||site==0x80223788;
            const int count=MEM_H(dying||hit?0x34:0x36,int32_t(physical));
            const float proposal=site==0x80225130||site==0x80225D78||dying?ctx->f2.fl:ctx->f0.fl;
            value={actor+0xA0,EquationKind::alternating_offset,real(rdram,actor+0xA0),proposal};
            value.phase=count-(countdown?-1:1);value.amplitude=countdown?-(count+1):charge?-5:4;
            if(site==0x8021E3D0||site==0x8021F5AC)value.amplitude=5;
            value.acceleration=countdown?1:0;
            value.base=charge?330:site==0x80224B28?430:real(rdram,physical+(hit?0x5Cu:0x58u));
            if(!countdown&&!hit&&!charge)if(auto e=captured(physical+0x58)) {
                value.upstream=std::make_shared<const Equation>(*e);
                // The parts reset36 every five Source periods for effects.
                // On 4->0 its parity stays even, so continue just the real
                // baseline drift instead of introducing an extra shake cycle.
                if(site==0x8021E3D0||site==0x8021F5AC) {
                    const auto& old=periods[index(actor)];
                    if(old&&same(old->owner,*owner)&&old->command.size()>4&&
                       (int64_t(old->command[4])&1)==(count&1)) {
                        value.kind=EquationKind::affine;value.amplitude=1;
                        value.base=double(proposal)-e->after;
                    }
                }
            }
            break;
        }
        case 0x80225FA4:case 0x80225FB0:case 0x80225FB8:case 0x80225FD8:case 0x80227198: {
            const bool final=site==0x80227198;
            value={site==0x80225FA4||site==0x80225FD8?physical+4:actor+0x10,EquationKind::sine_angle,0,site==0x80225FB8?ctx->f2.fl:ctx->f0.fl};
            value.step=final?5:30;value.phase=MEM_H(0x36,int32_t(physical))-value.step;
            value.base=real(rdram,physical+(final?0x58u:0x5Cu));value.amplitude=final?45:40;
            value.radians=rom_double(rdram,final?0x80228C20u:0x80228BC8u);value.before=value.evaluate(0);break;
        }
        case 0x80226444:
            value={physical+0x60,EquationKind::approach,real(rdram,physical+0x60),ctx->f0.fl};
            value.base=MEM_H(0x36,int32_t(physical))<10?0:MEM_H(0x38,int32_t(physical))&1?15:-15;
            value.retention=1-rom_double(rdram,MEM_H(0x36,int32_t(physical))<10?0x80228BD0u:0x80228BD8u);break;
        case 0x802265E8: {
            // Source first approaches its angular rate, then optionally halves
            // that NEW rate. Compose both operations before the angular sum.
            const auto prior=captured(physical+0x60);
            const double multiplier=rom_double(rdram,0x80228BE0);
            value={physical+0x60,EquationKind::exponential,real(rdram,physical+0x60),ctx->f0.fl};
            value.retention=multiplier;
            if(prior&&prior->kind==EquationKind::approach) {
                value.kind=EquationKind::approach;value.before=prior->before;
                value.retention=multiplier*prior->retention;
                value.base=multiplier*(1-prior->retention)*prior->base/(1-value.retention);
            }
            break;
        }
        case 0x80226614: {
            const auto spin=scalar(physical+0x60);value={actor+0x10,EquationKind::angle,real(rdram,actor+0x10),ctx->f0.fl};
            if(spin.kind==EquationKind::exponential) {value.kind=EquationKind::geometric_angle;value.retention=spin.retention;value.step=spin.after;}
            else if(spin.kind==EquationKind::approach) {value.kind=EquationKind::approach_drift_angle;value.base=spin.base;value.retention=spin.retention;value.amplitude=spin.before-spin.base;}
            else value.step=spin.after;
            break;
        }
        case 0x80226E78:case 0x80221A50:case 0x80221A5C: {
            const bool heading=site==0x80221A50;
            value={actor+(heading?0x10u:0xCu),heading?EquationKind::spatial_heading:EquationKind::spatial_pitch,0,ctx->f0.fl};
            value.upstream=std::make_shared<const Equation>(prior_position(0));value.coefficient=std::make_shared<const Equation>(prior_position(2));value.auxiliary=std::make_shared<const Equation>(prior_position(1));
            value.radians=rom_double(rdram,0x80228AA0);
            if(site==0x80226E78) {
                // Source negates atan2(Y-80, radius), unlike Fannel's
                // target-minus-current aim. Keep the same sign at fractions.
                value.initial_position=80;value.pitch_direction=1;
            }
            else {const uint32_t player=uint32_t(word(rdram,0x8021D49C));if(!pointer(player,0xA8))return;value.base=real(rdram,player+0x9C);value.amplitude=real(rdram,player+0xA4);value.initial_position=real(rdram,player+0xA0)+60;}
            value.before=value.evaluate(0);break;
        }
        case 0x8021FD6C:case 0x80221DF4:case 0x802221B0:
            value={physical+0x58,EquationKind::linear,real(rdram,physical+0x58),site==0x80221DF4?ctx->f0.fl:std::bit_cast<float>(uint32_t(ctx->r6))};
            value.step=-rom_double(rdram,site==0x8021FD6C?0x80228A78u:site==0x80221DF4?0x80228AE0u:0x80228AF0u);break;
        case 0x802202B8:case 0x802202C0: {
            const auto e=captured(actor+0x18);if(!e)return;value=*e;value.address=actor+(site==0x802202B8?0x20u:0x1Cu);value.after=ctx->f0.fl;break;
        }
        case 0x802218D8:case 0x802218F8: {
            value={actor+0xA0,EquationKind::sine,0,ctx->f0.fl};
            value.step=MEM_H(0x44,int32_t(physical))&1?10:-10;value.phase=MEM_H(0x34,int32_t(physical))-value.step;value.amplitude=10;value.radians=rom_double(rdram,0x80228AA8);
            if(site==0x802218F8) {
                const int slot=attachment_parent_slot(rdram,*owner);const auto part=slot>=30&&slot<dora3::distance::common_end?identity(rdram,0x804B1CD0u+unsigned(slot)*stride):std::nullopt;if(!part)return;
                value.base=real(rdram,part->actor+0xA0)-real(rdram,physical+0x60);
                const auto& parent=periods[index(part->actor)];
                if(command_slice&&parent&&same(parent->owner,*part)&&
                   std::bit_cast<int32_t>(parent->source_endpoint_position[1])==word(rdram,part->actor+0xA0)) {
                    // 218F8 is parent Y minus60 plus this satellite's sine.
                    // Carry the parent's real path, not its one-tick Y sample.
                    // The previous sine equation overwrote the live attachment
                    // Y on every slice, stepping all four spikes during ascent.
                    value.kind=EquationKind::parent_height_wave;
                    value.base=-real(rdram,physical+0x60);
                    value.motion=immutable_motion(*parent);
                    value.motion_start_age=double(parent->expected_phase-parent->start_phase)/parent->denominator-
                        double(command_slice->numerator)/command_slice->denominator;
                }
            }
            value.before=value.evaluate(0);break;
        }
        default: {
            const bool read=site>=0x8022800C&&site<=0x80228034,write=site>=0x8022803C&&site<=0x80228068;
            if(!read&&!write)goto boss6_other_store;
            const unsigned field=(site-(read?0x8022800C:0x8022803C))/8;if(field>=6)return;
            const unsigned offset=field<3?0x9C+4*field:0xC+4*(field-3);
            auto history=boss2_ghost_history.find(actor);
            if(history==boss2_ghost_history.end()||!same(history->second.owner,*owner)||history->second.epoch!=epoch||(read&&std::bit_cast<int32_t>(history->second.fields[field].after)!=word(rdram,physical+0x58+4*field))) {
                Boss2GhostHistory initial{*owner,epoch,{}};
                for(unsigned n=0;n<6;++n) {const float literal=real(rdram,physical+0x58+4*n);initial.fields[n]=Equation{0,EquationKind::linear,literal,literal};}
                history=boss2_ghost_history.insert_or_assign(actor,initial).first;
            }
            if(read) {value=history->second.fields[field];value.address=actor+offset;value.after=ctx->f0.fl;break;}
            const int slot=attachment_parent_slot(rdram,*owner);const auto part=slot>=30&&slot<dora3::distance::common_end?identity(rdram,0x804B1CD0u+unsigned(slot)*stride):std::nullopt;
            Equation sample{0,EquationKind::linear,ctx->f0.fl,ctx->f0.fl};
            if(part)if(const auto& prior=periods[index(part->actor)];prior&&same(prior->owner,*part)) {
                if(field<3&&std::bit_cast<int32_t>(sample_period_motion(*prior,1,field).first)==std::bit_cast<int32_t>(ctx->f0.fl)) {sample.kind=EquationKind::historical_axis;sample.motion=immutable_motion(*prior);sample.motion_axis=field;sample.before=sample_period_motion(*prior,0,field).first;}
                else if(field>=3) {
                    const auto e=std::find_if(prior->equations.begin(),prior->equations.end(),[&](const auto& e){return e.address==part->actor+offset&&std::bit_cast<int32_t>(e.after)==std::bit_cast<int32_t>(ctx->f0.fl);});
                    if(e!=prior->equations.end())sample=*e;
                    else if(field==4&&prior->proposal_yaw[0]==ctx->f0.fl) {sample.kind=EquationKind::angle;sample.before=prior->initial_yaw[0];sample.step=prior->turn[0];}
                }
            }
            history->second.fields[field]=sample;return;
        }
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
boss6_other_store:
    if(owner->rom==0x11D570) {
        const auto captured=[&](uint32_t address)->std::optional<Equation> {
            const auto e=std::find_if(equations.begin(),equations.end(),[&](const auto& e){return e.address==address&&std::bit_cast<int32_t>(e.after)==word(rdram,address);});
            return e==equations.end()?std::nullopt:std::optional<Equation>(*e);
        };
        switch(site) {
        case 0x8021E228:case 0x8021E22C:case 0x8021E230: {
            // These stores target the linked mask, not Bug's own scale.
            // The parent's command snapshot includes all eight part lifetimes.
            const auto mask=identity(rdram,uint32_t(ctx->r17));
            const int slot=MEM_H(0x80,int32_t(physical));
            if(!mask||mask->kind!=473||mask->rom!=owner->rom||slot<30||slot>=dora3::distance::common_end||mask->actor!=0x804B1CD0u+unsigned(slot)*stride)return;
            const unsigned offset=site==0x8021E228?0x18:site==0x8021E22C?0x20:0x1C;
            value={mask->actor+offset,EquationKind::sine,0,ctx->f0.fl};
            value.phase=MEM_H(0x36,int32_t(physical))-3;value.step=3;
            value.base=.5;value.amplitude=.5;value.radians=rom_double(rdram,0x8021F2D0);value.before=value.evaluate(0);break;
        }
        case 0x8021E284:case 0x8021E6B8: {
            const bool intro=site==0x8021E284;
            const double base=intro?MEM_H(0xC0,int32_t(physical)):real(rdram,physical+0x58);
            value={actor+0xA0,EquationKind::sine,0,ctx->f2.fl};value.base=base;
            value.amplitude=intro?real(rdram,physical+0x58):200-base;
            value.phase=MEM_H(0x36,int32_t(physical))-3;value.step=3;
            value.radians=rom_double(rdram,intro?0x8021F2D0:0x8021F2E0);value.before=value.evaluate(0);break;
        }
        case 0x8021E2D0:case 0x8021E704: {
            const bool yaw=site==0x8021E2D0;const double coefficient=rom_double(rdram,yaw?0x8021F2D8:0x8021F2E8);
            value={actor+(yaw?0x10u:0xCu),EquationKind::angle,real(rdram,actor+(yaw?0x10u:0xCu)),ctx->f0.fl};
            value.step=((yaw?100:90)-MEM_H(0x36,int32_t(physical)))*coefficient;
            value.acceleration=-3*coefficient;break;
        }
        case 0x8021E4F0:case 0x8021E508:case 0x8021EC1C: {
            const bool pitch=site==0x8021E508;const auto e=captured(physical+(pitch?0xCu:4u));if(!e)return;
            value=*e;value.address=actor+(pitch?0xCu:0x10u);value.after=pitch?ctx->f0.fl:site==0x8021E4F0?ctx->f2.fl:ctx->f0.fl;
            if(pitch)value.before=angle(value.before+90);
            break;
        }
        case 0x8021E954:case 0x8021EFA8: {
            value={actor+0xA0,EquationKind::harmonic_accumulation,real(rdram,actor+0xA0),ctx->f2.fl};
            value.radians=1;value.phase=-std::acos(-1.)/2;value.step=std::acos(-1.);
            value.amplitude=double(value.after)-value.before;break;
        }
        case 0x8021EADC:case 0x8021EAE0:case 0x8021D958:case 0x8021D960:case 0x8021DA4C:case 0x8021DA54: {
            const auto e=captured(actor+0x18);if(!e)return;value=*e;
            value.address=actor+((site==0x8021EADC||site==0x8021D958||site==0x8021DA4C)?0x20u:0x1Cu);value.after=ctx->f0.fl;break;
        }
        case 0x8021D954:case 0x8021DA48: {
            const bool growing=site==0x8021D954;
            value={actor+0x18,EquationKind::approach,real(rdram,actor+0x18),ctx->f0.fl};
            value.base=rom_double(rdram,growing?0x8021F298:0x8021F2B8);
            value.retention=1-rom_double(rdram,growing?0x8021F2A0:0x8021F2A8);break;
        }
        case 0x8021DBAC:value={physical+0x58,EquationKind::linear,real(rdram,physical+0x58),ctx->f0.fl};value.step=double(value.after)-value.before;break;
        default:goto boss7_other_store;
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
boss7_other_store:
    if(owner->rom==0x1561E0) {
        if(owner->kind==300||owner->kind==305) {
            const std::array<uint32_t,5> stores=owner->kind==300?
                std::array<uint32_t,5>{0x8023E2EC,0x8023E2F4,0x8023E2FC,0x8023E304,0x8023E30C}:
                std::array<uint32_t,5>{0x8023EBD0,0x8023EBD8,0x8023EBE0,0x8023EBE8,0x8023EBF0};
            if(const auto found=std::find(stores.begin(),stores.end(),site);found!=stores.end()) {
                const unsigned field=unsigned(found-stores.begin()),count=owner->kind==300?40:60;
                const uint32_t buffer=uint32_t(word(rdram,physical+0x48));
                if(!pointer(buffer,20*count))return;
                auto history=soundorago_history.find(actor);
                if(history==soundorago_history.end()||!same(history->second.owner,*owner)||
                   history->second.epoch!=epoch||history->second.buffer!=buffer) {
                    SoundoragoHistory initial{*owner,epoch,buffer,count,{}};
                    for(unsigned n=0;n<count;++n)for(unsigned j=0;j<5;++j) {
                        const float sample=real(rdram,buffer+20*n+4*j);
                        initial.samples[n][j]=Equation{0,j<3?EquationKind::linear:EquationKind::angle,sample,sample};
                    }
                    history=soundorago_history.insert_or_assign(actor,initial).first;
                }
                const int slot=MEM_H(0x44,int32_t(physical));
                if(slot<0||unsigned(slot)>=count)return;
                Equation sample{0,field<3?EquationKind::linear:EquationKind::angle,ctx->f0.fl,ctx->f0.fl};
                if(field<3) {
                    const auto& previous=periods[index(actor)];
                    if(previous&&same(previous->owner,*owner)&&
                       std::bit_cast<int32_t>(sample_period_motion(*previous,1,field).first)==std::bit_cast<int32_t>(ctx->f0.fl)) {
                        sample.kind=EquationKind::historical_axis;sample.motion=immutable_motion(*previous);
                        sample.motion_axis=field;sample.before=sample_period_motion(*previous,0,field).first;
                    }
                } else {
                    const uint32_t address=actor+(field==3?0x10:0xC);
                    const auto equation=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                        return e.address==address&&std::bit_cast<int32_t>(e.after)==std::bit_cast<int32_t>(ctx->f0.fl);
                    });
                    if(equation!=equations.end())sample=*equation;
                }
                sample.address=0;history->second.samples[unsigned(slot)][field]=sample;return;
            }
        }
        if((owner->kind==301||owner->kind==302||owner->kind==306||owner->kind==307)&&
           (site==0x8023EE08||site==0x8023EE10||site==0x8023EE18||site==0x8023EE20||site==0x8023EE28)) {
            const int slot=MEM_H(0x7C,int32_t(physical));
            if(slot<30||slot>=dora3::distance::common_end)return;
            const uint32_t root=0x804B1CD0u+unsigned(slot)*stride;
            const auto parent=identity(rdram,root);const auto history=soundorago_history.find(root);
            if(!parent||history==soundorago_history.end()||!same(history->second.owner,*parent)||history->second.epoch!=epoch||
               uint32_t(word(rdram,parent->physical+0x48))!=history->second.buffer)return;
            const uint32_t input=uint32_t(ctx->r4),buffer=history->second.buffer;
            if(input<buffer||input>=buffer+20*history->second.count||(input-buffer)%20)return;
            const unsigned field=site==0x8023EE08?0:site==0x8023EE10?1:site==0x8023EE18?2:site==0x8023EE20?3:4;
            value=history->second.samples[(input-buffer)/20][field];
            if(std::bit_cast<int32_t>(value.after)!=std::bit_cast<int32_t>(ctx->f0.fl))return;
            value.address=actor+(field<3?0x9Cu+4*field:field==3?0x10u:0xCu);
            std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
        }
        if(site==0x8023E9A8) {
            value={physical+0x58,EquationKind::sine,0,ctx->f0.fl};
            value.phase=double(MEM_H(0x36,int32_t(physical)))-15;value.step=15;
            value.amplitude=25;value.radians=rom_double(rdram,0x80240828);value.before=value.evaluate(0);
        } else if(site==0x8023E9D0) {
            const auto wave=std::find_if(equations.begin(),equations.end(),[&](const auto& e){return e.address==physical+0x58;});
            if(wave==equations.end())return;
            value=*wave;value.address=physical+4;value.kind=EquationKind::sine_angle;
            value.base=MEM_H(0x38,int32_t(physical));value.before=value.evaluate(0);value.after=ctx->f0.fl;
        } else if(site==0x8023EAF0) {
            value={physical+0xC,EquationKind::sine_angle,0,ctx->f0.fl};
            value.phase=double(MEM_H(0x3A,int32_t(physical)))-10;value.step=10;
            value.amplitude=20;value.radians=rom_double(rdram,0x80240830);value.before=value.evaluate(0);
        } else if(site==0x8023E150||site==0x8023EB08||site==0x8023EB44||site==0x8023EB6C||
                  site==0x8023EB84||site==0x8023EB8C) {
            const bool yaw=site==0x8023EB44||site==0x8023EB84;
            const uint32_t input=physical+(yaw?4:0xC);
            const auto source=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==input&&std::bit_cast<int32_t>(e.after)==word(rdram,input);
            });
            if(source==equations.end())return;
            value=*source;
            value.address=site==0x8023E150||site==0x8023EB84||site==0x8023EB8C?actor+(yaw?0x10:0xC):input;
            value.after=site==0x8023EB44?ctx->f14.fl:ctx->f0.fl;
        } else goto soundorago_other_store;
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
soundorago_other_store:
    if(owner->rom==0xD5D10&&owner->kind==353&&
       (site==0x802206F8||site==0x80220710||site==0x80220728)) {
        auto& history=delayed_heading_history[index(actor)];
        if(!history||!same(history->owner,*owner)||history->epoch!=epoch||
           (site==0x802206F8&&std::bit_cast<int32_t>(history->samples[0].after)!=word(rdram,physical+0x64))) {
            DelayedHeadingHistory initial{*owner,epoch,{}};
            for(unsigned n=0;n<6;++n) {
                const float heading=real(rdram,physical+0x64+4*n);
                initial.samples[n]=Equation{actor+0x10,EquationKind::angle,heading,heading};
            }
            history=initial;
        }
        if(site==0x80220710) {
            for(unsigned n=0;n<5;++n)history->samples[n]=history->samples[n+1];
            return;
        }
        if(site==0x80220728) {
            const int slot=attachment_parent_slot(rdram,*owner);
            const auto parent=slot>=30&&slot<dora3::distance::common_end?identity(rdram,0x804B1CD0u+unsigned(slot)*stride):std::nullopt;
            Equation sample{actor+0x10,EquationKind::angle,ctx->f0.fl,ctx->f0.fl};
            if(parent) {
                const auto& period=periods[index(parent->actor)];
                if(period&&same(period->owner,*parent)) {
                    const auto previous=std::find_if(period->equations.begin(),period->equations.end(),[&](const auto& e){
                        return e.address==parent->actor+0x10&&std::bit_cast<int32_t>(e.after)==std::bit_cast<int32_t>(ctx->f0.fl);
                    });
                    if(previous!=period->equations.end())sample=*previous;
                    else if(std::bit_cast<int32_t>(period->proposal_yaw[0])==std::bit_cast<int32_t>(ctx->f0.fl)) {
                        sample.before=period->initial_yaw[0];sample.step=period->turn[0];
                    }
                }
            }
            sample.address=actor+0x10;history->samples[5]=sample;return;
        }
        value=history->samples[0];value.address=actor+0x10;value.after=ctx->f0.fl;
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
    if(owner->rom==0x129A30&&owner->kind>=366&&owner->kind<=369&&
       (site==0x8022E320||site==0x8022E5EC||site==0x8022E970||site==0x8022F35C||site==0x8022F720||
        site==0x8022FAA8||site==0x8022FF6C||site==0x8022FF74||site==0x8022EB2C||site==0x8022EB44||
        site==0x8022EC2C||site==0x8022FC64||site==0x8022FC7C||site==0x8022FD64)) {
        if(site==0x8022EB2C||site==0x8022EB44||site==0x8022EC2C||site==0x8022FC64||site==0x8022FC7C||site==0x8022FD64) {
            const bool yaw=site==0x8022EC2C||site==0x8022FD64,roll=site==0x8022EB44||site==0x8022FC7C;
            const bool third=owner->kind==368;
            value={actor+(yaw?0x10u:roll?0x14u:0xCu),EquationKind::approach,0,yaw?ctx->f2.fl:roll?ctx->f4.fl:ctx->f0.fl};
            value.before=real(rdram,value.address);value.base=yaw?real(rdram,physical+0x58):60;
            value.retention=1-rom_double(rdram,third?(yaw?0x80231BD8u:0x80231BD0u):(yaw?0x80231BC8u:0x80231BC0u));
        } else {
            const bool pitch=site==0x8022FF74;
            const uint32_t input=physical+(pitch?0xCu:4u);
            const auto equation=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==input&&std::bit_cast<int32_t>(e.after)==word(rdram,input);
            });
            if(equation==equations.end())return;
            value=*equation;value.address=actor+(pitch?0xCu:0x10u);value.after=site==0x8022FF6C?ctx->f2.fl:ctx->f0.fl;
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
    if(owner->rom==0xE5480&&owner->kind>=336&&owner->kind<=338&&
       (site==0x80221344||site==0x80221390||site==0x802215F0||site==0x80221664||site==0x8022168C||
        site==0x80221908||site==0x80221938||site==0x80221F2C||site==0x802221F8||site==0x80222460||
        site==0x80222490||site==0x80222898||site==0x80222BEC)) {
        if(site==0x80221344||site==0x80221390||site==0x80221F2C||site==0x802221F8||site==0x80222490||site==0x80222898) {
            // Source floor/ceiling tests select an additive velocity kick.
            // Keep the already consumed trigonometric projection as well.
            if(projection&&projection->physical==physical&&projection->calls[1]==1&&
               std::bit_cast<int32_t>(float(projection->proposal[1]+projection->correction[1]))==word(rdram,physical+0x14))
                projection->correction[1]+=double(ctx->f0.fl)-real(rdram,physical+0x14);
            return;
        } else if(site==0x802215F0) {
            // A3F8 frame is an intermediate scratch write. Preserve the
            // real incoming speed before the triangle/scaling overwrites it.
            equation_operands[physical]=ctx->f20.fl;return;
        } else if(site==0x80221664) {
            const auto frame=equation_operands.find(physical);if(frame==equation_operands.end())return;
            const float stored_rate=real(rdram,physical+0xCC);
            const double rate=MEM_BU(0xC9,int32_t(physical))==1?0:stored_rate==0?3.:stored_rate;
            const uint32_t clip=uint32_t(word(rdram,owner->model+0x18));if(!pointer(clip,8))return;
            const double duration=real(rdram,clip+4);
            const double previous=frame->second-rate;
            // A3D4 actually returns clip duration; keep its exact model ABI.
            value={physical,EquationKind::linear,float((previous>duration*rom_double(rdram,0x80223CA0)?duration-previous:previous)*rom_double(rdram,0x80223CA8)),ctx->f0.fl};
            value.step=double(value.after)-value.before;
        } else if(site==0x80222BEC) {
            value={physical+0x58,EquationKind::linear,real(rdram,physical+0x58),std::bit_cast<float>(uint32_t(ctx->r6))};
            value.step=double(value.after)-value.before;
        } else {
            const bool pitch=site==0x8022168C||site==0x80221938;
            const uint32_t input=physical+(pitch?0xCu:4u);
            const auto equation=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==input&&std::bit_cast<int32_t>(e.after)==word(rdram,input);
            });
            if(equation==equations.end())return;
            value=*equation;value.address=site==0x8022168C?physical+0xC:actor+(pitch?0xCu:0x10u);value.after=ctx->f0.fl;
            if(site==0x8022168C) {
                const auto folded=[](float a){return a>90&&a<=180?180-a:a>180&&a<270?540-a:a;};
                value.before=folded(value.before);value.step=std::remainder(double(value.after)-value.before,360.);
            } else if(site==0x80221938) {
                value.before=angle(value.before+90);value.after=ctx->f0.fl;
            }
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
    if(owner->rom==0xE5480&&owner->kind>=332&&owner->kind<=335&&
       (site==0x8021F120||site==0x8021F3BC||site==0x8021F590||site==0x8021F594||site==0x8021F890||site==0x8021F950||
        site==0x8021F980||site==0x8021F9E0||site==0x8021FA30||site==0x8021FF64||site==0x802200E0||
        site==0x802202DC||site==0x80220554||site==0x8022055C||site==0x802209A0||site==0x802209A8||
        site==0x80220CF4||site==0x80220D08||site==0x80220D14||site==0x80220D1C||site==0x80220D8C||
        site==0x80220DE8||site==0x80220E38)) {
        if(site==0x8021FF64||site==0x802200E0) {
            integer_motion_operands[physical+0x3A]={double(MEM_H(0x3A,int32_t(physical))),site==0x8021FF64?1.:-1.};return;
        } else if(site==0x8021F950) {
            value={physical+0xC,EquationKind::sine_angle,0,ctx->f0.fl};
            value.phase=double(MEM_H(0x3A,int32_t(physical)))-20;value.step=20;
            value.amplitude=20;value.radians=rom_double(rdram,0x80223C38);value.before=value.evaluate(0);
        } else if(site==0x80220CF4||site==0x80220D08) {
            const bool yaw=site==0x80220CF4;
            value={physical+(yaw?4u:0xCu),EquationKind::angle,real(rdram,physical+(yaw?0x60u:0x64u)),ctx->f0.fl};
            value.step=real(rdram,physical)-35;
            const auto ramp=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==physical&&e.kind==EquationKind::linear&&std::bit_cast<int32_t>(e.after)==word(rdram,physical);
            });
            if(ramp!=equations.end())value.acceleration=ramp->step;
        } else if(site==0x80220D8C) {
            value={physical+0x68,EquationKind::linear,real(rdram,physical+0x68),std::bit_cast<float>(uint32_t(ctx->r6))};
            value.step=double(value.after)-value.before;
        } else {
            const bool scale=site==0x8021F590||site==0x8021F594||site==0x8021F890||site==0x8021F9E0||site==0x8021FA30||
                site==0x802209A0||site==0x802209A8||site==0x80220DE8||site==0x80220E38;
            const bool pitch=site==0x8021F980||site==0x802202DC||site==0x8022055C||site==0x80220D1C;
            const uint32_t input=scale?actor+0x18:physical+(pitch?0xCu:4u);
            const auto equation=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==input&&std::bit_cast<int32_t>(e.after)==word(rdram,input);
            });
            if(equation==equations.end())return;
            value=*equation;
            value.address=scale?actor+(site==0x8021F590||site==0x802209A0?0x20u:0x1Cu):
                site==0x8021F980||site==0x802202DC?physical+0xC:
                site==0x80220D14?physical+0x60:site==0x80220D1C?physical+0x64:actor+(pitch?0xCu:0x10u);
            value.after=ctx->f0.fl;
            if(site==0x802202DC) {
                const auto folded=[](float a){return a>90&&a<=180?180-a:a>180&&a<270?540-a:a;};
                value.before=folded(value.before);value.step=std::remainder(double(value.after)-value.before,360.);
            }
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
    if(owner->rom==0xE5480&&(owner->kind==330||owner->kind==331)&&
       (site==0x8021D98C||site==0x8021DB64||site==0x8021DB7C||site==0x8021DD98||site==0x8021E094||site==0x8021E0A8||
        site==0x8021E210||site==0x8021E660||site==0x8021E674||site==0x8021E6E0||site==0x8021E6E4||site==0x8021E704||
        site==0x8021E740||site==0x8021E744||site==0x8021E768||site==0x8021E800||site==0x8021E834||site==0x8021E83C||site==0x8021E964)) {
        if(site==0x8021E210||site==0x8021E964) {
            value={actor+0x14,EquationKind::approach,real(rdram,actor+0x14),ctx->f2.fl};
            value.base=180;value.retention=1-rom_double(rdram,site==0x8021E210?0x80223C00u:0x80223C10u);
        } else if(site==0x8021E660||site==0x8021E6E0||site==0x8021E740) {
            value={physical+(site==0x8021E660?0x58u:site==0x8021E6E0?0x5Cu:0x60u),EquationKind::linear,0,
                site==0x8021E6E0||site==0x8021E740?ctx->f2.fl:ctx->f0.fl};
            value.before=real(rdram,value.address);value.step=site==0x8021E740?6:12;
        } else if(site==0x8021E674||site==0x8021E704||site==0x8021E768) {
            const bool tilt=site==0x8021E674,small=site==0x8021E704;
            value={physical+(tilt?0xCu:8u),tilt?EquationKind::sine_angle:EquationKind::sine,0,ctx->f0.fl};
            value.phase=real(rdram,physical+(tilt?0x58u:small?0x5Cu:0x60u))-(small||tilt?12:6);
            value.step=small||tilt?12:6;value.amplitude=tilt?1:small?20:80;
            value.radians=rom_double(rdram,0x80223C08);value.before=value.evaluate(0);
        } else {
            const bool multiply=site==0x8021E6E4||site==0x8021E744;
            const bool heading=site==0x8021DB64||site==0x8021E094||site==0x8021E834;
            const uint32_t input=physical+(heading?4u:0xCu);
            const auto equation=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==input&&std::bit_cast<int32_t>(e.after)==word(rdram,input);
            });
            if(equation==equations.end())return;
            value=*equation;
            value.address=site==0x8021D98C||site==0x8021DD98||site==0x8021E800||multiply?physical+0xC:actor+(heading?0x10u:0xCu);
            value.after=site==0x8021DB7C?ctx->f2.fl:multiply?ctx->f4.fl:ctx->f0.fl;
            if(multiply){value.amplitude*=site==0x8021E6E4?30:60;value.before=value.evaluate(0);}
            if(site==0x8021D98C||site==0x8021DD98) {
                // Source 31BD4 folds pitch into +/-90 before E801C. Keep
                // that selected branch; targeting/turn bounds remain Source.
                const auto folded=[](float a){return a>90&&a<=180?180-a:a>180&&a<270?540-a:a;};
                value.before=folded(value.before);value.step=std::remainder(double(value.after)-value.before,360.);
            }
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
    if(owner->rom==0xE5480&&(owner->kind==339||owner->kind==340)&&
       (site==0x8022308C||site==0x802230AC||site==0x802230FC||site==0x80223224||site==0x802233D8||
        site==0x802237A0||site==0x80223818||site==0x8022386C)) {
        if(site==0x8022308C) {
            // Wave consumes OLD36; its increment/reset occurs after this store.
            value={physical+4,EquationKind::sine_angle,0,ctx->f0.fl};
            value.phase=double(MEM_H(0x36,int32_t(physical)))-10;value.step=10;
            value.amplitude=90;value.radians=rom_double(rdram,0x80223CD0);value.before=value.evaluate(0);
        } else if(site==0x80223224) {
            value={actor+0xA0,EquationKind::approach,real(rdram,actor+0xA0),ctx->f2.fl};
            value.base=real(rdram,0x8021BDF4)+100;value.retention=1-rom_double(rdram,0x80223CD8);
        } else if(site==0x802237A0) {
            value={physical+4,EquationKind::sine_drift_angle,real(rdram,physical+4),ctx->f0.fl};
            value.upstream=std::make_shared<Equation>(Equation{physical+4,EquationKind::angle,value.before,value.before});
            value.phase=double(MEM_H(0x3C,int32_t(physical)))-60;value.step=60;
            value.amplitude=30;value.radians=rom_double(rdram,0x80223CE8);
        } else {
            const bool scale=site==0x80223818||site==0x8022386C;
            const uint32_t input=scale?actor+0x18:physical+4;
            const auto equation=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==input&&std::bit_cast<int32_t>(e.after)==word(rdram,input);
            });
            if(equation==equations.end())return;
            value=*equation;value.address=scale?actor+0x1C:site==0x802233D8?actor+0x10:physical+4;
            value.after=ctx->f0.fl;
            if(site==0x802230FC) {
                value.base=real(rdram,physical+0x5C);value.before=value.evaluate(0);
            }
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
    if(owner->rom==0x125180&&owner->kind>=327&&owner->kind<=329&&
       (site==0x8022E360||site==0x8022E498||site==0x8022E6AC||site==0x8022E6B4||site==0x8022E6D8||
        site==0x8022E710||site==0x8022E714||site==0x8022E718||site==0x8022EB40||site==0x8022EB48)) {
        if(site==0x8022E360) {
            const double sign=MEM_H(0x34,int32_t(physical))&1?1.:-1.;
            const float before=real(rdram,actor+0xA0);
            value={actor+0xA0,EquationKind::cosine,before,ctx->f2.fl};
            value.base=double(before)+sign*5;value.amplitude=-sign*5;
            value.phase=0;value.step=180;value.radians=std::acos(-1.)/180.;
        } else if(site==0x8022E498) {
            value={physical+0x58,EquationKind::linear,real(rdram,physical+0x58),std::bit_cast<float>(uint32_t(ctx->r6))};
            value.step=double(value.after)-value.before;
        } else if(site==0x8022E710) {
            value={actor+0x18,EquationKind::approach,real(rdram,actor+0x18),ctx->f2.fl};
            value.base=2;value.retention=1-rom_double(rdram,0x8022F0C0);
        } else {
            const bool scale=site==0x8022E6AC||site==0x8022E6B4||site==0x8022E714||site==0x8022E718;
            const uint32_t input=scale?actor+0x18:physical+(site==0x8022EB48?4u:0xCu);
            const auto equation=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==input&&std::bit_cast<int32_t>(e.after)==word(rdram,input);
            });
            if(equation==equations.end())return;
            value=*equation;
            value.address=scale?actor+(site==0x8022E6AC||site==0x8022E714?0x20u:0x1Cu):site==0x8022EB48?actor+0x10:physical+0xC;
            value.after=site==0x8022E714||site==0x8022E718||site==0x8022EB48?ctx->f2.fl:ctx->f0.fl;
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
    if(owner->rom==0x129A30&&(owner->kind==382||owner->kind==384||owner->kind==386||owner->kind==398)&&
       (site==0x80230320||site==0x80230770||site==0x80230778||site==0x80230D78||site==0x80230D80||site==0x80230D84||
        site==0x80230E60||site==0x80230E68||site==0x80230E6C||site==0x80231730||site==0x8023184C||site==0x8023187C||site==0x802318DC)) {
        if(site==0x80230D78||site==0x80230E60) {
            const bool appear=site==0x80230D78;
            value={actor+0x18,EquationKind::sine,0,ctx->f0.fl};value.step=appear?9:-9;
            value.phase=double(MEM_H(0x36,int32_t(physical)))-value.step;
            value.radians=rom_double(rdram,appear?0x80231EB0u:0x80231EB8u);value.amplitude=1;
            value.before=value.evaluate(0);
        } else if(site==0x80231730) {
            value={physical+0x5C,EquationKind::exponential,real(rdram,physical+0x5C),ctx->f0.fl};
            value.retention=rom_double(rdram,0x80231EC0);
        } else {
            const bool spirit=owner->kind==386,orbit=owner->kind==398;
            const uint32_t input=spirit?actor+0x18:orbit?physical+0x58:physical+(site==0x80230778?0xCu:4u);
            const auto equation=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==input&&std::bit_cast<int32_t>(e.after)==word(rdram,input);
            });
            if(equation==equations.end())return;
            value=*equation;
            if(spirit) {
                value.address=actor+(site==0x80230D80||site==0x80230E68?0x20u:0x1Cu);value.after=ctx->f0.fl;
            } else if(orbit&&(site==0x8023184C||site==0x8023187C)) {
                value={actor+(site==0x8023184C?0x9Cu:0xA4u),site==0x8023184C?EquationKind::phase_sine:EquationKind::phase_cosine,0,ctx->f0.fl};
                value.upstream=std::make_shared<Equation>(*equation);value.amplitude=real(rdram,physical+0x60);
                value.radians=rom_double(rdram,0x80231EC8);value.before=value.evaluate(0);
            } else {
                value.address=actor+(site==0x80230778?0xCu:0x10u);value.after=site==0x80230770?ctx->f2.fl:ctx->f0.fl;
                if(orbit) {
                    const float offset=real(rdram,physical+0x5C)>0?90.f:-90.f;
                    const float angle=std::fmod(value.before+offset+360.f,360.f);value.before=angle;
                }
            }
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
    if(owner->rom==0x12E230&&(owner->kind==395||owner->kind==397)&&
       (site==0x8022EBB0||site==0x8022EF4C||site==0x8022EF50||site==0x8022EFE8)) {
        if(site==0x8022EBB0) {
            const int count=MEM_H(0x36,int32_t(physical));
            value={actor+0xA0,EquationKind::alternating_drift,real(rdram,actor+0xA0),ctx->f2.fl};
            value.phase=count;value.amplitude=count&1?-1:1;
        } else if(site==0x8022EFE8) {
            value={physical+0x58,EquationKind::linear,real(rdram,physical+0x58),std::bit_cast<float>(uint32_t(ctx->r6))};
            value.step=double(value.after)-value.before;
        } else {
            const auto x=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==actor+0x18&&std::bit_cast<int32_t>(e.after)==word(rdram,actor+0x18);
            });
            if(x==equations.end())return;
            value=*x;value.address=actor+(site==0x8022EF4C?0x20u:0x1Cu);value.after=ctx->f0.fl;
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
    if(owner->rom==0xDFC40&&(owner->kind==293||owner->kind==295)&&
       (site==0x80221B90||site==0x80221E44||site==0x80222024||site==0x8022202C||
        site==0x80222458||site==0x8022273C||site==0x80222940||site==0x80222948)) {
        const bool scale=site==0x80222024||site==0x8022202C||site==0x80222940||site==0x80222948;
        const uint32_t input=scale?actor+0x20:physical+4;
        const auto equation=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
            return e.address==input&&std::bit_cast<int32_t>(e.after)==word(rdram,input);
        });
        if(equation==equations.end())return;
        value=*equation;value.address=scale?actor+(site==0x80222024||site==0x80222940?0x1Cu:0x18u):actor+0x10;
        value.after=ctx->f0.fl;
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
    if(owner->rom==0x11F1C0&&owner->kind>=290&&owner->kind<=292&&
       (site==0x8022D950||site==0x8022DD54||site==0x8022E018||site==0x8022E1D0||site==0x8022E6DC||
        site==0x8022E7E8||site==0x8022E8E4||site==0x8022EBCC||site==0x8022EBD4)) {
        if(site==0x8022E018) {
            // Physical heading was reset to Source5C before this wave.
            // Counter38 is advanced with Angle(+10), then rounded to short.
            value={physical+4,EquationKind::sine_angle,0,ctx->f4.fl};
            value.phase=double(MEM_H(0x38,int32_t(physical)))-10;
            value.step=10;value.amplitude=20;value.base=real(rdram,physical+0x5C);
            value.radians=rom_double(rdram,0x8022F748);value.before=value.evaluate(0);
        } else if(site==0x8022E7E8) {
            value={physical+0x58,EquationKind::linear,real(rdram,physical+0x58),ctx->f14.fl};
            value.step=double(value.after)-value.before;
        } else if(site==0x8022E8E4) {
            value={physical+0x5C,EquationKind::linear,real(rdram,physical+0x5C),std::bit_cast<float>(uint32_t(ctx->r6))};
            value.step=double(value.after)-value.before;
        } else {
            const bool scale=site==0x8022EBCC||site==0x8022EBD4;
            const uint32_t input=scale?actor+0x18:physical+4;
            const auto equation=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==input&&std::bit_cast<int32_t>(e.after)==word(rdram,input);
            });
            if(equation==equations.end())return;
            value=*equation;value.address=scale?actor+(site==0x8022EBCC?0x20u:0x1Cu):actor+0x10;
            value.after=ctx->f0.fl;
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
    if(owner->rom==0xDFC40&&owner->kind>=283&&owner->kind<=286&&
       (site==0x80220444||site==0x80220950||site==0x80220CAC||site==0x80220DAC||site==0x80220DB4||
        site==0x802210D8||site==0x802212B8||site==0x802213B8||site==0x802213C0||site==0x802213CC)) {
        const bool scale=site==0x80220DAC||site==0x80220DB4||site==0x802213C0||site==0x802213CC;
        const uint32_t input=scale?actor+(owner->kind==283?0x20u:0x1Cu):physical+4;
        const auto equation=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
            return e.address==input&&std::bit_cast<int32_t>(e.after)==word(rdram,input);
        });
        if(equation==equations.end())return;
        value=*equation;
        value.address=scale?actor+(site==0x80220DAC?0x1Cu:site==0x802213C0?0x20u:0x18u):actor+0x10;
        value.after=scale&&owner->kind==284?ctx->f4.fl:ctx->f0.fl;
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
    if(owner->rom==0xD9910&&(owner->kind==281||owner->kind==282)&&
       (site==0x802218D4||site==0x80221EC4||site==0x80221CF0||site==0x80221F88||
        site==0x80222874||site==0x80222B54||site==0x80222D20)) {
        if(site==0x802218D4||site==0x80221EC4) {
            const float before=real(rdram,physical+0x5C);
            value={physical+0x5C,EquationKind::clamped,before,
                site==0x802218D4?std::min(ctx->f0.fl,1.f):std::max(ctx->f0.fl,.5f)};
            value.phase=before;value.step=site==0x802218D4?double(.01f):-double(.01f);
            value.clamp_min=site==0x802218D4?std::min(0.,double(before)):.5;
            value.clamp_max=site==0x802218D4?1.:std::max(1.,double(before));
        } else {
            const auto heading=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==physical+4&&std::bit_cast<int32_t>(e.after)==word(rdram,physical+4);
            });
            if(heading==equations.end())return;
            value=*heading;value.address=actor+0x10;value.after=ctx->f0.fl;
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
    if(owner->rom==0x121280&&owner->kind==373&&
       (site==0x80230C24||site==0x80230C28||site==0x80230CD8||site==0x80230CDC||site==0x80230D14)) {
        if(site==0x80230D14) {
            const float before=real(rdram,physical+0x68);
            value={physical+0x68,EquationKind::clamped,before,std::max(ctx->f0.fl,.1f)};
            value.phase=before;value.step=-.25;value.clamp_min=.1f;value.clamp_max=std::max(1.,double(before));
        } else {
            const auto x=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
                return e.address==actor+0x18&&std::bit_cast<int32_t>(e.after)==word(rdram,actor+0x18);
            });
            if(x==equations.end())return;
            value=*x;value.address=actor+((site==0x80230C24||site==0x80230CD8)?0x1Cu:0x20u);
            value.after=(site==0x80230C28||site==0x80230CDC)?ctx->f2.fl:ctx->f0.fl;
        }
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
    if(owner->rom==0x126BA0&&(owner->kind==364||owner->kind==365)&&
       (site==0x8022DA2C||site==0x8022DD88||site==0x8022DDB8||site==0x8022E3A0)) {
        const bool tilt=site==0x8022DA2C||site==0x8022DDB8;
        const uint32_t source=physical+(tilt?0xCu:4u);
        const auto input=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
            return e.address==source&&std::bit_cast<int32_t>(e.after)==word(rdram,source);
        });
        if(input==equations.end())return;
        value=*input;
        // DA2C is the same bounded pitch after the pure Source wrap helper.
        // Reflection/restart setters elsewhere cannot acquire this equation.
        value.address=site==0x8022DA2C?source:actor+(tilt?0xCu:0x10u);
        value.after=site==0x8022DDB8?ctx->f2.fl:ctx->f0.fl;
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);return;
    }
    // These are literal Source assignments from one already reviewed equation.
    // Copy its input, rather than treating the old destination as an input: the
    // Source X/Z collapse also handles externally supplied anisotropic scales.
    uint32_t scale_destination=actor+0x20;
    bool baguram_scale_copy=false;
    if(owner->rom==0xD9910)switch(site) {
        case 0x8021E268:scale_destination=actor+0x20;break;
        case 0x8021E270:scale_destination=actor+0x1C;break;
        case 0x8021F424:scale_destination=actor+0x20;break;
        case 0x8021F42C:scale_destination=actor+0x1C;break;
        default:scale_destination=0;break;
    } else if(owner->rom==0xCEC60)switch(site) {
        case 0x80221694:scale_destination=actor+0x20;break;
        case 0x8022169C:scale_destination=actor+0x1C;break;
        case 0x802221E0:scale_destination=actor+0x20;break;
        case 0x802221E8:scale_destination=actor+0x1C;break;
        case 0x80222690:scale_destination=actor+0x20;break;
        case 0x80222694:scale_destination=actor+0x1C;break;
        default:scale_destination=0;break;
    } else if(owner->rom==0xEBAC0)switch(site) {
        case 0x8021DB28:case 0x8021EC34:scale_destination=actor+0x20;break;
        case 0x8021DB2C:case 0x8021EC38:scale_destination=actor+0x1C;break;
        default:scale_destination=0;break;
    } else if(owner->rom==0xDFC40)switch(site) {
        case 0x8021E828:case 0x802200F4:scale_destination=actor+0x1C;break;
        case 0x8021E82C:case 0x802200F8:scale_destination=actor+0x20;break;
        default:scale_destination=0;break;
    } else scale_destination=0;
    baguram_scale_copy=scale_destination!=0;
    if(!baguram_scale_copy)scale_destination=actor+0x20;
    const bool scale_copy=baguram_scale_copy||(owner->rom==0xD9910&&site==0x80221628)||
        (owner->rom==0xEDDB0&&(site==0x80223A0C||site==0x80223BB0));
    const bool doragonrock_yaw_copy=owner->rom==0x121280&&
        (site==0x8022DD1C||site==0x8022E514||site==0x8022ED18||site==0x8022EEFC);
    const bool supaizan_yaw_copy=owner->rom==0xEBAC0&&
        (site==0x8021EEEC||site==0x8021F2CC);
    const bool oppi_yaw_copy=owner->rom==0x12FD90&&
        (site==0x8022F874||site==0x8022FCF4||site==0x8022FDD0||site==0x80230280||site==0x80230324);
    const bool kinopper_roam_yaw_copy=owner->rom==0x12FD90&&site==0x8022D884;
    const bool batti_yaw_copy=owner->rom==0xDFC40&&site==0x8021E5CC;
    const bool yaw_copy=kinopper_roam_yaw_copy||batti_yaw_copy||oppi_yaw_copy||supaizan_yaw_copy||doragonrock_yaw_copy||(owner->rom==0xEDDB0&&
        (site==0x80220C24||site==0x80221498||site==0x8022177C||site==0x8022234C||
         site==0x802225F0||site==0x80223210||site==0x802235A0||site==0x8022372C))||
        (owner->rom==0xCEC60&&(site==0x80223058||site==0x802231DC||site==0x802235CC||
         site==0x80223BF0||site==0x80223D44||site==0x8022425C));
    if(scale_copy||yaw_copy) {
        const auto source=scale_copy?actor+0x18:physical+4;
        const auto parent=std::find_if(equations.begin(),equations.end(),[&](const auto& equation){return equation.address==source;});
        if(parent==equations.end()||std::bit_cast<int32_t>(parent->after)!=word(rdram,source)) {
            // The literal heading copy consumes E88C4's result before any
            // subsequent wall reflection. Capture that actual bounded turn;
            // arbitrary heading setters still remain instantaneous commands.
            if(!(doragonrock_yaw_copy||supaizan_yaw_copy)||!turn||turn->calls!=1||turn->actor!=actor||
               turn->physical!=physical||turn->after!=ctx->f0.fl)return;
            value={actor+0x10,EquationKind::angle,turn->before,turn->after};
            value.step=std::remainder(double(turn->after)-turn->before,360.);
            std::erase_if(equations,[&](const auto& old){return old.address==value.address;});
            equations.push_back(value);return;
        }
        value=*parent;value.address=scale_copy?scale_destination:actor+0x10;
        value.after=ctx->f0.fl;
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});
        equations.push_back(value);return;
    }
    const auto additional=std::find_if(additional_stores.begin(),additional_stores.end(),
        [&](const auto& store){return store.rom==owner->rom&&store.site==site;});
    if(additional!=additional_stores.end()) {
        const auto& store=*additional;
        const auto address=(store.physical?physical:actor)+store.offset;
        const float operand=store.operand==6?ctx->f6.fl:store.operand==12?ctx->f12.fl:store.operand==4?ctx->f4.fl:store.operand==2?ctx->f2.fl:ctx->f0.fl;
        value={address,store.kind,real(rdram,address),operand};
        if(store.kind==EquationKind::exponential) {
            // Use the ROM multiplier, never Equation's default zero. A zero
            // ratio cancels fractional speed and makes projected log(ratio)
            // undefined (Boss6 final charge / Boss5 expanding effect).
            value.retention=rom_double(rdram,store.coefficient);
        }
        if(store.kind==EquationKind::approach||store.kind==EquationKind::wrapped_approach) {
            const uint64_t bits=(uint64_t(uint32_t(word(rdram,store.coefficient)))<<32)|uint32_t(word(rdram,store.coefficient+4));
            value.base=store.kind==EquationKind::wrapped_approach?
                double(value.before)+std::remainder(store.target-value.before,360.):store.target;
            value.retention=1-std::bit_cast<double>(bits);
        }
    } else if(owner->rom==0x12FD90&&owner->kind==272) {
        const auto captured=[&](uint32_t address) -> const Equation* {
            const auto found=std::find_if(equations.begin(),equations.end(),[&](const auto& e){return e.address==address&&std::bit_cast<int32_t>(e.after)==word(rdram,address);});
            return found==equations.end()?nullptr:&*found;
        };
        const int count=MEM_H(0x44,int32_t(physical));
        if(site==0x8022E9E4||site==0x8022EA80||site==0x8022EB1C) {
            const bool first=site==0x8022E9E4,second=site==0x8022EA80;
            value={actor+0xA0,EquationKind::sine,0,ctx->f0.fl};
            // Capture before Source replaces90->180 or360->540. The three
            // baselines are literal spawn/58/5C selected before this store.
            value.phase=double(MEM_H(0x36,int32_t(physical)))-15;value.step=15;
            value.radians=rom_double(rdram,first?0x80230858:second?0x80230860:0x80230868);
            value.amplitude=-(first?40:second?30:20)*double(count);
            value.base=real(rdram,actor+0xA0);value.before=value.evaluate(0);
        } else if(site==0x8022EB84) {
            const auto& old=periods[index(actor)];
            const bool ongoing=old&&same(old->owner,*owner)&&std::any_of(old->equations.begin(),old->equations.end(),[&](const auto& e){return e.address==physical+0x64;});
            value={physical+0x64,EquationKind::sine,0,ctx->f20.fl};
            // Source computes64 with OLD38/3A, then changes both counters.
            // The first new geometry is an actual reset command, not an
            // invented pre-birth sample at phase-10 with amplitude10.
            value.phase=double(MEM_H(0x38,int32_t(physical)))-(ongoing?10:0);
            value.step=ongoing?10:0;value.amplitude=MEM_H(0x3A,int32_t(physical));
            value.radians=rom_double(rdram,0x80230870);value.before=value.evaluate(0);
        } else if(site==0x8022EC4C||site==0x8022EE14) {
            const auto* wave=captured(physical+0x64);if(!wave)return;
            value=*wave;value.address=actor+0x14;value.kind=EquationKind::sine_angle;
            value.amplitude*=-rom_double(rdram,0x80230878);value.after=ctx->f0.fl;value.before=value.evaluate(0);
        } else if(site==0x8022ED00||site==0x8022ED78||site==0x8022EDC4) {
            const auto* wave=captured(physical+0x64);if(!wave)return;
            value=*wave;value.inner_amplitude=wave->amplitude;
            value.outer_radians=rom_double(rdram,0x80230880);
            const unsigned axis=site==0x8022ED00?0:site==0x8022EDC4?1:2;
            value.address=actor+0x9C+4*axis;value.base=real(rdram,value.address);
            value.kind=axis==1?EquationKind::nested_cosine:EquationKind::nested_sine;
            const double yaw=double(real(rdram,actor+0x10))*value.outer_radians;
            value.amplitude=40.*count*(axis==0?-std::cos(yaw):axis==1?-1:std::sin(yaw));
            value.after=ctx->f0.fl;value.before=value.evaluate(0);
        } else if(site==0x8022EE60) {
            value={physical+0x60,EquationKind::exponential,real(rdram,physical+0x60),ctx->f0.fl};
            value.retention=rom_double(rdram,0x80230888);
        } else if(site==0x8022EE98||site==0x8022EEC0||site==0x8022EEF0) {
            const unsigned axis=site==0x8022EE98?0:site==0x8022EEC0?1:2;
            const uint32_t address=actor+0x9C+4*axis;
            const auto* prior=captured(address);const auto* blend=captured(physical+0x60);
            const float position=real(rdram,address),weight=real(rdram,physical+0x60);
            Equation constant_position{address,EquationKind::linear,position,position};
            Equation constant_weight{physical+0x60,EquationKind::linear,weight,weight};
            value={address,EquationKind::composed_return,position,ctx->f4.fl};
            value.base=MEM_H(int32_t(0xBE + 2*axis),int32_t(physical));
            value.upstream=std::make_shared<Equation>(prior?*prior:constant_position);
            value.coefficient=std::make_shared<Equation>(blend?*blend:constant_weight);
            value.before=value.evaluate(0);
        } else throw std::logic_error("Unreviewed Native Bironn mathematical store");
    } else if(owner->rom==0xCEC60&&owner->kind>=297&&owner->kind<=299) {
        if(site==0x802219B8) {
            value={physical+0x78,EquationKind::sine,0,ctx->f0.fl};
            value.phase=double(MEM_H(0x44,int32_t(physical)))-6;
            value.step=6;value.amplitude=30;value.radians=rom_double(rdram,0x802246B0);
            value.before=value.evaluate(0);
        } else if(site==0x80221A08) {
            const auto wave=std::find_if(equations.begin(),equations.end(),[&](const auto& e){return e.address==physical+0x78;});
            if(wave==equations.end()||std::bit_cast<int32_t>(wave->after)!=word(rdram,physical+0x78))return;
            value=*wave;value.address=actor+0xA0;
            // Keep the real selected floor/ceiling baseline, before the Source
            // final add; its query and threshold selection remain commands.
            value.base=double(real(rdram,actor+0xA0))+300;
            value.after=ctx->f0.fl;value.before=value.evaluate(0);
        } else if(site==0x80222918||site==0x80222980) {
            const bool yaw=site==0x80222918;
            value={physical+(yaw?4u:0xCu),EquationKind::sine_angle,0,ctx->f0.fl};
            value.phase=double(MEM_H(0x34,int32_t(physical)))-20;
            value.step=20;value.amplitude=yaw&&MEM_H(0x44,int32_t(physical))? -20:20;
            value.base=real(rdram,physical+(yaw?0x58u:0x5Cu));
            value.radians=rom_double(rdram,yaw?(MEM_H(0x44,int32_t(physical))?0x80224718:0x80224710):0x80224720);
            value.before=value.evaluate(0);
        } else throw std::logic_error("Unreviewed Native Barupu mathematical store");
    } else if(owner->rom==0xEDDB0) {
        // Absolute circular/vertical paths use the actual phase counter and ROM
        // constants. Frame waits, dialogue and phase changes remain Source FSM.
        const auto wave=[&](uint32_t address,float proposal,unsigned counter,double step,uint32_t radians,
                            double amplitude,double base,EquationKind kind=EquationKind::sine,
                            double multiplier=1,double bias=0) {
            Equation result{address,kind,0,proposal};
            // Both MEM_H operands must stay signed: an unsigned offset would
            // promote the sign-extended guest address and add4GiB on the host.
            result.phase=(double(MEM_H(int32_t(counter),int32_t(physical)))-step)*multiplier+bias;
            result.step=step*multiplier;result.radians=rom_double(rdram,radians);
            result.amplitude=amplitude;result.base=base;result.before=result.evaluate(0);return result;
        };
        switch(site) {
        case 0x8022118C:case 0x80222028:
            value=wave(actor+0x9C,ctx->f2.fl,0x42,2,site==0x8022118C?0x80224708:0x80224758,200,real(rdram,actor+0x9C));break;
        case 0x802211CC:case 0x80222068:
            value=wave(actor+0xA4,ctx->f2.fl,0x42,2,site==0x802211CC?0x80224708:0x80224758,200,real(rdram,actor+0xA4),EquationKind::cosine);break;
        case 0x802211FC:case 0x80222108:
            value={actor+0x10,EquationKind::angle,float(MEM_H(0x42,int32_t(physical))+88),ctx->f0.fl};
            value.step=2;break;
        case 0x80221878:
            value=wave(actor+0x10,ctx->f4.fl,0x34,20,0x80224710,30,real(rdram,physical+0x58),EquationKind::sine_angle);break;
        case 0x80221F68:
            value=wave(actor+0xA0,ctx->f2.fl,0x36,5,0x80224750,200,real(rdram,actor+0xA0));break;
        case 0x802220D8:
            value=wave(actor+0xA0,ctx->f2.fl,0x42,2,0x80224758,200,real(rdram,actor+0xA0),EquationKind::sine,
                       -rom_double(rdram,0x80224760),rom_double(rdram,0x80224768));break;
        case 0x80223A04:case 0x80223BA8:
            value=wave(actor+0x18,ctx->f0.fl,0x36,site==0x80223A04?5:-5,
                       site==0x80223A04?0x80224848:0x80224868,
                       rom_double(rdram,site==0x80223A04?0x80224850:0x80224870),0);break;
        case 0x80223AB4:
            value=wave(actor+0x1C,ctx->f0.fl,0x36,5,0x80224858,rom_double(rdram,0x80224860),0);break;
        case 0x80223B48:
            value=wave(physical+0x58,ctx->f0.fl,0x36,-5,0x80224868,1,0);break;
        default:throw std::logic_error("Unreviewed Native Powan mathematical store");
        }
    } else switch(site) {
    case 0x8022DC5C:case 0x8022E4F8:case 0x8022DF60:
        if(owner->rom!=0x12FD90)return;
        value={actor+0x10,EquationKind::angle,real(rdram,actor+0x10),ctx->f0.fl};break;
    case 0x8021EF44:
        if(owner->rom!=0xCEC60)return;
        value={actor+0x14,EquationKind::angle,real(rdram,actor+0x14),ctx->f0.fl};break;
    case 0x80220324:case 0x80220B5C:
        if(owner->rom!=0xD9910)return;
        value={actor+0x14,EquationKind::angle,real(rdram,actor+0x14),ctx->f0.fl};break;
    case 0x80220A14:
        if(owner->rom!=0xD9910)return;
        value={actor+0x10,EquationKind::angle,real(rdram,actor+0x10),ctx->f0.fl};break;
    case 0x8021FDD8:case 0x802205E8:
        if(owner->rom!=0xD9910)return;
        value={physical+0xC,EquationKind::angle,real(rdram,physical+0xC),ctx->f0.fl};break;
    case 0x80220874:
        // Sabotti2 clears scalar speed at206BC, then assigns a fresh speed
        // from the player-bearing difference. This is not acceleration:
        // a zero-to-speed recurrence restarts on every Source period and
        // produces a repeated slowdown/surge during the ground spin.
        // Keep the real assigned speed; the captured heading still turns.
        return;
    case 0x802207E4:
        if(owner->rom!=0xD9910)return;
        value={physical+4,EquationKind::angle,real(rdram,physical+4),ctx->f0.fl};break;
    case 0x8022DA64:case 0x8022DAA8:
        if(owner->rom!=0x12FD90)return;
        value={physical,EquationKind::linear,real(rdram,physical),ctx->f0.fl};break;
    case 0x8021D85C:case 0x8021DC9C:case 0x8021E030:case 0x8021E970:case 0x8021EA44:
    case 0x8021F2EC:case 0x8021F5B4:case 0x8021F720:case 0x8021FACC:
    case 0x8021FD24:case 0x8021FFE0:case 0x8022012C:case 0x80220818:
        if(owner->rom!=0xCEC60)return;
        value={physical,EquationKind::linear,real(rdram,physical),ctx->f0.fl};break;
    case 0x8021DA28:case 0x8021E584: {
        if(owner->rom!=0xCEC60)return;
        const auto constant=site==0x8021DA28?0x80224630u:0x80224640u;
        const uint64_t bits=(uint64_t(uint32_t(word(rdram,constant)))<<32)|uint32_t(word(rdram,constant+4));
        value={actor+0xA0,EquationKind::sine,0,ctx->f2.fl};
        value.phase=std::fmod(double(MEM_H(0x34,int32_t(physical)))+350.,360.);value.step=10;value.amplitude=10;
        value.radians=std::bit_cast<double>(bits);value.base=real(rdram,actor+0xA0);
        value.before=value.evaluate(0);break;
    }
    case 0x8021DAC4:case 0x8021E624: {
        if(owner->rom!=0xCEC60)return;
        const auto constant=site==0x8021DAC4?0x80224638u:0x80224648u;
        const uint64_t bits=(uint64_t(uint32_t(word(rdram,constant)))<<32)|uint32_t(word(rdram,constant+4));
        value={actor+0xA0,EquationKind::approach,real(rdram,actor+0xA0),ctx->f0.fl};
        value.base=double(value.before)+real(rdram,physical+0x58);value.retention=1-std::bit_cast<double>(bits);break;
    }
    default:throw std::logic_error("Unreviewed Native enemy mathematical store");
    }
    if(owner->rom==0x121280&&(site==0x8022DD38||site==0x8022E544||site==0x8022ED58)) {
        // Rolling pitch consumes the actual scalar ramp, before floor/wall
        // commands can overwrite it. Continue the same kick-before-drift
        // recurrence; do not use a lerp between already produced poses.
        const uint32_t input=physical+(site==0x8022ED58?0x58u:0u);
        const auto scalar=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
            return e.address==input&&e.kind==EquationKind::linear&&std::bit_cast<int32_t>(e.after)==word(rdram,input);
        });
        if(scalar!=equations.end())value.acceleration=scalar->step*
            (site==0x8022E544&&MEM_H(0x36,int32_t(physical))!=0?-1:1);
        // Source explicitly writes0 after its exact360 proposal. Preserve
        // that endpoint without losing the fractional wrapped rotation.
        if(site==0x8022ED58&&value.after==360.f)value.after=0;
    }
    if(value.kind==EquationKind::angle)value.step=std::remainder(double(value.after)-value.before,360.);
    else if(value.kind==EquationKind::linear)value.step=double(value.after)-value.before;
    if(owner->rom==0xCEC60&&(site==0x80223BD4||site==0x80223BE8)) {
        // BroboA104 accelerates its turn rate5C before adding it to total58
        // and heading4. Keep that kick/drift coupling; state2 scalar speed is0.
        const auto rate=std::find_if(equations.begin(),equations.end(),[&](const auto& equation){return equation.address==physical+0x5C;});
        if(rate!=equations.end()&&std::bit_cast<int32_t>(rate->after)==word(rdram,physical+0x5C))value.acceleration=double(rate->after)-rate->before;
    }
    if(owner->rom==0x10DA30&&(site==0x8021FB74||site==0x802207A4)) {
        const uint32_t offset=site==0x8021FB74?0x38:0x36;
        value.step=MEM_H(int32_t(offset),int32_t(physical));
        if(const auto rate=integer_motion_operands.find(physical+offset);rate!=integer_motion_operands.end())value.acceleration=rate->second.step;
    }
    // Its hit reaction rotates by the old integer rate38, then decrements it.
    // Source counters remain discrete, while the angular drift continues.
    if(owner->rom==0xCEC60&&site==0x80224258)value.acceleration=-4;
    if(owner->rom==0xDFC40&&site==0x80220D9C)value.step=180;
    if(owner->rom==0xDFC40&&site==0x80221398) {
        // Source increments signed short34 by8 BEFORE its angular kick.
        // Preserve the real signed rate, including half-turns/wrapped angles.
        value.step=MEM_H(0x34,int32_t(physical));value.acceleration=8;
    }
    if(owner->rom==0x11F1C0&&(site==0x8022E808||site==0x8022E834)) {
        const auto rate=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
            return e.address==physical+0x58&&std::bit_cast<int32_t>(e.after)==word(rdram,physical+0x58);
        });
        if(rate!=equations.end()) {
            const double factor=site==0x8022E808?-1.:rom_double(rdram,0x8022F760);
            value.step=double(rate->after)*factor;value.acceleration=rate->step*factor;
        }
    }
    // Thunder2's shrink is based on Z even though its first store targets Y.
    if(owner->rom==0xDFC40&&site==0x802213B0) {
        value.before=real(rdram,actor+0x20);value.step=double(value.after)-value.before;
    }
    if(owner->rom==0x129A30&&owner->kind==398&&site==0x80231830) {
        const auto rate=std::find_if(equations.begin(),equations.end(),[&](const auto& e){
            return e.address==physical+0x5C&&e.kind==EquationKind::exponential&&
                std::bit_cast<int32_t>(e.after)==word(rdram,physical+0x5C);
        });
        value.step=real(rdram,physical+0x5C);
        if(rate!=equations.end()){value.kind=EquationKind::geometric_angle;value.retention=rate->retention;}
    }
    if(owner->rom==0x10DA30&&owner->kind==448&&site==0x8021DA54) {
        const auto rate=std::find_if(equations.begin(),equations.end(),[&](const auto& e){return e.address==physical+0x58&&std::bit_cast<int32_t>(e.after)==word(rdram,physical+0x58);});
        value.step=real(rdram,physical+0x58);
        if(rate!=equations.end()){value.kind=EquationKind::geometric_angle;value.retention=rate->retention;}
    }
    if(owner->rom==0x10DA30&&owner->kind==453&&(site==0x8021EBE8||site==0x8021EBFC)) {
        value.step=real(rdram,physical+0x5C);
        if(const auto rate=std::find_if(equations.begin(),equations.end(),[&](const auto& e){return e.address==physical+0x5C;});rate!=equations.end())value.acceleration=rate->step;
    }
    if(owner->rom==0xE5480&&owner->kind==334&&site==0x80220588) {
        // Counter3A remains an integer Source command. Only its actual
        // arithmetic kick is reused by the dependent continuous roll.
        value.step=MEM_H(0x3A,int32_t(physical));
        const auto rate=integer_motion_operands.find(physical+0x3A);
        if(rate!=integer_motion_operands.end())value.acceleration=rate->second.step;
    }
    if(owner->rom==0x70A50&&owner->kind==258) {
        if(site==0x801E568C) {
            value.base=real(rdram,physical+0x58);value.retention=1-rom_double(rdram,0x802113B0);
        } else if(site==0x801E56D4||site==0x801E56D8) {
            const auto root=std::find_if(equations.begin(),equations.end(),[&](const auto& e){return e.address==actor+0x18;});
            if(root!=equations.end()){value=*root;value.address=actor+(site==0x801E56D4?0x20:0x1C);value.after=ctx->f0.fl;}
        }
    }
    if(!std::isfinite(value.before)||!std::isfinite(value.after))throw std::logic_error("Invalid enemy equation");
    std::erase_if(equations,[&](const auto& old){return old.address==value.address;});equations.push_back(value);
}
extern "C" void dora3_native_enemy_turn(uint8_t* rdram,recomp_context* ctx,unsigned end) {
    if(!executing)return;
    if(!end) {
        const auto actor=uint32_t(ctx->r4),physical=uint32_t(word(rdram,actor+0x98));
        if(actor!=uint32_t(word(rdram,0x8021BDEC))||!pointer(physical,0xDC))return;
        const auto calls=turn?turn->calls+1:1;turn=Turn{actor,physical,real(rdram,physical+4),0,calls};
    } else if(turn)turn->after=ctx->f0.fl;
}
extern "C" void dora3_native_enemy_gravity(uint8_t* rdram,recomp_context* ctx,unsigned end) {
    if(!executing)return;
    if(!end) {
        const auto actor=uint32_t(ctx->r4),physical=uint32_t(word(rdram,actor+0x98));
        if(actor!=uint32_t(word(rdram,0x8021BDEC))||!pointer(physical,0xDC))return;
        const auto calls=gravity?gravity->calls+1:1;
        gravity=Gravity{actor,physical,real(rdram,physical+0x14),0,calls,0};
    } else if(gravity) {
        gravity->after=real(rdram,gravity->physical+0x14);gravity->collision=int32_t(ctx->r2);
    }
}
namespace {
std::pair<float,float> sample_period_motion(const Period& v,double age,unsigned axis) {
    const auto* owner=&v.owner;const auto actor=owner->actor;
    // Keep the actual Source endpoint bits even after live attachment fields
    // have been evaluated for an earlier Native slice.
    if(age==1)return {v.source_endpoint_position[axis],v.velocity[axis]};
    if(v.collision_hold[axis])return {*v.collision_hold[axis],v.velocity[axis]};
    // RealT1's Source tail grounds/corrects the child separately from its
    // root. Carry that Source offset through the same parent motion on every
    // slice; the pure attachment prefix alone loses it on slopes/contacts.
    const bool brobo_part=owner->rom==0xD5D10&&owner->kind==349;
    if((v.historical_attachment||brobo_part)&&v.attachment_motion&&v.attachment_axes[axis]) {
        if(age==1)return {v.source_endpoint_position[axis],v.velocity[axis]};
        const auto parent=sample_period_motion(*v.attachment_motion,v.attachment_start_age+age,axis);
        return {float(double(parent.first)+v.attachment_offsets[axis]+double(v.velocity[axis])*age),v.velocity[axis]};
    }
    double displacement=double(v.velocity[axis])*age;float velocity=v.velocity[axis];
    if(axis==1&&v.gravity&&v.gravity->collision==0) {
        // Source E810C already applied its cap. Extend that actual kick, with
        // h==1 returning exactly the real Source velocity/displacement.
        // E810C normalizes an externally supplied velocity below its cap
        // immediately. Do not spread that normalization over a whole period.
        const float before=v.gravity->after==-48.f?std::max(v.gravity->before,-48.f):v.gravity->before;
        const auto step=legacy_kinematic_step(before,double(v.gravity->after)-before,age);
        velocity=age==1?v.gravity->after:float(step.velocity);displacement=step.displacement;
    }
    if(owner->rom==0xE5480&&owner->kind==335&&v.projection&&v.projection->physical==owner->physical&&
       v.projection->parts[axis].size()==2&&std::bit_cast<int32_t>(float(v.projection->parts[axis][1].proposal+
            v.projection->parts[axis][0].proposal))==std::bit_cast<int32_t>(v.velocity[axis])) {
        displacement=0;double current=0;
        for(const auto& part:v.projection->parts[axis]) {
            double scalar=part.speed,ramp=0,yaw=part.heading,dyaw=0,ayaw=0,tilt=part.pitch,dtilt=0,atilt=0;
            if(part.scalar&&part.scalar->kind==EquationKind::linear){scalar=part.scalar->before;ramp=part.scalar->step;}
            if(part.heading_equation&&part.heading_equation->kind==EquationKind::angle){yaw=part.heading_equation->before;dyaw=part.heading_equation->step;ayaw=part.heading_equation->acceleration;}
            if(part.pitch_equation&&part.pitch_equation->kind==EquationKind::angle){tilt=part.pitch_equation->before;dtilt=part.pitch_equation->step;atilt=part.pitch_equation->acceleration;}
            if(!part.xyz&&axis!=1){tilt=0;dtilt=0;atilt=0;}
            yaw*=part.radians;dyaw*=part.radians;ayaw*=part.radians;
            tilt*=part.radians;dtilt*=part.radians;atilt*=part.radians;
            const auto sum=[&](double y,double p){return enemy_math::quadratic_harmonic_sum(y*yaw+p*tilt,
                y*dyaw+p*dtilt,y*ayaw+p*atilt,age,scalar,ramp,harmonic_sum);};
            const double speed=scalar+ramp*age,heading=yaw+dyaw*age+ayaw*age*(age-1)/2,
                pitch=tilt+dtilt*age+atilt*age*(age-1)/2;
            if(axis==1){displacement-=sum(0,1).imag();current-=speed*std::sin(pitch);}
            else {const auto path=(sum(1,1)+sum(1,-1))/2.;displacement+=axis==0?path.imag():path.real();
                current+=speed*std::cos(pitch)*(axis==0?std::sin(heading):std::cos(heading));}
        }
        velocity=age==1?v.velocity[axis]:float(current);
    }
    const bool corrected_projection=v.projection&&v.projection->physical==owner->physical&&v.projection->calls[axis]==1&&
        std::bit_cast<int32_t>(float(v.projection->proposal[axis]+v.projection->correction[axis]))==std::bit_cast<int32_t>(v.velocity[axis]);
    if(corrected_projection) {
        const auto& projected=*v.projection;
        double speed=projected.speed[axis],acceleration=0;
        double heading=projected.heading[axis],heading_step=0,heading_acceleration=0,pitch=projected.pitch[axis],pitch_step=0,pitch_acceleration=0;
        std::array<std::optional<Equation>,3> captured{
            projected.scalar_equation[axis],projected.heading_equation[axis],projected.pitch_equation[axis]};
        for(const auto& item:captured) {
            if(!item)continue;
            const auto& equation=*item;
            if(equation.address==owner->physical+projected.scalar_offset[axis]&&equation.kind==EquationKind::linear&&
               equation.after==projected.speed[axis]) {
                speed=equation.before;acceleration=double(equation.after)-equation.before;
                const float first=projected.negate[axis]?-equation.before:
                    float(equation.before*projected.first_factor[axis]);
                const float before=float(first*projected.unit[axis]);
                const auto step=legacy_kinematic_step(before,double(projected.proposal[axis])-before,age);
                velocity=age==1?v.velocity[axis]:float(step.velocity);displacement=step.displacement;
            }
            if(equation.kind==EquationKind::angle&&equation.address==owner->physical+4&&equation.after==projected.heading[axis]) {
                heading=equation.before;heading_step=equation.step;heading_acceleration=equation.acceleration;
            }
            if(equation.kind==EquationKind::angle&&equation.address==owner->physical+0xC&&equation.after==projected.pitch[axis]) {
                pitch=equation.before;pitch_step=equation.step;pitch_acceleration=equation.acceleration;
            }
        }
        if(heading_step==0&&v.turn[1]!=0&&projected.heading[axis]==v.proposal_yaw[1]) {
            heading=v.initial_yaw[1];heading_step=v.turn[1];
        }
        auto yaw_wave=projected.heading_equation[axis];
        if(yaw_wave&&yaw_wave->kind==EquationKind::sine_drift_angle) {
            // Sum A*sin(phi+w*j) is a cosine wave plus a constant. This
            // identity preserves the discrete recurrence, not a chord.
            const double omega=yaw_wave->step*yaw_wave->radians;
            const double half=std::sin(omega/2);
            if(std::abs(half)>1e-12&&yaw_wave->upstream&&yaw_wave->upstream->kind==EquationKind::angle) {
                const auto input=*yaw_wave;const double amplitude=input.amplitude/(2*half);
                yaw_wave->kind=EquationKind::sine_angle;
                yaw_wave->base=input.upstream->before+amplitude*std::cos(input.phase*input.radians+omega/2);
                yaw_wave->amplitude=amplitude;
                yaw_wave->phase=input.phase*input.radians+omega/2-std::acos(-1.)/2;
                yaw_wave->step=omega;yaw_wave->radians=1;
                heading_step=input.upstream->step;heading_acceleration=input.upstream->acceleration;
            }
        }
        const auto& pitch_wave=projected.pitch_equation[axis];
        const bool has_yaw_wave=yaw_wave&&yaw_wave->kind==EquationKind::sine_angle;
        const bool has_pitch_wave=pitch_wave&&pitch_wave->kind==EquationKind::sine_angle;
        const bool coupled_wave=projected.xyz[axis]&&has_yaw_wave&&has_pitch_wave;
        const bool pitch_approach=pitch_wave&&pitch_wave->kind==EquationKind::wrapped_approach;
        const auto& speed_wave=projected.scalar_equation[axis];
        const bool sinusoidal_speed=speed_wave&&speed_wave->kind==EquationKind::sine;
        const bool exponential_speed=speed_wave&&speed_wave->kind==EquationKind::exponential;
        const bool heading_approach=yaw_wave&&yaw_wave->kind==EquationKind::wrapped_approach;
        if(axis==1&&speed_wave&&speed_wave->kind==EquationKind::forced_sine_velocity) {
            // Banyatto hover: continue Y += gain*(floor+wave-Y), including
            // the captured Source gain choice and actual E7FC4 projection.
            displacement=(speed_wave->forced_position(age)-speed_wave->initial_position)*(-projected.unit[axis]);
            velocity=age==1?v.velocity[axis]:float(speed_wave->evaluate(age)*(-projected.unit[axis]));
        } else if(heading_approach&&has_pitch_wave&&projected.xyz[axis]) {
            const double radians=projected.radians[axis];
            const enemy_math::ExponentialHeadingWavePitchProjection path{
                yaw_wave->base*radians,(double(yaw_wave->before)-yaw_wave->base)*radians,yaw_wave->retention,
                pitch_wave->base*radians,pitch_wave->amplitude*radians,
                pitch_wave->phase*pitch_wave->radians,pitch_wave->step*pitch_wave->radians};
            displacement=path.displacement(axis,age,speed,acceleration);
            velocity=age==1?v.velocity[axis]:float(path.velocity(axis,age,speed,acceleration));
        } else if(has_pitch_wave&&!has_yaw_wave&&!heading_approach&&(axis==1||projected.xyz[axis])) {
            const double radians=projected.radians[axis];
            const enemy_math::WaveProjection path{heading*radians,pitch_wave->base*radians,0,
                pitch_wave->amplitude*radians,pitch_wave->phase*pitch_wave->radians,
                pitch_wave->step*pitch_wave->radians,heading_step*radians};
            const auto weighted_sum=[&](double phi,double omega,double h,double scalar,double ramp) {
                if(!sinusoidal_speed)return harmonic_sum(phi,omega,h,scalar,ramp);
                const double phase=speed_wave->phase*speed_wave->radians;
                const double step=speed_wave->step*speed_wave->radians;
                return harmonic_sum(phi,omega,h,speed_wave->base,0)+
                    (harmonic_sum(phi+phase,omega+step,h,speed_wave->amplitude,0)-
                     harmonic_sum(phi-phase,omega-step,h,speed_wave->amplitude,0))/std::complex<double>(0,2);
            };
            displacement=path.displacement(axis,age,speed,acceleration,weighted_sum);
            velocity=age==1?v.velocity[axis]:float(path.velocity(axis,age,sinusoidal_speed?speed_wave->evaluate(age):speed,
                sinusoidal_speed?0:acceleration));
        } else if(has_yaw_wave&&!has_pitch_wave&&axis!=1) {
            const double radians=projected.radians[axis];
            const enemy_math::WaveProjection path{yaw_wave->base*radians,projected.xyz[axis]?pitch*radians:0,
                yaw_wave->amplitude*radians,0,yaw_wave->phase*yaw_wave->radians,
                yaw_wave->step*yaw_wave->radians,heading_step*radians};
            displacement=path.displacement(axis,age,speed,acceleration,harmonic_sum);
            velocity=age==1?v.velocity[axis]:float(path.velocity(axis,age,speed,acceleration));
        } else if(heading_approach&&axis!=1&&!projected.xyz[axis]) {
            // Sakana's XZ shortest-angle recurrence has the same exponential
            // angle series as pitch; X=sin, Z=cos, with the actual speed ramp.
            const double radians=projected.radians[axis];
            const enemy_math::ExponentialPitchProjection path{0,0,yaw_wave->base*radians,
                (double(yaw_wave->before)-yaw_wave->base)*radians,yaw_wave->retention};
            displacement=axis==0?-path.displacement(1,age,speed,acceleration):path.displacement(2,age,speed,acceleration);
            velocity=age==1?v.velocity[axis]:float(axis==0?-path.velocity(1,age,speed,acceleration):path.velocity(2,age,speed,acceleration));
        } else if(exponential_speed&&!has_yaw_wave&&!has_pitch_wave&&!pitch_approach&&!heading_approach) {
            const double radians=projected.radians[axis];
            const double ratio=speed_wave->retention;
            const auto sum=[&](double phase,double omega) {
                const std::complex<double> logarithm(std::log(ratio),omega);
                const auto carrier=std::exp(logarithm),denominator=1.-carrier;
                const auto count=std::abs(denominator)<1e-12?std::complex<double>(age,0):
                    (1.-std::exp(logarithm*age))/denominator;
                return std::polar(double(speed_wave->before),phase)*carrier*count;
            };
            heading*=radians;heading_step*=radians;pitch*=radians;pitch_step*=radians;
            const double scalar=speed_wave->evaluate(age);double current=0;
            if(axis==1){displacement=-sum(pitch,pitch_step).imag();current=-scalar*std::sin(pitch+pitch_step*age);}
            else if(projected.xyz[axis]) {
                const auto path=(sum(heading+pitch,heading_step+pitch_step)+sum(heading-pitch,heading_step-pitch_step))/2.;
                displacement=axis==0?path.imag():path.real();
                current=scalar*std::cos(pitch+pitch_step*age)*(axis==0?std::sin(heading+heading_step*age):std::cos(heading+heading_step*age));
            } else {const auto path=sum(heading,heading_step);displacement=axis==0?path.imag():path.real();
                current=scalar*(axis==0?std::sin(heading+heading_step*age):std::cos(heading+heading_step*age));}
            velocity=age==1?v.velocity[axis]:float(current);
        } else if(sinusoidal_speed&&!has_yaw_wave&&!has_pitch_wave&&!pitch_approach) {
            // Tolneder: scalar15+10*sin(short phase), coupled to the actual
            // heading consumed by the projection. Integrate the product's
            // harmonic sidebands over the same kick-before-drift interval.
            const double radians=projected.radians[axis];
            const double phase=speed_wave->phase*speed_wave->radians;
            const double omega=speed_wave->step*speed_wave->radians;
            heading*=radians;heading_step*=radians;pitch*=radians;pitch_step*=radians;
            const auto sum=[&](double angle_phase,double angle_step) {
                return harmonic_sum(angle_phase,angle_step,age,speed_wave->base,0)+
                    (harmonic_sum(angle_phase+phase,angle_step+omega,age,speed_wave->amplitude,0)-
                     harmonic_sum(angle_phase-phase,angle_step-omega,age,speed_wave->amplitude,0))/std::complex<double>(0,2);
            };
            const double current_speed=speed_wave->evaluate(age);
            double current=0;
            if(axis==1) {
                displacement=-sum(pitch,pitch_step).imag();current=-current_speed*std::sin(pitch+pitch_step*age);
            } else if(projected.xyz[axis]) {
                if(axis==0)displacement=(sum(heading+pitch,heading_step+pitch_step).imag()+sum(heading-pitch,heading_step-pitch_step).imag())/2;
                else displacement=(sum(heading+pitch,heading_step+pitch_step).real()+sum(heading-pitch,heading_step-pitch_step).real())/2;
                current=current_speed*std::cos(pitch+pitch_step*age)*(axis==0?std::sin(heading+heading_step*age):std::cos(heading+heading_step*age));
            } else {
                const auto result=sum(heading,heading_step);displacement=axis==0?result.imag():result.real();
                current=current_speed*(axis==0?std::sin(heading+heading_step*age):std::cos(heading+heading_step*age));
            }
            velocity=age==1?v.velocity[axis]:float(current);
        } else if(pitch_approach&&(axis==1||projected.xyz[axis])) {
            // Oppi's real recurrence is pitch += shortest(300-pitch)*.05.
            // Continue its exponential operand in both angle and displacement;
            // a linear interpolation of two completed poses loses that coupling.
            const double radians=projected.radians[axis];
            const enemy_math::ExponentialPitchProjection path{
                heading*radians,heading_step*radians,pitch_wave->base*radians,
                (double(pitch_wave->before)-pitch_wave->base)*radians,pitch_wave->retention};
            displacement=path.displacement(axis,age,speed,acceleration);
            velocity=age==1?v.velocity[axis]:float(path.velocity(axis,age,speed,acceleration));
        } else if(coupled_wave) {
            const double radians=projected.radians[axis];
            if(yaw_wave->phase==pitch_wave->phase&&yaw_wave->step==pitch_wave->step&&yaw_wave->radians==pitch_wave->radians) {
                const enemy_math::WaveProjection path{
                    yaw_wave->base*radians,pitch_wave->base*radians,
                    yaw_wave->amplitude*radians,pitch_wave->amplitude*radians,
                    yaw_wave->phase*yaw_wave->radians,yaw_wave->step*yaw_wave->radians};
                displacement=path.displacement(axis,age,speed,acceleration,harmonic_sum);
                velocity=age==1?v.velocity[axis]:float(path.velocity(axis,age,speed,acceleration));
            } else {
                const enemy_math::IndependentWaveProjection path{
                    yaw_wave->base*radians,pitch_wave->base*radians,
                    yaw_wave->amplitude*radians,pitch_wave->amplitude*radians,
                    yaw_wave->phase*yaw_wave->radians,yaw_wave->step*yaw_wave->radians,
                    pitch_wave->phase*pitch_wave->radians,pitch_wave->step*pitch_wave->radians};
                displacement=path.displacement(axis,age,speed,acceleration,harmonic_sum);
                velocity=age==1?v.velocity[axis]:float(path.velocity(axis,age,speed,acceleration));
            }
        } else if((axis!=1&&(heading_step!=0||heading_acceleration!=0))||((axis==1||projected.xyz[axis])&&(pitch_step!=0||pitch_acceleration!=0))) {
            const double radians=projected.radians[axis];
            heading*=radians;heading_step*=radians;heading_acceleration*=radians;
            pitch*=radians;pitch_step*=radians;pitch_acceleration*=radians;
            // The same angular kick drives visible orientation AND projection.
            // A104 and Bug must not lose acceleration in nonzero-speed paths.
            const auto sum=[&](double phase,double step,double angular_acceleration){return enemy_math::quadratic_harmonic_sum(phase,step,angular_acceleration,age,speed,acceleration,harmonic_sum);};
            const double current_speed=speed+acceleration*age;
            double current=0;
            if(axis==1) {
                displacement=-sum(pitch,pitch_step,pitch_acceleration).imag();current=-current_speed*std::sin(pitch+pitch_step*age+pitch_acceleration*age*(age-1)/2);
            } else if(projected.xyz[axis]) {
                if(axis==0)displacement=(sum(heading+pitch,heading_step+pitch_step,heading_acceleration+pitch_acceleration).imag()+sum(heading-pitch,heading_step-pitch_step,heading_acceleration-pitch_acceleration).imag())/2;
                else displacement=(sum(heading+pitch,heading_step+pitch_step,heading_acceleration+pitch_acceleration).real()+sum(heading-pitch,heading_step-pitch_step,heading_acceleration-pitch_acceleration).real())/2;
                current=current_speed*std::cos(pitch+pitch_step*age+pitch_acceleration*age*(age-1)/2)*(axis==0?std::sin(heading+heading_step*age+heading_acceleration*age*(age-1)/2):std::cos(heading+heading_step*age+heading_acceleration*age*(age-1)/2));
            } else {
                const auto result=sum(heading,heading_step,heading_acceleration);displacement=axis==0?result.imag():result.real();
                current=current_speed*(axis==0?std::sin(heading+heading_step*age+heading_acceleration*age*(age-1)/2):std::cos(heading+heading_step*age+heading_acceleration*age*(age-1)/2));
            }
            velocity=age==1?v.velocity[axis]:float(current);
        }
    }
    if(corrected_projection&&v.projection->correction[axis]!=0) {
        displacement+=v.projection->correction[axis]*age;
        if(age!=1)velocity=float(double(velocity)+v.projection->correction[axis]);
    }
    float initial=v.initial_position[axis];
    for(const auto& equation:v.equations)if(equation.address==actor+0x9C+4*axis)initial=equation.evaluate(age);
    displacement+=v.constraint_correction[axis]*(age-1);
    const float position=age==1?float(initial+v.velocity[axis]):float(double(initial)+displacement);
    return {position,velocity};
}
}
int native_enemy_axis(uint8_t* rdram,recomp_context* ctx,uint32_t site) {
    dora3::profile::Scope detailProfile(dora3::profile::Tag::EnemyAxisSample,dora3::profile::sample_stage(dora3::profile::Tag::EnemyAxisSample,16));
    if(!native_gameplay_preview_object_pass()||!active_simulation_slice())return 0;
    const auto actor=uint32_t(word(rdram,0x8021BDEC));const auto owner=identity(rdram,actor);if(!owner)return 0;
    auto& stored=periods[index(actor)];if(!stored)return 0;
    auto& v=*stored;const auto& slice=*active_simulation_slice();
    if(!same(v.owner,*owner)||v.tick!=slice.tick||v.denominator!=slice.denominator)return 0;
    const unsigned axis=site==0x801E8E00?0:site==0x801E8E10?1:2;
    const double age=double(slice.phase+slice.numerator-v.start_phase)/double(slice.denominator);

    if(axis==0) {
        for(auto& step:v.foreign_steps)if(foreign_identity(rdram,step)) {
            const float actual=real(rdram,step.actor+step.offset);
            if(!std::isfinite(actual))continue;
            const bool uninterrupted=std::bit_cast<int32_t>(actual)==std::bit_cast<int32_t>(step.expected);
            const double foreign_age=double(slice.phase+slice.numerator-step.start_phase)/step.denominator;
            const float next=uninterrupted?(foreign_age==1?step.after:float(step.rebase+step.delta*foreign_age)):
                float(double(actual)+step.delta*(foreign_age-step.applied));
            if(!uninterrupted) {
                // Rebase the remaining additive displacement after a genuine
                // player physics/teleport write; do not restore a saved pose.
                step.rebase=double(actual)-step.delta*step.applied;
                step.after=float(step.rebase+step.delta);
            }
            put(rdram,step.actor+step.offset,next);step.expected=next;step.applied=foreign_age;
        }
        for(const auto& equation:v.equations)put(rdram,equation.address,equation.evaluate(age));
        if(v.opacity) {
            const auto alpha=v.opacity->evaluate(age);put(rdram,owner->model+0x78,alpha);
            MEM_W(0x74,int32_t(owner->model))=alpha==1.f?0:1;
            MEM_B(0x2A,int32_t(owner->physical))=alpha>=1.f?0:1;
            v.opacity->expected=alpha;
        }
        const auto kind=MEM_H(0x20,int32_t(owner->physical));
        if(owner->rom==0x70A50&&kind==261) {
            // E5A94 copies the player XYZ cached at the start of this pool
            // pass. Re-publish only that geometry on Native parts; the real
            // clip selection/end/destruction commands remain once per period.
            constexpr std::array<uint32_t,3> cached_player{0x8021BDE8,0x8021BDF4,0x8021BCE0};
            for(unsigned j=0;j<3;++j) {
                v.initial_position[j]=real(rdram,cached_player[j]);
                put(rdram,actor+0x9C+4*j,v.initial_position[j]);
            }
        }


        // Native attachment consumes the same Source-order parent path.
        // Restoring temporary fields is guarded against genuine guest writes.
        struct SceneParentPose {
            uint8_t* ram;std::optional<Identity> owner;
            struct Field {uint32_t address;int32_t saved,published;};
            std::vector<Field> fields;
            void publish(uint32_t address,float value) {
                uint8_t* rdram=ram;const auto bits=std::bit_cast<int32_t>(value);
                fields.push_back({address,word(rdram,address),bits});MEM_W(0,int32_t(address))=bits;
            }
            SceneParentPose(uint8_t* rdram,const Period& child,double age):ram(rdram) {
                if(!child.parent||!child.scene_parent_motion)return;
                owner=identity(rdram,child.parent->actor);
                if(!owner||!same(*owner,*child.parent)||!same(child.scene_parent_motion->owner,*owner)){owner.reset();return;}
                const auto& path=*child.scene_parent_motion;
                for(unsigned axis=0;axis<3;++axis)publish(owner->actor+0x9C+4*axis,sample_period_motion(path,age,axis).first);
                for(unsigned field=0;field<4;++field) {
                    const uint32_t address=field==3?owner->physical+4:owner->actor+0xC+4*field;
                    float value=field==1?(age==1?path.proposal_yaw[0]:path.turn[0]==0?path.initial_yaw[0]:angle(float(double(path.initial_yaw[0])+path.turn[0]*age))):
                        field==3?(age==1?path.proposal_yaw[1]:path.turn[1]==0?path.initial_yaw[1]:angle(float(double(path.initial_yaw[1])+path.turn[1]*age))):path.source_endpoint_angles[field];
                    for(const auto& equation:path.equations)if(equation.address==address)value=equation.evaluate(age);
                    publish(address,value);
                }
            }
            ~SceneParentPose() {
                if(!owner)return;
                const auto current=identity(ram,owner->actor);
                if(!current||!same(*current,*owner))return;
                uint8_t* rdram=ram;
                for(const auto& field:fields)if(word(rdram,field.address)==field.published)MEM_W(0,int32_t(field.address))=field.saved;
            }
        } scene_parent_pose(rdram,v,double(slice.phase+slice.numerator-v.scene_parent_start_phase)/slice.denominator);
        if(v.parent&&scene_parent_attachment_active(rdram,*owner)&&
           MEM_H(0x7C,int32_t(owner->physical))==int((v.parent->actor-0x804B1CD0u)/stride))
        if(const auto parent=identity(rdram,v.parent->actor);parent&&same(*parent,*v.parent)) {
            if(kind==133) {
                for(unsigned j=0;j<3;++j) {
                    const float offset=j==1?(MEM_H(0x7E,int32_t(owner->physical))?300.f:150.f):0;
                    v.initial_position[j]=float(real(rdram,parent->actor+0x9C+4*j)+offset);
                    put(rdram,actor+0x9C+4*j,v.initial_position[j]);
                }
            } else if(kind==527) {
                v.initial_position[1]=float(real(rdram,parent->actor+0xA0)+v.attachment_offsets[1]);
                put(rdram,actor+0xA0,v.initial_position[1]);
            } else if(kind==528) {
                const float heading=real(rdram,parent->actor+0x10);
                put(rdram,actor+0x10,heading);v.initial_yaw[0]=v.proposal_yaw[0]=heading;v.turn[0]=0;
            } else if(kind==564) {
                // ED3C after its initialized latch is pure Original roll +
                // translation matrix geometry, with four real local anchors.
                recomp_context pose;dora3::copy_recomp_context(pose,*ctx);
                struct PoseOnly {PoseOnly(){attachment_only=true;}~PoseOnly(){attachment_only=false;}} pose_only;
                {SimulationSuspension source;v.update(rdram,&pose);}
                for(unsigned j=0;j<3;++j)v.initial_position[j]=real(rdram,actor+0x9C+4*j);
            }
        }
        if(v.parent&&((((kind==268||kind==299||kind==292||kind==333)&&MEM_H(0x36,int32_t(owner->physical))==0)||((kind==273||kind==324||kind==285||kind==286)&&word(rdram,actor+0x94)==1)||(kind==329&&word(rdram,actor+0x94)==1&&MEM_H(0x34,int32_t(owner->physical))==0))||((kind==361||kind==338||kind==349||kind==353||kind==405)&&word(rdram,actor+0x94)==1)||(kind==401&&word(rdram,actor+0x94)!=10)||(kind==403&&(word(rdram,actor+0x94)==1||
            (word(rdram,actor+0x94)==10&&MEM_H(0x34,int32_t(owner->physical))==0)))||enemy_parent_attachment_active(rdram,*owner))&&
           attachment_parent_slot(rdram,*owner)==int((v.parent->actor-0x804B1CD0u)/stride))
        if(const auto parent=identity(rdram,v.parent->actor);parent&&same(*parent,*v.parent)) {
            // Execute the reviewed Original attachment geometry only. Its
            // constructor, animation, counters and collision paths are bypassed.
            // RealT1 adds its parent's Y delta while jumping. Its saved58
            // is the Source endpoint; restart from the immutable child
            // Source anchor so repeated pose-only calls cannot accumulate it.
            if(kind==349)put(rdram,actor+0xA0,v.attachment_source_position[1]);
            if(enemy_parent_attachment_active(rdram,*owner)) {
                const auto copy_field=[&](unsigned offset){put(rdram,actor+offset,real(rdram,parent->actor+offset));};
                if(owner->rom==0x11D570) {
                    for(const unsigned offset:{0x9Cu,0xA0u,0xA4u})copy_field(offset);
                    if(kind==473) {copy_field(0x10);copy_field(0xC);}
                } else if(owner->rom==0x111E70) {
                    if(kind==464||kind==465) {
                        recomp_context pose;dora3::copy_recomp_context(pose,*ctx);
                        struct PoseOnly {PoseOnly(){attachment_only=true;}~PoseOnly(){attachment_only=false;}} pose_only;
                        {SimulationSuspension source;v.update(rdram,&pose);}
                    } else if(kind==467) {
                        const double theta=(real(rdram,owner->physical+0x5C)+real(rdram,parent->actor+0x10))*rom_double(rdram,0x80228AA0);
                        const double radius=real(rdram,owner->physical+0x58);
                        put(rdram,actor+0x9C,float(real(rdram,parent->actor+0x9C)+radius*std::sin(theta)));
                        put(rdram,actor+0xA4,float(real(rdram,parent->actor+0xA4)+radius*std::cos(theta)));
                        const auto wave=std::find_if(v.equations.begin(),v.equations.end(),[&](const auto& e){return e.address==actor+0xA0;});
                        if(wave!=v.equations.end())put(rdram,actor+0xA0,
                            wave->kind==EquationKind::parent_height_wave?wave->evaluate(age):
                            float(wave->evaluate(age)-wave->base+real(rdram,parent->actor+0xA0)-real(rdram,owner->physical+0x60)));
                        copy_field(0x10);
                    } else {
                        copy_field(0x10);
                        if(kind!=459||MEM_H(0x44,int32_t(owner->physical))>=3) {
                            for(const unsigned offset:{0x9Cu,0xA0u,0xA4u})copy_field(offset);
                            if((kind==455||kind==456)&&word(rdram,actor+0x94)==1)
                                put(rdram,actor+0xA0,real(rdram,parent->actor+0xA0)-(kind==455?73.f:158.f));
                        }
                        if(kind==469) {copy_field(0xC);copy_field(0x14);}
                        if((kind==459||kind==469)&&MEM_H(0x44,int32_t(owner->physical))==2)
                            put(rdram,actor+0xA0,real(rdram,parent->actor+0xA0)-330.f);
                    }
                } else if(owner->rom==0x10DA30) {
                    if(kind==447) {
                        for(const unsigned offset:{0x9Cu,0xA0u,0xA4u,0x10u,0xCu})copy_field(offset);
                    } else {
                        // Pure Original matrix prefix only. Laser steering,
                        // beam acceleration, clip commands and fade stay Source.
                        recomp_context pose;dora3::copy_recomp_context(pose,*ctx);
                        struct PoseOnly {PoseOnly(){attachment_only=true;}~PoseOnly(){attachment_only=false;}} pose_only;
                        {SimulationSuspension source;v.update(rdram,&pose);}
                    }
                } else if(kind==441) {
                    // Source machine follows XYZ/yaw/pitch, never roll.
                    for(const unsigned offset:{0x9Cu,0xA0u,0xA4u,0x10u,0xCu})copy_field(offset);
                } else if(kind==423) {
                    // Orbit displacement uses the head's current yaw, with
                    // its own Source radius and angle offset. Y has its own wave.
                    const double theta=(real(rdram,owner->physical+0x5C)+real(rdram,parent->actor+0x10))*rom_double(rdram,0x802293F0);
                    const double radius=real(rdram,owner->physical+0x58);
                    put(rdram,actor+0x9C,float(real(rdram,parent->actor+0x9C)+radius*std::sin(theta)));
                    put(rdram,actor+0xA4,float(real(rdram,parent->actor+0xA4)+radius*std::cos(theta)));
                    copy_field(0x10);
                } else if(kind==420||kind==421) {
                    recomp_context pose;dora3::copy_recomp_context(pose,*ctx);
                    struct PoseOnly {PoseOnly(){attachment_only=true;}~PoseOnly(){attachment_only=false;}} pose_only;
                    {SimulationSuspension source;v.update(rdram,&pose);}
                } else {
                    copy_field(0x10);
                    if(kind!=415||MEM_H(0x44,int32_t(owner->physical))>=3) {
                        for(unsigned j=0;j<3;++j)copy_field(0x9C+4*j);
                        if((kind==411||kind==412)&&word(rdram,actor+0x94)==1)
                            put(rdram,actor+0xA0,real(rdram,parent->actor+0xA0)-(kind==411?73.f:158.f));
                    }
                    if(kind==415&&MEM_H(0x44,int32_t(owner->physical))==2)
                        put(rdram,actor+0xA0,real(rdram,parent->actor+0xA0)-330.f);
                    if(kind==425) {
                        copy_field(0xC);copy_field(0x14);
                        if(MEM_H(0x44,int32_t(owner->physical))==2)
                            put(rdram,actor+0xA0,real(rdram,parent->actor+0xA0)-330.f);
                    }
                }
            } else if(kind==405) {
                // Boss1Bironn state1 uses FF20:20054 as pure attachment
                // geometry. Its reviewed entry bypasses FF1C's lui a0,8022;
                // provide that input and call the geometry body directly.
                // The root callback's 25-degree turn remains once per Source
                // command, already captured in this period's yaw equation.
                recomp_context pose;dora3::copy_recomp_context(pose,*ctx);
                pose.r4=int32_t(0x80220000u);
                struct PoseOnly {PoseOnly(){attachment_only=true;}~PoseOnly(){attachment_only=false;}} pose_only;
                {SimulationSuspension source;dora3_s33_remaining_8021FED8(rdram,&pose);}
            } else if(kind==403) {
                // Charge copies only yaw; the linked ending phase copies XYZ
                // too. The same Source region also resets parent36 for id5,
                // so repeat these proven copies without replaying that command.
                if(word(rdram,actor+0x94)==10)for(unsigned j=0;j<3;++j)
                    put(rdram,actor+0x9C+4*j,real(rdram,parent->actor+0x9C+4*j));
                put(rdram,actor+0x10,real(rdram,parent->actor+0x10));
            } else {
                recomp_context pose;dora3::copy_recomp_context(pose,*ctx);
                struct PoseOnly {PoseOnly(){attachment_only=true;}~PoseOnly(){attachment_only=false;}} pose_only;
                {SimulationSuspension source;v.update(rdram,&pose);}
            }
            if(kind==299||kind==292)put(rdram,actor+0x10,real(rdram,owner->physical+4));
            for(unsigned j=0;j<3;++j)v.initial_position[j]=real(rdram,actor+0x9C+4*j);
            // Attachment owns both headings at this instant. The old period
            // proposal must not overwrite its newly evaluated parent pose.
            if(owner->rom==0xD5D10&&kind==353) {
                // Hob's Source command shifted the six-entry heading queue
                // after publishing its OLD head (206F8, then 20710..20728).
                // The pose-only prefix copies the now NEW head over the
                // fractional equation. Restore the captured old-head path;
                // otherwise every Native slice shows a future stepped yaw.
                for(const auto& equation:v.equations)
                    if(equation.address==actor+0x10)put(rdram,equation.address,equation.evaluate(age));
            }
            v.initial_yaw=v.proposal_yaw={real(rdram,actor+0x10),real(rdram,owner->physical+4)};
            v.turn={};
        }
    }
    const auto [position,velocity]=sample_period_motion(v,age,axis);
    put(rdram,owner->physical+0x10+4*axis,velocity);ctx->f0.fl=position;ctx->f2.fl=0;
    if(axis==0)for(unsigned angle_axis=0;angle_axis<2;++angle_axis) {
        const auto yaw_address=angle_axis?owner->physical+4:actor+0x10;
        if(std::any_of(v.equations.begin(),v.equations.end(),[&](const auto& equation){return equation.address==yaw_address;}))continue;
        const float yaw=age==1||v.turn[angle_axis]==0?v.proposal_yaw[angle_axis]:
            angle(float(double(v.initial_yaw[angle_axis])+double(v.turn[angle_axis])*age));
        put(rdram,angle_axis?owner->physical+4:actor+0x10,yaw);
    }
    return 1;
}
void native_enemy_motion_end(uint8_t* rdram,uint32_t actor) {
    if(!native_gameplay_preview_object_pass()||actor<pool||actor>=pool+dora3::distance::common_capacity*stride||(actor-pool)%stride||!active_simulation_slice())return;
    auto& stored=periods[index(actor)];if(!stored)return;
    const auto owner=identity(rdram,actor);if(!owner||!same(stored->owner,*owner)){stored.reset();return;}
    auto& v=*stored;const auto& slice=*active_simulation_slice();
    if(v.tick!=slice.tick||v.denominator!=slice.denominator)return;
    for(unsigned axis=0;axis<3;++axis) {
        v.expected_position[axis]=real(rdram,actor+0x9C+4*axis);v.expected_velocity[axis]=real(rdram,owner->physical+0x10+4*axis);
    }
    v.expected_yaw={real(rdram,actor+0x10),real(rdram,owner->physical+4)};
    for(auto& equation:v.equations)equation.expected=real(rdram,equation.address);
    v.expected_phase=slice.phase+slice.numerator;v.command=commands(rdram,*owner);v.run=game_model_animation().run_token(owner->model);
    v.callback_opacity_flag=MEM_BU(0x2A,int32_t(owner->physical));
#ifdef DORA3_CRASH_LOG
    log_boss5_turn(rdram,*owner,"motion-end");
    log_sentoutei_motion(rdram,*owner,"motion-end");
    log_boss6_form(rdram,*owner,"motion-end");
#endif
}
// The real draw helper writes actor pitch/yaw to face the current camera.
// Those visual writes must not look like an external movement command on
// the next Native slice, which would discard growth/fade equations. Preserve
// the ROM writes and only acknowledge them in the live period's guard.
extern "C" int dora3_native_enemy_billboard(uint8_t* rdram,recomp_context* ctx,void (*draw)(uint8_t*,recomp_context*)) {
    static bool inside=false;
    if(inside||!active_frame_binding())return 0;
    const auto actor=uint32_t(ctx->r4);const auto owner=identity(rdram,actor);
    if(!owner)return 0;
    auto& stored=periods[index(actor)];
    if(!stored||!same(stored->owner,*owner)||!unchanged(rdram,*stored))return 0;
    struct DrawScope {bool& flag;DrawScope(bool& f):flag(f){flag=true;}~DrawScope(){flag=false;}} scope(inside);
    draw(rdram,ctx);
    // 801E8ECC -> 80031B38 writes only actor+C and actor+10. Physics yaw,
    // position, scale, counters and the Original function remain untouched.
    stored->expected_yaw[0]=real(rdram,actor+0x10);
    for(auto& equation:stored->equations)
        if(equation.address==actor+0xC||equation.address==actor+0x10)
            equation.expected=real(rdram,equation.address);
    return 1;
}
extern "C" int dora3_native_enemy_blink_skip(uint8_t* rdram) {
    return native_gameplay_preview_object_pass()&&active_simulation_slice()&&!active_simulation_slice()->ends_tick()&&
        (native_enemy_owned(rdram,uint32_t(word(rdram,0x8021BDEC)))||native_scene_object_owned(rdram,uint32_t(word(rdram,0x8021BDEC))));
}

// Objects share the proven once-per-Source-period command engine and its
// lifetime/epoch guards. Their eligibility and ROM registry remain separate
// from enemy AI; Original and unreviewed scene owners take their normal path.
extern "C" int dora3_native_scene_object_callback(uint8_t* rdram,recomp_context* ctx,uint32_t callback,uint32_t rom,void (*update)(uint8_t*,recomp_context*)) {
    const auto actor=uint32_t(word(rdram,0x8021BDEC));
    if(!native_scene_object_owned(rdram,actor))return 0;
    return dora3_native_enemy_callback(rdram,ctx,callback,rom,update);
}
