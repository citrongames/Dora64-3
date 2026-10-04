#include "native_enemy.hpp"
#include "native_model_animation.hpp"
#include "recomp_context_copy.hpp"
#include <array>
#include <algorithm>
#include <bit>
#include <cmath>
#include <complex>
#include <optional>
#include <stdexcept>
#include <vector>
#include <map>

extern "C" int dora3_overlay_loaded(uint32_t);
using namespace dora3::native_time;
namespace {
constexpr uint32_t pool=0x801734B8u;
constexpr uint32_t stride=0xCCu;
struct Definition {int kind;uint32_t rom,callback,contact;};
// Only these complete callback/contact closures join this first stage domain.
// Other ordinary enemies and bosses are added after their own command review.
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
    Definition{391,0x0EDDB0,0x80223880,0x80223878}
};
const Definition* definition(int kind) {
    for(const auto& v:definitions)if(v.kind==kind)return &v;
    return nullptr;
}
bool pointer(uint32_t p,unsigned size) {return !(p&3)&&p>=0x80000000u&&uint64_t(p)+size<=0x80400000ull;}
int32_t word(uint8_t* rdram,uint32_t p) {return MEM_W(0,int32_t(p));}
float real(uint8_t* rdram,uint32_t p) {return std::bit_cast<float>(word(rdram,p));}
void put(uint8_t* rdram,uint32_t p,float v) {MEM_W(0,int32_t(p))=std::bit_cast<int32_t>(v);}
double rom_double(uint8_t* rdram,uint32_t address) {
    return std::bit_cast<double>((uint64_t(uint32_t(word(rdram,address)))<<32)|uint32_t(word(rdram,address+4)));
}
unsigned index(uint32_t actor) {return (actor-pool)/stride;}
struct Identity {uint32_t actor,physical,model,callback,rom;uint64_t lifetime;int kind;};
std::optional<Identity> identity(uint8_t* rdram,uint32_t actor) {
    if(!rdram||actor<pool||actor>=pool+200*stride||(actor-pool)%stride||!word(rdram,actor+0x94))return std::nullopt;
    const auto physical=uint32_t(word(rdram,actor+0x98)),model=uint32_t(word(rdram,actor+0x74));
    if(!pointer(physical,0xDC)||!pointer(model,0x84))return std::nullopt;
    const auto kind=MEM_H(0x20,int32_t(physical));const auto* def=definition(kind);
    if(!def||!dora3_overlay_loaded(def->rom)||uint32_t(word(rdram,0x80202064u+unsigned(def->kind)*84))!=def->callback||
       uint32_t(word(rdram,0x80202068u+unsigned(def->kind)*84))!=def->contact)return std::nullopt;
    const auto lifetime=game_model_animation().lifetime_token(model);if(!lifetime)return std::nullopt;
    for(unsigned axis=0;axis<3;++axis)
        if(!std::isfinite(real(rdram,actor+0x9C+4*axis))||!std::isfinite(real(rdram,physical+0x10+4*axis)))return std::nullopt;
    return Identity{actor,physical,model,def->callback,def->rom,lifetime,kind};
}
bool same(const Identity& a,const Identity& b) {
    return a.actor==b.actor&&a.physical==b.physical&&a.model==b.model&&a.callback==b.callback&&a.rom==b.rom&&a.lifetime==b.lifetime&&a.kind==b.kind;
}
// Model time advances independently. State/latch/counter/callback commands
// invalidate a decision, while this module's own velocity/pose writes do not.
std::array<int32_t,15> commands(uint8_t* rdram,const Identity& v) {
    const auto kind=MEM_H(0x20,int32_t(v.physical));
    return {word(rdram,v.actor+0x94),MEM_BU(0x28,int32_t(v.physical)),MEM_BU(0x29,int32_t(v.physical)),
        MEM_H(0x34,int32_t(v.physical)),MEM_H(0x36,int32_t(v.physical)),MEM_H(0x38,int32_t(v.physical)),
        MEM_H(0x3A,int32_t(v.physical)),MEM_H(0x3C,int32_t(v.physical)),MEM_H(0x3E,int32_t(v.physical)),
        MEM_H(0x40,int32_t(v.physical)),MEM_H(0x42,int32_t(v.physical)),MEM_H(0x44,int32_t(v.physical)),
        (kind==268||kind==361)?MEM_H(0x7C,int32_t(v.physical)):0,
        (kind==267||kind==359||kind==360)?MEM_H(0x80,int32_t(v.physical)):0,word(rdram,v.physical+0x48)};
}
struct Gravity {uint32_t actor=0,physical=0;float before=0,after=0;unsigned calls=0;int collision=0;};
struct Turn {uint32_t actor=0,physical=0;float before=0,after=0;unsigned calls=0;};
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
};
// Captured only at reviewed stores. A setter elsewhere cannot accidentally
// acquire a continuous equation just because its numeric delta is small.
enum class EquationKind {linear,angle,sine,cosine,sine_angle,approach};
struct Equation {
    uint32_t address;EquationKind kind;float before,after;
    double phase=0,step=0,amplitude=0,radians=0,base=0,retention=0,acceleration=0;
    float expected=0;
    float evaluate(double age) const {
        if(age==1)return after;
        if(kind==EquationKind::sine||kind==EquationKind::cosine||kind==EquationKind::sine_angle) {
            float value=float(base+amplitude*(kind==EquationKind::cosine?
                std::cos((phase+step*age)*radians):std::sin((phase+step*age)*radians)));
            if(kind==EquationKind::sine_angle){value=std::fmod(value,360.f);if(value<0)value+=360.f;}
            return value;
        }
        if(kind==EquationKind::approach)return float(base+(double(before)-base)*std::pow(retention,age));
        const float value=float(double(before)+step*age+acceleration*age*(age-1)/2);
        if(kind!=EquationKind::angle)return value;
        const auto wrapped=std::fmod(value,360.f);return wrapped<0?wrapped+360.f:wrapped;
    }
};
// ROM is part of a store's identity: e.g. 8021E030 is Brobo scalar speed in
// demo_slot0 and MiniCar visual roll in level1_slot0. VRAM alone is insufficient.
struct Store {
    uint32_t rom,site,offset;bool physical;EquationKind kind;unsigned operand;
    uint32_t coefficient=0;double target=0;
};
constexpr std::array additional_stores{
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
struct Period {
    Identity owner;uint64_t tick,denominator,start_phase,expected_phase,run;
    std::array<int32_t,15> command;
    std::array<float,3> initial_position{},velocity{},expected_position{},expected_velocity{};
    std::array<float,2> initial_yaw{},turn{},proposal_yaw{},expected_yaw{};
    std::optional<Gravity> gravity{};
    std::vector<Equation> equations{};
    ModelEventCheckpoint model_events{};
    std::map<uint32_t,ModelEventCheckpoint> model_reads{};
    void (*update)(uint8_t*,recomp_context*)=nullptr;
    std::optional<Identity> parent{};
    int32_t acceleration=0;
    std::optional<Projection> projection{};
    std::optional<Equation> opacity{};
    int32_t callback_opacity_flag=0;
};
std::array<std::optional<Period>,200> periods;
bool executing=false;
bool attachment_only=false;
std::vector<Equation> equations;
std::map<uint32_t,ModelEventCheckpoint> model_reads;
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
    const auto rotation=std::polar(1.,phase+omega*(age+1)/2);
    if(std::abs(omega)<1e-6)
        return rotation*(speed*age+acceleration*age*(age+1)/2);
    const double half=omega/2,extent=age*half;
    const double ratio=std::sin(extent)/std::sin(half);
    const double derivative=(age*std::cos(extent)*std::sin(half)-std::sin(extent)*std::cos(half))/(2*std::sin(half)*std::sin(half));
    return rotation*std::complex<double>(ratio*(speed+acceleration*(age+1)/2),-acceleration*derivative);
}

