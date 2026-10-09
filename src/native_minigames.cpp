#include "native_minigames.hpp"
#include "native_minigame_math.hpp"
#include "native_minigame_registry.hpp"
#include "native_model_animation.hpp"
#include "native_frame_bridge.hpp"
#include "recomp_context_copy.hpp"
#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <limits>
#include <unordered_map>
#include <vector>

unsigned dora3_requested_native_fps();
extern "C" {
int dora3_overlay_loaded(std::uint32_t);
int dora3_native_loop_minigame_active();
void dora3_s0_8002DA98(std::uint8_t*,recomp_context*);
void dora3_s0_8002F064(std::uint8_t*,recomp_context*);
void dora3_s0_80040104(std::uint8_t*,recomp_context*);
void dora3_s0_800327A4(std::uint8_t*,recomp_context*);
}
namespace {
using namespace dora3::native_time;
using Expression=MinigameEquations::Expression;
using Operation=MinigameEquations::Operation;
using Role=MinigameEquations::FieldRole;
constexpr std::uint32_t pool=0x804B1CD0,cameras=0x801A99F8;
constexpr unsigned actor_count=230,actor_stride=0xCC;
struct CircularCameraStore {std::uint32_t rom,site;bool opposite;};
constexpr std::array circular_camera_stores{
    CircularCameraStore{0x1961B0,0x801C58E8,true},
    CircularCameraStore{0x1961B0,0x801C5A84,false},
    CircularCameraStore{0x19F510,0x801C530C,true},
    CircularCameraStore{0x19F510,0x801C5494,false},
};
bool pointer(std::uint32_t p,unsigned size=4) {
    return !(p&3)&&p>=0x80000000u&&std::uint64_t(p)+size<=0x80800000ull;
}
std::uint32_t word(std::uint8_t* rdram,std::uint32_t p) {
    return std::uint32_t(MEM_W(0,std::int32_t(p)));
}
// Pure guest math uses temporary memory below the caller SP. Native owns
// that scratch only during this update, including exception unwinding.
class BorrowedStack {
    std::uint8_t* ram_;
    std::uint32_t address_;
    std::array<std::uint32_t,0xA00/4> saved_{};
public:
    BorrowedStack(std::uint8_t* ram,const recomp_context& c):ram_(ram),address_(std::uint32_t(c.r29)-0x800) {
        if(!pointer(address_,0xA00))throw std::logic_error("Invalid native minigame math stack");
        std::memcpy(saved_.data(),ram_+(address_&0xFFFFFF),sizeof(saved_));
    }
    ~BorrowedStack(){std::memcpy(ram_+(address_&0xFFFFFF),saved_.data(),sizeof(saved_));}
};
std::uint64_t reg(const recomp_context& c,unsigned index) {
    std::uint64_t result=0;
    std::memcpy(&result,reinterpret_cast<const std::uint8_t*>(&c)+offsetof(recomp_context,r0)+index*sizeof(gpr),sizeof(result));
    return index?result:0;
}
std::uint32_t float_bits(const recomp_context& c,unsigned index) {
    if((index&1)&&!c.mips3_float_mode)return c.f_odd[(index-1)*2];
    std::uint32_t result=0;
    std::memcpy(&result,reinterpret_cast<const std::uint8_t*>(&c)+offsetof(recomp_context,f0)+index*sizeof(fpr),sizeof(result));
    return result;
}
double double_value(const recomp_context& c,unsigned index) {
    double result=0;
    std::memcpy(&result,reinterpret_cast<const std::uint8_t*>(&c)+offsetof(recomp_context,f0)+index*sizeof(fpr),sizeof(result));
    return result;
}
float float_value(const recomp_context& c,unsigned index) {return std::bit_cast<float>(float_bits(c,index));}
struct Tag {Expression expression=0;std::uint64_t expected=0;bool wide=false;};
struct ActorIdentity {
    std::uint32_t active=0,resources=0;
    std::array<std::uint32_t,7> objects{};
    std::array<std::uint8_t,7> types{};
    bool operator==(const ActorIdentity&) const=default;
};
struct MatrixLease {
    std::uint32_t actor=0;
    ActorIdentity identity;
    std::array<std::uint32_t,17> source{},shown{};
};
struct GroundOwner {
    std::uint32_t actor=0;
    ActorIdentity identity;
    unsigned mode=0;
};
struct GroundLease {
    GroundOwner owner;
    std::array<std::uint32_t,3> source{},shown{};
};
constexpr std::array<std::uint32_t,3> ground_offsets{0xA0,0xC,0x14};
struct CameraLease {
    std::uint32_t address=0;
    std::array<std::uint32_t,4> source{},shown{};
};
constexpr std::array<std::uint32_t,4> eye_offsets{0xC,0x10,0x14,0x28};
struct DrawLease {
    std::uint32_t actor=0;
    ActorIdentity identity;
    std::array<std::uint32_t,28> derivatives{};
    std::uint8_t dirty=0;
};
struct IntegerLease {std::uint32_t address=0,source=0,shown=0;};
struct Runtime {
    MinigameEquations equations;
    std::array<Tag,32> fp{},gpr{};
    std::unordered_map<std::uint32_t,Tag> stack;
    std::unordered_map<std::uint32_t,std::array<Tag,2>> paired_spills;
    std::uint32_t source_stack_top=0,source_stack_low=0;
    std::unordered_map<std::uint32_t,double> counter_rates;
    struct Parameter {std::uint32_t model=0,actor=0;Tag value;MinigameEquations::Lifetime owner;};
    std::vector<Parameter> parameters;
    unsigned indirect_depth=0;
    std::vector<MatrixLease> matrices;
    std::vector<CameraLease> camera_eyes;
    std::vector<GroundOwner> ground_owners;
    std::vector<GroundLease> ground_leases;
    const dora3::minigame::Definition* definition=nullptr;
    std::uint8_t* ram=nullptr;
    std::uint64_t epoch=0;
    std::optional<Nanoseconds> source_time,render_time;
    bool in_source=false,enabled=false,drawing=false;
    std::vector<DrawLease> draw_leases;
    std::vector<IntegerLease> integer_leases;
    std::array<std::uint32_t,6> echo_source{};
    std::array<std::uint32_t,3> pulse_source{};
} runtime;
ActorIdentity actor_identity(std::uint8_t* rdram,std::uint32_t actor) {
    ActorIdentity result;
    result.active=word(rdram,actor+0x94);result.resources=word(rdram,actor+0xB0);
    for(unsigned i=0;i<7;++i) {
        result.objects[i]=word(rdram,actor+0x74+4*i);
        result.types[i]=MEM_BU(std::int32_t(0x6B+i),std::int32_t(actor));
    }
    return result;
}
const dora3::minigame::Definition* current(std::uint8_t* rdram) {
    if(!rdram||!dora3_overlay_loaded(0x18BDF0))return nullptr;
    for(const auto& d:dora3::minigame::definitions) {
        if(!dora3_overlay_loaded(d.rom))continue;
        bool slots=true;
        for(unsigned i=0;i<8;++i) {
            const auto p=0x801A44F0+i*20;
            const auto kind=word(rdram,p);
            if(i<2)slots=slots&&kind==2&&word(rdram,p+12)==i&&word(rdram,p+16)==(i?d.draw:d.update);
            else slots=slots&&(kind==0||kind==5);
        }
        if(slots)return &d;
    }
    return nullptr;
}
void restore_matrices(std::uint8_t* rdram) {
    if(rdram!=runtime.ram)return;
    for(auto& lease:runtime.matrices) {
        if(actor_identity(rdram,lease.actor)!=lease.identity)continue;
        bool same=MEM_BU(0x6A,std::int32_t(lease.actor))==lease.shown[16];
        for(unsigned i=0;i<16;++i)same=same&&word(rdram,lease.actor+0x24+4*i)==lease.shown[i];
        if(!same)continue;
        for(unsigned i=0;i<16;++i)
            MEM_W(0,std::int32_t(lease.actor+0x24+4*i))=std::int32_t(lease.source[i]);
        MEM_B(0x6A,std::int32_t(lease.actor))=std::int8_t(lease.source[16]);
    }
    runtime.matrices.clear();
}
void restore_camera_eyes(std::uint8_t* rdram) {
    if(rdram!=runtime.ram)return;
    for(const auto& lease:runtime.camera_eyes) {
        bool same=true;
        for(unsigned i=0;i<4;++i)same=same&&word(rdram,lease.address+eye_offsets[i])==lease.shown[i];
        if(!same)continue;
        for(unsigned i=0;i<4;++i)MEM_W(0,std::int32_t(lease.address+eye_offsets[i]))=std::int32_t(lease.source[i]);
    }
    runtime.camera_eyes.clear();
}
void restore_ground(std::uint8_t* rdram) {
    if(rdram!=runtime.ram)return;
    for(const auto& lease:runtime.ground_leases) {
        if(actor_identity(rdram,lease.owner.actor)!=lease.owner.identity)continue;
        for(unsigned i=0;i<3;++i)
            if(word(rdram,lease.owner.actor+ground_offsets[i])==lease.shown[i])
                MEM_W(0,std::int32_t(lease.owner.actor+ground_offsets[i]))=std::int32_t(lease.source[i]);
    }
    runtime.ground_leases.clear();
}
void sample_ground(std::uint8_t* rdram,recomp_context* ctx) {
    for(const auto& owner:runtime.ground_owners) {
        if(actor_identity(rdram,owner.actor)!=owner.identity||!owner.identity.active)continue;
        GroundLease lease;lease.owner=owner;
        for(unsigned i=0;i<3;++i)lease.source[i]=word(rdram,owner.actor+ground_offsets[i]);
        // Original queries write collision scratch and caller stack only. Keep
        // the whole reviewed scratch region and borrowed stack exact; no contact
        // FSM, sound, allocation or graphics call is repeated here.
        constexpr std::uint32_t scratch=0x800DAC00;constexpr unsigned scratch_size=0x400;
        std::array<std::uint32_t,scratch_size/4> saved{};
        for(unsigned i=0;i<saved.size();++i)saved[i]=word(rdram,scratch+4*i);
        const auto stack=std::uint32_t(ctx->r29)-0x800;
        if(!pointer(stack,0xA00))continue;
        std::array<std::uint32_t,0xA00/4> stack_saved{};
        for(unsigned i=0;i<stack_saved.size();++i)stack_saved[i]=word(rdram,stack+4*i);
        const auto original_stack=std::uint32_t(ctx->r29);
        const auto real=[&](std::uint32_t p){return std::bit_cast<float>(word(rdram,p));};
        recomp_context query;dora3::copy_recomp_context(query,*ctx);
        query.f12.d=real(owner.actor+0x9C);
        query.f14.d=real(owner.actor+0xA0)+(owner.mode==4?100.:0.);
        const auto depth=std::bit_cast<std::uint64_t>(double(real(owner.actor+0xA4)));
        MEM_W(0x10,std::int32_t(original_stack))=std::int32_t(depth>>32);
        MEM_W(0x14,std::int32_t(original_stack))=std::int32_t(depth);
        dora3_s0_80040104(rdram,&query);
        const auto selected=std::uint32_t(word(rdram,0x800DACF0)&1);
        MEM_W(0,std::int32_t(owner.actor+0xA0))=std::int32_t(word(rdram,0x800DACB8+(owner.mode==4?0:28*selected)));
        if(owner.mode==1) {
            const auto record=0x800DACB8+28*selected;
            dora3::copy_recomp_context(query,*ctx);
            query.f12.fl=real(record+4);query.f14.fl=real(record+8);
            query.r6=std::int32_t(word(rdram,record+0xC));query.r7=std::int32_t(word(rdram,record+0x10));
            for(unsigned i=0;i<4;++i)MEM_W(std::int32_t(0x10+4*i),std::int32_t(original_stack))=std::int32_t(original_stack+0x40+4*i);
            dora3_s0_800327A4(rdram,&query);
            MEM_W(0,std::int32_t(owner.actor+0xC))=std::int32_t(word(rdram,original_stack+0x40));
            MEM_W(0,std::int32_t(owner.actor+0x14))=std::int32_t(word(rdram,original_stack+0x48));
        }
        for(unsigned i=0;i<saved.size();++i)MEM_W(0,std::int32_t(scratch+4*i))=std::int32_t(saved[i]);
        for(unsigned i=0;i<stack_saved.size();++i)MEM_W(0,std::int32_t(stack+4*i))=std::int32_t(stack_saved[i]);
        for(unsigned i=0;i<3;++i)lease.shown[i]=word(rdram,owner.actor+ground_offsets[i]);
        runtime.ground_leases.push_back(lease);
    }
}
void refresh_terrain_followers(std::uint8_t* ram) {
    if(!runtime.definition||runtime.definition->rom!=0x19F510)return;
    // Actual 5E74/56C8 copies use world XYZ. An opaque terrain query can
    // replace body Y after the expression graph has sampled its Source load.
    // Refresh only the reached Derived copies; Source effect births are absent
    // from that set and must keep their one-time spawn positions.
    for(unsigned i=0;i<4;++i) {
        const auto body=pool+(i+1)*actor_stride;
        if(!word(ram,body+0x94))continue;
        for(const auto follower:{pool+(i+5)*actor_stride,pool+(i+21)*actor_stride})
            for(unsigned axis=0;axis<3;++axis)
                runtime.equations.publish_derived_float(ram,follower+0x9C+4*axis,
                    std::bit_cast<float>(word(ram,body+0x9C+4*axis)));
    }
}
void refresh_bomb_participant_poses(std::uint8_t* ram) {
    if(!runtime.definition||runtime.definition->rom!=0x1BC460)return;
    // 6E6C/70/78 publish the body after physics; 7570 then copies its XZ to
    // the shadow. Render matrices consume local XYZ, not world XYZ. Preserve
    // those reached Derived copies after sampling the actual motion equations.
    // This does not advance input, velocity, animation, contacts or the FSM.
    for(unsigned participant=0;participant<4;++participant) {
        const auto body=pool+participant*actor_stride;
        if(!word(ram,body+0x94))continue;
        for(unsigned axis=0;axis<3;++axis)
            runtime.equations.publish_derived_float(ram,body+4*axis,
                std::bit_cast<float>(word(ram,body+0x9C+4*axis)));
        const auto shadow=pool+(65+participant)*actor_stride;
        if(!word(ram,shadow+0x94))continue;
        for(unsigned axis:{0u,2u})
            runtime.equations.publish_derived_float(ram,shadow+4*axis,
                std::bit_cast<float>(word(ram,body+4*axis)));
    }
}
void refresh_view_origins(std::uint8_t* ram) {
    if(!runtime.definition)return;
    std::array<std::uint32_t,3> arrays{};
    if(runtime.definition->rom==0x1961B0)arrays={0x801CC9E8,0x801CCAB0,0x801CD028};
    else if(runtime.definition->rom==0x19F510)arrays={0x801C8908,0x801C8938,0x801C8AE8};
    else return;
    // Main Source callbacks copy each participant's world XYZ into four view
    // origins. Terrain Y is supplied by an opaque SDK query, so the captured
    // arithmetic cannot derive that copy from the newly sampled ground height.
    // Publish the exact Native query's position for the same viewport/body.
    for(unsigned i=0;i<4;++i) {
        const auto actor=pool+(i+1)*actor_stride;
        if(!word(ram,actor+0x94))continue;
        for(unsigned j=0;j<3;++j)
            runtime.equations.publish_derived_float(ram,arrays[j]+4*i,std::bit_cast<float>(word(ram,actor+0x9C+4*j)));
    }
}
bool stack_address(std::uint32_t address,unsigned bytes=4) {
    // The actual game SP is801AEAF8. A64KiB guessed stack interval also
    // contains camera801A99F8 and resident animation/input globals, turning
    // their loads into constants. Only reached Source frames are spills.
    return runtime.source_stack_low>=0x80000000u &&
        address>=runtime.source_stack_low &&
        std::uint64_t(address)+bytes<=std::uint64_t(runtime.source_stack_top);
}
Expression read_fp(recomp_context* c,unsigned index,bool wide) {
    auto& t=runtime.fp[index];
    const auto actual=wide?std::bit_cast<std::uint64_t>(double_value(*c,index)):float_bits(*c,index);
    if(t.expression&&t.wide==wide&&t.expected==actual)return t.expression;
    t={runtime.equations.constant(wide?double_value(*c,index):double(float_value(*c,index))),actual,wide};
    return t.expression;
}
void tag_fp(unsigned index,Expression expression,double actual,bool wide) {
    runtime.fp[index]={expression,wide?std::bit_cast<std::uint64_t>(actual):std::uint64_t(std::bit_cast<std::uint32_t>(float(actual))),wide};
}
std::optional<std::uint32_t> actor_for(std::uint32_t address) {
    if(address<pool||address>=pool+actor_count*actor_stride)return std::nullopt;
    return pool+(address-pool)/actor_stride*actor_stride;
}
MinigameEquations::Lifetime lifetime(std::uint8_t* rdram,std::uint32_t address,std::uint32_t hint=0) {
    MinigameEquations::Lifetime out;out.scene=runtime.epoch;
    auto actor=actor_for(address);
    if(hint) {
        actor=actor_for(hint);
        if(!actor||*actor!=hint)return out;
        const auto driver=word(rdram,hint+0x98);
        if(!pointer(driver)||address<driver||address-driver>=0x20)return out;
        out.driver=driver;
    }
    if(actor) {
        out.actor=*actor;out.identity=word(rdram,*actor+0x94);out.resources=word(rdram,*actor+0xB0);
        // Components can be sprite handles, not SDK model pointers. A registered
        // allocation token is used only for an actual registered model.
        for(unsigned i=0;i<7;++i) {
            out.components[i]=word(rdram,*actor+0x74+4*i);
            out.types[i]=MEM_BU(std::int32_t(0x6B+i),std::int32_t(*actor));
            const auto model=word(rdram,*actor+0x74+4*i);
            out.component_allocations[i]=game_model_animation().lifetime_token(model);
            if(auto token=out.component_allocations[i]) {
                out.model=model;out.model_slot=*actor+0x74+4*i;out.allocation=token;
            }
        }
    }
    return out;
}
void capture_bomb_object_contact_positions(std::uint8_t* rdram) {
    if(!runtime.definition||runtime.definition->rom!=0x1BC460)return;
    // Original4EC8 copies world X/Z to local0/8 before drawing slots10..59.
    // Source7304/713C subsequently reads local0/8 for the75-unit hit radius.
    // A Native draw borrows/reverts local fields, so that first authoritative
    // publication must happen once at Source completion, after spawn/motion.
    // Keep Y alone: falling type2 has its separate authored draw height5/30.
    for(unsigned slot=10;slot<60;++slot) {
        const auto actor=pool+slot*actor_stride;
        if(!word(rdram,actor+0x94))continue;
        for(unsigned axis:{0u,2u}) {
            const auto source=actor+0x9C+4*axis,destination=actor+4*axis;
            const auto actual=std::bit_cast<float>(word(rdram,source));
            const auto expression=runtime.equations.load_float(source,actual);
            runtime.equations.store_float(destination,expression,actual,Role::Derived,lifetime(rdram,destination));
            MEM_W(0,std::int32_t(destination))=std::int32_t(word(rdram,source));
        }
    }
}
}
bool native_minigame_eligible(std::uint8_t* rdram) {
    if(!rdram||!dora3::minigame::implementation_ready)return false;
    const auto divider=word(rdram,0x801A39DC);
    return (divider==2||divider==3)&&word(rdram,0x801AA648)&&word(rdram,0x801A9500)&&current(rdram);
}
std::uint32_t native_minigame_module(std::uint8_t* ram) {
    const auto* definition=ram?current(ram):nullptr;
    return definition?definition->rom:0;
}
void native_minigame_begin(std::uint8_t* rdram,Nanoseconds now,unsigned) {
    runtime.enabled=true;runtime.render_time=now;
    if(runtime.ram!=rdram||runtime.definition!=current(rdram))native_minigame_reset(rdram);
    runtime.enabled=true;runtime.ram=rdram;runtime.definition=current(rdram);runtime.render_time=now;
    // Source was frozen while Acquiring awaited the old host read. That wait
    // must not become a minigame movement/opacity/rotation interval.
    runtime.equations.resume_after_handoff(rdram,now);
    if(runtime.source_time)runtime.source_time=now;
}
void native_minigame_update(std::uint8_t* rdram,recomp_context* ctx,Nanoseconds now) {
    if(!native_minigame_eligible(rdram)||runtime.in_source)return;
    if(runtime.ram!=rdram||runtime.definition!=current(rdram)) {native_minigame_reset(rdram);return;}
    BorrowedStack stack(rdram,*ctx);
    restore_matrices(rdram);restore_camera_eyes(rdram);restore_ground(rdram);runtime.equations.sample(rdram,now);
    sample_ground(rdram,ctx);
    refresh_terrain_followers(rdram);
    refresh_bomb_participant_poses(rdram);
    refresh_view_origins(rdram);
    const auto changed=runtime.equations.changed_fields();
    for(unsigned i=0;i<4;++i) {
        const auto address=cameras+i*0x34;
        const bool moving=std::any_of(changed.begin(),changed.end(),[&](auto p){
            return p>=address&&p<=address+0x30&&
                (p-address<0xC||p-address==0x18||p-address==0x1C||p-address==0x30);
        });
        if(!moving)continue;
        CameraLease lease;lease.address=address;
        for(unsigned j=0;j<4;++j)lease.source[j]=word(rdram,address+eye_offsets[j]);
        recomp_context math;dora3::copy_recomp_context(math,*ctx);math.r4=std::int32_t(address);
        dora3_s0_8002F064(rdram,&math);
        for(unsigned j=0;j<4;++j)lease.shown[j]=word(rdram,address+eye_offsets[j]);
        runtime.camera_eyes.push_back(lease);
    }
    for(const auto actor:runtime.equations.changed_local_actors()) {
        const auto identity=actor_identity(rdram,actor);
        if(!identity.active)continue;
        MatrixLease lease;lease.actor=actor;lease.identity=identity;
        for(unsigned i=0;i<16;++i)lease.source[i]=word(rdram,actor+0x24+4*i);
        lease.source[16]=MEM_BU(0x6A,std::int32_t(actor));
        recomp_context math;dora3::copy_recomp_context(math,*ctx);math.r4=std::int32_t(actor);
        dora3_s0_8002DA98(rdram,&math);
        for(unsigned i=0;i<16;++i)lease.shown[i]=word(rdram,actor+0x24+4*i);
        lease.shown[16]=MEM_BU(0x6A,std::int32_t(actor));runtime.matrices.push_back(lease);
    }
    runtime.render_time=now;
}
void native_minigame_source_completed(std::uint8_t* ram,Nanoseconds now) {
    if(runtime.in_source||runtime.drawing||runtime.ram!=ram||!runtime.source_time||
       runtime.definition!=current(ram))return;
    runtime.equations.resume_after_handoff(ram,now);
    runtime.source_time=runtime.render_time=now;
}
void native_minigame_reset(std::uint8_t* rdram) {
    // A replacement callback/module/RDRAM owns its own fields, even if some
    // addresses or values match the retired scene. Restore only the old owner.
    if(rdram&&rdram==runtime.ram&&runtime.definition==current(rdram)) {
        restore_matrices(rdram);restore_camera_eyes(rdram);restore_ground(rdram);runtime.equations.restore_source(rdram);
    }
    runtime.equations.reset();runtime.fp={};runtime.gpr={};runtime.stack.clear();runtime.paired_spills.clear();runtime.parameters.clear();runtime.counter_rates.clear();runtime.indirect_depth=0;runtime.matrices.clear();runtime.camera_eyes.clear();runtime.ground_owners.clear();runtime.ground_leases.clear();
    runtime.definition=nullptr;runtime.ram=nullptr;runtime.source_time.reset();runtime.render_time.reset();
    runtime.in_source=runtime.enabled=false;++runtime.epoch;
}
std::optional<Nanoseconds> native_minigame_render_time(){return runtime.render_time;}
extern "C" void dora3_native_minigame_source(std::uint8_t* rdram,recomp_context* c,std::uint32_t rom,std::uint32_t root,unsigned end) {
    const auto now=monotonic_time();
    if(end) {
        if(!runtime.in_source||runtime.ram!=rdram||!runtime.definition||
           runtime.definition->rom!=rom||runtime.definition->update!=root)return;
        for(const auto& parameter:runtime.parameters) {
            if(game_model_animation().lifetime_token(parameter.model)!=parameter.owner.allocation)continue;
            const auto actual=std::bit_cast<float>(word(rdram,parameter.model+0x78));
            if(std::bit_cast<std::uint32_t>(actual)!=parameter.value.expected)continue;
            runtime.equations.store_float(parameter.model+0x78,parameter.value.expression,actual,Role::Derived,parameter.owner);
        }
        capture_bomb_object_contact_positions(rdram);
        runtime.equations.end_source(rdram,now);runtime.source_time=runtime.render_time=now;runtime.in_source=false;
        if(rom==0x18BDF0&&root==0x801BF8CC)
            runtime.pulse_source={word(rdram,0x801C3650),word(rdram,0x801C3670),word(rdram,0x801C3674)};
        if(runtime.definition!=current(rdram))native_minigame_reset(rdram);
        return;
    }
    const auto* d=current(rdram);
    if(!dora3::minigame::implementation_ready||!d||d->rom!=rom||d->update!=root)return;
    if(!end) {
        if(!dora3_requested_native_fps())return;
        if(runtime.ram!=rdram||runtime.definition!=d){native_minigame_reset(rdram);runtime.ram=rdram;runtime.definition=d;}
        restore_matrices(rdram);restore_camera_eyes(rdram);restore_ground(rdram);runtime.ground_owners.clear();runtime.fp={};runtime.gpr={};runtime.stack.clear();runtime.paired_spills.clear();runtime.parameters.clear();runtime.counter_rates.clear();runtime.indirect_depth=0;runtime.source_stack_low=runtime.source_stack_top=std::uint32_t(c->r29);runtime.in_source=true;
        runtime.equations.begin_source(rdram,rom,runtime.epoch,now,unsigned(word(rdram,0x801A39DC)));
    }
}
extern "C" void dora3_native_minigame_instruction(std::uint8_t* rdram,recomp_context* c,std::uint32_t rom,
                                                 std::uint32_t site,std::uint32_t instruction,unsigned role,std::uint32_t owner_hint) {
    if(!runtime.in_source||!runtime.definition||(runtime.definition->rom!=rom&&rom!=0x18BDF0))return;
    const auto sp=std::uint32_t(c->r29);
    if(pointer(sp)&&sp<=runtime.source_stack_top)runtime.source_stack_low=std::min(runtime.source_stack_low,sp);
    const unsigned op=instruction>>26,rs=(instruction>>21)&31,rt=(instruction>>16)&31;
    const auto address=std::uint32_t(reg(*c,rs)+std::int16_t(instruction));
    const auto tagged_gpr=[&](unsigned index)->Tag {
        auto t=runtime.gpr[index];
        if(!t.expression||t.expected!=std::uint32_t(reg(*c,index)))return {};
        return t;
    };
    if(op==35&&pointer(address)) {
        const auto at=runtime.stack.find(address);const auto actual=word(rdram,address);
        if(at!=runtime.stack.end()&&at->second.expected==actual)runtime.gpr[rt]=at->second;
        else if(!stack_address(address))runtime.gpr[rt]={runtime.equations.load_float(address,std::bit_cast<float>(actual)),actual,false};
        else runtime.gpr[rt]={};
        return;
    }
    if(op==33||op==37||op==32||op==36) {
        runtime.gpr[rt]={};
        if(const auto at=runtime.counter_rates.find(address);at!=runtime.counter_rates.end()) {
            const double actual=op==33?MEM_H(0,std::int32_t(address)):op==37?MEM_HU(0,std::int32_t(address)):op==32?MEM_B(0,std::int32_t(address)):MEM_BU(0,std::int32_t(address));
            const auto rate=at->second;
            runtime.gpr[rt]={runtime.equations.time_operand(actual,[actual,rate](double h){return actual+rate*h;}),std::uint32_t(std::int32_t(actual)),true};
        }
        return;
    }
    if(op==0&&((instruction&63)==33||(instruction&63)==37)) {
        const unsigned dest=(instruction>>11)&31;
        runtime.gpr[dest]=rt==0?tagged_gpr(rs):rs==0?tagged_gpr(rt):Tag{};
        return;
    }
    if(op==9||(op==0&&((instruction&63)==0||(instruction&63)==3))) {
        runtime.gpr[op==9?rt:(instruction>>11)&31]={};return;
    }
    if(op==49&&pointer(address)) {
        const auto actual=std::bit_cast<float>(word(rdram,address));
        // Stack spill addresses are reused after a call; their expressions
        // are immutable values, never rebased from the final stack contents.
        const auto at=runtime.stack.find(address);
        const bool spill=stack_address(address)&&at!=runtime.stack.end()&&!at->second.wide&&at->second.expected==std::bit_cast<std::uint32_t>(actual);
        Expression expression=spill?at->second.expression:
            stack_address(address)?runtime.equations.constant(actual):runtime.equations.load_float(address,actual);
        // Match a real registered model+64 field, never an SDK call name.
        // 2A14C/3AFEC are draw helpers and are deliberately not clock getters.
        const auto model=address-0x64u;
        const auto allocation=game_model_animation().lifetime_token(model);
        if(!spill&&!stack_address(address)&&allocation&&pointer(model,0x80)&&std::isfinite(actual)) {
            const auto command=game_model_animation().time_command_token(model);
            const auto run=game_model_animation().run_token(model);
            expression=runtime.equations.time_operand(actual,[rdram,model,actual,allocation,command,run](double h) {
                const auto& animation=game_model_animation();
                if(animation.lifetime_token(model)!=allocation||animation.time_command_token(model)!=command||
                   animation.run_token(model)!=run)return std::numeric_limits<double>::quiet_NaN();
                const auto observed=animation.original_clock_sample(rdram,model);
                if(!observed||actual<0||actual>observed->first.duration)return std::numeric_limits<double>::quiet_NaN();
                auto state=observed->first;state.time=actual;
                Animation clock(state,observed->second);
                return double(clock.advance(Nanoseconds(std::llround(h*double(second)*observed->second/60.))).time);
            });
        }
        tag_fp(rt,expression,actual,false);return;
    }
    if(op==53&&pointer(address,8)) {
        const auto raw=std::uint64_t(word(rdram,address))<<32|word(rdram,address+4);
        const double actual=std::bit_cast<double>(raw);
        const auto at=runtime.stack.find(address);
        const bool saved=stack_address(address,8)&&at!=runtime.stack.end()&&at->second.wide&&at->second.expected==raw;
        if(saved&&!c->mips3_float_mode) {
            const auto pair=runtime.paired_spills.find(address);
            if(pair!=runtime.paired_spills.end()) {
                // FR0 sdc1/ldc1 spills both halves of an even register. A saved
                // single-precision expression must survive that opaque bit copy.
                runtime.fp[rt]=pair->second[0];
                if(rt+1<32)runtime.fp[rt+1]=pair->second[1];
                return;
            }
        }
        const auto expression=saved?at->second.expression:runtime.equations.constant(actual);
        tag_fp(rt,expression,actual,true);return;
    }
    if((op==57||op==61)&&pointer(address,op==61?8:4)) {
        const bool wide=op==61;
        const std::array<Tag,2> pair{runtime.fp[rt],rt+1<32?runtime.fp[rt+1]:Tag{}};
        auto expression=read_fp(c,rt,wide);
        const double actual=wide?double_value(*c,rt):double(float_value(*c,rt));
        if(!wide&&rom==0x1BC460&&(site==0x801C6E6C||site==0x801C6E70||site==0x801C6E78)) {
            // Exact post-physics world->local copies. Tie them to their owned
            // world fields explicitly instead of relying on a volatile FPR tag
            // surviving every branch/helper in the participant update.
            if(const auto actor=actor_for(address);actor&&address-*actor<=8) {
                const auto source=*actor+0x9C+(address-*actor);
                if(word(rdram,source)==float_bits(*c,rt))
                    expression=runtime.equations.load_float(source,float(actual));
            }
        }
        if(stack_address(address,wide?8:4)) {
            runtime.stack[address]={expression,wide?std::bit_cast<std::uint64_t>(actual):std::uint64_t(float_bits(*c,rt)),wide};
            if(wide&&!c->mips3_float_mode)runtime.paired_spills[address]=pair;
        }
        // An sdc1 bit spill does not itself change the live single register tag.
        if(wide&&!c->mips3_float_mode) {runtime.fp[rt]=pair[0];if(rt+1<32)runtime.fp[rt+1]=pair[1];}
        if(!wide) {
            const auto owner=lifetime(rdram,address,owner_hint);
            for(const auto& projected:dora3::minigame::projected_positions)
                if(projected.rom==rom&&projected.site==site) {
                    role=unsigned(Role::Motion);
                    // Mini1 computes projected XZ from its Source snapshot at
                    // driver50/58. Its actor publication owns that recurrence.
                    if(rom==0x1961B0)runtime.equations.bind_snapshot(expression,
                        std::uint32_t(c->r16)+(site==0x801C97DC?0x50:0x58),address);
                }
            bool circular_camera=false;
            const auto camera_store=std::find_if(circular_camera_stores.begin(),circular_camera_stores.end(),
                [&](const auto& store){return store.rom==rom&&store.site==site;});
            if(camera_store!=circular_camera_stores.end()&&role==unsigned(Role::Angle)&&
               address>=cameras+0x1C&&address<cameras+4*0x34&&
               (address-cameras)%0x34==0x1C&&std::uint32_t(c->r5)<4) {
                // All four exact Original helpers retain a1=participant index;
                // their target load is80171DAC+a1*CC, body(1+a1).yaw10.
                const auto heading_address=pool+(1+std::uint32_t(c->r5))*actor_stride+0x10;
                const float heading=std::bit_cast<float>(word(rdram,heading_address));
                const float before=std::bit_cast<float>(word(rdram,address));
                const auto source_wrap=[](float v){return v>=360.f?v-360.f:v<0?v+360.f:v;};
                float delta=source_wrap((360.f-heading)+(camera_store->opposite?180.f:0.f));
                delta=source_wrap(delta+before);
                if(delta>180.f)delta=source_wrap(360.f-delta);
                // This is the exact Source90-degree branch. Keep that chosen
                // gain while the target's registered heading equation moves.
                const double gain=1./(delta>90.f?30.:15.);
                auto target=runtime.equations.load_float(heading_address,heading);
                if(camera_store->opposite)target=runtime.equations.operation(Operation::Add,target,runtime.equations.constant(180.));
                runtime.equations.store_circular_approach(address,target,float(actual),gain,owner);
                circular_camera=true;
            }
            if(!circular_camera)
                runtime.equations.store_float(address,expression,float(actual),owner_hint&&!owner.actor?Role::Scratch:Role(role),owner);
            for(const auto& bounds:dora3::minigame::motion_bounds)
                if(bounds.rom==rom&&bounds.site==site)
                    runtime.equations.constrain_float(address,bounds.minimum,bounds.maximum,bounds.clamp);
        }
        return;
    }
    if((op==40||op==41||op==43)&&pointer(address&~3u)) {
        const unsigned bytes=op==40?1:op==41?2:4;
        const auto tagged=tagged_gpr(rt);
        if(op==43&&tagged.expression&&!tagged.wide) {
            const auto actual=std::bit_cast<float>(std::uint32_t(reg(*c,rt)));
            if(stack_address(address))runtime.stack[address]=tagged;
            runtime.equations.store_float(address,tagged.expression,actual,Role(role),lifetime(rdram,address,owner_hint));
            return;
        }
        runtime.equations.command_write(address,bytes);
        for(unsigned i=0;i<bytes;++i) {
            runtime.stack.erase((address+i)&~3u);
            runtime.paired_spills.erase((address+i)&~7u);
            runtime.paired_spills.erase(((address+i)&~3u)-4u);
        }
        return;
    }
    if(op!=17)return;
    const unsigned fmt=rs,ft=rt,fs=(instruction>>11)&31,fd=(instruction>>6)&31,fn=instruction&63;
    if(fmt==4) {
        const auto tag=tagged_gpr(rt);const auto actual=std::bit_cast<float>(std::uint32_t(reg(*c,rt)));
        if(tag.expression&&!tag.wide)tag_fp(fs,tag.expression,actual,false);
        else if(tag.expression)runtime.fp[fs]=tag;
        else tag_fp(fs,runtime.equations.constant(actual),actual,false);
        return;
    }
    if(fmt==0) {runtime.gpr[rt]=runtime.fp[fs];return;}
    if(fmt==20&&(fn==32||fn==33)) {
        const auto actual=double(std::int32_t(float_bits(*c,fs)));
        const auto& tag=runtime.fp[fs];
        const auto expression=tag.expression&&tag.wide&&tag.expected==float_bits(*c,fs)?tag.expression:runtime.equations.constant(actual);
        tag_fp(fd,expression,actual,fn==33);return;
    }
    if(fmt!=16&&fmt!=17)return;
    const bool wide=fmt==17;const double a=wide?double_value(*c,fs):double(float_value(*c,fs));
    const double b=wide?double_value(*c,ft):double(float_value(*c,ft));
    auto x=read_fp(c,fs,wide),y=Expression(0);double result=a;Operation operation=Operation::Add;
    if(fn<4)y=read_fp(c,ft,wide);
    switch(fn) {
    case 0:operation=Operation::Add;result=a+b;break;
    case 1:operation=Operation::Subtract;result=a-b;break;
    case 2:operation=Operation::Multiply;result=a*b;break;
    case 3:operation=Operation::Divide;result=a/b;break;
    case 4:operation=Operation::SquareRoot;result=std::sqrt(a);break;
    case 5:operation=Operation::Absolute;result=std::abs(a);break;
    case 6:tag_fp(fd,x,a,wide);return;
    case 7:operation=Operation::Negate;result=-a;break;
    case 32:tag_fp(fd,runtime.equations.operation(Operation::RoundFloat,x),float(a),false);return;
    case 33:tag_fp(fd,x,a,true);return;
    default:
        // c.* instructions write the condition bit, not fd.
        if(fn<48)runtime.fp[fd]={};
        return;
    }
    tag_fp(fd,runtime.equations.operation(operation,x,y),wide?result:double(float(result)),wide);
    (void)site;
}
extern "C" void dora3_native_minigame_call_return(std::uint8_t*,recomp_context* c,std::uint32_t rom,std::uint32_t target,unsigned captured) {
    if(!runtime.in_source||!runtime.definition||(runtime.definition->rom!=rom&&rom!=0x18BDF0))return;
    Expression result=0;
    if(target==0x80031A0C) {
        const auto x=runtime.fp[12].expression,y=runtime.fp[14].expression;
        if(x&&y)result=runtime.equations.operation(Operation::AngleWrap,runtime.equations.operation(Operation::Add,x,y));
    } else if(target==0x80055BD0||target==0x80057A80||target==0x80055BC0) {
        const auto x=runtime.fp[12].expression;
        if(x)result=runtime.equations.operation(target==0x80055BD0?Operation::Cosine:target==0x80057A80?Operation::Sine:Operation::SquareRoot,x);
    } else if(target==0x80031884||target==0x80031AA4||target==0x80031A58) {
        const auto x=runtime.fp[12].expression,y=runtime.fp[14].expression;
        if(x&&y) {
            auto depth=y;
            if(target==0x80031A58)depth=runtime.equations.operation(Operation::Negate,y);
            result=runtime.equations.operation(Operation::Heading,x,depth);
            if(target==0x80031AA4)result=runtime.equations.operation(Operation::AngleDelta,result);
            if(target==0x80031A58)result=runtime.equations.operation(Operation::AngleWrap,runtime.equations.operation(Operation::Add,result,runtime.equations.constant(180)));
        }
    } else if(captured)result=read_fp(c,0,runtime.fp[0].wide);
    const auto returned=runtime.fp[0];
    // SDK calls are opaque to the expression ledger. Matching numerical bits
    // after a call are not sufficient evidence that an old caller tag survived.
    for(unsigned i=0;i<20;++i)runtime.fp[i]={};
    for(unsigned i=2;i<16;++i)runtime.gpr[i]={};
    if(result) {
        if(captured&&target!=0x80031A0C&&target!=0x80055BD0&&target!=0x80057A80&&target!=0x80055BC0&&target!=0x80031884&&target!=0x80031AA4&&target!=0x80031A58)
            runtime.fp[0]=returned;
        else tag_fp(0,result,float_value(*c,0),false);
    }
}

namespace dora3::native_time {
MinigameFrameScope::MinigameFrameScope(std::uint8_t* rdram) {
    if(!dora3_native_loop_minigame_active())return;
    if(runtime.drawing||runtime.in_source||runtime.ram!=rdram||runtime.definition!=current(rdram))
        throw std::logic_error("Invalid minigame draw ownership");
    runtime.draw_leases.clear();runtime.integer_leases.clear();
    runtime.draw_leases.reserve(actor_count);
    for(unsigned slot=0;slot<actor_count;++slot) {
        const auto actor=pool+slot*actor_stride;const auto identity=actor_identity(rdram,actor);
        if(!identity.active)continue;
        DrawLease lease;lease.actor=actor;lease.identity=identity;
        for(unsigned i=0;i<16;++i)lease.derivatives[i]=word(rdram,actor+0x24+4*i);
        for(unsigned i=0;i<3;++i)lease.derivatives[16+i]=word(rdram,actor+0x9C+4*i);
        for(unsigned i=0;i<9;++i)lease.derivatives[19+i]=word(rdram,actor+4*i);
        lease.dirty=MEM_BU(0x6A,std::int32_t(actor));runtime.draw_leases.push_back(lease);
    }
    for(unsigned i=0;i<6;++i)runtime.echo_source[i]=word(rdram,std::array<std::uint32_t,6>{0x800DAE54,0x800FE878,0x800DD3F0,0x801AC6D0,0x801A9530,0x801A48DC}[i]);
    runtime.drawing=true;ram_=rdram;
    if(runtime.definition->rom==0x18BDF0&&runtime.definition->update==0x801BF8CC&&runtime.source_time)
        for(const auto address:{0x801C3670u,0x801C3674u}) {
            IntegerLease lease{address,word(rdram,address),std::uint32_t(std::lround(dora3_native_minigame_visual(rdram,address)))};
            MEM_W(0,std::int32_t(address))=std::int32_t(lease.shown);runtime.integer_leases.push_back(lease);
        }
}
MinigameFrameScope::~MinigameFrameScope() {
    if(!ram_)return;
    auto* rdram=ram_;
    for(const auto& lease:runtime.draw_leases) {
        if(actor_identity(rdram,lease.actor)!=lease.identity)continue;
        // The draw phase owns these reviewed SDK derivatives exclusively.
        // Local/world poses and matrices are borrowed; allocations, component
        // identities and commands are not. Mini8 contact XZ has already been
        // committed once at Source completion, so this restores that baseline.
        for(unsigned i=0;i<16;++i)MEM_W(0,std::int32_t(lease.actor+0x24+4*i))=std::int32_t(lease.derivatives[i]);
        for(unsigned i=0;i<3;++i)MEM_W(0,std::int32_t(lease.actor+0x9C+4*i))=std::int32_t(lease.derivatives[16+i]);
        for(unsigned i=0;i<9;++i)MEM_W(0,std::int32_t(lease.actor+4*i))=std::int32_t(lease.derivatives[19+i]);
        MEM_B(0x6A,std::int32_t(lease.actor))=std::int8_t(lease.dirty);
    }
    for(const auto& lease:runtime.integer_leases)
        if(word(rdram,lease.address)==lease.shown)MEM_W(0,std::int32_t(lease.address))=std::int32_t(lease.source);
    for(unsigned i=0;i<6;++i)MEM_W(0,std::int32_t(std::array<std::uint32_t,6>{0x800DAE54,0x800FE878,0x800DD3F0,0x801AC6D0,0x801A9530,0x801A48DC}[i]))=std::int32_t(runtime.echo_source[i]);
    runtime.draw_leases.clear();runtime.integer_leases.clear();runtime.drawing=false;
}
}
extern "C" int dora3_native_minigame_drawing() {
    return runtime.drawing&&runtime.definition&&runtime.definition->rom==0x18BDF0&&runtime.definition->update==0x801BF8CC;
}
extern "C" double dora3_native_minigame_visual(std::uint8_t* rdram,std::uint32_t address) {
    if(!dora3_native_minigame_drawing()||runtime.ram!=rdram||!runtime.source_time||!runtime.render_time)
        return std::int32_t(word(rdram,address));
    const double phase=std::clamp(double(*runtime.render_time-*runtime.source_time)*60./
                                  (double(second)*word(rdram,0x801A39DC)),0.,1.);
    // These are Source integer equations from BF8CC. Only their draw operands
    // borrow fractional time; direction, FSM, exit wait and controls stay Source.
    if(address==0x801C3650)
        return std::clamp(double(runtime.pulse_source[0])+(word(rdram,0x801C3678)?-phase:phase),0.,5.);
    if(address==0x801C3670)
        return std::clamp(double(runtime.pulse_source[1])+(word(rdram,0x801C3644)?20.*phase:-20.*phase),80.,255.);
    if(address==0x801C3674)
        return std::clamp(double(runtime.pulse_source[2])+(word(rdram,0x801C365C)?40.*phase:-40.*phase),80.,255.);
    return std::int32_t(word(rdram,address));
}