// The Original common pool finishes an earlier parent's whole step before
// entering its child callback. Native visible motion has only finished this
// slice. Publish that parent's actual Source endpoint for the child's discrete
// decision, then restore visible fields; the pose-only path uses visible motion.
class SourceParentView {
    uint8_t* ram;
    std::optional<Identity> owner;
    struct Field {uint32_t address;int32_t saved,published;};
    std::vector<Field> fields;
    void publish(uint32_t address,float value) {
        uint8_t* rdram=ram;
        const auto bits=std::bit_cast<int32_t>(value);
        fields.push_back({address,word(rdram,address),bits});MEM_W(0,int32_t(address))=bits;
    }
public:
    SourceParentView(uint8_t* rdram,const Identity& child):ram(rdram) {
        const auto kind=MEM_H(0x20,int32_t(child.physical));
        if(kind!=268&&kind!=361)return;
        const int parent_slot=MEM_H(0x7C,int32_t(child.physical));
        if(parent_slot<30||parent_slot>=230)return;
        owner=identity(rdram,0x80171CD0u+unsigned(parent_slot)*stride);
        if(!owner||owner->actor>=child.actor)return;
        const auto& stored=periods[index(owner->actor)];
        if(!stored||!same(stored->owner,*owner))return;
        const auto& parent=*stored;
        // A real command/write after the earlier pool entry owns its fields.
        if(!unchanged(rdram,parent))return;
        for(unsigned axis=0;axis<3;++axis) {
            float initial=parent.initial_position[axis];
            for(const auto& equation:parent.equations)
                if(equation.address==owner->actor+0x9C+4*axis)initial=equation.after;
            publish(owner->actor+0x9C+4*axis,float(initial+parent.velocity[axis]));
        }
        for(unsigned axis=0;axis<2;++axis) {
            const auto address=axis?owner->physical+4:owner->actor+0x10;
            float proposal=parent.proposal_yaw[axis];
            for(const auto& equation:parent.equations)if(equation.address==address)proposal=equation.after;
            publish(address,proposal);
        }
    }
    ~SourceParentView() {
        if(!owner)return;
        const auto current=identity(ram,owner->actor);
        if(!current||!same(*current,*owner))return;
        uint8_t* rdram=ram;
        for(const auto& field:fields)
            if(word(rdram,field.address)==field.published)MEM_W(0,int32_t(field.address))=field.saved;
    }
};
}
bool native_enemy_kind(int kind) {return definition(kind)!=nullptr;}
bool native_enemy_owned(uint8_t* rdram,uint32_t actor) {return identity(rdram,actor).has_value();}
void native_enemy_reset() {++epoch;periods={};gravity.reset();turn.reset();projection.reset();equations.clear();model_reads.clear();}
extern "C" int dora3_native_enemy_callback(uint8_t* rdram,recomp_context* ctx,uint32_t callback,uint32_t rom,void (*update)(uint8_t*,recomp_context*)) {
    if(executing||attachment_only||!native_gameplay_preview_object_pass()||!active_simulation_slice())return 0;
    const auto actor=uint32_t(word(rdram,0x8021BDEC));const auto owner=identity(rdram,actor);
    if(!owner||owner->callback!=callback||owner->rom!=rom)return 0;
    auto slice=*active_simulation_slice();auto& previous=periods[index(actor)];
    if(previous&&same(previous->owner,*owner)&&previous->tick==slice.tick&&previous->denominator==slice.denominator&&previous->expected_phase==slice.phase) {
        if(previous->command==commands(rdram,*owner)) {
            // E8D44 clears physical2A before every callback. Original's
            // callback can then set it via E89CC; an unchanged Native period
            // skips that callback, so preserve its result after this prelude.
            // Otherwise transparent projectiles lose their flag every slice,
            // and fading actors spuriously cancel their opacity equation.
            if(MEM_BU(0xC8,int32_t(owner->physical))==0&&MEM_BU(0x29,int32_t(owner->physical))==0)
                MEM_B(0x2A,int32_t(owner->physical))=int8_t(previous->callback_opacity_flag);
            if(unchanged(rdram,*previous))return 1;
            // A displacement/impulse/seek is a motion command. It must not
            // consume another wait counter, RNG draw or attack command.
            auto& v=*previous;v.start_phase=slice.phase;v.gravity.reset();v.projection.reset();v.equations.clear();v.opacity.reset();
            for(unsigned j=0;j<3;++j) {
                v.initial_position[j]=v.expected_position[j]=real(rdram,actor+0x9C+4*j);
                v.velocity[j]=v.expected_velocity[j]=real(rdram,owner->physical+0x10+4*j);
            }
            v.initial_yaw=v.proposal_yaw=v.expected_yaw={real(rdram,actor+0x10),real(rdram,owner->physical+4)};v.turn={};
            v.acceleration=word(rdram,owner->physical+0x1C);return 1;
        }
    }
    if(slice.phase) {
        // A genuinely new state/type/lifetime or newly acquired domain begins
        // at this command's timestamp. Its first counter period is full length.
        // The motion-only rebase above returns without repeating AI commands.
        native_gameplay_preview_enemy_command_clock(rdram,actor);
        slice=*active_simulation_slice();
    }
    const std::array<float,2> before_yaw{real(rdram,actor+0x10),real(rdram,owner->physical+4)};
    const auto entered_epoch=epoch;
    struct CommandScope {
        bool prior=executing;
        CommandScope(){executing=true;gravity.reset();turn.reset();projection.reset();equations.clear();model_reads.clear();}
        ~CommandScope(){executing=prior;}
    } command_scope;
    // The complete real FSM runs once, preserving wait/RNG/clip/sound/spawn/
    // damage/retirement order. Recursion re-enters the untouched Original body.
    // Suspension keeps its full Source operands; Native motion is below.
    {SourceParentView parent_view(rdram,*owner);SimulationSuspension source;update(rdram,ctx);}
    const auto after=identity(rdram,actor);
    if(epoch!=entered_epoch||!after||!same(*owner,*after)||!active_simulation_slice())return 1;
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
    for(const auto& equation:equations)if(std::bit_cast<int32_t>(equation.after)==word(rdram,equation.address)) {
        // Direct floor/initialization writes have already run. Only the final
        // actual mathematical store receives its fractional continuation.
        value.equations.push_back(equation);
        value.equations.back().expected=equation.evaluate(0);
        put(rdram,equation.address,value.equations.back().expected);
    }
    for(const auto& equation:value.equations)if((kind==361||kind==389||kind==390||kind==391)&&equation.address==after->physical+0x58) {
        const auto published=real(rdram,after->model+0x78);
        if(std::bit_cast<int32_t>(published)==std::bit_cast<int32_t>(equation.after)) {
            // Powan fade/effect publishes the new counter. Reproduce the real
            // 3B168 model alpha/flag and E89CC physics flag along that equation.
            value.opacity=equation;value.opacity->address=after->model+0x78;
        } else if(kind==361&&std::bit_cast<int32_t>(published)==std::bit_cast<int32_t>(equation.before)&&
                  previous&&same(previous->owner,*after)&&previous->opacity) {
            // Smoke publishes BEFORE decrement. Its model alpha follows the
            // previous published value to this proposal; the first bind is an
            // instantaneous command, not an invented pre-birth fade sample.
            value.opacity=Equation{after->model+0x78,EquationKind::linear,previous->opacity->after,published};
            value.opacity->step=double(published)-previous->opacity->after;
        } else if(kind==361&&std::bit_cast<int32_t>(published)==std::bit_cast<int32_t>(equation.before)) {
            value.opacity=Equation{after->model+0x78,EquationKind::linear,published,published};
        }
    }
    value.update=update;
    value.acceleration=word(rdram,after->physical+0x1C);
    value.callback_opacity_flag=MEM_BU(0x2A,int32_t(after->physical));
    value.projection=projection;
    value.model_events=game_model_animation().command_event_checkpoint(rdram,after->model);
    value.model_reads=model_reads;
    if((kind==268&&MEM_H(0x36,int32_t(after->physical))==0)||(kind==361&&word(rdram,actor+0x94)==1)) {
        const int parent_slot=MEM_H(0x7C,int32_t(after->physical));
        if(parent_slot>=30&&parent_slot<230)value.parent=identity(rdram,0x80171CD0u+unsigned(parent_slot)*stride);
    }
    previous=value;return 1;
}
extern "C" int dora3_native_enemy_attachment_only() {return attachment_only;}
extern "C" int dora3_native_enemy_commands_active() {return executing;}
extern "C" void dora3_native_enemy_clip_end(uint8_t* rdram,recomp_context* ctx) {
    if(!executing)return;
    const auto model=uint32_t(ctx->r4);
    for(const auto& period:periods)if(period&&period->owner.model==model&&
        identity(rdram,period->owner.actor)&&same(*identity(rdram,period->owner.actor),period->owner)&&
        period->owner.lifetime==game_model_animation().lifetime_token(model)&&
        period->run==game_model_animation().run_token(model)&&game_model_animation().command_ended(model,period->model_events)) {
        ctx->r2|=2;return;
    }
    (void)rdram;
}
extern "C" int dora3_native_enemy_model_event(uint8_t* rdram,uint32_t model,float observed,float target,uint64_t site) {
    if(executing) {
        const auto actor=uint32_t(word(rdram,0x8021BDEC));
        if(actor>=pool&&actor<pool+200*stride&&(actor-pool)%stride==0)if(const auto& period=periods[index(actor)];period) {
            const auto current=identity(rdram,actor);if(!current||!same(*current,period->owner))return observed==target;
            const auto checkpoint=period->model_reads.find(model);
            if(checkpoint!=period->model_reads.end())
                return game_model_animation().command_frame_event(rdram,model,checkpoint->second,observed,target,site);
            if(period->owner.model==model&&period->owner.lifetime==game_model_animation().lifetime_token(model))
                return game_model_animation().command_frame_event(rdram,model,period->model_events,observed,target,site);
        }
    }
    return observed==target;
}
extern "C" void dora3_native_enemy_model_read(uint8_t* rdram,uint32_t model) {
    if(executing)model_reads.insert_or_assign(model,game_model_animation().command_event_checkpoint(rdram,model));
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
    const float first=axis==1?-value.speed[axis]:float(value.speed[axis]*value.first_factor[axis]);
    value.proposal[axis]=site==0x801E8004?ctx->f20.fl:
        float((site==0x801E8074?ctx->f20.fl:site==0x801E80B0?ctx->f22.fl:xyz?ctx->f20.fl:first)*value.unit[axis]);
}
extern "C" void dora3_native_enemy_equation(uint8_t* rdram,recomp_context* ctx,uint32_t site) {
    if(!executing)return;
    const auto actor=uint32_t(word(rdram,0x8021BDEC));const auto owner=identity(rdram,actor);if(!owner)return;
    const auto physical=owner->physical;Equation value{};
    // These are literal Source assignments from one already reviewed equation.
    // Copy its input, rather than treating the old destination as an input: the
    // Source X/Z collapse also handles externally supplied anisotropic scales.
    const bool scale_copy=(owner->rom==0xD9910&&site==0x80221628)||
        (owner->rom==0xEDDB0&&(site==0x80223A0C||site==0x80223BB0));
    const bool yaw_copy=(owner->rom==0xEDDB0&&
        (site==0x80220C24||site==0x80221498||site==0x8022177C||site==0x8022234C||
         site==0x802225F0||site==0x80223210||site==0x802235A0||site==0x8022372C))||
        (owner->rom==0xCEC60&&(site==0x80223058||site==0x802231DC||site==0x802235CC||
         site==0x80223BF0||site==0x80223D44||site==0x8022425C));
    if(scale_copy||yaw_copy) {
        const auto source=scale_copy?actor+0x18:physical+4;
        const auto parent=std::find_if(equations.begin(),equations.end(),[&](const auto& equation){return equation.address==source;});
        if(parent==equations.end()||std::bit_cast<int32_t>(parent->after)!=word(rdram,source))return;
        value=*parent;value.address=scale_copy?actor+0x20:actor+0x10;
        value.after=ctx->f0.fl;
        std::erase_if(equations,[&](const auto& old){return old.address==value.address;});
        equations.push_back(value);return;
    }
    const auto additional=std::find_if(additional_stores.begin(),additional_stores.end(),
        [&](const auto& store){return store.rom==owner->rom&&store.site==site;});
    if(additional!=additional_stores.end()) {
        const auto& store=*additional;
        const auto address=(store.physical?physical:actor)+store.offset;
        const float operand=store.operand==12?ctx->f12.fl:store.operand==4?ctx->f4.fl:store.operand==2?ctx->f2.fl:ctx->f0.fl;
        value={address,store.kind,real(rdram,address),operand};
        if(store.kind==EquationKind::approach) {
            const uint64_t bits=(uint64_t(uint32_t(word(rdram,store.coefficient)))<<32)|uint32_t(word(rdram,store.coefficient+4));
            value.base=store.target;value.retention=1-std::bit_cast<double>(bits);
        }
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
        if(owner->rom!=0xD9910)return;
        value={physical,EquationKind::linear,real(rdram,physical),ctx->f0.fl};break;
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
    if(value.kind==EquationKind::angle)value.step=std::remainder(double(value.after)-value.before,360.);
    else if(value.kind==EquationKind::linear)value.step=double(value.after)-value.before;
    if(owner->rom==0xCEC60&&(site==0x80223BD4||site==0x80223BE8)) {
        // BroboA104 accelerates its turn rate5C before adding it to total58
        // and heading4. Keep that kick/drift coupling; state2 scalar speed is0.
        const auto rate=std::find_if(equations.begin(),equations.end(),[&](const auto& equation){return equation.address==physical+0x5C;});
        if(rate!=equations.end()&&std::bit_cast<int32_t>(rate->after)==word(rdram,physical+0x5C))value.acceleration=double(rate->after)-rate->before;
    }
    // Its hit reaction rotates by the old integer rate38, then decrements it.
    // Source counters remain discrete, while the angular drift continues.
    if(owner->rom==0xCEC60&&site==0x80224258)value.acceleration=-4;
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
int native_enemy_axis(uint8_t* rdram,recomp_context* ctx,uint32_t site) {
    if(!native_gameplay_preview_object_pass()||!active_simulation_slice())return 0;
    const auto actor=uint32_t(word(rdram,0x8021BDEC));const auto owner=identity(rdram,actor);if(!owner)return 0;
    auto& stored=periods[index(actor)];if(!stored)return 0;
    auto& v=*stored;const auto& slice=*active_simulation_slice();
    if(!same(v.owner,*owner)||v.tick!=slice.tick||v.denominator!=slice.denominator)return 0;
    const unsigned axis=site==0x801E8E00?0:site==0x801E8E10?1:2;
    const double age=double(slice.phase+slice.numerator-v.start_phase)/double(slice.denominator);
    if(axis==0) {
        for(const auto& equation:v.equations)put(rdram,equation.address,equation.evaluate(age));
        if(v.opacity) {
            const auto alpha=v.opacity->evaluate(age);put(rdram,owner->model+0x78,alpha);
            MEM_W(0x74,int32_t(owner->model))=alpha==1.f?0:1;
            MEM_B(0x2A,int32_t(owner->physical))=alpha>=1.f?0:1;
            v.opacity->expected=alpha;
        }
        const auto kind=MEM_H(0x20,int32_t(owner->physical));
        if(v.parent&&((kind==268&&MEM_H(0x36,int32_t(owner->physical))==0)||(kind==361&&word(rdram,actor+0x94)==1))&&
           MEM_H(0x7C,int32_t(owner->physical))==int((v.parent->actor-0x80171CD0u)/stride))
        if(const auto parent=identity(rdram,v.parent->actor);parent&&same(*parent,*v.parent)) {
            // Execute the reviewed Original attachment geometry only. Its
            // constructor, animation, counters and collision paths are bypassed.
            recomp_context pose;dora3::copy_recomp_context(pose,*ctx);
            struct PoseOnly {PoseOnly(){attachment_only=true;}~PoseOnly(){attachment_only=false;}} pose_only;
            {SimulationSuspension source;v.update(rdram,&pose);}
            for(unsigned j=0;j<3;++j)v.initial_position[j]=real(rdram,actor+0x9C+4*j);
            // Attachment owns both headings at this instant. The old period
            // proposal must not overwrite its newly evaluated parent pose.
            v.initial_yaw=v.proposal_yaw={real(rdram,actor+0x10),real(rdram,owner->physical+4)};
            v.turn={};
        }
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
    if(v.projection&&v.projection->physical==owner->physical&&v.projection->calls[axis]==1&&
       std::bit_cast<int32_t>(v.projection->proposal[axis])==std::bit_cast<int32_t>(v.velocity[axis])) {
        const auto& projected=*v.projection;
        double speed=projected.speed[axis],acceleration=0;
        double heading=projected.heading[axis],heading_step=0,pitch=projected.pitch[axis],pitch_step=0;
        for(const auto& equation:v.equations) {
            if(equation.address==owner->physical+projected.scalar_offset[axis]&&equation.kind==EquationKind::linear&&
               equation.after==projected.speed[axis]) {
                speed=equation.before;acceleration=double(equation.after)-equation.before;
                const float first=projected.negate[axis]?-equation.before:
                    float(equation.before*projected.first_factor[axis]);
                const float before=float(first*projected.unit[axis]);
                const auto step=legacy_kinematic_step(before,double(v.velocity[axis])-before,age);
                velocity=age==1?v.velocity[axis]:float(step.velocity);displacement=step.displacement;
            }
            if(equation.kind==EquationKind::angle&&equation.address==owner->physical+4&&equation.after==projected.heading[axis]) {
                heading=equation.before;heading_step=equation.step;
            }
            if(equation.kind==EquationKind::angle&&equation.address==owner->physical+0xC&&equation.after==projected.pitch[axis]) {
                pitch=equation.before;pitch_step=equation.step;
            }
        }
        if(heading_step==0&&v.turn[1]!=0&&projected.heading[axis]==v.proposal_yaw[1]) {
            heading=v.initial_yaw[1];heading_step=v.turn[1];
        }
        if((axis!=1&&heading_step!=0)||((axis==1||projected.xyz[axis])&&pitch_step!=0)) {
            const double radians=projected.radians[axis];
            heading*=radians;heading_step*=radians;pitch*=radians;pitch_step*=radians;
            const auto sum=[&](double phase,double step){return harmonic_sum(phase,step,age,speed,acceleration);};
            const double current_speed=speed+acceleration*age;
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
        }
    }
    float initial=v.initial_position[axis];
    for(const auto& equation:v.equations)if(equation.address==actor+0x9C+4*axis)initial=equation.evaluate(age);
    const float position=age==1?float(initial+v.velocity[axis]):float(double(initial)+displacement);
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
    if(!native_gameplay_preview_object_pass()||actor<pool||actor>=pool+200*stride||(actor-pool)%stride||!active_simulation_slice())return;
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
}
extern "C" int dora3_native_enemy_blink_skip(uint8_t* rdram) {
    return native_gameplay_preview_object_pass()&&active_simulation_slice()&&!active_simulation_slice()->ends_tick()&&
        native_enemy_owned(rdram,uint32_t(word(rdram,0x8021BDEC)));
}