extern "C" void dora3_native_minigame_ground(std::uint8_t* rdram,recomp_context* c,std::uint32_t rom,unsigned mode) {
    if(!runtime.in_source||!runtime.definition||runtime.definition->rom!=rom)return;
    const auto actor=std::uint32_t(mode==1?c->r5:c->r17);
    if(actor<pool||actor>=pool+actor_count*actor_stride||(actor-pool)%actor_stride)return;
    GroundOwner owner{actor,actor_identity(rdram,actor),mode};
    if(!owner.identity.active)return;
    const auto at=std::find_if(runtime.ground_owners.begin(),runtime.ground_owners.end(),[&](const auto& p){return p.actor==actor;});
    if(at==runtime.ground_owners.end())runtime.ground_owners.push_back(owner);else *at=owner;
}

extern "C" void dora3_native_minigame_indirect(std::uint8_t* ram,recomp_context* c,std::uint32_t rom,unsigned end) {
    if(!runtime.in_source||!runtime.definition||runtime.definition->rom!=rom)return;
    if(!end) {++runtime.indirect_depth;return;}
    if(runtime.indirect_depth)--runtime.indirect_depth;
    dora3_native_minigame_call_return(ram,c,rom,0,1);
}
extern "C" void dora3_native_minigame_counter(std::uint8_t*,recomp_context* c,std::uint32_t instruction,double rate) {
    if(!runtime.in_source)return;
    const auto address=std::uint32_t(reg(*c,(instruction>>21)&31)+std::int16_t(instruction));
    runtime.counter_rates[address]=rate;
}
extern "C" void dora3_native_minigame_parameter(std::uint8_t* rdram,recomp_context* c) {
    if(!runtime.in_source)return;
    const auto actor=std::uint32_t(c->r4),slot=unsigned(c->r5);
    if(slot>=7||actor<pool||actor>=pool+actor_stride*actor_count||(actor-pool)%actor_stride)return;
    const auto t=runtime.gpr[6];
    if(!t.expression||t.wide||t.expected!=std::uint32_t(c->r6))return;
    const auto model=word(rdram,actor+0x74+4*slot);
    const auto token=game_model_animation().lifetime_token(model);
    if(!token||!pointer(model,0x80))return;
    auto owner=lifetime(rdram,actor);owner.model=model;owner.model_slot=actor+0x74+4*slot;owner.allocation=token;
    runtime.parameters.push_back({model,actor,t,owner});
}
