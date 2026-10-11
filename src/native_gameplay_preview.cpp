#include "graphics_profile.hpp"
#include "performance_profile.hpp"
#include "guest_storage.hpp"
#include "native_slot_reset.hpp"
#include "native_gameplay_preview.hpp"
#include "native_enemy.hpp"
#include "native_scene_objects.hpp"
#include "native_simulation_time.hpp"
#include "native_camera_timeline.hpp"
#include "native_input_timeline.hpp"
#include "native_game_loop.hpp"
#include "native_frame_bridge.hpp"
#include "native_graphics_pump.hpp"
#include "native_model_animation.hpp"
#include "recomp_context_copy.hpp"
#include <optional>
#include <algorithm>
#include <bit>
#include <memory>
#include <stdexcept>
#include <array>
#include <cmath>
#include <limits>
#include <cstring>
#include <unordered_map>
extern "C" void dora3_s3_801C8B28(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801E8BDC(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801E89CC(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801E810C(uint8_t*,recomp_context*);
extern "C" void dora3_s3_demo_801E88C4(uint8_t*,recomp_context*);
extern "C" void dora3_s0_80031A0C(uint8_t*,recomp_context*);
extern "C" void dora3_s3_demo_801E8590(uint8_t*,recomp_context*);
extern "C" void dora3_s3_demo_801E8838(uint8_t*,recomp_context*);
extern "C" int dora3_overlay_loaded(uint32_t);
extern "C" void dora3_s3_801D10D8(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801DBD58(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801BBDDC(uint8_t*,recomp_context*);
extern "C" void dora3_s0_8002F064(uint8_t*,recomp_context*);
extern "C" void dora3_s0_menu_8002A3B0(uint8_t*,recomp_context*);
extern "C" void dora3_s0_8002EA4C(uint8_t*,recomp_context*);
extern "C" void dora3_s0_8002A2E8(uint8_t*,recomp_context*);
extern "C" void dora3_s0_8002A328(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801C05F0(uint8_t*,recomp_context*);
extern "C" void dora3_s3_core_801CFA94(uint8_t*,recomp_context*);
extern "C" void dora3_s3_core_801D0088(uint8_t*,recomp_context*);
extern "C" void dora3_s0_8002A1B8(uint8_t*,recomp_context*);
extern "C" void dora3_s0_80028C74(uint8_t*,recomp_context*);
extern "C" void dora3_s3_core_801CB2F8(uint8_t*,recomp_context*);
extern "C" void dora3_s3_core_801CB3A8(uint8_t*,recomp_context*);
extern "C" void dora3_s3_core_801CB870(uint8_t*,recomp_context*);
extern "C" void dora3_lib_80057A80(uint8_t*,recomp_context*);
extern "C" void dora3_lib_80055BD0(uint8_t*,recomp_context*);
extern "C" void dora3_s3_core_801D05A8(uint8_t*,recomp_context*);
extern "C" void dora3_s3_core_801C120C(uint8_t*,recomp_context*);
extern "C" void dora3_s3_core_801C2798(uint8_t*,recomp_context*);
extern "C" void dora3_s3_core_801C554C(uint8_t*,recomp_context*);
extern "C" void dora3_s3_core_801C1A3C(uint8_t*,recomp_context*);
extern "C" void dora3_s3_core_801C1ADC(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801C0D88(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801C0E3C(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801C0EC0(uint8_t*,recomp_context*);
extern "C" void dora3_s3_core_801C8118(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801C0624(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801B922C(uint8_t*,recomp_context*);
extern "C" void dora3_s0_8002A328(uint8_t*,recomp_context*);
extern "C" void dora3_s3_core_801B91D4(uint8_t*,recomp_context*);
extern "C" void dora3_s1_801B4FBC(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801BA0C4(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801DCC1C(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801DCC50(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801DC880(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801DCDF0(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801DD8FC(uint8_t*,recomp_context*);
using namespace dora3::native_time;
static bool native_gameplay_preview_conditions(uint8_t*,bool,bool=false);
namespace {
int32_t word(uint8_t* ram,uint32_t p) {uint8_t* rdram=ram;return MEM_W(0,int32_t(p));}
bool byte_pointer(uint32_t p,unsigned size) {return p>=0x80000000u&&uint64_t(p)+size<=0x80800000ull;}
bool pointer(uint32_t p,unsigned size) {return !(p&3)&&byte_pointer(p,size);}
std::shared_ptr<SimulationTimeline> timeline;
uint64_t epoch=0;
// Reset clears permission flags, but an Original callback may still be on
// the stack. Its continuation must retain the epoch it entered with.
std::optional<uint64_t> active_player_epoch;
std::optional<SourceTimestamp> active_player_command_stamp;
Nanoseconds previous=0,source_origin=0;
bool motion_call=false,camera_call=false,object_call=false,projectile_call=false;
// One Source dialogue closure can precede its first Native slice by the
// sub-nanosecond ceiling used to submit an Original world update.
bool object_command_at_source_start=false;
bool motion_allowed=false,camera_allowed=false,object_allowed=false,projectile_allowed=false;
bool interaction_after_pose=false;
bool interaction_geometry_only=false;
bool object_command_at_end=false;
bool player_projectile_birth=false;
bool player_weapon_command_only=false;
std::array<uint32_t,20> projectile_before{};
std::array<uint32_t,20> projectile_birth_done{};
std::array<uint64_t,20> projectile_before_lifetimes{},projectile_birth_lifetimes{};
std::optional<SimulationSlice> gameplay_slice;
// BA05C changes pending-room globals in the middle of Original BB080.
// Finish only that already entered gameplay slice; never acquire a new one
// while the request is pending. Reset, a different request, or another
// eligibility condition revokes this permission immediately.
struct SceneCompletionSlice {SimulationSlice slice;uint64_t run;};
std::optional<SceneCompletionSlice> scene_completion_slice;
// Door109's real B8ED0 command sets D494=16. BB080 keeps its common
// world pool running during that fade, but skips player C8B28. Retain
// only a fade actually entered by this Native door lifetime/epoch.
struct DoorFadeOwner {uint32_t actor,physical,model;uint64_t lifetime,epoch;};
std::optional<DoorFadeOwner> door_fade_owner;
bool scene_fd_request(uint8_t* ram) {
    uint8_t* rdram=ram;
    return ram&&MEM_H(0,int32_t(0x801B36B8))==0xFD&&word(ram,0x802170EC)==1&&
        word(ram,0x80217104)==0x175&&word(ram,0x802170FC)==1&&
        uint32_t(word(ram,0x80217100))==0x80217104u&&uint32_t(word(ram,0x8021D474))==0x80217104u;
}
bool same_gameplay_slice(const SimulationSlice& a,const SimulationSlice& b) {
    return a.tick==b.tick&&a.phase==b.phase&&a.numerator==b.numerator&&a.denominator==b.denominator;
}
bool scene_completion_active(uint8_t* ram) {
    return scene_completion_slice&&gameplay_slice&&active_simulation_slice()&&
        scene_completion_slice->run==epoch&&same_gameplay_slice(scene_completion_slice->slice,*gameplay_slice)&&scene_fd_request(ram);
}
std::optional<SimulationSuspension> projectile_suspension;
std::optional<SimulationScope> projectile_scope;
std::optional<SimulationSuspension> object_suspension;
std::optional<SimulationScope> object_scope;
struct ProjectileClock {uint32_t physical=0,model=0;int kind=0;uint64_t origin=0,lifetime=0;};
std::array<ProjectileClock,20> projectile_clocks{};
struct ObjectClock {uint32_t physical=0,model=0;int kind=0;uint64_t origin=0,lifetime=0;bool local=false;};
std::array<ObjectClock,dora3::distance::common_capacity> object_clocks{};
struct DeferredEnemyBirth {ObjectClock owner;uint64_t deadline,denominator,epoch;};
std::array<std::optional<DeferredEnemyBirth>,dora3::distance::common_capacity> deferred_enemy_births{};
std::optional<std::array<ProjectileClock,20>> zero_shot_before;
uint64_t zero_shot_epoch=0;
std::array<uint64_t,20> projectile_start_births{};
struct ShotAttachment {
    struct Owner {uint32_t actor,physical,model;uint64_t lifetime,run;};
    std::array<Owner,2> owners{};
    std::array<int32_t,6> positions{};
    uint64_t generation=0;
};
std::optional<ShotAttachment> player_source_positions,player_action_positions;
struct PlayerFlightClock {
    uint32_t actor=0,physical=0,model=0;
    uint64_t lifetime=0,run=0,tick=0,denominator=0,start_phase=0,expected_phase=0;
    std::array<int32_t,12> expected{};
    std::array<int32_t,7> command{};
    std::array<float,6> initial{},target{};
    std::array<float,3> velocity{},change{};
    bool open=false;
};
std::optional<PlayerFlightClock> player_flight_clock;
struct PlayerHoverClock {PlayerFlightClock motion;int32_t expected_vertical_speed;};
std::optional<PlayerHoverClock> player_hover_clock;
struct PlayerHoverHeightClock {
    uint32_t actor,physical,model;
    uint64_t lifetime,run,tick,denominator,start_phase,expected_phase;
    int32_t velocity;
    float initial,expected;
};
std::optional<PlayerHoverHeightClock> player_hover_height;
struct PlayerActionCounter {
    uint32_t actor,physical,model;
    uint64_t lifetime,run,tick,denominator;
    uint16_t expected;
};
std::optional<PlayerActionCounter> player_hover_counter;
struct PlayerHoverThreshold {PlayerActionCounter owner;int16_t initial;};
std::optional<PlayerHoverThreshold> player_hover_threshold;
struct PlayerDeathPeriod {
    uint32_t actor,physical,model;
    uint64_t lifetime,run,tick,denominator;
    int32_t state;
};
std::optional<PlayerDeathPeriod> player_death_period,player_surface_action_period;
struct PlayerDriveClock {
    uint32_t actor,physical,model;
    uint64_t lifetime,run,tick,denominator,start_phase,expected_phase;
    std::array<int32_t,4> command;
    float initial,target,expected;
    bool clamped;
};
std::optional<PlayerDriveClock> player_drive_clock;
struct PlayerGravityClock {
    uint32_t actor,physical,model;
    uint64_t lifetime,run,tick,denominator,start_phase,expected_phase;
    int32_t gravity;
    float initial,target,expected;
    bool clamped;
};
std::optional<PlayerGravityClock> player_gravity_clock;
struct PlayerHeightClock {
    uint32_t actor,physical,model;
    uint64_t lifetime,run,tick,denominator,start_phase,expected_phase,gravity_start;
    int32_t initial_velocity,gravity;
    float initial,expected;
};
std::optional<PlayerHeightClock> player_height_clock;
struct PlayerTranslationClock {
    uint32_t actor,physical,model;
    uint64_t lifetime,run,tick,denominator,expected_phase;
    double position;
    float expected;
    std::array<int32_t,2> velocity;
    bool collision;
};
std::array<std::optional<PlayerTranslationClock>,2> player_translation_clocks;
struct PlayerCooldownRearm {
    uint32_t actor,physical,model;
    uint64_t lifetime,tick,denominator;
};
std::optional<PlayerCooldownRearm> player_cooldown_rearm;
struct PlayerDamageCounter {uint32_t actor,physical,model;uint64_t lifetime,run,tick,denominator;};
std::optional<PlayerDamageCounter> player_damage_counter;
std::optional<PlayerDamageCounter> player_surface_init,player_surface_counter;
std::optional<PlayerDamageCounter> player_wait_damage_counter,player_wait_damage_impulse;
struct PlayerControlPeriod {
    uint32_t actor,physical,control,model;
    uint64_t lifetime,tick,denominator;
};
std::optional<PlayerControlPeriod> player_idle_period;
struct PlayerTurnClock {
    uint32_t actor,physical,model;
    uint64_t lifetime,run,tick,denominator,start_phase,expected_phase;
    std::array<int32_t,2> command;
    float initial,delta,expected;
};
std::optional<PlayerTurnClock> player_turn_clock;
struct ObjectPeriodicPose {
    uint32_t actor,physical,model;uint64_t lifetime;
    float angle;bool first_period;
    uint64_t update_tick=0,update_phase=0;bool reset_xz=false;
};
struct ObjectReturnTurn {
    uint32_t actor,physical,model;uint64_t lifetime,run,tick,denominator,expected_phase;
    int32_t target,expected_yaw,direction;
};
std::array<std::optional<ObjectPeriodicPose>,dora3::distance::common_capacity> object_periodic_poses;
std::array<std::optional<ObjectReturnTurn>,dora3::distance::common_capacity> object_return_turns;
// The flash delay is a discrete command measured in Original periods. Its
// model lifetime/run and observed count/state prevent duplicate end commits.
struct FlashDelayCommit {
    uint32_t actor,physical,model;uint64_t lifetime,run,epoch,tick,denominator;
    std::array<int32_t,3> fields;
};
std::array<std::optional<FlashDelayCommit>,dora3::distance::common_capacity> flash_delay_commits;
// A Source-born reward gets its first actual Source pool pass before Native
// claims this model lifetime. Slot reuse and clip changes are different events.
struct RewardPoolClaim {uint32_t actor,physical,model;uint64_t lifetime,epoch;};
std::array<std::optional<RewardPoolClaim>,dora3::distance::common_capacity> reward_pool_claims;
struct RewardSourceCeiling {uint32_t actor,physical,model;uint64_t lifetime,epoch,tick,phase,denominator;};
std::array<std::optional<RewardSourceCeiling>,dora3::distance::common_capacity> reward_source_ceilings;
bool reward_pool_claim_matches(uint8_t* ram,uint32_t actor) {
    if(actor<0x804B34B8u||(actor-0x804B34B8u)%0xCC)return false;
    const auto slot=(actor-0x804B34B8u)/0xCC;
    if(slot>=reward_pool_claims.size()||!reward_pool_claims[slot])return false;
    const auto& claim=*reward_pool_claims[slot];
    return claim.actor==actor&&claim.epoch==epoch&&claim.physical==uint32_t(word(ram,actor+0x98))&&
        claim.model==uint32_t(word(ram,actor+0x74))&&claim.lifetime==game_model_animation().lifetime_token(claim.model);
}
struct FlashMotionClock {
    uint32_t actor,physical,model;
    uint64_t lifetime,run,epoch,tick,denominator,start_phase,expected_phase;
    float phase,radius,alpha,scale,velocity,gravity,y,expected_y;
    bool first_motion;
    std::array<int32_t,9> expected;
};
std::array<std::optional<FlashMotionClock>,dora3::distance::common_capacity> flash_motion_clocks;
struct PickupMotionClock {
    uint32_t actor,physical,model;int kind;
    uint64_t lifetime,run,epoch,tick,denominator,start_phase,expected_phase;
    float yaw,alpha,velocity,gravity,y,expected_y,end_velocity;
    std::array<int32_t,5> expected;
};
std::array<std::optional<PickupMotionClock>,dora3::distance::common_capacity> pickup_motion_clocks;
bool standard_pickup_kind(int kind) {return kind>=111&&kind<=128;}
struct PickupIdleSourcePeriod {
    uint32_t actor,physical,model;uint64_t lifetime,run,epoch,tick,denominator,start_phase,expected_phase,source_pass;
    std::array<int32_t,4> initial,expected;
};
std::array<std::optional<PickupIdleSourcePeriod>,dora3::distance::common_capacity> pickup_idle_source_periods;
// Pure reward geometry/scale/model time is displayed at Native intervals. Before
// the real Source callback, replay only unchanged continuous fields from the
// period's command baseline; actual threshold/heap/resource order stays Source.
struct RewardSourcePeriod {
    uint32_t actor,physical,model;
    int16_t kind;int32_t state;double velocity;
    uint64_t lifetime,run,time_command,epoch,tick,denominator,start_phase,start_scale_phase,expected_phase,source_pass;
    std::array<int32_t,8> initial,expected;
    int32_t initial_model_time,expected_model_time;
    int32_t initial_model_opacity,expected_model_opacity,initial_model_render_flag,expected_model_render_flag;
    uint8_t initial_model_flags,expected_model_flags;
    bool replayed=false;
};
std::array<std::optional<RewardSourcePeriod>,dora3::distance::common_capacity> reward_source_periods;
std::array<int32_t,8> reward_source_fields(uint8_t* rdram,uint32_t actor,uint32_t physical) {
    return {word(rdram,actor+0xA0),word(rdram,physical+0xC4),MEM_BU(0xC8,int32_t(physical)),MEM_BU(0x2A,int32_t(physical)),
        word(rdram,actor+0x18),word(rdram,actor+0x1C),word(rdram,actor+0x20),word(rdram,physical+0x30)};
}
bool reward_sequence_state(uint8_t* rdram,uint32_t actor,uint32_t physical) {
    const auto kind=MEM_H(0x20,int32_t(physical));const auto state=word(rdram,actor+0x94);
    return kind==129||kind==131||(standard_pickup_kind(kind)&&(state==5||state==6));
}
bool reward_source_identity(uint8_t* rdram,const RewardSourcePeriod& period) {
    return period.epoch==epoch&&pointer(period.actor,0xCC)&&pointer(period.physical,0xF4)&&
        uint32_t(word(rdram,period.actor+0x98))==period.physical&&uint32_t(word(rdram,period.actor+0x74))==period.model&&
        MEM_H(0x20,int32_t(period.physical))==period.kind&&
        game_model_animation().lifetime_token(period.model)==period.lifetime;
}
void reward_source_restore(uint8_t* rdram,RewardSourcePeriod& period) {
    const auto actor=period.actor,physical=period.physical,model=period.model;
    if(word(rdram,actor+0xA0)==period.expected[0])MEM_W(0,int32_t(actor+0xA0))=period.initial[0];
    if(word(rdram,physical+0xC4)==period.expected[1])MEM_W(0,int32_t(physical+0xC4))=period.initial[1];
    if(MEM_BU(0xC8,int32_t(physical))==period.expected[2])MEM_B(0xC8,int32_t(physical))=int8_t(period.initial[2]);
    if(MEM_BU(0x2A,int32_t(physical))==period.expected[3])MEM_B(0x2A,int32_t(physical))=int8_t(period.initial[3]);
    for(unsigned i=4;i<8;++i) {
        const auto address=i==7?physical+0x30:actor+0x18+4*(i-4);
        if(word(rdram,address)==period.expected[i])MEM_W(0,int32_t(address))=period.initial[i];
    }
    // 8003B168 copies common opacity into model78 and derives model74.
    // A Source actor pause skips that setter, so both previewed fields must
    // return to their unchanged command baseline along with physics C4.
    if(word(rdram,model+0x78)==period.expected_model_opacity)MEM_W(0,int32_t(model+0x78))=period.initial_model_opacity;
    if(word(rdram,model+0x74)==period.expected_model_render_flag)MEM_W(0,int32_t(model+0x74))=period.initial_model_render_flag;
    // An actual seek/channel command owns its new time even if it happens to
    // equal the displayed Native value. Flag commands preserve their new flag
    // while the unchanged time still returns to the Source command baseline.
    if(game_model_animation().time_command_token(model)==period.time_command) {
        if(word(rdram,model+0x64)==period.expected_model_time)MEM_W(0,int32_t(model+0x64))=period.initial_model_time;
        const auto flags=MEM_BU(0x7C,int32_t(model));
        // Native changes only the completion pulse bit2. Preserve a new
        // pause/clamp/enable command while restoring that unchanged pulse.
        if((flags&2)==(period.expected_model_flags&2))MEM_B(0x7C,int32_t(model))=int8_t((flags&~2u)|(period.initial_model_flags&2));
    }
    period.replayed=true;
}

// Idle1 has a pure yaw callback; the actual common tail adds VX/VY/VZ.
// Retain a Source command baseline for moving and stopped items alike, so
// collection, a late pause, or an external velocity command cannot consume
// a displayed fractional position as a second Original movement step.
bool pickup_idle_motion_eligible(uint8_t* rdram,uint32_t actor,uint32_t physical) {
    if(!standard_pickup_kind(MEM_H(0x20,int32_t(physical)))||word(rdram,actor+0x94)!=1||MEM_H(0x7C,int32_t(physical))!=-1)return false;
    const auto blink=MEM_H(0xD8,int32_t(physical));if(blink!=0&&blink!=1)return false;
    for(auto address:{actor+0x10,actor+0x9C,actor+0xA0,actor+0xA4,physical+0x10,physical+0x14,physical+0x18})
        if(!std::isfinite(std::bit_cast<float>(uint32_t(word(rdram,address)))))return false;
    return true;
}
struct PickupIdleMotionSourcePeriod {
    RewardSourcePeriod common;
    std::array<int32_t,6> initial,expected;
};
std::array<std::optional<PickupIdleMotionSourcePeriod>,dora3::distance::common_capacity> pickup_idle_motion_source_periods;
std::array<uint32_t,6> pickup_idle_motion_addresses(uint32_t actor,uint32_t physical) {
    return {actor+0x10,actor+0x9C,actor+0xA4,physical+0x10,physical+0x14,physical+0x18};
}
std::array<int32_t,6> pickup_idle_motion_fields(uint8_t* rdram,uint32_t actor,uint32_t physical) {
    std::array<int32_t,6> fields{};const auto addresses=pickup_idle_motion_addresses(actor,physical);
    for(unsigned i=0;i<fields.size();++i)fields[i]=word(rdram,addresses[i]);
    return fields;
}
void pickup_idle_motion_restore(uint8_t* rdram,PickupIdleMotionSourcePeriod& period) {
    const auto addresses=pickup_idle_motion_addresses(period.common.actor,period.common.physical);
    for(unsigned i=0;i<addresses.size();++i)
        if(word(rdram,addresses[i])==period.expected[i])MEM_W(0,int32_t(addresses[i]))=period.initial[i];
    reward_source_restore(rdram,period.common);
}
void pickup_idle_motion_begin(uint8_t* rdram,uint32_t actor,uint32_t physical,uint32_t model,unsigned slot,uint64_t source_pass) {
    const auto& slice=*active_simulation_slice();const auto fields=reward_source_fields(rdram,actor,physical);const auto motion=pickup_idle_motion_fields(rdram,actor,physical);
    const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model),time_command=game_model_animation().time_command_token(model);
    const auto model_time=word(rdram,model+0x64);const auto flags=MEM_BU(0x7C,int32_t(model));auto& prior=pickup_idle_motion_source_periods[slot];
    if(!prior||!reward_source_identity(rdram,prior->common)||prior->common.actor!=actor||prior->common.tick!=slice.tick||
       prior->common.denominator!=slice.denominator||prior->common.expected_phase!=slice.phase) {
        const RewardSourcePeriod common{actor,physical,model,MEM_H(0x20,int32_t(physical)),1,0,lifetime,run,time_command,epoch,slice.tick,slice.denominator,slice.phase,slice.phase,slice.phase,
            source_pass,fields,fields,model_time,model_time,word(rdram,model+0x78),word(rdram,model+0x78),word(rdram,model+0x74),word(rdram,model+0x74),flags,flags,false};
        prior=PickupIdleMotionSourcePeriod{common,motion,motion};
    } else {
        auto& common=prior->common;
        for(unsigned i=0;i<fields.size();++i)if(common.expected[i]!=fields[i])common.initial[i]=fields[i];
        for(unsigned i=0;i<motion.size();++i)if(prior->expected[i]!=motion[i])prior->initial[i]=motion[i];
        if(common.time_command!=time_command||common.expected_model_time!=model_time)common.initial_model_time=model_time;
        if(common.time_command!=time_command||common.expected_model_flags!=flags)common.initial_model_flags=flags;
        if(common.expected_model_opacity!=word(rdram,model+0x78))common.initial_model_opacity=word(rdram,model+0x78);
        if(common.expected_model_render_flag!=word(rdram,model+0x74))common.initial_model_render_flag=word(rdram,model+0x74);
        common.run=run;common.time_command=time_command;
    }
}
void pickup_idle_motion_end(uint8_t* rdram,uint32_t actor,unsigned slot,uint64_t source_pass,bool native,bool entered) {
    if(slot>=pickup_idle_motion_source_periods.size())return;
    auto& pending=pickup_idle_motion_source_periods[slot];if(!pending)return;
    auto& common=pending->common;
    if(native&&active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        if(entered&&reward_source_identity(rdram,common)&&common.tick==slice.tick&&common.denominator==slice.denominator) {
            pending->expected=pickup_idle_motion_fields(rdram,actor,common.physical);common.expected=reward_source_fields(rdram,actor,common.physical);
            common.expected_phase=slice.phase+slice.numerator;common.expected_model_time=word(rdram,common.model+0x64);
            common.expected_model_opacity=word(rdram,common.model+0x78);common.expected_model_render_flag=word(rdram,common.model+0x74);
            common.expected_model_flags=MEM_BU(0x7C,int32_t(common.model));common.run=game_model_animation().run_token(common.model);
            common.time_command=game_model_animation().time_command_token(common.model);
        }
    } else if(!native) {
        if(!common.replayed&&reward_source_identity(rdram,common)&&MEM_BU(0xC9,int32_t(common.physical))==1&&common.expected_phase<=common.denominator&&common.source_pass+1==source_pass)
            pickup_idle_motion_restore(rdram,*pending);
        pending.reset();
    }
}

// Source owns initialized flash130 query/retirement commands. Replay only
// unchanged displayed fields before that callback, including a late C9 pause.
struct FlashSourcePeriod {
    RewardSourcePeriod common;
    std::array<int32_t,7> initial,expected;
};
std::array<std::optional<FlashSourcePeriod>,dora3::distance::common_capacity> flash_source_periods;
std::array<int32_t,7> flash_source_fields(uint8_t* rdram,uint32_t actor,uint32_t physical) {
    return {word(rdram,actor+0x9C),word(rdram,actor+0xA4),MEM_H(0x34,int32_t(physical)),
        word(rdram,physical+0x5C),word(rdram,physical+0x58),word(rdram,physical+0x14),word(rdram,physical+0x1C)};
}
void flash_source_restore(uint8_t* rdram,FlashSourcePeriod& period) {
    const auto actor=period.common.actor,physical=period.common.physical;
    const std::array<uint32_t,7> addresses{actor+0x9C,actor+0xA4,physical+0x34,physical+0x5C,physical+0x58,physical+0x14,physical+0x1C};
    for(unsigned i=0;i<addresses.size();++i) {
        const auto actual=i==2?MEM_H(0,int32_t(addresses[i])):word(rdram,addresses[i]);
        if(actual==period.expected[i]) {
            if(i==2)MEM_H(0,int32_t(addresses[i]))=int16_t(period.initial[i]);
            else MEM_W(0,int32_t(addresses[i]))=period.initial[i];
        }
    }
    reward_source_restore(rdram,period.common);
}
void flash_source_begin(uint8_t* rdram,uint32_t actor,uint32_t physical,uint32_t model,unsigned slot,uint64_t source_pass) {
    const auto& slice=*active_simulation_slice();const auto fields=reward_source_fields(rdram,actor,physical);const auto motion=flash_source_fields(rdram,actor,physical);
    const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model),time_command=game_model_animation().time_command_token(model);
    const auto model_time=word(rdram,model+0x64);const auto flags=MEM_BU(0x7C,int32_t(model));auto& old=flash_source_periods[slot];
    if(!old||!reward_source_identity(rdram,old->common)||old->common.actor!=actor||old->common.physical!=physical||old->common.model!=model||
       old->common.tick!=slice.tick||old->common.denominator!=slice.denominator||old->common.expected_phase!=slice.phase) {
        const RewardSourcePeriod common{actor,physical,model,130,10,0,lifetime,run,time_command,epoch,slice.tick,slice.denominator,slice.phase,slice.phase,slice.phase,
            source_pass,fields,fields,model_time,model_time,word(rdram,model+0x78),word(rdram,model+0x78),word(rdram,model+0x74),word(rdram,model+0x74),flags,flags,false};
        old=FlashSourcePeriod{common,motion,motion};
    } else {
        auto& common=old->common;
        for(unsigned i=0;i<fields.size();++i)if(common.expected[i]!=fields[i])common.initial[i]=fields[i];
        for(unsigned i=0;i<motion.size();++i)if(old->expected[i]!=motion[i])old->initial[i]=motion[i];
        if(common.time_command!=time_command||common.expected_model_time!=model_time)common.initial_model_time=model_time;
        if(common.time_command!=time_command||common.expected_model_flags!=flags)common.initial_model_flags=flags;
        if(common.expected_model_opacity!=word(rdram,model+0x78))common.initial_model_opacity=word(rdram,model+0x78);
        if(common.expected_model_render_flag!=word(rdram,model+0x74))common.initial_model_render_flag=word(rdram,model+0x74);
        common.run=run;common.time_command=time_command;
    }
}
void flash_source_end(uint8_t* rdram,uint32_t actor,unsigned slot,uint64_t source_pass,bool native,bool entered) {
    if(slot>=flash_source_periods.size())return;
    auto& old=flash_source_periods[slot];if(!old)return;
    auto& common=old->common;
    if(native&&active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        if(entered&&reward_source_identity(rdram,common)&&common.tick==slice.tick&&common.denominator==slice.denominator) {
            old->expected=flash_source_fields(rdram,actor,common.physical);common.expected=reward_source_fields(rdram,actor,common.physical);
            common.expected_phase=slice.phase+slice.numerator;common.expected_model_time=word(rdram,common.model+0x64);
            common.expected_model_opacity=word(rdram,common.model+0x78);common.expected_model_render_flag=word(rdram,common.model+0x74);
            common.expected_model_flags=MEM_BU(0x7C,int32_t(common.model));common.run=game_model_animation().run_token(common.model);
            common.time_command=game_model_animation().time_command_token(common.model);
        }
    } else if(!native) {
        if(!common.replayed&&reward_source_identity(rdram,common)&&MEM_BU(0xC9,int32_t(common.physical))==1&&common.expected_phase<=common.denominator&&common.source_pass+1==source_pass)
            flash_source_restore(rdram,*old);
        old.reset();
    }
}

// A Source command can reject an owner partway through its period. Replay
// every valid displayed fraction, preserving external fields; a full-period
// requirement would discard the baseline on a mid-interval detach or pause.
// Source owns state2 initialization, surface decisions and the state1 command.
// Native previews only the initialized no-parent trajectory. The actual query
// below uses the Source command baseline and rolls back all of its guest writes.
struct PickupSettleSourcePeriod {
    RewardSourcePeriod common;
    int32_t initial_velocity,expected_velocity;
    float end_velocity=0;
    uint32_t parent=0;
    std::array<int32_t,5> initial_pose{},expected_pose{};
};
std::array<std::optional<PickupSettleSourcePeriod>,dora3::distance::common_capacity> pickup_settle_source_periods;
std::array<uint32_t,5> pickup_attachment_addresses(uint32_t actor) {
    return {actor+0xC,actor+0x10,actor+0x9C,actor+0xA0,actor+0xA4};
}
std::array<int32_t,5> pickup_attachment_fields(uint8_t* rdram,uint32_t actor) {
    std::array<int32_t,5> values{};const auto addresses=pickup_attachment_addresses(actor);
    for(unsigned i=0;i<values.size();++i)values[i]=word(rdram,addresses[i]);
    return values;
}
void pickup_settle_restore(uint8_t* rdram,PickupSettleSourcePeriod& period) {
    if(period.parent) {
        const auto addresses=pickup_attachment_addresses(period.common.actor);
        for(unsigned i=0;i<addresses.size();++i)
            if(word(rdram,addresses[i])==period.expected_pose[i])MEM_W(0,int32_t(addresses[i]))=period.initial_pose[i];
    }
    const auto address=period.common.physical+0x14;
    if(word(rdram,address)==period.expected_velocity)MEM_W(0,int32_t(address))=period.initial_velocity;
    reward_source_restore(rdram,period.common);
}
void pickup_settle_begin(uint8_t* rdram,uint32_t actor,uint32_t physical,uint32_t model,unsigned slot,uint64_t source_pass) {
    const auto& slice=*active_simulation_slice();const auto fields=reward_source_fields(rdram,actor,physical);
    const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model),time_command=game_model_animation().time_command_token(model);
    const auto model_time=word(rdram,model+0x64);const auto flags=MEM_BU(0x7C,int32_t(model));const auto velocity=word(rdram,physical+0x14);
    const auto parent_index=MEM_H(0x7C,int32_t(physical));
    const auto parent=parent_index==-1?0u:0x804B1CD0u+uint32_t(parent_index)*0xCC;
    const auto pose=pickup_attachment_fields(rdram,actor);
    auto& prior=pickup_settle_source_periods[slot];
    if(!prior||prior->common.actor!=actor||prior->common.physical!=physical||prior->common.model!=model||prior->common.lifetime!=lifetime||
       prior->common.epoch!=epoch||prior->common.tick!=slice.tick||prior->common.denominator!=slice.denominator||prior->common.expected_phase!=slice.phase||prior->parent!=parent) {
        const auto previous=prior;
        const RewardSourcePeriod common{actor,physical,model,MEM_H(0x20,int32_t(physical)),2,0,lifetime,run,time_command,epoch,slice.tick,slice.denominator,slice.phase,slice.phase,slice.phase,
            source_pass,fields,fields,model_time,model_time,word(rdram,model+0x78),word(rdram,model+0x78),word(rdram,model+0x74),word(rdram,model+0x74),flags,flags,false};
        prior=PickupSettleSourcePeriod{common,velocity,velocity};
        prior->parent=parent;prior->initial_pose=prior->expected_pose=pose;
        // Rebinding within this Source period changes the geometry domain,
        // not the unchanged common opacity/model/pose command baselines.
        // Preserve them even if the old owner was rejected for a fraction;
        // explicit writes still win through each expected-value comparison.
        if(previous&&reward_source_identity(rdram,previous->common)&&previous->common.actor==actor&&previous->common.physical==physical&&previous->common.model==model&&
           previous->common.tick==slice.tick&&previous->common.denominator==slice.denominator&&previous->common.source_pass==source_pass) {
            const auto& old=previous->common;auto& next=prior->common;
            for(unsigned i=0;i<fields.size();++i)if(fields[i]==old.expected[i])next.initial[i]=old.initial[i];
            if(velocity==previous->expected_velocity)prior->initial_velocity=previous->initial_velocity;
            for(unsigned i=0;i<pose.size();++i) {
                const auto expected=i==3?old.expected[0]:previous->expected_pose[i];
                if(pose[i]==expected)prior->initial_pose[i]=i==3?old.initial[0]:previous->initial_pose[i];
            }
            if(time_command==old.time_command) {
                if(model_time==old.expected_model_time)next.initial_model_time=old.initial_model_time;
                if((flags&2)==(old.expected_model_flags&2))next.initial_model_flags=uint8_t((flags&~2u)|(old.initial_model_flags&2));
            }
            if(word(rdram,model+0x78)==old.expected_model_opacity)next.initial_model_opacity=old.initial_model_opacity;
            if(word(rdram,model+0x74)==old.expected_model_render_flag)next.initial_model_render_flag=old.initial_model_render_flag;
        }
    } else {
        auto& common=prior->common;
        for(unsigned i=0;i<fields.size();++i)if(common.expected[i]!=fields[i]) {
            common.initial[i]=fields[i];if(i==0)common.start_phase=slice.phase;
        }
        if(parent)for(unsigned i=0;i<pose.size();++i)
            if(prior->expected_pose[i]!=pose[i])prior->initial_pose[i]=pose[i];
        if(prior->expected_velocity!=velocity){prior->initial_velocity=velocity;common.start_phase=slice.phase;}
        if(common.time_command!=time_command||common.expected_model_time!=model_time)common.initial_model_time=model_time;
        if(common.time_command!=time_command||common.expected_model_flags!=flags)common.initial_model_flags=flags;
        if(common.expected_model_opacity!=word(rdram,model+0x78))common.initial_model_opacity=word(rdram,model+0x78);
        if(common.expected_model_render_flag!=word(rdram,model+0x74))common.initial_model_render_flag=word(rdram,model+0x74);
        common.run=run;common.time_command=time_command;
    }
}
struct PickupVerticalForecast {float velocity;bool collision;};
PickupVerticalForecast pickup_vertical_forecast(uint8_t* rdram,recomp_context* ctx,uint32_t actor,uint32_t physical,int32_t height,int32_t velocity,float offset) {
    const auto stack=uint32_t(ctx->r29);
    // Reviewed closure: E810C/E7EB8/E75D0/40104, two result resets/copy and
    // pure triangle math. It writes Y velocity, the60-byte candidate/flag block
    // and guest stack only. Maximum reviewed frame chain is below512 bytes;
    // preserve4KiB below SP plus the outgoing argument area independently of
    // the caller context. No heap, RNG, model or resource command is invoked.
    if((stack&7)||stack<4096||!pointer(stack-4096,4128))throw std::logic_error("Invalid pickup settle query stack");
    struct QueryRollback {
        uint8_t* ram;uint32_t actor,physical,stack;
        int32_t height,velocity;
        std::array<uint8_t,4128> scratch;
        std::array<uint8_t,60> candidates;
        QueryRollback(uint8_t* rdram,uint32_t a,uint32_t ph,uint32_t sp):ram(rdram),actor(a),physical(ph),stack(sp),height(word(rdram,a+0xA0)),velocity(word(rdram,ph+0x14)) {
            std::memcpy(scratch.data(),ram+((stack-4096)&0x1FFFFFFFu),scratch.size());
            std::memcpy(candidates.data(),ram+(0x800DACB8u&0x1FFFFFFFu),candidates.size());
        }
        ~QueryRollback() {
            uint8_t* rdram=ram;
            MEM_W(0,int32_t(actor+0xA0))=height;MEM_W(0,int32_t(physical+0x14))=velocity;
            std::memcpy(ram+((stack-4096)&0x1FFFFFFFu),scratch.data(),scratch.size());
            std::memcpy(ram+(0x800DACB8u&0x1FFFFFFFu),candidates.data(),candidates.size());
        }
    } rollback(rdram,actor,physical,stack);
    MEM_W(0,int32_t(actor+0xA0))=height;MEM_W(0,int32_t(physical+0x14))=velocity;
    recomp_context query;dora3::copy_recomp_context(query,*ctx);
    query.r4=int32_t(actor);query.r5=0;query.r6=0;query.r7=std::bit_cast<int32_t>(offset);MEM_W(0x10,query.r29)=0;
    {SimulationSuspension source;dora3_s3_801E810C(rdram,&query);}
    return {std::bit_cast<float>(uint32_t(word(rdram,physical+0x14))),query.r2!=0};
}
float pickup_settle_forecast(uint8_t* rdram,recomp_context* ctx,const PickupSettleSourcePeriod& period) {
    const auto query=pickup_vertical_forecast(rdram,ctx,period.common.actor,period.common.physical,period.common.initial[0],period.initial_velocity,-50.f);
    // EBAD4 overwrites even the collision-adjusted velocity with zero before
    // the common movement tail. It does not snap Y to the plane's height.
    return query.collision?0.f:query.velocity;
}

// Source owns initialization/RNG, collision headings, count34 and state1.
// Native exposes only the initialized standard trajectory and visual turn.
struct PickupScatterSourcePeriod {
    RewardSourcePeriod common;
    std::array<int32_t,7> initial,expected; // yaw10,X,Z,speed0,VX,VY,VZ
    std::array<int32_t,12> proposal{};
};
std::array<std::optional<PickupScatterSourcePeriod>,dora3::distance::common_capacity> pickup_scatter_source_periods;
std::array<uint32_t,7> pickup_scatter_addresses(uint32_t actor,uint32_t physical) {
    return {actor+0x10,actor+0x9C,actor+0xA4,physical,physical+0x10,physical+0x14,physical+0x18};
}
std::array<int32_t,7> pickup_scatter_fields(uint8_t* ram,uint32_t actor,uint32_t physical) {
    std::array<int32_t,7> result{};const auto fields=pickup_scatter_addresses(actor,physical);
    for(unsigned i=0;i<fields.size();++i)result[i]=word(ram,fields[i]);
    return result;
}
void pickup_scatter_restore(uint8_t* ram,PickupScatterSourcePeriod& period) {
    uint8_t* rdram=ram;const auto addresses=pickup_scatter_addresses(period.common.actor,period.common.physical);
    for(unsigned i=0;i<addresses.size();++i)if(word(ram,addresses[i])==period.expected[i])MEM_W(0,int32_t(addresses[i]))=period.initial[i];
    reward_source_restore(ram,period.common);
}
void pickup_scatter_begin(uint8_t* rdram,uint32_t actor,uint32_t physical,uint32_t model,unsigned slot,uint64_t source_pass) {
    const auto& slice=*active_simulation_slice();const auto fields=reward_source_fields(rdram,actor,physical);
    const auto scatter=pickup_scatter_fields(rdram,actor,physical);
    const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model),time_command=game_model_animation().time_command_token(model);
    const auto model_time=word(rdram,model+0x64);const auto flags=MEM_BU(0x7C,int32_t(model));auto& prior=pickup_scatter_source_periods[slot];
    if(!prior||prior->common.actor!=actor||prior->common.physical!=physical||prior->common.model!=model||prior->common.lifetime!=lifetime||
       prior->common.epoch!=epoch||prior->common.tick!=slice.tick||prior->common.denominator!=slice.denominator||prior->common.expected_phase!=slice.phase) {
        const RewardSourcePeriod common{actor,physical,model,MEM_H(0x20,int32_t(physical)),3,0,lifetime,run,time_command,epoch,slice.tick,slice.denominator,slice.phase,slice.phase,slice.phase,
            source_pass,fields,fields,model_time,model_time,word(rdram,model+0x78),word(rdram,model+0x78),word(rdram,model+0x74),word(rdram,model+0x74),flags,flags,false};
        prior=PickupScatterSourcePeriod{common,scatter,scatter,{}};
    } else {
        auto& common=prior->common;
        for(unsigned i=0;i<fields.size();++i)if(common.expected[i]!=fields[i]) {common.initial[i]=fields[i];if(i==0)common.start_phase=slice.phase;}
        for(unsigned i=0;i<scatter.size();++i)if(prior->expected[i]!=scatter[i]) {prior->initial[i]=scatter[i];common.start_phase=slice.phase;}
        if(common.time_command!=time_command||common.expected_model_time!=model_time)common.initial_model_time=model_time;
        if(common.time_command!=time_command||common.expected_model_flags!=flags)common.initial_model_flags=flags;
        if(common.expected_model_opacity!=word(rdram,model+0x78))common.initial_model_opacity=word(rdram,model+0x78);
        if(common.expected_model_render_flag!=word(rdram,model+0x74))common.initial_model_render_flag=word(rdram,model+0x74);
        common.run=run;common.time_command=time_command;
    }
}

extern "C" void dora3_s3_core_801EBCC0(uint8_t*,recomp_context*);
extern "C" void dora3_s3_core_801EBAD4(uint8_t*,recomp_context*);
std::array<int32_t,12> pickup_scatter_forecast(uint8_t* rdram,recomp_context* ctx,const PickupScatterSourcePeriod& period) {
    const auto actor=period.common.actor,physical=period.common.physical,stack=uint32_t(ctx->r29);
    if(!pointer(actor,0xCC)||!pointer(physical,0xF4)||(stack&7)||stack<4096||!pointer(stack-4096,4128))
        throw std::logic_error("Invalid scatter forecast fixture identity/stack");
    // Static28-function closure has no external call. The initialized state3
    // proposal can write its own actor/physics, stack, the two collision result
    // candidates, halfword collision flags and the Original RNG seed. This
    // reviewed rollback forecast supports initialized Native3 movement. The
    // separate Source prototype checks ALL4MiB RAM/context/heap/resources/commands
    // and model count; Native guards also compare RNG/cache/terrain and whole lifetimes.
    struct ScatterRollback {
        uint8_t* ram;uint32_t actor,physical,stack;int32_t random_word,contact_word;
        std::array<uint8_t,0xCC> actor_record;
        std::array<uint8_t,0xF4> physical_record;
        std::array<uint8_t,4128> scratch;
        std::array<uint8_t,60> candidates;
        ScatterRollback(uint8_t* rdram,uint32_t a,uint32_t ph,uint32_t sp):ram(rdram),actor(a),physical(ph),stack(sp),
            random_word(word(rdram,0x801AA620)),contact_word(word(rdram,0x8021BDFC)) {
            std::memcpy(actor_record.data(),ram+(actor&0x1FFFFFFFu),actor_record.size());
            std::memcpy(physical_record.data(),ram+(physical&0x1FFFFFFFu),physical_record.size());
            std::memcpy(scratch.data(),ram+((stack-4096)&0x1FFFFFFFu),scratch.size());
            std::memcpy(candidates.data(),ram+(0x800DACB8u&0x1FFFFFFFu),candidates.size());
        }
        ~ScatterRollback() {
            uint8_t* rdram=ram;
            std::memcpy(ram+(actor&0x1FFFFFFFu),actor_record.data(),actor_record.size());
            std::memcpy(ram+(physical&0x1FFFFFFFu),physical_record.data(),physical_record.size());
            std::memcpy(ram+((stack-4096)&0x1FFFFFFFu),scratch.data(),scratch.size());
            std::memcpy(ram+(0x800DACB8u&0x1FFFFFFFu),candidates.data(),candidates.size());
            MEM_W(0,int32_t(0x801AA620))=random_word;MEM_W(0,int32_t(0x8021BDFC))=contact_word;
        }
    } rollback(rdram,actor,physical,stack);
    const auto addresses=pickup_scatter_addresses(actor,physical);
    for(unsigned i=0;i<addresses.size();++i)MEM_W(0,int32_t(addresses[i]))=period.initial[i];
    MEM_W(0,int32_t(actor+0xA0))=period.common.initial[0];
    recomp_context query;dora3::copy_recomp_context(query,*ctx);
    if(active_simulation_slice()) {SimulationSuspension source;dora3_s3_core_801EBCC0(rdram,&query);}
    else dora3_s3_core_801EBCC0(rdram,&query); // Source entry already has no Native simulation to suspend.
    const std::array<int32_t,12> values{word(rdram,actor+0x10),word(rdram,actor+0x9C),word(rdram,actor+0xA0),word(rdram,actor+0xA4),
        word(rdram,physical),word(rdram,physical+4),word(rdram,physical+0x10),word(rdram,physical+0x14),word(rdram,physical+0x18),
        word(rdram,actor+0x94),MEM_BU(0x28,int32_t(physical)),MEM_H(0x34,int32_t(physical))};
    return values;
}

// State7 has a separate replay record: Source owns first initialization,
// blink reset and final retirement; Native exposes yaw/speed/model opacity.
struct PickupSpinSourcePeriod {
    RewardSourcePeriod common;
    std::array<int32_t,3> initial,expected; // yaw10, spin58, opacity5C
};
std::array<std::optional<PickupSpinSourcePeriod>,dora3::distance::common_capacity> pickup_spin_source_periods;
std::array<int32_t,3> pickup_spin_fields(uint8_t* ram,uint32_t actor,uint32_t physical) {
    return {word(ram,actor+0x10),word(ram,physical+0x58),word(ram,physical+0x5C)};
}
void pickup_spin_restore(uint8_t* ram,PickupSpinSourcePeriod& period) {
    uint8_t* rdram=ram;
    const std::array<uint32_t,3> addresses{period.common.actor+0x10,period.common.physical+0x58,period.common.physical+0x5C};
    for(unsigned i=0;i<3;++i)if(word(ram,addresses[i])==period.expected[i])MEM_W(0,int32_t(addresses[i]))=period.initial[i];
    reward_source_restore(ram,period.common);
}
void pickup_spin_begin(uint8_t* ram,uint32_t actor,uint32_t physical,uint32_t model,unsigned slot,uint64_t source_pass) {
    uint8_t* rdram=ram;const auto& slice=*active_simulation_slice();
    const auto fields=reward_source_fields(ram,actor,physical);const auto spin=pickup_spin_fields(ram,actor,physical);
    const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model),time_command=game_model_animation().time_command_token(model);
    const auto model_time=word(ram,model+0x64);const auto flags=MEM_BU(0x7C,int32_t(model));
    auto& prior=pickup_spin_source_periods[slot];
    if(!prior||prior->common.actor!=actor||prior->common.physical!=physical||prior->common.model!=model||prior->common.lifetime!=lifetime||
       prior->common.epoch!=epoch||prior->common.tick!=slice.tick||prior->common.denominator!=slice.denominator||prior->common.expected_phase!=slice.phase) {
        const RewardSourcePeriod common{actor,physical,model,MEM_H(0x20,int32_t(physical)),7,0,lifetime,run,time_command,epoch,slice.tick,slice.denominator,slice.phase,slice.phase,slice.phase,
            source_pass,fields,fields,model_time,model_time,word(ram,model+0x78),word(ram,model+0x78),word(ram,model+0x74),word(ram,model+0x74),flags,flags,false};
        prior=PickupSpinSourcePeriod{common,spin,spin};
    } else {
        for(unsigned i=0;i<3;++i)if(prior->expected[i]!=spin[i])prior->initial[i]=spin[i];
        auto& common=prior->common;
        for(unsigned i=0;i<fields.size();++i)if(common.expected[i]!=fields[i])common.initial[i]=fields[i];
        if(common.time_command!=time_command||common.expected_model_time!=model_time)common.initial_model_time=model_time;
        if(common.time_command!=time_command||common.expected_model_flags!=flags)common.initial_model_flags=flags;
        if(common.expected_model_opacity!=word(ram,model+0x78))common.initial_model_opacity=word(ram,model+0x78);
        if(common.expected_model_render_flag!=word(ram,model+0x74))common.initial_model_render_flag=word(ram,model+0x74);
        common.run=run;common.time_command=time_command;
    }
}
// State8 exposes its two-phase geometry without publishing integer angle,
// phase, latch or state commands. Source retains their original order.
struct PickupArcSourcePeriod {
    RewardSourcePeriod common;
    std::array<int32_t,2> initial,expected; // actor X9C and ZA4; Y/scales in common
    std::array<int16_t,5> command; // phase34, angle36, target44, anchorBE/C0
    uint64_t start_angle_phase;
};
std::array<std::optional<PickupArcSourcePeriod>,dora3::distance::common_capacity> pickup_arc_source_periods;
std::array<int32_t,2> pickup_arc_fields(uint8_t* ram,uint32_t actor) {
    return {word(ram,actor+0x9C),word(ram,actor+0xA4)};
}
std::array<int16_t,5> pickup_arc_commands(uint8_t* rdram,uint32_t physical) {
    return {MEM_H(0x34,int32_t(physical)),MEM_H(0x36,int32_t(physical)),
            MEM_H(0x44,int32_t(physical)),MEM_H(0xBE,int32_t(physical)),MEM_H(0xC0,int32_t(physical))};
}
void pickup_arc_restore(uint8_t* rdram,PickupArcSourcePeriod& period) {
    const std::array<uint32_t,2> addresses{period.common.actor+0x9C,period.common.actor+0xA4};
    for(unsigned i=0;i<2;++i)
        if(word(rdram,addresses[i])==period.expected[i])MEM_W(0,int32_t(addresses[i]))=period.initial[i];
    reward_source_restore(rdram,period.common);
}
void pickup_arc_begin(uint8_t* rdram,uint32_t actor,uint32_t physical,uint32_t model,unsigned slot,uint64_t source_pass) {
    const auto& slice=*active_simulation_slice();
    const auto fields=reward_source_fields(rdram,actor,physical);
    const auto arc=pickup_arc_fields(rdram,actor);
    const auto command=pickup_arc_commands(rdram,physical);
    const auto lifetime=game_model_animation().lifetime_token(model);
    const auto run=game_model_animation().run_token(model),time_command=game_model_animation().time_command_token(model);
    const auto model_time=word(rdram,model+0x64);const auto flags=MEM_BU(0x7C,int32_t(model));
    auto& prior=pickup_arc_source_periods[slot];
    if(!prior||prior->common.actor!=actor||prior->common.physical!=physical||prior->common.model!=model||prior->common.lifetime!=lifetime||
       prior->common.epoch!=epoch||prior->common.tick!=slice.tick||prior->common.denominator!=slice.denominator||prior->common.expected_phase!=slice.phase) {
        const RewardSourcePeriod common{actor,physical,model,MEM_H(0x20,int32_t(physical)),8,0,lifetime,run,time_command,epoch,slice.tick,slice.denominator,slice.phase,slice.phase,slice.phase,
            source_pass,fields,fields,model_time,model_time,word(rdram,model+0x78),word(rdram,model+0x78),word(rdram,model+0x74),word(rdram,model+0x74),flags,flags,false};
        prior=PickupArcSourcePeriod{common,arc,arc,command,slice.phase};
    } else {
        for(unsigned i=0;i<2;++i)if(prior->expected[i]!=arc[i])prior->initial[i]=arc[i];
        if(prior->command!=command) {prior->command=command;prior->start_angle_phase=slice.phase;}
        auto& common=prior->common;
        for(unsigned i=0;i<fields.size();++i)if(common.expected[i]!=fields[i])common.initial[i]=fields[i];
        if(common.time_command!=time_command||common.expected_model_time!=model_time)common.initial_model_time=model_time;
        if(common.time_command!=time_command||common.expected_model_flags!=flags)common.initial_model_flags=flags;
        if(common.expected_model_opacity!=word(rdram,model+0x78))common.initial_model_opacity=word(rdram,model+0x78);
        if(common.expected_model_render_flag!=word(rdram,model+0x74))common.initial_model_render_flag=word(rdram,model+0x74);
        common.run=run;common.time_command=time_command;
    }
}

// State4 changes only uniform actor scale before its ordered Source
// angle90 transition into collection. The half34 angle and latch stay Source.
struct PickupRevealSourcePeriod {
    RewardSourcePeriod common;
    int16_t angle;
    uint64_t start_angle_phase;
};
std::array<std::optional<PickupRevealSourcePeriod>,dora3::distance::common_capacity> pickup_reveal_source_periods;
void pickup_reveal_begin(uint8_t* rdram,uint32_t actor,uint32_t physical,uint32_t model,unsigned slot,uint64_t source_pass) {
    const auto& slice=*active_simulation_slice();
    const auto fields=reward_source_fields(rdram,actor,physical);
    const auto lifetime=game_model_animation().lifetime_token(model);
    const auto run=game_model_animation().run_token(model),time_command=game_model_animation().time_command_token(model);
    const auto model_time=word(rdram,model+0x64);const auto flags=MEM_BU(0x7C,int32_t(model));
    const auto angle=MEM_H(0x34,int32_t(physical));auto& prior=pickup_reveal_source_periods[slot];
    if(!prior||prior->common.actor!=actor||prior->common.physical!=physical||prior->common.model!=model||prior->common.lifetime!=lifetime||
       prior->common.epoch!=epoch||prior->common.tick!=slice.tick||prior->common.denominator!=slice.denominator||prior->common.expected_phase!=slice.phase) {
        const RewardSourcePeriod common{actor,physical,model,MEM_H(0x20,int32_t(physical)),4,0,lifetime,run,time_command,epoch,slice.tick,slice.denominator,slice.phase,slice.phase,slice.phase,
            source_pass,fields,fields,model_time,model_time,word(rdram,model+0x78),word(rdram,model+0x78),word(rdram,model+0x74),word(rdram,model+0x74),flags,flags,false};
        prior=PickupRevealSourcePeriod{common,angle,slice.phase};
    } else {
        if(prior->angle!=angle){prior->angle=angle;prior->start_angle_phase=slice.phase;}
        auto& common=prior->common;
        for(unsigned i=0;i<fields.size();++i)if(common.expected[i]!=fields[i])common.initial[i]=fields[i];
        if(common.time_command!=time_command||common.expected_model_time!=model_time)common.initial_model_time=model_time;
        if(common.time_command!=time_command||common.expected_model_flags!=flags)common.initial_model_flags=flags;
        if(common.expected_model_opacity!=word(rdram,model+0x78))common.initial_model_opacity=word(rdram,model+0x78);
        if(common.expected_model_render_flag!=word(rdram,model+0x74))common.initial_model_render_flag=word(rdram,model+0x74);
        common.run=run;common.time_command=time_command;
    }
}

std::array<int32_t,4> pickup_idle_source_fields(uint8_t* rdram,uint32_t actor,uint32_t physical) {
    return {word(rdram,actor+0x10),word(rdram,physical+0xC4),MEM_BU(0xC8,int32_t(physical)),MEM_BU(0x2A,int32_t(physical))};
}


std::array<bool,dora3::distance::common_capacity> flash_motion_initializers{};

// A city's wait/clip/patrol decision is a once-per-period command. Common
// geometry and model time still advance over every actual Native interval.
struct CityDecisionPeriod {
    uint32_t actor,physical,model;uint64_t lifetime,tick,denominator;
    std::array<int32_t,3> expected;
};
// Dialogue can look and begin returning in the same Original callback.
// Retain its complete private proposal and fractionally expose both angles.
struct CityDialoguePeriod {
    uint32_t actor,physical,model;uint64_t lifetime,tick,denominator;
    std::array<int32_t,9> expected;std::array<int32_t,2> expected_yaw;
    std::array<float,2> start,delta;bool continuous;
};
std::array<std::optional<CityDialoguePeriod>,dora3::distance::common_capacity> city_dialogue_periods;
// The special201 handler has discrete yaw snaps and a six-tick release.
// Its stored rate is not applied by Original; preserve that distinction.
struct CityKaminariPeriod {
    uint32_t actor,physical,model;uint64_t lifetime,tick,denominator;
    std::array<int32_t,11> expected;
};
struct CityKaminariCounter {
    uint32_t actor,physical,model;uint64_t lifetime,tick,denominator;int32_t expected;
};
std::array<std::optional<CityKaminariPeriod>,dora3::distance::common_capacity> city_kaminari_periods;
std::array<std::optional<CityKaminariCounter>,dora3::distance::common_capacity> city_kaminari_counters;
struct CityAxisClock {
    uint32_t actor,physical,model;uint64_t lifetime,tick,denominator,start_phase,expected_phase;
    int32_t expected,velocity;float initial;
};
std::array<std::optional<CityDecisionPeriod>,dora3::distance::common_capacity> city_decision_periods;
std::array<std::array<std::optional<CityAxisClock>,2>,dora3::distance::common_capacity> city_axis_clocks;
// Slot09 is a distinct ROM owner even where VRAM overlaps city/lake code.
// Dialogue and event-producing states are acquired in their later review.
std::array<std::optional<CityDecisionPeriod>,dora3::distance::common_capacity> slot09_decision_periods;
std::array<std::optional<CityDialoguePeriod>,dora3::distance::common_capacity> slot09_dialogue_periods;
struct Slot09SourceHandoff {
    uint32_t actor,physical,model;uint64_t lifetime;
};
std::array<std::optional<Slot09SourceHandoff>,dora3::distance::common_capacity> slot09_source_handoffs;
std::optional<Slot09SourceHandoff> slot09_wait_decision;
// SceneFD needs its real flag/request at the Source end, after the common
// actor tail has finished. The next Original common pass must not update
// that completed actor twice merely because the request revoked eligibility.
struct Slot09ScenePublication {
    uint32_t actor,physical,model;uint64_t lifetime,tick,denominator;
};
struct Slot09SceneHandoff {
    uint32_t actor,physical,model;uint64_t lifetime,run,source_pass;
    std::array<int32_t,11> expected;
};
std::array<std::optional<Slot09ScenePublication>,dora3::distance::common_capacity> slot09_scene_publications;
std::array<std::optional<Slot09SceneHandoff>,dora3::distance::common_capacity> slot09_scene_handoffs;
uint64_t slot09_source_pass=0;
std::array<int32_t,11> slot09_scene_fields(uint8_t* ram,uint32_t actor,uint32_t physical,uint32_t model) {
    uint8_t* rdram=ram;
    return {word(ram,actor+0x94),MEM_B(0x28,int32_t(physical)),MEM_H(0x34,int32_t(physical)),MEM_H(0x36,int32_t(physical)),
        MEM_H(0x42,int32_t(physical)),MEM_H(0x44,int32_t(physical)),word(ram,physical+0x58),word(ram,physical+0x5C),
        word(ram,actor+0x10),word(ram,physical+4),word(ram,model+0x64)};
}
bool slot09_scene_identity(uint8_t* ram,uint32_t actor,uint32_t physical,uint32_t model,uint64_t lifetime) {
    uint8_t* rdram=ram;
    return pointer(actor,0xCC)&&pointer(physical,0xF4)&&pointer(model,0x84)&&lifetime&&
        uint32_t(word(ram,actor+0x98))==physical&&uint32_t(word(ram,actor+0x74))==model&&
        game_model_animation().lifetime_token(model)==lifetime&&word(ram,actor+0x94)==102&&
        MEM_H(0x20,int32_t(physical))==26&&MEM_BU(0xC9,int32_t(physical))!=1&&MEM_B(0x29,int32_t(physical))==0&&MEM_H(0,int32_t(0x801B36B8))==0xFD&&
        dora3_overlay_loaded(0x001398A0u)&&uint32_t(word(ram,0x80202064u+26u*84))==0x8022F674u;
}
bool slot09_scene_handoff_valid(uint8_t* ram,const Slot09SceneHandoff& v) {
    return ram&&slot09_scene_identity(ram,v.actor,v.physical,v.model,v.lifetime)&&
        game_model_animation().run_token(v.model)==v.run&&
        word(ram,0x802170EC)==1&&word(ram,0x80217104)==0x175&&word(ram,0x802170FC)==1&&
        uint32_t(word(ram,0x80217100))==0x80217104u&&
        slot09_scene_fields(ram,v.actor,v.physical,v.model)==v.expected;
}

// An Original world pass immediately following the Native room request
// must not repeat actors that already finished that exact Native frame.
// These records cover only actual model-backed reviewed owners. They
// expire at the next Source pass and never transfer to a changed actor,
// callback, clip run, model lifetime, pending request, or pose/command.
bool owns_common_object(uint8_t*,uint32_t);
struct SceneCompletedObject {
    uint32_t actor,physical,model,callback;uint64_t lifetime,run,source_pass;
    std::array<int32_t,23> expected;
};
std::array<std::optional<SceneCompletedObject>,dora3::distance::common_capacity> scene_frame_completed,scene_source_handoffs;
std::array<bool,dora3::distance::common_capacity> scene_frame_entered{};
void pickup_spin_end(uint8_t* ram,uint32_t actor,unsigned slot,uint64_t source_pass,bool native) {
    if(slot>=pickup_spin_source_periods.size()||!pickup_spin_source_periods[slot])return;
    auto& pending=pickup_spin_source_periods[slot];auto& common=pending->common;
    uint8_t* rdram=ram;
    if(native&&active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        if(scene_frame_entered[slot]&&reward_source_identity(ram,common)&&common.tick==slice.tick&&common.denominator==slice.denominator) {
            pending->expected=pickup_spin_fields(ram,actor,common.physical);common.expected=reward_source_fields(ram,actor,common.physical);
            common.expected_phase=slice.phase+slice.numerator;common.expected_model_time=word(ram,common.model+0x64);
            common.expected_model_opacity=word(ram,common.model+0x78);common.expected_model_render_flag=word(ram,common.model+0x74);
            common.expected_model_flags=MEM_BU(0x7C,int32_t(common.model));common.run=game_model_animation().run_token(common.model);
            common.time_command=game_model_animation().time_command_token(common.model);
        }
    } else if(!native) {
        if(!common.replayed&&reward_source_identity(ram,common)&&MEM_BU(0xC9,int32_t(common.physical))==1&&common.expected_phase<=common.denominator&&common.source_pass+1==source_pass)
            pickup_spin_restore(ram,*pending);
        pending.reset();
    }
}
void pickup_scatter_end(uint8_t* ram,uint32_t actor,unsigned slot,uint64_t source_pass,bool native) {
    if(slot>=pickup_scatter_source_periods.size()||!pickup_scatter_source_periods[slot])return;
    auto& pending=pickup_scatter_source_periods[slot];auto& common=pending->common;
    uint8_t* rdram=ram;
    if(native&&active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        if(scene_frame_entered[slot]&&reward_source_identity(ram,common)&&common.tick==slice.tick&&common.denominator==slice.denominator) {
            pending->expected=pickup_scatter_fields(ram,actor,common.physical);common.expected=reward_source_fields(ram,actor,common.physical);
            common.expected_phase=slice.phase+slice.numerator;common.expected_model_time=word(ram,common.model+0x64);
            common.expected_model_opacity=word(ram,common.model+0x78);common.expected_model_render_flag=word(ram,common.model+0x74);
            common.expected_model_flags=MEM_BU(0x7C,int32_t(common.model));common.run=game_model_animation().run_token(common.model);
            common.time_command=game_model_animation().time_command_token(common.model);
        }
    } else if(!native) {
        if(!common.replayed&&reward_source_identity(ram,common)&&MEM_BU(0xC9,int32_t(common.physical))==1&&common.expected_phase<=common.denominator&&common.source_pass+1==source_pass)
            pickup_scatter_restore(ram,*pending);
        pending.reset();
    }
}
void pickup_settle_end(uint8_t* ram,uint32_t actor,unsigned slot,uint64_t source_pass,bool native) {
    if(slot>=pickup_settle_source_periods.size()||!pickup_settle_source_periods[slot])return;
    auto& pending=pickup_settle_source_periods[slot];auto& common=pending->common;
    uint8_t* rdram=ram;
    if(native&&active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        if(scene_frame_entered[slot]&&reward_source_identity(ram,common)&&common.tick==slice.tick&&common.denominator==slice.denominator) {
            pending->expected_velocity=word(ram,common.physical+0x14);common.expected=reward_source_fields(ram,actor,common.physical);
            if(pending->parent)pending->expected_pose=pickup_attachment_fields(ram,actor);
            common.expected_phase=slice.phase+slice.numerator;common.expected_model_time=word(ram,common.model+0x64);
            common.expected_model_opacity=word(ram,common.model+0x78);common.expected_model_render_flag=word(ram,common.model+0x74);
            common.expected_model_flags=MEM_BU(0x7C,int32_t(common.model));common.run=game_model_animation().run_token(common.model);
            common.time_command=game_model_animation().time_command_token(common.model);
        }
    } else if(!native) {
        if(!common.replayed&&reward_source_identity(ram,common)&&MEM_BU(0xC9,int32_t(common.physical))==1&&common.expected_phase<=common.denominator&&common.source_pass+1==source_pass)
            pickup_settle_restore(ram,*pending);
        pending.reset();
    }
}
void pickup_reveal_end(uint8_t* ram,uint32_t actor,unsigned slot,uint64_t source_pass,bool native) {
    if(slot>=pickup_reveal_source_periods.size()||!pickup_reveal_source_periods[slot])return;
    auto& pending=pickup_reveal_source_periods[slot];auto& common=pending->common;
    uint8_t* rdram=ram;
    if(native&&active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        if(scene_frame_entered[slot]&&reward_source_identity(ram,common)&&common.tick==slice.tick&&common.denominator==slice.denominator) {
            common.expected=reward_source_fields(ram,actor,common.physical);
            common.expected_phase=slice.phase+slice.numerator;common.expected_model_time=word(ram,common.model+0x64);
            common.expected_model_opacity=word(ram,common.model+0x78);common.expected_model_render_flag=word(ram,common.model+0x74);
            common.expected_model_flags=MEM_BU(0x7C,int32_t(common.model));common.run=game_model_animation().run_token(common.model);
            common.time_command=game_model_animation().time_command_token(common.model);
        }
    } else if(!native) {
        if(!common.replayed&&reward_source_identity(ram,common)&&MEM_BU(0xC9,int32_t(common.physical))==1&&common.expected_phase<=common.denominator&&common.source_pass+1==source_pass)
            reward_source_restore(ram,common);
        pending.reset();
    }
}
void pickup_arc_end(uint8_t* ram,uint32_t actor,unsigned slot,uint64_t source_pass,bool native) {
    if(slot>=pickup_arc_source_periods.size()||!pickup_arc_source_periods[slot])return;
    auto& pending=pickup_arc_source_periods[slot];auto& common=pending->common;
    uint8_t* rdram=ram;
    if(native&&active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        if(scene_frame_entered[slot]&&reward_source_identity(ram,common)&&common.tick==slice.tick&&common.denominator==slice.denominator) {
            pending->expected=pickup_arc_fields(ram,actor);common.expected=reward_source_fields(ram,actor,common.physical);
            common.expected_phase=slice.phase+slice.numerator;common.expected_model_time=word(ram,common.model+0x64);
            common.expected_model_opacity=word(ram,common.model+0x78);common.expected_model_render_flag=word(ram,common.model+0x74);
            common.expected_model_flags=MEM_BU(0x7C,int32_t(common.model));common.run=game_model_animation().run_token(common.model);
            common.time_command=game_model_animation().time_command_token(common.model);
        }
    } else if(!native) {
        if(!common.replayed&&reward_source_identity(ram,common)&&MEM_BU(0xC9,int32_t(common.physical))==1&&common.expected_phase<=common.denominator&&common.source_pass+1==source_pass)
            pickup_arc_restore(ram,*pending);
        pending.reset();
    }
}


std::array<int32_t,23> scene_object_fields(uint8_t* ram,uint32_t actor,uint32_t physical,uint32_t model) {
    uint8_t* rdram=ram;
    return {word(ram,actor+0x94),word(ram,actor+0x10),word(ram,physical+4),word(ram,model+0x64),
        word(ram,actor+0x9C),word(ram,actor+0xA0),word(ram,actor+0xA4),word(ram,actor+8),word(ram,actor+0xC),
        word(ram,physical),word(ram,physical+0x14),word(ram,physical+0x58),word(ram,physical+0x5C),
        MEM_B(0x28,int32_t(physical)),MEM_H(0x34,int32_t(physical)),MEM_H(0x36,int32_t(physical)),
        MEM_H(0x42,int32_t(physical)),MEM_H(0x44,int32_t(physical)),MEM_B(0x29,int32_t(physical)),
        MEM_BU(0xC9,int32_t(physical)),MEM_B(0xC8,int32_t(physical)),MEM_H(0x20,int32_t(physical)),word(ram,physical+0x18)};
}
bool scene_completed_identity(uint8_t* ram,const SceneCompletedObject& v) {
    if(!ram||!pointer(v.actor,0xCC)||!pointer(v.physical,0xDC)||!pointer(v.model,0x84)||!v.lifetime||
        uint32_t(word(ram,v.actor+0x98))!=v.physical||uint32_t(word(ram,v.actor+0x74))!=v.model||
        game_model_animation().lifetime_token(v.model)!=v.lifetime||game_model_animation().run_token(v.model)!=v.run||
        !owns_common_object(ram,v.actor))return false;
    uint8_t* rdram=ram;const auto kind=MEM_H(0x20,int32_t(v.physical));
    return kind>=0&&kind<592&&uint32_t(word(ram,0x80202064u+unsigned(kind)*84))==v.callback&&
        scene_object_fields(ram,v.actor,v.physical,v.model)==v.expected;
}
bool scene_completed_handoff_valid(uint8_t* ram,const SceneCompletedObject& v) {
    return scene_fd_request(ram)&&scene_completed_identity(ram,v);
}
std::optional<SceneCompletedObject> scene_capture_completed(uint8_t* ram,uint32_t actor) {
    if(!owns_common_object(ram,actor))return std::nullopt;
    const auto physical=uint32_t(word(ram,actor+0x98)),model=uint32_t(word(ram,actor+0x74));
    if(!pointer(physical,0xDC)||!pointer(model,0x84))return std::nullopt;
    const auto lifetime=game_model_animation().lifetime_token(model);if(!lifetime)return std::nullopt;
    uint8_t* rdram=ram;const auto kind=MEM_H(0x20,int32_t(physical));
    if(kind<0||kind>=592)return std::nullopt;
    return SceneCompletedObject{actor,physical,model,uint32_t(word(ram,0x80202064u+unsigned(kind)*84)),lifetime,
        game_model_animation().run_token(model),slot09_source_pass+1,scene_object_fields(ram,actor,physical,model)};
}

// The special24 clip starts continuously at Native interval start, but the
// actual dialog publication retains Source end. Cancellation never writes
// into a replaced actor/model/overlay or leaves a stale initialized latch.
struct Slot09StoryStart {
    uint32_t actor,physical,model;uint64_t lifetime,tick,denominator;
};
std::array<std::optional<Slot09StoryStart>,dora3::distance::common_capacity> slot09_story_starts;
bool slot09_story_start_identity(uint8_t* ram,const Slot09StoryStart& pending) {
    uint8_t* rdram=ram;
    return pointer(pending.actor,0xCC)&&pointer(pending.physical,0xF4)&&
        uint32_t(word(ram,pending.actor+0x98))==pending.physical&&
        uint32_t(word(ram,pending.actor+0x74))==pending.model&&
        game_model_animation().lifetime_token(pending.model)==pending.lifetime&&
        pending.lifetime&&word(ram,pending.actor+0x94)==200&&
        MEM_H(0x20,int32_t(pending.physical))==24&&MEM_BU(0x28,int32_t(pending.physical))==1;
}
void cancel_slot09_story_start(uint8_t* ram,unsigned slot) {
    auto& pending=slot09_story_starts[slot];if(!pending)return;
    if(ram&&slot09_story_start_identity(ram,*pending)&&dora3_overlay_loaded(0x001398A0u)&&
       uint32_t(word(ram,0x80202064u+24u*84))==0x8022DFA4u) {
        uint8_t* rdram=ram;MEM_B(0x28,int32_t(pending->physical))=0;
    }
    pending.reset();
}

// Predict only special28 geometry. Its complete Original decision, including
// dialog publication, sprites, signed release count and story completion,
// executes at the Source end. A canceled fraction restores only its own pose.
struct Slot09SpecialTurn {
    uint32_t actor,physical,model;uint64_t lifetime,tick,denominator;
    std::array<int32_t,9> fields;std::array<int32_t,2> start,expected;
    std::array<float,2> delta;float proposal,target;bool executing=false;
};
std::array<std::optional<Slot09SpecialTurn>,dora3::distance::common_capacity> slot09_special_turns;
bool slot09_special_turn_identity(uint8_t* ram,const Slot09SpecialTurn& v) {
    uint8_t* rdram=ram;
    return pointer(v.actor,0xCC)&&pointer(v.physical,0xF4)&&v.lifetime&&
        uint32_t(word(ram,v.actor+0x98))==v.physical&&uint32_t(word(ram,v.actor+0x74))==v.model&&
        game_model_animation().lifetime_token(v.model)==v.lifetime&&word(ram,v.actor+0x94)==200&&
        MEM_H(0x20,int32_t(v.physical))==28&&dora3_overlay_loaded(0x001398A0u)&&
        uint32_t(word(ram,0x80202064u+28u*84))==0x8022FE4Cu;
}
void cancel_slot09_special_turn(uint8_t* ram,unsigned slot) {
    auto& v=slot09_special_turns[slot];if(!v)return;
    if(ram&&slot09_special_turn_identity(ram,*v)&&!v->executing) {
        uint8_t* rdram=ram;
        for(unsigned axis=0;axis<2;++axis) {
            const auto address=axis?v->physical+4:v->actor+0x10;
            if(word(ram,address)==v->expected[axis])MEM_W(0,int32_t(address))=v->start[axis];
        }
    }
    v.reset();
}


uint32_t slot09_callback(int kind) {
    switch(kind) {
    case 24:return 0x8022DFA4u;case 25:return 0x8022EB94u;
    case 26:return 0x8022F674u;case 27:return 0x80230448u;
    case 28:return 0x8022FE4Cu;default:return 0;
    }
}
// The unresolved event-producing closure remains Original. These exact
// quest predicates are the branches in Source26 F254, checked every pass.
bool slot09_dialogue26_can_own(uint8_t* ram) {
    uint8_t* rdram=ram;
    const auto scene=MEM_H(0,int32_t(0x801B36B8));
    const auto quest=[&](unsigned flag) {
        return (uint32_t(word(ram,0x801B3628u+4u*(flag/32u)))&(1u<<(flag%32u)))!=0;
    };
    if(scene==0xFD)return true; // The quest-clear tail has its own Source-end publication gate.
    if(scene==0xAC)return quest(0x10);
    if(scene==0xC1)return !quest(0x7B)||quest(0x2F);
    return true;
}
bool slot09_dialogue_state(int kind,int state) {
    return kind==24?state==3:kind==25?(state==2||state==102):kind==26?state==102:(kind==27||kind==28)&&state==2;
}
bool slot09_patrol_state(int kind,int state) {
    if(kind==24)return state==1||state==2||state==3||state==4||state==100||state==200;
    if(kind==25)return state==1||state==2||state==100||state==101||state==102;
    if(kind==26)return state==1||state==100||state==101||state==102;
    if(kind==27)return state==1||state==2||state==100;
    return kind==28&&(state==1||state==2||state==100||state==200);
}
uint32_t city_patrol_callback(int kind) {
    switch(kind) {
    case 13:return 0x8022E480u;
    case 14:return 0x8022ED78u;
    case 15:return 0x8022F648u;
    case 16:return 0x8022FF44u;
    case 17:return 0x80230830u;
    case 20:return 0x802317CCu;
    case 21:return 0x802322ECu;
    case 22:return 0x80232BD0u;
    default:return 0;
    }
}


// Linked parent/leg clip requests retain Original pool order and execute
// once per parent's period. Continuous geometry is evaluated separately.
struct LinkedCityPeriod {
    uint32_t actor,physical,model;uint64_t lifetime,tick,denominator;
    std::array<int32_t,10> expected;std::array<uint64_t,5> link;std::array<int32_t,2> expected_yaw;
    int geometry;float start,delta;
};
std::array<std::optional<LinkedCityPeriod>,dora3::distance::common_capacity> linked_city_periods;
// The child must consume a newly published command even when its numeric
// clip index matches the previous request within the same Source period.
std::array<uint64_t,dora3::distance::common_capacity> linked_city_publications;
// Story100 has a one-shot clip/attachment command, then common geometry.
// A wrapped-slime child uses its reviewed parent's allocation clock. Both
// secondary groups and reciprocal pool links must remain live.
bool city_story_family_owner(uint8_t* ram,uint32_t actor) {
    uint8_t* rdram=ram;
    if(actor<0x804B34B8u||actor>=0x804B34B8u+dora3::distance::common_capacity*0xCC||(actor-0x804B34B8u)%0xCC||!word(ram,actor+0x94))return false;
    const auto physical=uint32_t(word(ram,actor+0x98));if(!pointer(physical,0xDC))return false;
    const auto kind=MEM_H(0x20,int32_t(physical));uint32_t parent=actor;
    if(kind==6) {
        const auto index=MEM_H(0x7C,int32_t(physical));
        if(index<30||index>=dora3::distance::common_end||word(ram,actor+0x94)!=1)return false;
        parent=0x804B1CD0u+unsigned(index)*0xCC;if(parent>=actor)return false;
    } else if(!city_patrol_callback(kind))return false;
    const auto pp=uint32_t(word(ram,parent+0x98));if(!pointer(pp,0xDC))return false;
    const auto parent_kind=MEM_H(0x20,int32_t(pp));const auto callback=city_patrol_callback(parent_kind);
    if(!callback||word(ram,parent+0x94)!=100||!dora3_overlay_loaded(0x00132F80u)||uint32_t(word(ram,0x80202064+unsigned(parent_kind)*84))!=callback)return false;
    if(!game_model_animation().lifetime_token(uint32_t(word(ram,parent+0x74))))return false;
    const auto secondary=uint32_t(word(ram,parent+0x78));
    if(secondary&&!game_model_animation().lifetime_token(secondary))return false;
    const auto ci=MEM_H(0x80,int32_t(pp));if(ci==-1)return parent==actor;
    const auto parent_index=(parent-0x804B1CD0u)/0xCC;
    if(ci<30||ci>=dora3::distance::common_end||ci<=int(parent_index))return false;
    const auto child=0x804B1CD0u+unsigned(ci)*0xCC,cp=uint32_t(word(ram,child+0x98));
    if((kind==6&&child!=actor)||!pointer(cp,0xDC)||word(ram,child+0x94)!=1||MEM_H(0x20,int32_t(cp))!=6||
       MEM_H(0x7C,int32_t(cp))!=int16_t(parent_index)||MEM_H(0x7E,int32_t(cp))!=0||
       MEM_H(0x9E,int32_t(cp))!=int16_t(parent_index)||MEM_H(0x9E,int32_t(pp))!=ci||
       MEM_BU(0xC9,int32_t(cp))!=MEM_BU(0xC9,int32_t(pp)))return false;
    return game_model_animation().lifetime_token(uint32_t(word(ram,child+0x74)))&&uint32_t(word(ram,0x80202064+6*84))==0x801EB864u;
}

bool linked_city_owner(uint8_t* ram,uint32_t actor) {
    uint8_t* rdram=ram;
    if(actor<0x804B34B8u||actor>=0x804B34B8u+dora3::distance::common_capacity*0xCC||(actor-0x804B34B8u)%0xCC||!word(ram,actor+0x94))return false;
    auto physical=uint32_t(word(ram,actor+0x98));if(!pointer(physical,0xDC))return false;
    const auto kind=MEM_H(0x20,int32_t(physical));if(kind!=18&&kind!=19&&kind!=6)return false;
    uint32_t parent=actor;
    if(kind==19||kind==6) {
        const auto index=MEM_H(0x7C,int32_t(physical));
        if(index<30||index>=dora3::distance::common_end||word(ram,actor+0x94)!=1)return false;
        parent=0x804B1CD0u+unsigned(index)*0xCC;
        if(parent>=actor)return false; // Exact reviewed E5DF4 allocator ordering.
    }
    const auto pp=uint32_t(word(ram,parent+0x98));if(!pointer(pp,0xDC)||MEM_H(0x20,int32_t(pp))!=18)return false;
    const auto state=word(ram,parent+0x94);const auto offset=MEM_H(0x44,int32_t(pp));
    const bool story=state==100&&offset==8;
    if(!story&&((state!=1&&state!=2)||(offset!=0&&offset!=4)))return false;
    const auto pm=uint32_t(word(ram,parent+0x74));if(!game_model_animation().lifetime_token(pm))return false;
    if(!dora3_overlay_loaded(0x00132F80u)||uint32_t(word(ram,0x80202064+18*84))!=0x80230F70u)return false;
    const auto ci=MEM_H(0x80,int32_t(pp));
    if(ci==-1)return kind==18;
    if((!story&&offset!=0)||ci<30||ci>=dora3::distance::common_end||ci<=int((parent-0x804B1CD0u)/0xCC))return false;
    const auto child=0x804B1CD0u+unsigned(ci)*0xCC,cp=uint32_t(word(ram,child+0x98));
    if((kind!=18&&child!=actor)||!pointer(cp,0xDC)||word(ram,child+0x94)!=1||MEM_H(0x20,int32_t(cp))!=(story?6:19)||
       MEM_H(0x7C,int32_t(cp))!=int16_t((parent-0x804B1CD0u)/0xCC)||MEM_H(0x7E,int32_t(cp))!=0||
       MEM_H(0x9E,int32_t(cp))!=int16_t((parent-0x804B1CD0u)/0xCC)||MEM_H(0x9E,int32_t(pp))!=ci||
       MEM_BU(0xC9,int32_t(pp))!=MEM_BU(0xC9,int32_t(cp)))return false;
    return game_model_animation().lifetime_token(uint32_t(word(ram,child+0x74)))&&
        uint32_t(word(ram,0x80202064+(story?6:19)*84))==(story?0x801EB864u:0x8023098Cu);
}

struct PlayerStopClock {
    uint32_t actor,physical,model;
    uint64_t lifetime,run,tick,denominator;
    int32_t state,command,expected;
};
std::optional<PlayerStopClock> player_stop_clock;
struct PlayerSourceContact {
    uint32_t actor,physical,model;
    uint64_t lifetime,run,tick,denominator;
    int32_t state,flags;
};
std::optional<PlayerSourceContact> player_source_contact;
bool player_flight_state(uint8_t* ram) {
    const auto state=word(ram,uint32_t(word(ram,0x8021D49C))+0x94);
    const auto control=uint32_t(word(ram,0x8021D6C0));uint8_t* rdram=ram;
    const auto status=uint32_t(word(ram,0x8021D4A8));
    const bool live_damage=state==48 && byte_pointer(status,2);
    return MEM_BU(0x28,int32_t(control))==1 && (state==20||state==27||state==57||state==58||state==66||state==67||live_damage);
}
std::array<uint32_t,9> flight_fields(uint32_t actor,uint32_t physical) {
    return {physical,physical+4,physical+0xC,actor+0xC,actor+0x10,actor+0x14,
            physical+0x10,physical+0x14,physical+0x18};
}
std::array<int32_t,12> flight_observation(uint8_t* ram,uint32_t actor,uint32_t physical) {
    std::array<int32_t,12> result{};const auto fields=flight_fields(actor,physical);
    for(unsigned i=0;i<fields.size();++i)result[i]=word(ram,fields[i]);
    for(unsigned i=0;i<3;++i)result[9+i]=word(ram,actor+0x9C+4*i);
    return result;
}
bool same_flight_owner(uint8_t* ram,const PlayerFlightClock& clock) {
    return uint32_t(word(ram,0x8021D49C))==clock.actor &&
        uint32_t(word(ram,0x8021D4B0))==clock.physical && uint32_t(word(ram,clock.actor+0x74))==clock.model &&
        game_model_animation().lifetime_token(clock.model)==clock.lifetime &&
        game_model_animation().run_token(clock.model)==clock.run;
}
void finish_player_flight(uint8_t* ram) {
    const auto* slice=active_simulation_slice();
    if(!slice || !player_flight_clock || !player_flight_clock->open)return;
    auto& clock=*player_flight_clock;
    if(!same_flight_owner(ram,clock) || !player_flight_state(ram)) {player_flight_clock.reset();return;}
    clock.expected=flight_observation(ram,clock.actor,clock.physical);
    clock.expected_phase=slice->phase+slice->numerator;clock.open=false;
}

bool uses_source_attachment(uint8_t* ram) {
    const auto weapon=word(ram,0x801B36E0);
    return weapon==1||weapon==2||weapon==4||weapon==5;
}
std::optional<ShotAttachment> capture_shot_attachment(uint8_t* ram) {
    ShotAttachment result;result.generation=epoch;
    for(unsigned i=0;i<2;++i) {
        const auto actor=uint32_t(word(ram,i?0x8021D640:0x8021D49C));
        if(!pointer(actor,0xCC))return std::nullopt;
        const auto model=uint32_t(word(ram,actor+0x74)),physical=uint32_t(word(ram,actor+0x98));
        const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model);
        if(!pointer(model,0x84)||!lifetime||!run)return std::nullopt;
        result.owners[i]={actor,physical,model,lifetime,run};
        for(unsigned axis=0;axis<3;++axis)result.positions[3*i+axis]=word(ram,actor+0x9C+4*axis);
    }
    return result;
}
bool same_shot_attachment(uint8_t* ram,const ShotAttachment& value,bool check_run=true) {
    if(value.generation!=epoch)return false;
    for(unsigned i=0;i<2;++i) {
        const auto& owner=value.owners[i];
        if(uint32_t(word(ram,i?0x8021D640:0x8021D49C))!=owner.actor ||
           uint32_t(word(ram,owner.actor+0x98))!=owner.physical || uint32_t(word(ram,owner.actor+0x74))!=owner.model ||
           game_model_animation().lifetime_token(owner.model)!=owner.lifetime ||
           (check_run && game_model_animation().run_token(owner.model)!=owner.run))return false;
    }
    return true;
}
uint64_t slice_position(const SimulationSlice& slice,bool end) {
    return slice.tick*slice.denominator+slice.phase+(end?slice.numerator:0);
}
struct PlayerShotCue {float frame;uint64_t site;};
std::optional<PlayerShotCue> player_shot_cue(int state) {
    if(state==20)return PlayerShotCue{18.f,0x10801C496Cull};
    if(state==17||state==18)return PlayerShotCue{18.f,0x10801C4674ull};
    if(state==21||state==22)return PlayerShotCue{15.f,0x10801C4B80ull};
    if(state==28||state==29)return PlayerShotCue{18.f,0x10801C5584ull};
    return std::nullopt;
}
// EBAD4 addresses the parent through the real common-pool index, strideCC.
// This first attached domain uses an earlier, live Source-born flash130;
// other parent controllers retain Source ownership until separately proven.
uint32_t pickup_attachment_parent(uint8_t* ram,uint32_t actor,uint32_t physical) {
    uint8_t* rdram=ram;const auto index=MEM_H(0x7C,int32_t(physical));
    if(index<30||index>=dora3::distance::common_end)return 0;
    const auto parent=0x804B1CD0u+uint32_t(index)*0xCC;
    if(parent>=actor||!pointer(parent,0xCC)||!word(ram,parent+0x94))return 0;
    const auto pp=uint32_t(word(ram,parent+0x98)),pm=uint32_t(word(ram,parent+0x74));
    if(!pointer(pp,0xF4)||MEM_H(0x20,int32_t(pp))!=130||!pointer(pm,0x84)||
       !game_model_animation().lifetime_token(pm)||game_model_animation().native_birth(pm)||!owns_common_object(ram,parent))return 0;
    for(auto address:pickup_attachment_addresses(parent))
        if(!std::isfinite(std::bit_cast<float>(uint32_t(word(ram,address)))))return 0;
    return parent;
}
bool owns_linked_pickup_animation(uint8_t* rdram,uint32_t actor,uint32_t physics) {
    const auto model=uint32_t(word(rdram,actor+0x74));
    // Source alone initializes the cached centre, RNG delay, clip/seek,
    // instantaneous pose and retirement. There is no private velocity.
    // Acquire common model time only after a real Source pool visit and
    // while the constructor's actual reciprocal parent links stay live.
    // A real E89CC opacity command sets2A. Preserve it until the Source
    // common pool clears it at E8D44; Native must not consume that command.
    if(actor<0x804B34B8u||actor>=0x804B34B8u+dora3::distance::common_capacity*0xCC||(actor-0x804B34B8u)%0xCC||
       word(rdram,actor+0x94)!=1||MEM_BU(0x28,int32_t(physics))!=1||
       MEM_BU(0xCA,int32_t(physics))!=1||MEM_BU(0xC8,int32_t(physics))!=0||
        MEM_BU(0x2A,int32_t(physics))!=0||MEM_H(0xD8,int32_t(physics))!=0||
       !pointer(model,0x84)||!game_model_animation().lifetime_token(model)||game_model_animation().native_birth(model)||
       uint32_t(word(rdram,0x80202064u+129u*84u))!=0x801ED2CCu||
       uint32_t(word(rdram,0x80202068u+129u*84u))!=0x801ED2C4u)return false;
    for(unsigned axis=0;axis<3;++axis)if(uint32_t(word(rdram,physics+0x10+4*axis))&0x7fffffffu)return false;
    const auto parent_index=MEM_H(0x7C,int32_t(physics)),child_index=MEM_H(0x44,int32_t(physics));
    if(parent_index<30||parent_index>=dora3::distance::common_end||child_index<0||child_index>=3)return false;
    const auto parent=0x804B1CD0u+unsigned(parent_index)*0xCC,pp=uint32_t(word(rdram,parent+0x98));
    if(!word(rdram,parent+0x94)||!pointer(pp,0xF4)||!standard_pickup_kind(MEM_H(0x20,int32_t(pp)))||
       MEM_H(0x80+2*child_index,int32_t(pp))!=int16_t((actor-0x804B1CD0u)/0xCC))return false;
    const auto parent_model=uint32_t(word(rdram,parent+0x74));
    return pointer(parent_model,0x84)&&game_model_animation().lifetime_token(parent_model);
}

bool owns_common_object(uint8_t* ram,uint32_t actor) {
    dora3::profile::Scope detailProfile(dora3::profile::Tag::CommonOwnershipSample,dora3::profile::sample_stage(dora3::profile::Tag::CommonOwnershipSample,16));
    if(!pointer(actor,0xCC))return false;
    const auto physics=uint32_t(word(ram,actor+0x98));
    if(!pointer(physics,0xDC))return false;
    if(native_enemy_owned(ram,actor)||native_scene_object_owned(ram,actor))return true;
    uint8_t* rdram=ram;
    const auto kind=MEM_H(0x20,int32_t(physics));
    // Reviewed callbacks in .scene1 share common movement/opacity/timers
    // and animation. Each accepted type also checks the live table entry;
    // active stage AI retains its original owner until separately reviewed.
    const bool noop=(kind>=0&&kind<=5)||kind==8||(kind>=10&&kind<=12)||kind==23||(kind>=82&&kind<=108);
    uint32_t callback=0;
    if(noop)callback=0x801E5D84u;
    // Kind6 only initializes clip0 when entering state1. Its byte latch is
    // written before the real clip command, so repeated native passes cannot
    // restart the animation. Kind80 has a separate Original no-op callback.
    else if(kind==6) {
        const auto parent_index=MEM_H(0x7C,int32_t(physics));
        if(parent_index>=30&&parent_index<dora3::distance::common_end) {
            const auto parent=0x804B1CD0u+unsigned(parent_index)*0xCC,pp=uint32_t(word(ram,parent+0x98));
            if(pointer(pp,0xDC)&&MEM_H(0x20,int32_t(pp))==18)return linked_city_owner(ram,actor);
            if(pointer(pp,0xDC)&&city_patrol_callback(MEM_H(0x20,int32_t(pp))))return city_story_family_owner(ram,actor);
        }
        callback=0x801EB864u;
    }
    else if(standard_pickup_kind(kind)) {
        const auto model=uint32_t(word(ram,actor+0x74));
        static constexpr std::array<uint32_t,18> updates{0x801EC938u,0x801EC994u,0x801EC9F0u,0x801ECAA8u,0x801EC8DCu,0x801ECA4Cu,0x801ECB6Cu,0x801ECC6Cu,0x801ECD3Cu,0x801ECDD4u,0x801ECE6Cu,0x801ECF3Cu,0x801ED020u,0x801ED07Cu,0x801ED104u,0x801ED1E8u,0x801ED244u,0x801ED2A0u};
        const auto update=updates[unsigned(kind-111)];
        static constexpr std::array<uint32_t,18> contacts{0x801EC91Cu,0x801EC978u,0x801EC9D4u,0x801ECA8Cu,0x801EC8C0u,0x801ECA30u,0x801ECB1Cu,0x801ECBE4u,0x801ECCECu,0x801ECD8Cu,0x801ECE24u,0x801ECEECu,0x801ECFB4u,0x801ED060u,0x801ED0BCu,0x801ED17Cu,0x801ED228u,0x801ED284u};
        const auto contact=contacts[unsigned(kind-111)];
        const auto state=word(ram,actor+0x94);
        const bool stationary_idle=state==1&&(uint32_t(word(ram,physics+0x10))&0x7fffffffu)==0&&(uint32_t(word(ram,physics+0x14))&0x7fffffffu)==0&&(uint32_t(word(ram,physics+0x18))&0x7fffffffu)==0;
        const auto scale=std::bit_cast<float>(uint32_t(word(ram,actor+0x18)));
        const auto vy=word(ram,physics+0x14);
        // Original D8 values0/1 do not blink or decrement (E8DA4..DB0).
        // Real constructors use1; larger blink timers remain Source-owned.
        const auto blink=MEM_H(0xD8,int32_t(physics));
        const bool sequence=(state==5||state==6)&&MEM_BU(0x28,int32_t(physics))==1&&(blink==0||blink==1)&&
            word(ram,physics+0x10)==0&&word(ram,physics+0x18)==0&&std::isfinite(scale)&&scale>0&&
            word(ram,actor+0x18)==word(ram,actor+0x1C)&&word(ram,actor+0x18)==word(ram,actor+0x20)&&
            word(ram,actor+0x18)==word(ram,physics+0x30)&&
            (state==5?(vy==0||vy==std::bit_cast<int32_t>(5.f)):(vy==std::bit_cast<int32_t>(-1.f)));
        const auto spin_speed=std::bit_cast<float>(uint32_t(word(ram,physics+0x58))),spin_alpha=std::bit_cast<float>(uint32_t(word(ram,physics+0x5C)));
        const bool spin=state==7&&MEM_BU(0x28,int32_t(physics))==1&&(blink==0||blink==1)&&
            word(ram,physics+0x10)==0&&word(ram,physics+0x14)==0&&word(ram,physics+0x18)==0&&
            std::isfinite(spin_speed)&&spin_speed>0&&spin_speed<36&&std::isfinite(spin_alpha)&&spin_alpha>0&&spin_alpha<=1;
        const auto arc_phase=MEM_H(0x34,int32_t(physics)),arc_angle=MEM_H(0x36,int32_t(physics)),arc_target=MEM_H(0x44,int32_t(physics));
        const bool arc=state==8&&MEM_BU(0x28,int32_t(physics))==1&&(blink==0||blink==1)&&
            word(ram,physics+0x10)==0&&word(ram,physics+0x14)==0&&word(ram,physics+0x18)==0&&
            (arc_phase==0||arc_phase==1)&&arc_angle>=0&&arc_target>0&&arc_target<360&&
            (arc_phase==0?arc_angle<90:arc_angle<arc_target);
        // Source phase0 normally remains below90; phase1 stays below target.
        // An external incompatible angle/phase command keeps Source ownership
        // through its actual wrap/clamp before reacquiring the valid arc.
        const auto reveal_angle=MEM_H(0x34,int32_t(physics));
        const bool reveal=state==4&&MEM_BU(0x28,int32_t(physics))==1&&(blink==0||blink==1)&&reveal_angle>=0&&reveal_angle<90&&
            word(ram,physics+0x10)==0&&word(ram,physics+0x14)==0&&word(ram,physics+0x18)==0;
        const auto settle_velocity=std::bit_cast<float>(uint32_t(vy));
        const auto settle_y=std::bit_cast<float>(uint32_t(word(ram,actor+0xA0)));
        const bool settle=state==2&&MEM_BU(0x28,int32_t(physics))==1&&MEM_H(0x34,int32_t(physics))==1&&MEM_H(0x7C,int32_t(physics))==-1&&
            (blink==0||blink==1)&&word(ram,physics+0x10)==0&&word(ram,physics+0x18)==0&&
            word(ram,physics+0x1C)==std::bit_cast<int32_t>(2.f)&&std::isfinite(settle_y)&&
            std::isfinite(settle_velocity)&&settle_velocity<=0&&settle_velocity>=-48;
        const auto scatter_speed=std::bit_cast<float>(uint32_t(word(ram,physics)));
        const auto scatter_slot=(actor-0x804B34B8u)/0xCC;
        const bool source_scatter_speed=std::isfinite(scatter_speed)&&scatter_speed>=2&&scatter_speed<=20&&std::fmod(scatter_speed,2.f)==0;
        const bool displayed_scatter_speed=scatter_slot<pickup_scatter_source_periods.size()&&pickup_scatter_source_periods[scatter_slot]&&
            reward_source_identity(ram,pickup_scatter_source_periods[scatter_slot]->common)&&
            pickup_scatter_source_periods[scatter_slot]->expected[3]==word(ram,physics);
        const bool scatter=state==3&&MEM_BU(0x28,int32_t(physics))==1&&MEM_H(0x34,int32_t(physics))>=0&&MEM_H(0x34,int32_t(physics))<=5&&
            MEM_H(0x7C,int32_t(physics))==-1&&(blink==0||blink==1)&&vy==0&&(source_scatter_speed||displayed_scatter_speed)&&
            std::isfinite(std::bit_cast<float>(uint32_t(word(ram,physics+4))))&&
            std::isfinite(std::bit_cast<float>(uint32_t(word(ram,actor+0x10))))&&std::isfinite(std::bit_cast<float>(uint32_t(word(ram,actor+0x9C))))&&
            std::isfinite(std::bit_cast<float>(uint32_t(word(ram,actor+0xA0))))&&std::isfinite(std::bit_cast<float>(uint32_t(word(ram,actor+0xA4))))&&
            std::isfinite(std::bit_cast<float>(uint32_t(word(ram,physics+0x10))))&&std::isfinite(std::bit_cast<float>(uint32_t(word(ram,physics+0x18))));
        const bool attached=state==2&&MEM_BU(0x28,int32_t(physics))==1&&MEM_H(0x34,int32_t(physics))==0&&
            (blink==0||blink==1)&&word(ram,physics+0x10)==0&&vy==0&&word(ram,physics+0x18)==0&&
            pickup_attachment_parent(ram,actor,physics)!=0;
        return ((state==10&&MEM_BU(0x28,int32_t(physics))==1)||stationary_idle||pickup_idle_motion_eligible(rdram,actor,physics)||sequence||spin||arc||reveal||settle||scatter||attached)&&pointer(model,0x84)&&
            game_model_animation().lifetime_token(model)&&!game_model_animation().native_birth(model)&&
            uint32_t(word(ram,0x80202064u+unsigned(kind)*84u))==update&&
            uint32_t(word(ram,0x80202068u+unsigned(kind)*84u))==contact;
    }
    else if(kind==129)return owns_linked_pickup_animation(rdram,actor,physics);
    else if(kind==131) {
        const auto model=uint32_t(word(ram,actor+0x74));
        // Only the initialized, pure downward drift is reviewed. Source owns
        // all RNG/clip/seek commands and the ordered pre-step destructor.
        return word(ram,actor+0x94)==1&&MEM_BU(0x28,int32_t(physics))==1&&MEM_BU(0xCA,int32_t(physics))==1&&
            word(ram,physics+0x10)==0&&word(ram,physics+0x18)==0&&word(ram,physics+0x14)==std::bit_cast<int32_t>(-5.f)&&
            MEM_BU(0xC8,int32_t(physics))==0&&MEM_H(0xD8,int32_t(physics))==0&&pointer(model,0x84)&&
            game_model_animation().lifetime_token(model)&&!game_model_animation().native_birth(model)&&
            uint32_t(word(ram,0x80202064u+131u*84u))==0x801ED8FCu&&
            uint32_t(word(ram,0x80202068u+131u*84u))==0x801ED8F4u;
    }
    else if(kind==130) {
        const auto state=word(ram,actor+0x94);const auto model=uint32_t(word(ram,actor+0x74));
        // Real state10 initialization/clip commands remain Source-owned.
        // Native acquires only the initialized motion after latch28=1.
        // Only the exact flash state dispatcher and its no-op partner are
        // reviewed. An effect not yet visited by Source cannot receive past dt.
        // Delay has no visible movement. Its discrete clip command stays in
        // the ordered Source pool while reward producers are Source-owned:
        // an effect can reuse a free slot before its own producer this pass.
        return state==10&&MEM_BU(0x28,int32_t(physics))==1&&pointer(model,0x84)&&game_model_animation().lifetime_token(model)&&
            !game_model_animation().native_birth(model)&&
            uint32_t(word(ram,0x80202064u+130u*84u))==0x801ED898u&&
            uint32_t(word(ram,0x80202068u+130u*84u))==0x801ED4A4u;
    }
    else if(kind==80)callback=0x801EF2ECu;
    else if(kind>=71&&kind<=74)callback=0x801EEFC8u;
    else if(kind==7)callback=0x8022D938u;
    else if(kind==9)callback=0x8022DBB0u;
    else if(kind==75||kind==76)callback=0x80223FBCu;
    else if(kind==77)callback=0x8023DF3Cu;
    else if(kind==18||kind==19)return linked_city_owner(ram,actor);
    else if(slot09_callback(kind)) {
        if(!slot09_patrol_state(kind,word(ram,actor+0x94))||(kind==26&&word(ram,actor+0x94)==102&&!slot09_dialogue26_can_own(ram))||!dora3_overlay_loaded(0x001398A0u)||
           !game_model_animation().lifetime_token(uint32_t(word(ram,actor+0x74))))return false;
        callback=slot09_callback(kind);
    }
    else if(city_patrol_callback(kind)) {
        const auto state=word(ram,actor+0x94);
        if(state==100)return city_story_family_owner(ram,actor);
        if(state!=1&&state!=2&&state!=3&&!(kind==21&&(state==200||state==201)))return false; // Other unreviewed AI states retain Source ownership.
        callback=city_patrol_callback(kind);
    }
    return callback && uint32_t(word(ram,0x80202064+unsigned(kind)*84))==callback &&
        ((kind!=7&&kind!=9&&!city_patrol_callback(kind))||dora3_overlay_loaded(0x00132F80u)) &&
        ((kind!=75&&kind!=76)||dora3_overlay_loaded(0x000EDDB0u)) &&
        (kind!=77||dora3_overlay_loaded(0x0015E440u));
}
ObjectClock object_identity(uint8_t* ram,uint32_t actor,uint64_t origin) {
    const auto physical=uint32_t(word(ram,actor+0x98)),model=uint32_t(word(ram,actor+0x74));uint8_t* rdram=ram;
    const auto lifetime=game_model_animation().lifetime_token(model);
    return {physical,model,MEM_H(0x20,int32_t(physical)),origin,lifetime,
        lifetime&&game_model_animation().native_birth(model)};
}
bool same_object(const ObjectClock& a,const ObjectClock& b) {
    return a.physical&&a.lifetime&&a.physical==b.physical&&a.model==b.model&&a.kind==b.kind&&a.lifetime==b.lifetime;
}
void register_object_clocks(uint8_t* ram,const SimulationSlice& slice) {
    dora3::graphics_profile::SampleScope graphics_sample(dora3::graphics_profile::Stage::ObjectRegistrationSample); dora3::profile::Scope profile(dora3::profile::Tag::ObjectClockRegistration);
    // A fresh Native model has no Source lookahead. Its integer angular
    // commands must end at full periods measured from this actor's birth.
    // Existing Source owners retain the acquired world's original phase.
    const auto start=slice_position(slice,false);
    for(unsigned slot=0;slot<object_clocks.size();++slot) {
        const auto actor=0x804B34B8u+slot*0xCC;
        if(!word(ram,actor+0x94)||!owns_common_object(ram,actor)) {cancel_slot09_story_start(ram,slot);cancel_slot09_special_turn(ram,slot);object_clocks[slot]={};continue;}
        const auto identity=object_identity(ram,actor,start);
        if(identity.kind!=7&&identity.kind!=9&&identity.kind!=18&&identity.kind!=129&&identity.kind!=130&&identity.kind!=131&&!standard_pickup_kind(identity.kind)&&!city_patrol_callback(identity.kind)&&!slot09_callback(identity.kind)&&!native_enemy_kind(identity.kind)&&!native_scene_object_kind(identity.kind)&&(identity.kind<71||identity.kind>77)) {object_clocks[slot]={};continue;}
        auto& clock=object_clocks[slot];
        if(!same_object(clock,identity)) {clock=identity;if(!clock.local)clock.origin=0;}
    }
}
struct Slot09DialogueReentry {
    uint32_t actor,physical,model;uint64_t lifetime,tick,phase,denominator;int state;
};
std::array<std::optional<Slot09DialogueReentry>,dora3::distance::common_capacity> slot09_source_dialogues,slot09_native_reentries;
void begin_object_scope(uint8_t* ram,uint32_t actor) {
    if(!gameplay_slice)return;
    const auto flash_physical=uint32_t(word(ram,actor+0x98));uint8_t* flash_ram=ram;
    if(pointer(flash_physical,0xF4)&&([&] {uint8_t* rdram=flash_ram;const auto kind=MEM_H(0x20,int32_t(flash_physical));return kind==129||kind==130||kind==131||standard_pickup_kind(kind);})()&&
       gameplay_slice->phase&&gameplay_slice->phase<60&&
       (!reward_pool_claim_matches(ram,actor)||([&] {
           const auto slot=(actor-0x804B34B8u)/0xCC;
           if(slot>=reward_source_ceilings.size()||!reward_source_ceilings[slot])return false;
           const auto ceiling=*reward_source_ceilings[slot];reward_source_ceilings[slot].reset();
           return ceiling.actor==actor&&ceiling.physical==flash_physical&&ceiling.model==uint32_t(word(ram,actor+0x74))&&
               ceiling.lifetime==game_model_animation().lifetime_token(ceiling.model)&&ceiling.epoch==epoch&&
               ceiling.tick==gameplay_slice->tick&&ceiling.phase==gameplay_slice->phase&&ceiling.denominator==gameplay_slice->denominator;
       })())&&
       !game_model_animation().native_birth(uint32_t(word(ram,actor+0x74)))) {
        // Source submits rational ends with a <1ns ceiling. Include that
        // ceiling only at the first acquisition, preserving full Original steps.
        const auto& slice=*gameplay_slice;
        object_suspension.emplace();
        try {object_scope.emplace(SimulationSlice{slice.numerator+slice.phase,slice.denominator,slice.tick,0});}
        catch(...) {object_suspension.reset();throw;}
        object_command_at_source_start=true;return;
    }
    const auto reentry_slot=(actor-0x804B34B8u)/0xCC;
    if(reentry_slot<slot09_native_reentries.size())if(auto& pending=slot09_native_reentries[reentry_slot];pending) {
        const auto handoff=*pending;pending.reset();
        const auto physical=uint32_t(word(ram,actor+0x98)),model=uint32_t(word(ram,actor+0x74));
        const auto& slice=*gameplay_slice;uint8_t* rdram=ram;
        if(handoff.actor==actor&&handoff.physical==physical&&handoff.model==model&&
           handoff.lifetime==game_model_animation().lifetime_token(model)&&
           handoff.tick==slice.tick&&handoff.phase==slice.phase&&handoff.denominator==slice.denominator&&
           handoff.phase<60&&word(ram,actor+0x94)==handoff.state&&MEM_H(0x20,int32_t(physical))==26&&
           dora3_overlay_loaded(0x001398A0u)&&owns_common_object(ram,actor)&&!game_model_animation().native_birth(model)) {
            // The Original callback closes dialogue at a rational Source
            // boundary. Include only its <1ns submission ceiling once;
            // otherwise clip0 starts late and its exact wrap loses a tick.
            if(handoff.phase) {
                const SimulationSlice aligned{slice.numerator+slice.phase,slice.denominator,slice.tick,0};
                object_suspension.emplace();
                try {object_scope.emplace(aligned);}catch(...) {object_suspension.reset();throw;}
                object_command_at_source_start=true;
            }
            return;
        }
    }
    // A late-born leg shares its parent's command period. Its animation
    // still advances by the actual interval from its own constructor value.
    const auto physical=uint32_t(word(ram,actor+0x98));uint8_t* rdram=ram;
    if(pointer(physical,0xDC)&&(MEM_H(0x20,int32_t(physical))==19||MEM_H(0x20,int32_t(physical))==6)) {
        const auto parent_index=MEM_H(0x7C,int32_t(physical));
        if(parent_index<30||parent_index>=dora3::distance::common_end)return;
        const auto parent=0x804B1CD0u+unsigned(parent_index)*0xCC,pp=uint32_t(word(ram,parent+0x98));
        if(!pointer(pp,0xDC)||(MEM_H(0x20,int32_t(pp))!=18&&!city_patrol_callback(MEM_H(0x20,int32_t(pp)))))return;
        actor=parent;
    }
    const auto slot=(actor-0x804B34B8u)/0xCC;
    if(slot>=object_clocks.size())return;
    auto& clock=object_clocks[slot];
    const auto current=object_identity(ram,actor,slice_position(*gameplay_slice,false));
    if((native_enemy_kind(current.kind)||native_scene_object_kind(current.kind))&&current.local&&!same_object(clock,current))clock=current;
    if(!clock.local||!clock.origin||!same_object(clock,object_identity(ram,actor,0)))return;
    const auto start=slice_position(*gameplay_slice,false);
    if(start<clock.origin)throw std::logic_error("Object received time before birth");
    const auto age=start-clock.origin,denominator=gameplay_slice->denominator;
    const SimulationSlice local{gameplay_slice->numerator,denominator,age/denominator,age%denominator};
    object_suspension.emplace();
    try {object_scope.emplace(local);}catch(...) {object_suspension.reset();throw;}
}
bool owns_projectile(uint8_t* ram,uint32_t actor) {
    if(!pointer(actor,0xCC))return false;
    const auto physical=uint32_t(word(ram,actor+0x98));if(!pointer(physical,0xDC))return false;
    uint8_t* rdram=ram;const auto kind=MEM_H(0x20,int32_t(physical));
    // Straight-flight geometry uses shared reviewed velocity/displacement;
    // terminal effects wait for the actual clip and retire once. Other
    // temporal/branching families retain their original owner.
    uint32_t callback=0;
    switch(kind) {
    case 82:callback=0x801CBFDC;break;
    case 83:callback=0x801CBD0C;break;
    case 84:callback=0x801CC5D4;break;case 88:callback=0x801CD5A4;break;
    case 89:callback=0x801CD2C8;break;
    case 85:callback=0x801CC33C;break;case 87:callback=0x801CC7F4;break;
    case 86:callback=0x801CCAAC;break;
    case 90:callback=0x801CD86C;break;
    case 101:callback=0x801CFA94;break;case 103:callback=0x801D0088;break;
    case 105:callback=0x801D05A8;break;
    case 91:callback=0x801CDCB8;break;case 93:callback=0x801CE338;break;
    case 95:callback=0x801CE954;break;case 97:callback=0x801CED88;break;
    case 99:callback=0x801CF52C;break;case 107:callback=0x801D0F48;break;
    case 92:callback=0x801CDAE4;break;case 94:callback=0x801CE074;break;
    case 96:callback=0x801CE620;break;case 98:callback=0x801CEBA8;break;
    case 100:callback=0x801CF1F0;break;case 102:callback=0x801CF7AC;break;
    case 104:callback=0x801CFDB4;break;case 106:callback=0x801D0398;break;
    case 108:callback=0x801D0BC0;break;default:return false;
    }
    return uint32_t(word(ram,0x801FAB48+unsigned(kind-82)*8))==callback;
}
ProjectileClock projectile_identity(uint8_t* ram,uint32_t actor,uint64_t origin) {
    const auto physical=uint32_t(word(ram,actor+0x98));uint8_t* rdram=ram;
    const auto model=uint32_t(word(ram,actor+0x74));
    return {physical,model,MEM_H(0x20,int32_t(physical)),origin,game_model_animation().lifetime_token(model)};
}
bool same_projectile(const ProjectileClock& a,const ProjectileClock& b) {
    return a.physical && a.lifetime && a.physical==b.physical && a.model==b.model && a.kind==b.kind && a.lifetime==b.lifetime;
}
void register_projectile_birth(uint8_t* ram,uint32_t actor,bool force=false,bool at_start=false) {
    const auto slot=(actor-0x804B24C8u)/0xCC;
    if(slot>=projectile_clocks.size() || !word(ram,actor+0x94) || !owns_projectile(ram,actor))return;
    // Ordinary hits and positive animation cues commit at the slice end.
    // A reviewed zero-cue player constructor commits at the slice start and
    // receives that elapsed interval after its initial Source command pass.
    const auto origin=gameplay_slice?slice_position(*gameplay_slice,!at_start):timeline->ticks()*uint64_t(timeline->divider())*second+timeline->phase();
    const auto identity=projectile_identity(ram,actor,origin);
    if(force || !same_projectile(projectile_clocks[slot],identity))projectile_clocks[slot]=identity;
}
void begin_projectile_scope(uint8_t* ram,uint32_t actor) {
    if(!gameplay_slice)return;
    const auto slot=(actor-0x804B24C8u)/0xCC;auto& clock=projectile_clocks[slot];
    if(!clock.origin)return; // Source-aligned owners already use this scope.
    const auto start=slice_position(*gameplay_slice,false);
    if(start<clock.origin)throw std::logic_error("Projectile received time before birth");
    const auto age=start-clock.origin,denominator=gameplay_slice->denominator;
    const SimulationSlice local{gameplay_slice->numerator,denominator,age/denominator,age%denominator};
    projectile_suspension.emplace();
    try {projectile_scope.emplace(local);}catch(...) {projectile_suspension.reset();throw;}
    (void)ram;
}
// C8B28 clears physics+CC before dispatch, then defaults zero to3 at90A8.
// These second bodies explicitly write1. The mirror always advances at3.
// Keep acquisition and per-slice planning on the same ROM-derived rate law.
float primary_player_model_rate(uint8_t* rdram,const SimulationSlice* slice=nullptr) {
    const auto actor=uint32_t(word(rdram,0x8021D49C));
    const auto state=word(rdram,actor+0x94);
    switch(state) {
    case 37: // C6084 -> C6108
    case 43: // C67C8 -> C684C
    case 46: // C6B08 -> C6B2C
    case 51: // C729C -> C72BC
    case 52: case 53: case 54: case 55: case 56: // C73F0 and wrappers
        return 1.f;
    case 40: { // C63F8: freeze through15, kick at16, rate1 after16.
        const auto physical=uint32_t(word(rdram,0x8021D4B0));
        const auto count=MEM_H(0x44,int32_t(physical));
        if(count>16)return 1.f;
        if(count!=16||!slice)return 3.f;
        // A kick already consumed in this Source period must not forecast
        // the next counter increment. Match the existing C6418 phase guard.
        const auto model=uint32_t(word(rdram,actor+0x74));
        if(player_damage_counter) {
            const auto& c=*player_damage_counter;
            if(c.actor==actor&&c.physical==physical&&c.model==model&&
               c.lifetime==game_model_animation().lifetime_token(model)&&
               c.run==game_model_animation().run_token(model)&&
               c.tick==slice->tick&&c.denominator==slice->denominator)return 3.f;
        }
        return 1.f; // Next unconsumed C6418 increments16 to17.
    }
    default:return 3.f;
    }
}
std::optional<float> planned_model_rate(uint8_t* ram,uint32_t model) {
    if(motion_allowed&&word(ram,0x8021D494)==0)for(auto slot:{0x8021D49Cu,0x8021D640u}) {
        const auto actor=uint32_t(word(ram,slot));
        if(pointer(actor,0xCC)&&uint32_t(word(ram,actor+0x74))==model) {
            return slot==0x8021D49Cu?primary_player_model_rate(ram):3.f;
        }
    }
    if(object_allowed)for(unsigned i=30;i<dora3::distance::common_end;++i) {
        const auto actor=0x804B1CD0u+i*0xCC;
        if(!word(ram,actor+0x94)||!owns_common_object(ram,actor)||uint32_t(word(ram,actor+0x74))!=model)continue;
        const auto physical=uint32_t(word(ram,actor+0x98));uint8_t* rdram=ram;
        if(MEM_BU(0xC9,int32_t(physical))==1)continue;
        const float rate=std::bit_cast<float>(word(ram,physical+0xCC));
        return rate==0?3.0f:rate;
    }
    if(projectile_allowed)for(unsigned i=10;i<30;++i) {
        const auto actor=0x804B1CD0u+i*0xCC;
        if(!word(ram,actor+0x94)||!owns_projectile(ram,actor)||uint32_t(word(ram,actor+0x74))!=model)continue;
        uint8_t* rdram=ram;const auto physical=uint32_t(word(ram,actor+0x98));
        if(MEM_BU(0xC9,int32_t(physical))!=1)return 3.0f;
    }
    return std::nullopt;
}
// clip_boundary_limit observes RAM without advancing any owner. Resolve its
// rates once at this boundary instead of repeating the whole actor pool for
// every registered model. Rebuild after every actual simulation slice; no
// lifetime, state, command, pause or overlay decision survives a mutation.
std::unordered_map<uint32_t,float> boundary_model_rates(uint8_t* ram,const SimulationSlice& slice) {
    dora3::graphics_profile::SampleScope graphics_sample(dora3::graphics_profile::Stage::ModelRatePlanSample); dora3::profile::Scope profile(dora3::profile::Tag::ModelRatePlan);
    std::unordered_map<uint32_t,float> rates;
    rates.reserve(dora3::distance::common_capacity+22);
    // emplace preserves the old search order, including shared model pointers:
    // primary player, mirrored player, common actors, then projectiles.
    if(motion_allowed&&word(ram,0x8021D494)==0)for(auto address:{0x8021D49Cu,0x8021D640u}) {
        const auto actor=uint32_t(word(ram,address));
        if(!pointer(actor,0xCC))continue;
        rates.emplace(uint32_t(word(ram,actor+0x74)),
            address==0x8021D49Cu?primary_player_model_rate(ram,&slice):3.f);
    }
    if(object_allowed)for(unsigned slot=30;slot<dora3::distance::common_end;++slot) {
        const auto actor=0x804B1CD0u+slot*0xCC;
        if(!word(ram,actor+0x94)||!owns_common_object(ram,actor))continue;
        uint8_t* rdram=ram;const auto physical=uint32_t(word(ram,actor+0x98));
        if(MEM_BU(0xC9,int32_t(physical))==1)continue;
        const float rate=std::bit_cast<float>(word(ram,physical+0xCC));
        rates.emplace(uint32_t(word(ram,actor+0x74)),rate==0?3.f:rate);
    }
    if(projectile_allowed)for(unsigned slot=10;slot<30;++slot) {
        const auto actor=0x804B1CD0u+slot*0xCC;
        if(!word(ram,actor+0x94)||!owns_projectile(ram,actor))continue;
        uint8_t* rdram=ram;const auto physical=uint32_t(word(ram,actor+0x98));
        if(MEM_BU(0xC9,int32_t(physical))!=1)
            rates.emplace(uint32_t(word(ram,actor+0x74)),3.f);
    }
    return rates;
}
bool door_fade_active(uint8_t* rdram) {
    if(!rdram||!timeline||word(rdram,0x8021D494)!=16)return false;
    const auto valid=[&](const DoorFadeOwner& v) {
        return v.epoch==epoch&&native_scene_object_owned(rdram,v.actor)&&
            uint32_t(word(rdram,v.actor+0x98))==v.physical&&
            uint32_t(word(rdram,v.actor+0x74))==v.model&&
            game_model_animation().lifetime_token(v.model)==v.lifetime&&
            MEM_H(0x20,int32_t(v.physical))==109&&word(rdram,v.actor+0x94)==2&&
            MEM_BU(0x28,int32_t(v.physical))==1;
    };
    if(door_fade_owner&&valid(*door_fade_owner))return true;
    door_fade_owner.reset();
    if(!object_call||!active_simulation_slice())return false;
    const uint32_t actor=uint32_t(word(rdram,0x8021BDEC));
    if(!native_scene_object_owned(rdram,actor))return false;
    const uint32_t physical=uint32_t(word(rdram,actor+0x98)),model=uint32_t(word(rdram,actor+0x74));
    DoorFadeOwner owner{actor,physical,model,game_model_animation().lifetime_token(model),epoch};
    if(!valid(owner))return false;
    door_fade_owner=owner;return true;
}
bool live(uint8_t* ram) {
    return timeline && dora3_native_loop_gameplay_active() &&
        (native_gameplay_preview_eligible(ram)||(scene_completion_active(ram)&&native_gameplay_preview_conditions(ram,true)));
}
bool capture_actor(uint8_t* ram,uint32_t actor) {
    return actor>=0x804B34B8u && actor<0x804B1CD0u+dora3::distance::common_end*0xCC && (actor-0x804B1CD0u)%0xCC==0 &&
        word(ram,actor+0x94) && pointer(uint32_t(word(ram,actor+0x98)),0xDC);
}
struct InteractionScope {
    bool saved=interaction_after_pose;
    InteractionScope(){interaction_after_pose=true;}
    ~InteractionScope(){interaction_after_pose=saved;}
};
void update_interaction(uint8_t* ram,recomp_context* ctx,uint32_t actor,uint32_t physical) {
    recomp_context command;dora3::copy_recomp_context(command,*ctx);
    command.r4=int32_t(actor);command.r5=int32_t(physical);
    InteractionScope after_pose;dora3_s3_core_801D05A8(ram,&command);
}
void initialize_player_projectiles(uint8_t* ram,recomp_context* ctx) {
    // Original runs newly fired projectiles in the following pool pass. Keep
    // that initial command/geometry and its pose at the birth timestamp;
    // existing actors must not receive an additional Source update.
    struct BirthScope {
        bool saved=player_projectile_birth;
        BirthScope(){player_projectile_birth=true;}
        ~BirthScope(){player_projectile_birth=saved;}
    } births;
    recomp_context command;dora3::copy_recomp_context(command,*ctx);
    {SimulationSuspension birth_commands;dora3_s3_801D10D8(ram,&command);}
    game_model_animation().acquire_simulation(ram,[&](uint32_t model)->std::optional<float> {
        for(unsigned i=0;i<projectile_before.size();++i) {
            const auto actor=0x804B24C8u+i*0xCC;
            const auto physical=uint32_t(word(ram,actor+0x98));
        if(word(ram,actor+0x94) && (physical!=projectile_before[i] || game_model_animation().lifetime_token(model)!=projectile_before_lifetimes[i]) && owns_projectile(ram,actor) &&
               uint32_t(word(ram,actor+0x74))==model) {
                uint8_t* rdram=ram;const auto kind=MEM_H(0x20,int32_t(physical));
                if(kind!=101 && kind!=103 && kind!=105)return 3.f;
            }
        }
        return std::nullopt;
    },false);
}
}
static bool native_gameplay_preview_conditions(uint8_t* rdram,bool allow_room_request,bool allow_door_fade) {
    dora3::profile::Scope detailProfile(dora3::profile::Tag::GameplayEligibilitySample,dora3::profile::sample_stage(dora3::profile::Tag::GameplayEligibilitySample,32));
    for(unsigned i=0;i<8;++i) {
        const uint32_t slot=0x801A44F0+i*20;
        if(i<2) {
            if(word(rdram,slot)!=2||word(rdram,slot+12)!=int(i)||uint32_t(word(rdram,slot+16))!=(i?0x801BB848u:0x801BB730u))return false;
        } else if(word(rdram,slot)!=0&&word(rdram,slot)!=5)return false;
    }
    // BA0F4/BA310 consume a room request in two Original updates; B8FB4
    // holds the world while the asynchronous transition is pending. Acquire
    // only after that whole Original callback has completed.
    if((!allow_room_request&&word(rdram,0x802170EC)!=0) || MEM_BU(0,int32_t(0x80217118))!=0 || word(rdram,0x802170F0)!=0)return false;
    if(word(rdram,0x8021D484)!=0||(word(rdram,0x8021D494)!=0&&!(allow_door_fade&&word(rdram,0x8021D494)==16))||word(rdram,0x80218AD8)!=0||MEM_BU(0,int32_t(0x801B36EC))!=0||
       word(rdram,0x801A9C88)!=0||word(rdram,0x801AA648)==0||word(rdram,0x801A9500)==0)return false;
    const auto player=uint32_t(word(rdram,0x8021D49C)),control=uint32_t(word(rdram,0x8021D6C0));
    if(!pointer(player,0xD4)||!pointer(uint32_t(word(rdram,0x8021D640)),0xD4)||!pointer(control,0x58)||!pointer(uint32_t(word(rdram,0x8021D4B0)),0xDC)||
       !pointer(uint32_t(word(rdram,0x8021D648)),0x20))return false;
    const auto state=word(rdram,player+0x94);
    // The two weapon action pairs share this drive/contact closure; their
    // shot cues belong to the model pose advanced by the current interval.
    const auto status=uint32_t(word(rdram,0x8021D4A8));
    const bool live_ground_damage=(state>=32&&state<=47) && byte_pointer(status,2);
    const bool live_hover=(state==49 || state==63) && byte_pointer(status,2) && (state==49 || MEM_B(1,int32_t(status))>0) &&
        MEM_BU(0x28,int32_t(control))==0 && MEM_BU(0x2C,int32_t(control))==1;
    const bool death_owner=state>=50 && state<=56 &&
        (MEM_BU(0x2C,int32_t(control))==0 || (state==56 && MEM_BU(0x28,int32_t(control))==0 && MEM_BU(0x2C,int32_t(control))==1));
    return MEM_BU(0,int32_t(control))==0 && (death_owner || live_hover || (MEM_BU(0x2C,int32_t(control))==0 &&
        ((((state>=1&&state<=18)||state==19||state==21||state==22||state==23||state==24||state==25||state==26||state==28||state==29||state==30||state==31||state==59||state==60||state==61||state==62||state==64||state==65||state==66||state==67||live_ground_damage) && MEM_BU(0x28,int32_t(control))==0)
         || player_flight_state(rdram))));
}
bool native_gameplay_preview_eligible(uint8_t* ram) {
    return native_gameplay_preview_conditions(ram,false)||
        (door_fade_active(ram)&&native_gameplay_preview_conditions(ram,false,true));
}
void native_gameplay_preview_begin(uint8_t* ram,Nanoseconds now,unsigned divider) {
    native_gameplay_preview_reset();timeline=std::make_shared<SimulationTimeline>(divider);previous=source_origin=now;
    native_input_begin(ram,now);
    for(unsigned i=0;i<projectile_clocks.size();++i) {
        const auto actor=0x804B24C8u+i*0xCC;
        if(word(ram,actor+0x94) && owns_projectile(ram,actor))projectile_clocks[i]=projectile_identity(ram,actor,0);
    }
    // Acquisition happens after a completed original world update.
    motion_allowed=camera_allowed=object_allowed=projectile_allowed=true;
    // Reviewed shot actions consume the acquired pose after model advance.
    // Other player actions keep their existing counter consumption contract.
    game_model_animation().acquire_simulation(ram,[&](uint32_t model)->std::optional<float> {
        for(auto slot:{0x8021D49Cu,0x8021D640u}) {
            const auto actor=uint32_t(word(ram,slot));
            if(pointer(actor,0xCC)&&uint32_t(word(ram,actor+0x74))==model) {
                const auto state=word(ram,uint32_t(word(ram,0x8021D49C))+0x94);
                if(!player_shot_cue(state) && state!=24 && state!=25 && state!=27)return std::nullopt;
            }
        }
        return planned_model_rate(ram,model);
    });
    for(unsigned i=10;i<30;++i) {
        const auto actor=0x804B1CD0u+i*0xCC;
        if(!word(ram,actor+0x94)||!owns_projectile(ram,actor))continue;
        const auto physical=uint32_t(word(ram,actor+0x98));uint8_t* rdram=ram;
        if(MEM_H(0x20,int32_t(physical))!=105)continue;
        const auto target=uint32_t(word(ram,physical+0x48));
        if(capture_actor(ram,target))game_model_animation().bind_actor(ram,uint32_t(word(ram,actor+0x74)),physical+0x48,target);
    }
}
extern "C" int dora3_native_interaction_skip() {return active_simulation_slice()&&!interaction_after_pose;}
extern "C" int dora3_native_defer_player_shot() {return motion_call;}
extern "C" int dora3_native_player_weapon_command_only() {return player_weapon_command_only;}
extern "C" int dora3_native_interaction_enabled() {return interaction_after_pose;}
extern "C" int dora3_native_interaction_geometry_only() {return interaction_geometry_only;}
extern "C" float dora3_native_interaction_reach(uint8_t* ram,recomp_context* ctx) {
    const auto model=uint32_t(word(ram,uint32_t(ctx->r17)+0x74));
    const auto frame=std::bit_cast<float>(word(ram,model+0x64));
    if(!std::isfinite(frame)||frame<0)return 0;
    auto reach=[&](unsigned index)->float {
        if(index>=49)return 0;
        switch(uint32_t(word(ram,0x80210898u+4*index))) {
        case 0x801D0650:return 400;case 0x801D0660:return 380;case 0x801D0670:return 360;
        case 0x801D0680:return 290;case 0x801D0690:return 220;case 0x801D06A0:return 150;
        case 0x801D06B0:return 80;case 0x801D06C0:return 31;case 0x801D06D0:return 18;default:return 0;
        }
    };
    if(frame>=51)return 0;
    // Original advances this clip by three frames. Intermediate jump-table
    // entries are unused defaults, not zero-reach animation keys.
    const auto index=3*unsigned(frame/3);const auto a=reach(index),b=reach(index+3);
    return a+(b-a)*(frame-float(index))/3;
}
extern "C" void dora3_native_interaction_bind(uint8_t* ram,recomp_context* ctx) {
    const auto physical=uint32_t(ctx->r16),actor=uint32_t(ctx->r17),target=uint32_t(ctx->r2);
    if(capture_actor(ram,target))game_model_animation().bind_actor(ram,uint32_t(word(ram,actor+0x74)),physical+0x48,target);
}
extern "C" int dora3_native_interaction_target_valid(uint8_t* ram,recomp_context* ctx) {
    const auto physical=uint32_t(ctx->r16),actor=uint32_t(ctx->r17),target=uint32_t(word(ram,physical+0x48));
    return capture_actor(ram,target)&&game_model_animation().bound_actor_valid(ram,uint32_t(word(ram,actor+0x74)),physical+0x48,target);
}
extern "C" void dora3_native_attached_projectile_offset(uint8_t* rdram,recomp_context* ctx,void (*update)(uint8_t*,recomp_context*)) {
    // These callbacks first place the actor at the current player attachment.
    // Their projected velocity is a spatial offset from that point; it is not
    // another free-flight integral over elapsed time.
    SimulationSuspension geometry;update(rdram,ctx);
}
extern "C" void dora3_native_homing_motion(uint8_t* rdram,recomp_context* ctx) {
    const auto* slice=active_simulation_slice();if(!slice)throw std::logic_error("Homing motion outside native interval");
    const auto actor=uint32_t(ctx->r17),physical=uint32_t(ctx->r16),model=uint32_t(word(rdram,actor+0x74));
    auto read=[&](uint32_t p){return std::bit_cast<float>(word(rdram,p));};
    auto write=[&](uint32_t p,float value){MEM_W(0,int32_t(p))=std::bit_cast<int32_t>(value);};
    const std::array<int32_t,3> published_velocity{word(rdram,physical+0x10),word(rdram,physical+0x14),word(rdram,physical+0x18)};
    const auto& sample=game_model_animation().motion_begin(rdram,model,actor,physical,[&] {
        SourceMotionSample s;
        for(unsigned k=0;k<3;++k)s.position[k]=read(actor+0x9C+4*k);
        s.yaw=read(physical+4);s.pitch=read(physical+0x58);s.oscillator=MEM_H(0x36,int32_t(physical));
        recomp_context geometry;dora3::copy_recomp_context(geometry,*ctx);geometry.r4=int32_t(physical);
        dora3_s3_core_801CB2F8(rdram,&geometry);geometry.r4=int32_t(physical);dora3_s3_core_801CB3A8(rdram,&geometry);
        for(unsigned k=0;k<3;++k)s.velocity[k]=read(physical+0x10+4*k);
        const auto pitch_bits=word(rdram,physical+0xC);
        union {uint64_t bits;double value;} radians;
        radians.bits=LD(0,int32_t(0x80210858));
        geometry.f12.fl=float(s.oscillator*radians.value);dora3_lib_80057A80(rdram,&geometry);
        s.pitch_offset=std::remainder(std::bit_cast<float>(pitch_bits)-(s.pitch+geometry.f0.fl*30.f),360.f);
        const double remaining=double(slice->denominator-slice->phase)/double(slice->denominator);
        geometry.f12.fl=float(std::fmod(s.oscillator+40*remaining,360.)*radians.value);
        dora3_lib_80057A80(rdram,&geometry);
        float future=std::fmod(s.pitch+geometry.f0.fl*30.f,360.f);if(future<0)future+=360;
        write(physical+0xC,future);geometry.r4=int32_t(physical);dora3_s3_core_801CB2F8(rdram,&geometry);
        geometry.r4=int32_t(physical);dora3_s3_core_801CB3A8(rdram,&geometry);
        for(unsigned k=0;k<3;++k) {s.bend[k]=read(physical+0x10+4*k)-s.velocity[k];write(physical+0x10+4*k,s.velocity[k]);}
        MEM_W(0,int32_t(physical+0xC))=pitch_bits;
        return s;
    });
    const double duration=double(slice->denominator-sample.start_phase)/double(slice->denominator);
    const double age=double(slice->phase+slice->numerator-sample.start_phase)/double(slice->denominator);
    // This curved extension evaluates actual positions at every interval.
    // Its correction vanishes at the source endpoint, where Original used
    // the incoming projected velocity once. It never accumulates Euler drift.
    for(unsigned k=0;k<3;++k) {
        write(actor+0x9C+4*k,float(sample.position[k]+sample.velocity[k]*age+sample.bend[k]*age*(age-duration)*0.5));
        // These guest fields are the preceding Original projection command,
        // rather than the derivative of the continuous motion curve. Publish
        // the new command at its source end; forecast geometry must not expose
        // the next command a fraction of a nanosecond after that boundary.
        if(slice->ends_tick())write(physical+0x10+4*k,sample.velocity[k]);
        else MEM_W(0,int32_t(physical+0x10+4*k))=published_velocity[k];
    }
}
extern "C" int dora3_native_player_flight_hook(uint8_t* ram,recomp_context* ctx,uint32_t site) {
    const auto* slice=active_simulation_slice();
    if(!slice || !motion_call || !player_flight_state(ram))return 0;
    const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
    // Scripted66 sets its scalar speed directly and calls only C0D88's
    // fixed-velocity projection. It has no C1A3C/C1ADC command to forecast.
    // Existing paired constant-velocity stores feed the real collision path.
    if(word(ram,actor+0x94)==66 || word(ram,actor+0x94)==67)return 0;
    if(site==0x801C1ADC)return player_flight_clock && player_flight_clock->open && same_flight_owner(ram,*player_flight_clock);
    auto read=[&](uint32_t address){return std::bit_cast<float>(word(ram,address));};
    uint8_t* rdram=ram;
    auto write=[&](uint32_t address,float value){MEM_W(0,int32_t(address))=std::bit_cast<int32_t>(value);};
    if(site==0x801C1A3C) {
        const auto model=uint32_t(word(ram,actor+0x74));
        const auto params=uint32_t(word(ram,0x8021D648));
        const std::array<int32_t,7> command{int32_t(ctx->r4),MEM_HU(0,int32_t(0x8021D644))&0x8000,
            word(ram,0x8021D470),word(ram,0x8021D688),word(ram,0x8021D68C)&3,MEM_H(0xA,int32_t(params)),
            word(ram,actor+0x94)!=48};
        const auto observation=flight_observation(ram,actor,physical);
        if(!player_flight_clock || !same_flight_owner(ram,*player_flight_clock) ||
           player_flight_clock->tick!=slice->tick || player_flight_clock->denominator!=slice->denominator ||
           player_flight_clock->expected_phase!=slice->phase || player_flight_clock->expected!=observation ||
           player_flight_clock->command!=command) {
            PlayerFlightClock clock;clock.actor=actor;clock.physical=physical;clock.model=model;
            clock.lifetime=game_model_animation().lifetime_token(model);clock.run=game_model_animation().run_token(model);
            if(!clock.lifetime || !clock.run)throw std::logic_error("Unregistered flight model");
            clock.tick=slice->tick;clock.denominator=slice->denominator;clock.start_phase=slice->phase;
            clock.expected_phase=slice->phase;clock.expected=observation;clock.command=command;
            const auto fields=flight_fields(actor,physical);
            for(unsigned i=0;i<6;++i)clock.initial[i]=read(fields[i]);
            recomp_context source;dora3::copy_recomp_context(source,*ctx);
            // Only these reviewed helpers execute as Source geometry. They
            // have no allocation, input, random, audio or state commands.
            {SimulationSuspension geometry;
             dora3_s3_801C0D88(ram,&source);
             for(unsigned i=0;i<3;++i)clock.velocity[i]=read(physical+0x10+4*i);
             source.r4=command[0];dora3_s3_core_801C1A3C(ram,&source);
             // Flight damage48 only invokes the scalar speed helper;
             // its orientation remains fixed by the impact command.
             if(command[6])dora3_s3_core_801C1ADC(ram,&source);
             for(unsigned i=0;i<6;++i)clock.target[i]=read(fields[i]);
             dora3_s3_801C0D88(ram,&source);
             for(unsigned i=0;i<3;++i)clock.change[i]=read(physical+0x10+4*i)-clock.velocity[i];
             for(unsigned i=0;i<fields.size();++i)MEM_W(0,int32_t(fields[i]))=observation[i];}
            player_flight_clock=clock;
        }
        auto& clock=*player_flight_clock;clock.open=true;
        const double age=double(slice->phase+slice->numerator-clock.start_phase)/double(slice->denominator);
        const auto fields=flight_fields(actor,physical);
        for(unsigned i=0;i<6;++i) {
            const double delta=i?std::remainder(double(clock.target[i])-clock.initial[i],360.):double(clock.target[i])-clock.initial[i];
            double value=clock.initial[i]+delta*age;
            // Source may retain360 for one command before writing0. Equal
            // orientations must retain that representation until the command
            // end, including tiny positive intervals beyond a rounded stamp.
            if(i && delta!=0.) {value=std::fmod(value,360.);if(value<0)value+=360.;}
            write(fields[i],age==1.?clock.target[i]:float(value));
        }
        return 1;
    }
    if(site==0x801C0E24) {
        if(!player_flight_clock || !player_flight_clock->open || !same_flight_owner(ram,*player_flight_clock))
            throw std::logic_error("Flight projection without motion command");
        const auto& clock=*player_flight_clock;
        const double a=double(slice->phase-clock.start_phase)/double(slice->denominator);
        const double b=double(slice->phase+slice->numerator-clock.start_phase)/double(slice->denominator);
        for(unsigned i=0;i<3;++i) {
            const float velocity=float(clock.velocity[i]+clock.change[i]*b);
            const double distance=clock.velocity[i]*(b-a)+0.5*clock.change[i]*(b*b+b-a*a-a);
            write(physical+0x10+4*i,velocity);
            publish_simulation_motion(physical+0x10+4*i,{velocity,distance});
        }
    }
    return 0;
}
extern "C" float dora3_native_projectile_gravity_f32(uint8_t* ram,recomp_context* ctx) {
    const auto physical=uint32_t(ctx->r4),address=physical+0x14;
    const auto velocity=ctx->f0.fl,gravity=ctx->f2.fl;
    const auto rounded=dora3_native_gravity_f32(velocity,gravity,-50.f,address);
    if(!projectile_call || !active_simulation_slice())return rounded;
    for(unsigned i=0;i<20;++i) {
        const auto actor=0x804B24C8u+i*0xCC;
        if(!word(ram,actor+0x94) || uint32_t(word(ram,actor+0x98))!=physical || !owns_projectile(ram,actor))continue;
        const auto model=uint32_t(word(ram,actor+0x74));
        // Re-evaluate from this owner's Source baseline. Repeated rounded
        // subtractions could miss an exact velocity threshold at the end.
        // Contacts/impulses and allocation replacement invalidate that sample.
        const auto exact=std::max(-50.f,game_model_animation().scalar_linear(ram,model,address,velocity,-double(gravity)));
        const auto displacement=dora3_native_gravity_displacement_f32(rounded,address);
        publish_simulation_motion(address,{exact,displacement});
        return exact;
    }
    return rounded;
}
extern "C" float dora3_native_homing_sample(uint8_t* rdram,recomp_context* ctx,unsigned field) {
    const auto actor=uint32_t(ctx->r17),model=uint32_t(word(rdram,actor+0x74));
    const auto* sample=game_model_animation().motion_current(model,actor);
    if(!sample)throw std::logic_error("Missing homing source sample");
    if(field==3)return sample->yaw;
    if(field==4)return sample->pitch;
    if(field==5) {
        const auto* slice=active_simulation_slice();
        const double duration=double(slice->denominator-sample->start_phase);
        const double age=double(slice->phase+slice->numerator-sample->start_phase);
        return float(sample->pitch+sample->pitch_offset*(1-age/duration));
    }
    if(field>=3)throw std::logic_error("Invalid homing sample field");
    const auto* slice=active_simulation_slice();
    const double remaining=double(slice->denominator-sample->start_phase)/double(slice->denominator);
    return float(sample->position[field]+sample->velocity[field]*remaining);
}
extern "C" void dora3_native_homing_target(uint8_t* rdram,recomp_context* ctx) {
    const auto actor=uint32_t(ctx->r17),physical=uint32_t(ctx->r16);
    std::array<int32_t,4> saved{word(rdram,actor+0x9C),word(rdram,actor+0xA0),word(rdram,actor+0xA4),word(rdram,physical+4)};
    for(unsigned k=0;k<3;++k)MEM_W(0,int32_t(actor+0x9C+4*k))=std::bit_cast<int32_t>(dora3_native_homing_sample(rdram,ctx,k));
    MEM_W(0,int32_t(physical+4))=std::bit_cast<int32_t>(dora3_native_homing_sample(rdram,ctx,3));
    ctx->r4=int32_t(actor);dora3_s3_core_801CB870(rdram,ctx);
    for(unsigned k=0;k<3;++k)MEM_W(0,int32_t(actor+0x9C+4*k))=saved[k];
    MEM_W(0,int32_t(physical+4))=saved[3];
}
extern "C" void dora3_native_homing_end(uint8_t* rdram,recomp_context* ctx) {
    const auto actor=uint32_t(ctx->r17),model=uint32_t(word(rdram,actor+0x74));
    game_model_animation().motion_end(rdram,model,actor);
}
bool native_gameplay_preview_object_pass() {return object_call&&active_simulation_slice()&&timeline;}
std::optional<uint8_t> native_gameplay_preview_npc_model_flags(uint8_t* rdram,uint32_t model) {
    const auto* slice=active_simulation_slice();
    if(!object_call||!slice||!live(rdram))return std::nullopt;
    const auto actor=uint32_t(word(rdram,0x8021BDEC));
    if(actor<0x804B34B8u||(actor-0x804B34B8u)%0xCC||
       (actor-0x804B34B8u)/0xCC>=dora3::distance::common_capacity)return std::nullopt;
    const auto physical=uint32_t(word(rdram,actor+0x98));
    if(!pointer(physical,0xDC))return std::nullopt;
    const auto kind=MEM_H(0x20,int32_t(physical));
    // These callbacks cache discrete decisions once per authored period,
    // while their models advance on every fractional slice. The raw bit2
    // pulse can disappear before the next decision (an opening talk clip
    // then loops forever). Use the same whole-period sample as scene NPCs.
    // Ownership validates the current overlay, callback and supported state.
    if((!city_patrol_callback(kind)&&!slot09_callback(kind)&&kind!=18&&kind!=19)||
       !owns_common_object(rdram,actor)||!pointer(model,0x84)||
       !game_model_animation().lifetime_token(model))return std::nullopt;
    for(unsigned slot=0;slot<8;++slot)if(uint32_t(word(rdram,actor+0x74+4*slot))==model)
        return game_model_animation().source_command_sample(rdram,model,*slice).flags;
    return std::nullopt;
}
void native_gameplay_preview_enemy_defer_birth(uint8_t* ram,uint32_t actor) {
    if(!object_call||!gameplay_slice||!(native_enemy_owned(ram,actor)||native_scene_object_owned(ram,actor)))return;
    const auto slot=(actor-0x804B34B8u)/0xCC;const auto& slice=*gameplay_slice;
    if(slot>=deferred_enemy_births.size())return;
    const uint64_t deadline=(slice.tick+1)*slice.denominator;
    deferred_enemy_births[slot]=DeferredEnemyBirth{object_identity(ram,actor,deadline),deadline,slice.denominator,epoch};
}
void native_gameplay_preview_enemy_command_clock(uint8_t* ram,uint32_t actor) {
    if(!object_call||!gameplay_slice||!(native_enemy_owned(ram,actor)||native_scene_object_owned(ram,actor)))throw std::logic_error("Invalid enemy command clock");
    const auto slot=(actor-0x804B34B8u)/0xCC;
    auto fresh=object_identity(ram,actor,slice_position(*gameplay_slice,false));fresh.local=true;
    object_clocks[slot]=fresh;
    object_scope.reset();object_suspension.reset();object_suspension.emplace();
    try {object_scope.emplace(SimulationSlice{gameplay_slice->numerator,gameplay_slice->denominator,0,0});}
    catch(...){object_suspension.reset();throw;}
}

void native_gameplay_preview_reset(uint8_t* ram) {
    native_enemy_reset();
    if(ram)native_input_end(ram);
    for(unsigned slot=0;slot<slot09_story_starts.size();++slot)cancel_slot09_story_start(ram,slot);
    for(unsigned slot=0;slot<slot09_special_turns.size();++slot)cancel_slot09_special_turn(ram,slot);
    scene_completion_slice.reset();
    reset_slots(scene_frame_completed);
    reset_slots(scene_frame_entered);
    reset_slots(flash_delay_commits);
    reset_slots(reward_pool_claims);
    reset_slots(reward_source_ceilings);
    reset_slots(flash_motion_clocks);
    reset_slots(flash_source_periods);
    reset_slots(pickup_motion_clocks);
    reset_slots(flash_motion_initializers);
    reset_slots(pickup_idle_source_periods);
    reset_slots(pickup_idle_motion_source_periods);
    reset_slots(reward_source_periods);
    reset_slots(pickup_spin_source_periods);
    reset_slots(pickup_arc_source_periods);
    reset_slots(pickup_reveal_source_periods);
    reset_slots(pickup_settle_source_periods);
    reset_slots(pickup_scatter_source_periods);
    for(auto& handoff:scene_source_handoffs)if(handoff&&!scene_completed_handoff_valid(ram,*handoff))handoff.reset();
    reset_slots(slot09_scene_publications);
    for(auto& handoff:slot09_scene_handoffs)if(handoff&&!slot09_scene_handoff_valid(ram,*handoff))handoff.reset();
    ++epoch;timeline.reset();previous=source_origin=0;motion_call=camera_call=object_call=projectile_call=motion_allowed=camera_allowed=object_allowed=projectile_allowed=false;
    native_input_reset();
    reset_slots(object_clocks);
    reset_slots(slot09_source_dialogues);
    reset_slots(slot09_native_reentries);
    object_command_at_source_start=false;
    door_fade_owner.reset();
    reset_slots(slot09_decision_periods);
    reset_slots(slot09_dialogue_periods);
    reset_slots(slot09_source_handoffs);
    slot09_wait_decision.reset();
    reset_slots(deferred_enemy_births);
    projectile_clocks={};
    reset_slots(object_periodic_poses);
    reset_slots(object_return_turns);
    reset_slots(city_decision_periods);
    reset_slots(city_dialogue_periods);
    reset_slots(city_kaminari_periods);
    reset_slots(city_kaminari_counters);
    reset_slots(city_axis_clocks);
    reset_slots(linked_city_periods);
    reset_slots(linked_city_publications);
    player_source_positions.reset();
    player_action_positions.reset();
    player_flight_clock.reset();
    player_hover_clock.reset();
    player_hover_height.reset();
    player_hover_counter.reset();
    player_hover_threshold.reset();
    player_death_period.reset();
    player_surface_action_period.reset();
    player_drive_clock.reset();
    player_gravity_clock.reset();
    player_height_clock.reset();
    player_translation_clocks={};
    player_cooldown_rearm.reset();
    player_source_contact.reset();
    player_damage_counter.reset();
    player_surface_init.reset();
    player_surface_counter.reset();
    player_idle_period.reset();
    player_wait_damage_counter.reset();
    player_wait_damage_impulse.reset();
    player_turn_clock.reset();
    player_stop_clock.reset();
    zero_shot_before.reset();
    projectile_start_births={};
}
std::optional<Nanoseconds> native_gameplay_preview_render_time() {
    return timeline?std::optional<Nanoseconds>{previous}:std::nullopt;
}
std::optional<double> native_gameplay_preview_source_fraction() {
    if(!timeline)return std::nullopt;
    return double(timeline->phase())/(double(timeline->divider())*double(second));
}
std::optional<SourceTimestamp> native_gameplay_preview_command_time() {
    // Commands belong to the shared world slice, including while Original
    // contact/geometry callbacks suspend continuous motion. In particular,
    // dispatch_native_npc_action suspends simulation before DD8FC may start
    // a fade. Losing this stamp falls back to wall time inside the callback,
    // which is later than the frame being updated and breaks fade handoff.
    // Actor/projectile local age must not replace the world command clock.
    const auto* slice=gameplay_slice?&*gameplay_slice:active_simulation_slice();
    if(!timeline || !slice || !(motion_call || object_call || projectile_call)) {
        // State commands after a resource barrier still belong to the same
        // first callback. Only its movement/model continuation is cancelled.
        if(active_player_epoch && *active_player_epoch!=epoch)return active_player_command_stamp;
        return std::nullopt;
    }
    // Commands in first callbacks belong to the interval start. Preserve
    // the rational Source position, including its sub-nanosecond part.
    const auto whole=slice->tick/60;
    const auto units=(slice->tick%60)*slice->denominator+(object_command_at_source_start?0:slice->phase)+(object_command_at_end?slice->numerator:0);
    const auto partial=units/60;
    const auto available=uint64_t(std::numeric_limits<Nanoseconds>::max()-source_origin);
    if(partial>available || whole>(available-partial)/slice->denominator)
        throw std::overflow_error("Gameplay command time range");
    return SourceTimestamp{source_origin+Nanoseconds(whole*slice->denominator+partial),unsigned(units%60)};
}
std::optional<SimulationSlice> native_gameplay_preview_slice() {
    if(gameplay_slice)return gameplay_slice;
    const auto* slice=active_simulation_slice();
    if(!slice)return std::nullopt;
    return *slice;
}
extern "C" void dora3_native_projectile_scope_end() {
    if(player_projectile_birth)return;
    projectile_scope.reset();projectile_suspension.reset();
}
extern "C" void dora3_native_object_scope_end() {
    object_command_at_end=false;object_command_at_source_start=false;
    object_scope.reset();object_suspension.reset();
}
namespace {
std::array<int32_t,9> flash_motion_fields(uint8_t* rdram,uint32_t actor,uint32_t physical) {
    return {MEM_H(0x34,int32_t(physical)),word(rdram,physical+0x5C),word(rdram,physical+0x58),
        word(rdram,actor+0x18),word(rdram,physical+0x14),word(rdram,physical+0x1C),
        word(rdram,physical+0x44),word(rdram,physical+0xBC),word(rdram,physical+0xC0)};
}
bool flash_motion_site(uint32_t site) {
    switch(site) {
    case 0x801ED5F4:case 0x801ED664:case 0x801ED6B0:case 0x801ED6E0:
    case 0x801ED70C:case 0x801ED72C:case 0x801ED74C:case 0x801ED76C:
    case 0x801ED778:case 0x801ED79C:case 0x801ED7B8:case 0x801ED7F4:
    case 0x801ED814:case 0x801ED840:case 0x801ED85C:case 0x801ED884:return true;
    default:return false;
    }
}
// Native geometry retains the Original period's old-velocity branch and
// float baselines. The probe/retirement decisions still commit at its end.
// Common tail integration publishes the continuous kick-then-drift curve.
int flash_motion_hook(uint8_t* rdram,recomp_context* ctx,uint32_t site) {
    const auto* active=active_simulation_slice();if(!active)return 0;
    const bool axis=site==0x801E8E00||site==0x801E8E10||site==0x801E8E20;
    if(!axis&&!flash_motion_site(site))return 0;
    const auto actor=uint32_t(word(rdram,0x8021BDEC)),physical=uint32_t(word(rdram,0x8021BCDC));
    // E656C has already cleared actor98 and EA4C has replaced model74 by
    // FFFFFFFF at this exact Original end. Close the known period before
    // validating a live actor, without reading either retired allocation.
    if(site==0x801ED884&&actor>=0x804B34B8u&&(actor-0x804B34B8u)%0xCC==0&&pointer(actor,0xCC)&&
       !word(rdram,actor+0x94)&&!word(rdram,actor+0x98)&&uint32_t(word(rdram,actor+0x74))==0xFFFFFFFFu) {
        const auto index=(actor-0x804B34B8u)/0xCC;
        if(index<flash_motion_clocks.size())if(auto& prior=flash_motion_clocks[index];prior&&
           prior->actor==actor&&prior->physical==physical&&prior->epoch==epoch&&
           prior->tick==active->tick&&prior->denominator==active->denominator&&
           prior->expected_phase==active->phase&&active->ends_tick()) {prior.reset();return 0;}
        throw std::logic_error("Unreviewed flash retirement continuation");
    }
    if(axis&&(!pointer(physical,0xF4)||MEM_H(0x20,int32_t(physical))!=130))return 0;
    if(actor<0x804B34B8u||(actor-0x804B34B8u)%0xCC||!pointer(actor,0xCC)||!pointer(physical,0xF4)||
       uint32_t(word(rdram,actor+0x98))!=physical||MEM_H(0x20,int32_t(physical))!=130||
       uint32_t(word(rdram,0x80202064u+130u*84u))!=0x801ED898u||
       uint32_t(word(rdram,0x80202068u+130u*84u))!=0x801ED4A4u)
        throw std::logic_error("Invalid flash motion identity");
    const auto slot=(actor-0x804B34B8u)/0xCC,model=uint32_t(word(rdram,actor+0x74));
    if(slot>=flash_motion_clocks.size())throw std::logic_error("Invalid flash motion slot");
    auto& value=flash_motion_clocks[slot];const auto& slice=*active;
    auto real=[&](uint32_t address){return std::bit_cast<float>(word(rdram,address));};
    auto put=[&](uint32_t address,float x){MEM_W(0,int32_t(address))=std::bit_cast<int32_t>(x);};
    if(site==0x801ED5F4) {flash_motion_initializers[slot]=MEM_B(0x28,int32_t(physical))==0;return 0;}
    if(site==0x801ED884&&!word(rdram,actor+0x94)) {value.reset();return 0;}
    const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model);
    if(!lifetime)throw std::logic_error("Unregistered flash motion model");
    if(site==0x801ED664) {
        const auto fields=flash_motion_fields(rdram,actor,physical);
        if(!value||value->actor!=actor||value->physical!=physical||value->model!=model||
           value->lifetime!=lifetime||value->run!=run||value->epoch!=epoch||
           value->tick!=slice.tick||value->denominator!=slice.denominator||
           value->expected_phase!=slice.phase||value->expected!=fields||
           std::bit_cast<int32_t>(value->expected_y)!=word(rdram,actor+0xA0)) {
            value=FlashMotionClock{actor,physical,model,lifetime,run,epoch,slice.tick,slice.denominator,
                slice.phase,slice.phase,float(MEM_H(0x34,int32_t(physical))),real(physical+0x5C),
                real(physical+0x58),real(actor+0x18),real(physical+0x14),real(physical+0x1C),
                real(actor+0xA0),real(actor+0xA0),flash_motion_initializers[slot],fields};
            for(float x:{value->phase,value->radius,value->alpha,value->scale,value->velocity,value->gravity,value->y})
                if(!std::isfinite(x))throw std::logic_error("Nonfinite flash motion baseline");
        }
        return 0;
    }
    if(!value||value->actor!=actor||value->physical!=physical||value->model!=model||
       value->lifetime!=lifetime||value->run!=run||value->epoch!=epoch||
       value->tick!=slice.tick||value->denominator!=slice.denominator)
        {if(axis)return 0;throw std::logic_error("Stale flash motion period");}
    auto& clock=*value;
    const double age=double(slice.phase+slice.numerator-clock.start_phase)/double(slice.denominator);
    auto phase=[&] {double x=clock.phase+30.*age;if(x>=360)x-=360;else if(x<0)x+=360;return float(x);};
    if(axis) {
        if(site!=0x801E8E10)return 1; // Absolute orbit X/Z keep their Original velocity offsets.
        double displacement;
        if(clock.velocity<0)displacement=-5.*age;
        else {
            const float end=slice.ends_tick()?real(physical+0x14):std::max(-48.f,clock.velocity-clock.gravity);
            displacement=legacy_kinematic_step(clock.velocity,double(end)-clock.velocity,age).displacement;
        }
        ctx->f0.fl=float(double(clock.y)+displacement);ctx->f2.fl=0;
        clock.expected_y=ctx->f0.fl;return 1;
    }
    switch(site) {
    case 0x801ED6B0:ctx->f12.fl=ctx->f0.fl=phase();ctx->f14.fl=30;ctx->f2.fl=360;return 1;
    case 0x801ED6E0:case 0x801ED72C:ctx->f12.d=double(phase());return 0;
    case 0x801ED70C:case 0x801ED74C:
        // A new motion state starts at its home position. Later periods
        // acquire the already advanced radius and its preceding pose.
        ctx->f2.fl=float(clock.first_motion?double(clock.radius)*age:double(clock.radius)+5.*(age-1.));return 0;
    case 0x801ED76C:ctx->f0.fl=float(double(clock.radius)+5.*age);return 1;
    case 0x801ED778:ctx->f4.fl=clock.velocity;return 0;
    case 0x801ED79C:ctx->f0.d=double(clock.alpha)-ctx->f2.d*age;return 1;
    case 0x801ED7B8:return object_call?1:!slice.ends_tick(); // Original Source alone retires model/heap/resources.
    case 0x801ED7F4: {
        // E7F4 receives zero a1/a2/a3/stack10. Probe the real endpoint
        // without committing its candidate block before a late Source command.
        const auto query=pickup_vertical_forecast(rdram,ctx,actor,physical,
            std::bit_cast<int32_t>(clock.y),std::bit_cast<int32_t>(clock.velocity),0.f);
        const float end=slice.ends_tick()?query.velocity:std::max(-48.f,clock.velocity-clock.gravity);
        put(physical+0x14,float(double(clock.velocity)+(double(end)-clock.velocity)*age));
        return 1;
    }
    case 0x801ED814:ctx->f0.d=double(clock.scale)+ctx->f2.d*age;return 1;
    case 0x801ED840:ctx->f4.fl=clock.alpha;return 0;
    case 0x801ED85C:ctx->f0.d=double(clock.alpha)+ctx->f2.d*age;return 1;
    case 0x801ED884:
        clock.expected=flash_motion_fields(rdram,actor,physical);
        clock.expected_phase=slice.phase+slice.numerator;
        clock.expected_y=real(actor+0xA0);return 0;
    default:return 0;
    }
}
}

namespace {
bool pickup_motion_site(uint32_t site) {
    switch(site) {
    case 0x801EC6F4:case 0x801EC704:case 0x801EC72C:case 0x801EC740:
    case 0x801EC764:case 0x801EC79C:case 0x801EC7B8:return true;
    default:return false;
    }
}
std::array<int32_t,5> pickup_motion_fields(uint8_t* rdram,uint32_t actor,uint32_t physical) {
    return {word(rdram,actor+0x10),word(rdram,physical+0x58),word(rdram,physical+0x14),
        word(rdram,physical+0x1C),MEM_BU(0x28,int32_t(physical))};
    // The common pool owns byte2A and changes it around opacity publication.
    // Only EC598's own initialization latch28 belongs to this motion clock.
}
// Collection initializes its six Source effects once. This continuous period
// then keeps the Original NEW-velocity fade branch and its float baselines.
int pickup_motion_hook(uint8_t* rdram,recomp_context* ctx,uint32_t site) {
    const auto* active=active_simulation_slice();if(!active)return 0;
    const bool axis=site==0x801E8E10;
    if(!axis&&!pickup_motion_site(site))return 0;
    const auto actor=uint32_t(word(rdram,0x8021BDEC)),physical=uint32_t(word(rdram,0x8021BCDC));
    if(site==0x801EC7B8&&actor>=0x804B34B8u&&(actor-0x804B34B8u)%0xCC==0&&pointer(actor,0xCC)&&
       !word(rdram,actor+0x94)&&!word(rdram,actor+0x98)&&uint32_t(word(rdram,actor+0x74))==0xFFFFFFFFu) {
        const auto index=(actor-0x804B34B8u)/0xCC;
        if(index<pickup_motion_clocks.size())if(auto& prior=pickup_motion_clocks[index];prior&&
           prior->actor==actor&&prior->physical==physical&&prior->epoch==epoch&&prior->tick==active->tick&&
           prior->denominator==active->denominator&&prior->expected_phase==active->phase&&active->ends_tick()) {
            prior.reset();return 0;
        }
        throw std::logic_error("Unreviewed pickup retirement continuation");
    }
    if(axis&&(!pointer(physical,0xF4)||!standard_pickup_kind(MEM_H(0x20,int32_t(physical)))||word(rdram,actor+0x94)!=10))return 0;
    if(actor<0x804B34B8u||(actor-0x804B34B8u)%0xCC||!pointer(actor,0xCC)||!pointer(physical,0xF4)||
       uint32_t(word(rdram,actor+0x98))!=physical||!standard_pickup_kind(MEM_H(0x20,int32_t(physical)))||
       word(rdram,actor+0x94)!=10||MEM_BU(0x28,int32_t(physical))!=1||!owns_common_object(rdram,actor))
        throw std::logic_error("Invalid collected pickup motion identity");
    const auto slot=(actor-0x804B34B8u)/0xCC,model=uint32_t(word(rdram,actor+0x74));
    if(slot>=pickup_motion_clocks.size())throw std::logic_error("Invalid collected pickup motion slot");
    const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model);
    auto real=[&](uint32_t address){return std::bit_cast<float>(word(rdram,address));};
    auto put=[&](uint32_t address,float x){MEM_W(0,int32_t(address))=std::bit_cast<int32_t>(x);};
    auto& value=pickup_motion_clocks[slot];const auto& slice=*active;
    if(site==0x801EC6F4) {
        const auto fields=pickup_motion_fields(rdram,actor,physical);
        if(!value||value->actor!=actor||value->physical!=physical||value->model!=model||value->lifetime!=lifetime||
           value->run!=run||value->epoch!=epoch||value->tick!=slice.tick||value->denominator!=slice.denominator||
           value->expected_phase!=slice.phase||value->expected!=fields||std::bit_cast<int32_t>(value->expected_y)!=word(rdram,actor+0xA0)) {
            const float velocity=real(physical+0x14),gravity=real(physical+0x1C);
            value=PickupMotionClock{actor,physical,model,MEM_H(0x20,int32_t(physical)),lifetime,run,epoch,
                slice.tick,slice.denominator,slice.phase,slice.phase,real(actor+0x10),real(physical+0x58),
                velocity,gravity,real(actor+0xA0),real(actor+0xA0),std::max(-48.f,velocity-gravity),fields};
            for(float x:{value->yaw,value->alpha,value->velocity,value->gravity,value->y})
                if(!std::isfinite(x))throw std::logic_error("Nonfinite pickup motion baseline");
        }
        // EC740 tests the NEW collision-adjusted velocity. A floor below an
        // externally positioned item can turn a falling command into upward
        // correction; fading from gravity alone would consume opacity before
        // the actual end-of-period query. Re-evaluate the exact Original probe
        // from this period's baseline and roll back stack/candidates/velocity.
        value->end_velocity=pickup_vertical_forecast(rdram,ctx,actor,physical,
            std::bit_cast<int32_t>(value->y),std::bit_cast<int32_t>(value->velocity),0.f).velocity;
        return 0;
    }
    if(!value||value->actor!=actor||value->physical!=physical||value->model!=model||value->lifetime!=lifetime||
       value->run!=run||value->epoch!=epoch||value->tick!=slice.tick||value->denominator!=slice.denominator)
        {if(axis)return 0;throw std::logic_error("Stale pickup motion period");}
    auto& clock=*value;
    const double age=double(slice.phase+slice.numerator-clock.start_phase)/double(slice.denominator);
    if(axis) {
        const float end=std::max(0.f,clock.end_velocity);
        const auto displacement=legacy_kinematic_step(clock.velocity,double(end)-clock.velocity,age).displacement;
        ctx->f0.fl=float(double(clock.y)+displacement);ctx->f2.fl=0;clock.expected_y=ctx->f0.fl;return 1;
    }
    switch(site) {
    case 0x801EC704: {
        double yaw=double(clock.yaw)+20.*age;if(yaw>=360)yaw-=360;else if(yaw<0)yaw+=360;
        ctx->f12.fl=clock.yaw;ctx->f14.fl=20;ctx->f0.fl=float(yaw);return 1;
    }
    case 0x801EC72C:
        if(slice.ends_tick()) {
            const float current_y=real(actor+0xA0);put(actor+0xA0,clock.y);put(physical+0x14,clock.velocity);
            MEM_W(0x10,ctx->r29)=ctx->f20.u32l;
            recomp_context source;dora3::copy_recomp_context(source,*ctx);
            {SimulationSuspension original;dora3_s3_801E810C(rdram,&source);}
            dora3::copy_recomp_context(*ctx,source);put(actor+0xA0,current_y);
            clock.end_velocity=real(physical+0x14);
        } else {
            put(physical+0x14,float(double(clock.velocity)+(double(clock.end_velocity)-clock.velocity)*age));
        }
        return 1;
    case 0x801EC740:ctx->f0.fl=clock.end_velocity;return 0;
    case 0x801EC764:ctx->f0.d=double(clock.alpha)-ctx->f2.d*age;return 1;
    case 0x801EC79C:if(slice.ends_tick()&&object_call)object_command_at_end=true;return !slice.ends_tick();
    case 0x801EC7B8:
        clock.expected=pickup_motion_fields(rdram,actor,physical);clock.expected_phase=slice.phase+slice.numerator;
        clock.expected_y=real(actor+0xA0);return 0;
    default:return 0;
    }
}
}

extern "C" int dora3_native_preview_hook(uint8_t* ram,recomp_context* ctx,uint32_t site) {
    dora3::profile::Scope detailProfile(dora3::profile::Tag::PreviewHookSample,dora3::profile::sample_stage(dora3::profile::Tag::PreviewHookSample,16));
    // Original reaches E8E7C for every pool slot, including inactive/paused
    // entries. Run its complete existing cleanup/publication handlers directly;
    // unrelated action dispatch and repeated eligibility probes do no work for
    // this site. Keep the final live() observation (including the door memo).
    if(site==0x801E8E7C) {
        if(site==0x801E8E7C) {
            const auto actor=uint32_t(word(ram,0x8021BDEC)),slot=(actor-0x804B34B8u)/0xCC;
            native_enemy_motion_end(ram,actor);
            pickup_idle_motion_end(ram,actor,slot,slot09_source_pass,object_call,slot<scene_frame_entered.size()&&scene_frame_entered[slot]);
            flash_source_end(ram,actor,slot,slot09_source_pass,object_call,slot<scene_frame_entered.size()&&scene_frame_entered[slot]);
            pickup_scatter_end(ram,actor,slot,slot09_source_pass,object_call);
            pickup_settle_end(ram,actor,slot,slot09_source_pass,object_call);
            pickup_spin_end(ram,actor,slot,slot09_source_pass,object_call);
            pickup_arc_end(ram,actor,slot,slot09_source_pass,object_call);
            pickup_reveal_end(ram,actor,slot,slot09_source_pass,object_call);
            if(slot<reward_source_periods.size())if(auto& pending=reward_source_periods[slot];pending) {
                if(object_call&&active_simulation_slice()) {
                    const auto& slice=*active_simulation_slice();uint8_t* rdram=ram;
                    // A skipped Source submission ceiling cannot replace the
                    // completed period, nor can a paused Native actor update it.
                    if(scene_frame_entered[slot]&&reward_source_identity(ram,*pending)&&
                       pending->tick==slice.tick&&pending->denominator==slice.denominator) {
                        pending->expected=reward_source_fields(ram,actor,pending->physical);
                        pending->expected_phase=slice.phase+slice.numerator;
                        pending->expected_model_time=word(ram,pending->model+0x64);
                        pending->expected_model_opacity=word(ram,pending->model+0x78);
                        pending->expected_model_render_flag=word(ram,pending->model+0x74);
                        pending->expected_model_flags=MEM_BU(0x7C,int32_t(pending->model));
                        pending->run=game_model_animation().run_token(pending->model);
                        pending->time_command=game_model_animation().time_command_token(pending->model);
                    }
                } else if(!object_call) {
                    // Source C9 pause skips the entry hook. Restore an unchanged
                    // previewed period here, while that pause command stays set.
                    uint8_t* rdram=ram;
                    if(!pending->replayed&&reward_source_identity(ram,*pending)&&
                       MEM_BU(0xC9,int32_t(pending->physical))==1&&pending->expected_phase<=pending->denominator&&
                       pending->source_pass+1==slot09_source_pass)reward_source_restore(ram,*pending);
                    pending.reset();
                }
            }
        }
        if(site==0x801E8E7C&&object_call&&active_simulation_slice()) {
            uint8_t* rdram=ram;
            const auto& slice=*active_simulation_slice();const auto actor=uint32_t(word(ram,0x8021BDEC)),slot=(actor-0x804B34B8u)/0xCC;
            if(gameplay_slice&&slot<scene_frame_entered.size()&&scene_frame_entered[slot]) {
                scene_frame_entered[slot]=false;scene_frame_completed[slot]=scene_capture_completed(ram,actor);
                if(scene_completion_active(ram))scene_source_handoffs[slot]=scene_frame_completed[slot];
            }
            if(slot<slot09_scene_publications.size())if(auto& pending=slot09_scene_publications[slot];pending) {
                const auto v=*pending;
                if(!live(ram)||!owns_common_object(ram,v.actor)||!slot09_scene_identity(ram,v.actor,v.physical,v.model,v.lifetime)||
                   MEM_BU(0,int32_t(0x8021710D))!=0||(uint32_t(word(ram,0x801B3630))&(1u<<(0x5B%32)))!=0||
                   v.tick!=slice.tick||v.denominator!=slice.denominator)pending.reset();
                else if(slice.ends_tick()) {
                    pending.reset();object_command_at_end=true;
                    recomp_context command;dora3::copy_recomp_context(command,*ctx);command.r4=0x5B;
                    dora3_s1_801B4FBC(ram,&command);
                    command.r4=0x175;command.r5=1;dora3_s3_801BA0C4(ram,&command);
                    if(gameplay_slice&&scene_fd_request(ram)) {
                        scene_completion_slice=SceneCompletionSlice{*gameplay_slice,epoch};
                        for(unsigned i=0;i<scene_frame_completed.size();++i)
                            if(scene_frame_completed[i]&&scene_completed_identity(ram,*scene_frame_completed[i]))scene_source_handoffs[i]=scene_frame_completed[i];
                    }
                    slot09_scene_handoffs[slot]=Slot09SceneHandoff{v.actor,v.physical,v.model,v.lifetime,game_model_animation().run_token(v.model),slot09_source_pass+1,
                        slot09_scene_fields(ram,v.actor,v.physical,v.model)};
                }
            }
        }
        (void)live(ram);
        return 0;
    }

    if(site==0x801E8E00||site==0x801E8E10||site==0x801E8E20)
        if(native_enemy_axis(ram,ctx,site))return 1;
    if(site==0x801EBCC0&&object_call&&active_simulation_slice()) {
        uint8_t* rdram=ram;const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,actor+0x98)),slot=(actor-0x804B34B8u)/0xCC;
        if(!owns_common_object(ram,actor)||slot>=pickup_scatter_source_periods.size()||!pickup_scatter_source_periods[slot])throw std::logic_error("Missing Native scatter period");
        auto& period=*pickup_scatter_source_periods[slot];const auto& slice=*active_simulation_slice();period.proposal=pickup_scatter_forecast(ram,ctx,period);
        const double age=double(slice.phase+slice.numerator-period.common.start_phase)/double(slice.denominator);
        recomp_context angle;dora3::copy_recomp_context(angle,*ctx);angle.f12.fl=std::bit_cast<float>(uint32_t(period.initial[0]));angle.f14.fl=float(5.*age);
        dora3_s0_80031A0C(ram,&angle);MEM_W(0x10,int32_t(actor))=angle.f0.u32l;
        const auto addresses=pickup_scatter_addresses(actor,physical);
        for(unsigned i=3;i<addresses.size();++i) {
            const auto first=std::bit_cast<float>(uint32_t(period.initial[i]));const auto last=std::bit_cast<float>(uint32_t(period.proposal[i==3?4:i+2]));
            MEM_W(0,int32_t(addresses[i]))=std::bit_cast<int32_t>(float(double(first)+(double(last)-first)*age));
        }
        return 1; // Source commits collision heading/count/RNG/state transition once.
    }
    if(site==0x801EBAD4&&object_call&&active_simulation_slice()) {
        uint8_t* rdram=ram;const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,actor+0x98)),slot=(actor-0x804B34B8u)/0xCC;
        if(!owns_common_object(ram,actor)||slot>=pickup_settle_source_periods.size()||!pickup_settle_source_periods[slot])
            throw std::logic_error("Missing Native pickup settle Source period");
        auto& period=*pickup_settle_source_periods[slot];const auto& slice=*active_simulation_slice();
        if(period.parent) {
            if(pickup_attachment_parent(ram,actor,physical)!=period.parent)
                throw std::logic_error("Changed Native pickup attachment parent");
            // Initialized attached EBAD4 writes only child XYZ/pitch/yaw and
            // its matrix stack. Run those actual Original matrix operations
            // against the parent's current Native pose; preserve caller/stack.
            // Maximum reviewed attached stack depth is208+72=280 bytes.
            const auto stack=uint32_t(ctx->r29);if(stack<0x80000140u||!pointer(stack-320,352))
                throw std::logic_error("Invalid pickup attachment matrix stack");
            std::array<uint8_t,352> saved{};std::memcpy(saved.data(),ram+((stack-320)&0xFFFFFF),saved.size());
            struct RestoreStack {uint8_t* ram;uint32_t stack;const std::array<uint8_t,352>& saved;
                ~RestoreStack(){std::memcpy(ram+((stack-320)&0xFFFFFF),saved.data(),saved.size());}} restore{ram,stack,saved};
            recomp_context geometry;dora3::copy_recomp_context(geometry,*ctx);
            {SimulationSuspension matrix;dora3_s3_core_801EBAD4(ram,&geometry);}
            return 1; // Source keeps initialization, parent index and detach/settle command.
        }
        period.end_velocity=pickup_settle_forecast(ram,ctx,period);
        const auto initial=std::bit_cast<float>(uint32_t(period.initial_velocity));
        const double age=double(slice.phase+slice.numerator-period.common.start_phase)/double(slice.denominator);
        MEM_W(0,int32_t(physical+0x14))=std::bit_cast<int32_t>(float(double(initial)+(double(period.end_velocity)-initial)*age));
        return 1; // Source retains probe decision, latch and state1 transition.
    }
    if(site==0x801EBEDC&&object_call&&active_simulation_slice()) {
        uint8_t* rdram=ram;
        const auto actor=uint32_t(word(ram,0x8021BDEC)),slot=(actor-0x804B34B8u)/0xCC;
        if(!owns_common_object(ram,actor)||slot>=pickup_reveal_source_periods.size()||!pickup_reveal_source_periods[slot])
            throw std::logic_error("Missing Native pickup reveal Source period");
        const auto& period=*pickup_reveal_source_periods[slot];const auto& slice=*active_simulation_slice();
        const double age=double(slice.phase+slice.numerator-period.start_angle_phase)/double(slice.denominator);
        recomp_context geometry;dora3::copy_recomp_context(geometry,*ctx);
        geometry.f12.fl=float(period.angle);geometry.f14.fl=float(5.*age);dora3_s0_80031A0C(ram,&geometry);
        const double radians=std::bit_cast<double>(uint64_t(LD(int32_t(0x802116A8u),0)));
        geometry.f12.fl=float(double(geometry.f0.fl)*radians);dora3_lib_80057A80(ram,&geometry);
        for(auto address:{actor+0x18,actor+0x1C,actor+0x20})MEM_W(0,int32_t(address))=geometry.f0.u32l;
        return 1; // Actual half-angle/latch and state10 transition retain Source order.
    }
    if(site==0x801EC2A8&&object_call&&active_simulation_slice()) {
        uint8_t* rdram=ram;
        const auto actor=uint32_t(word(ram,0x8021BDEC)),slot=(actor-0x804B34B8u)/0xCC;
        if(!owns_common_object(ram,actor)||slot>=pickup_arc_source_periods.size()||!pickup_arc_source_periods[slot])
            throw std::logic_error("Missing Native pickup arc Source period");
        const auto& period=*pickup_arc_source_periods[slot];const auto& slice=*active_simulation_slice();
        const bool first=period.command[0]==0;
        const double age=double(slice.phase+slice.numerator-period.start_angle_phase)/double(slice.denominator);
        recomp_context geometry;dora3::copy_recomp_context(geometry,*ctx);
        geometry.f12.fl=float(period.command[1]);geometry.f14.fl=float((first?5.:3.)*age);
        dora3_s0_80031A0C(ram,&geometry);
        float angle=geometry.f0.fl;
        if(!first&&angle>=float(period.command[2]))angle=float(period.command[2]);
        const double radians=std::bit_cast<double>(uint64_t(LD(int32_t(first?0x802116C8u:0x802116D0u),0)));
        auto trig=[&](bool second) {
            geometry.f12.fl=float(double(angle)*radians);
            if(second)dora3_lib_80055BD0(ram,&geometry);else dora3_lib_80057A80(ram,&geometry);
            return geometry.f0.fl;
        };
        auto put=[&](uint32_t address,float value){MEM_W(0,int32_t(address))=std::bit_cast<int32_t>(value);};
        const float sine=trig(false);
        if(first) {
            const float anchor=float(period.command[3]);
            const float x_product=sine*anchor;put(actor+0x9C,anchor-x_product);
            put(actor+0xA4,sine*300.f);
            const float y_product=trig(true)*float(int(period.command[4])-50);
            put(actor+0xA0,50.f+y_product);
            const float scale=sine+sine;
            for(auto address:{actor+0x18,actor+0x1C,actor+0x20})put(address,scale);
        } else {
            put(actor+0x9C,sine*300.f);put(actor+0xA4,trig(true)*300.f);
        }
        return 1; // Actual integer angle/phase/latch/return remain Source commands.
    }
    if(site==0x801EC194&&object_call&&active_simulation_slice()) {
        uint8_t* rdram=ram;
        const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,actor+0x98)),slot=(actor-0x804B34B8u)/0xCC;
        if(!owns_common_object(ram,actor)||slot>=pickup_spin_source_periods.size()||!pickup_spin_source_periods[slot])throw std::logic_error("Missing Native pickup spin Source period");
        const auto& period=*pickup_spin_source_periods[slot];const auto model=period.common.model;
        const auto speed=std::bit_cast<float>(uint32_t(period.initial[1]));
        const auto yaw=std::bit_cast<float>(uint32_t(word(ram,actor+0x10)));
        MEM_W(0,int32_t(actor+0x10))=std::bit_cast<int32_t>(game_model_animation().scalar_latched_turn(ram,model,actor+0x10,yaw,speed));
        if(speed<35.f) {
            const auto step=std::bit_cast<double>(uint64_t(LD(int32_t(0x802116B8u),0)));
            const auto current=std::bit_cast<float>(uint32_t(word(ram,physical+0x58)));
            MEM_W(0,int32_t(physical+0x58))=std::bit_cast<int32_t>(game_model_animation().scalar_linear(ram,model,physical+0x58,current,step));
        } else {
            const auto step=std::bit_cast<double>(uint64_t(LD(int32_t(0x802116C0u),0)));
            const auto current=std::bit_cast<float>(uint32_t(word(ram,physical+0x5C)));
            const auto opacity=game_model_animation().scalar_linear(ram,model,physical+0x5C,current,-step);
            MEM_W(0,int32_t(physical+0x5C))=std::bit_cast<int32_t>(opacity);
            auto command=*ctx;command.f_odd=&command.f0.u32h;command.r4=int32_t(actor);command.r5=0;command.r6=std::bit_cast<int32_t>(opacity);
            dora3_s3_801E89CC(ram,&command);
        }
        return 1; // Actual initialization, D8 reset and destructor keep Source order.
    }
    if(site==0x801ED2CC&&object_call&&active_simulation_slice()) {
        const auto actor=uint32_t(word(ram,0x8021BDEC));
        if(!owns_common_object(ram,actor))throw std::logic_error("Unreviewed linked pickup animation owner");
        return 1; // Keep RNG/count/clip/seek/teleports/unlink/retirement in Source.
    }
    if(site==0x801ED8FC&&object_call&&active_simulation_slice()) {
        const auto actor=uint32_t(word(ram,0x8021BDEC));
        if(!owns_common_object(ram,actor))throw std::logic_error("Unreviewed periodic effect private callback");
        return 1; // Actual private init/threshold/destructor keeps Source order.
    }
    if((site==0x801EBFC8||site==0x801EC0D0)&&object_call&&active_simulation_slice()) {
        uint8_t* rdram=ram;
        const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,actor+0x98)),slot=(actor-0x804B34B8u)/0xCC;
        if(!owns_common_object(ram,actor)||slot>=reward_source_periods.size()||!reward_source_periods[slot])
            throw std::logic_error("Invalid Native rising/shrinking pickup period");
        const auto& period=*reward_source_periods[slot];const auto& slice=*active_simulation_slice();
        if(site==0x801EC0D0) {
            const auto scale=std::bit_cast<float>(uint32_t(period.initial[4]));
            const auto decrement=std::bit_cast<double>(uint64_t(LD(int32_t(0x802116B0u),0)));
            const auto value=float(double(scale)-decrement*double(slice.phase+slice.numerator-period.start_scale_phase)/double(slice.denominator));
            for(auto address:{actor+0x18,actor+0x1C,actor+0x20,physical+0x30})MEM_W(0,int32_t(address))=std::bit_cast<int32_t>(value);
        }
        return 1; // Source owns counters, periodic spawns, sound and signal.
    }
    if(site==0x801E8C9C&&!object_call) {
        const auto actor=uint32_t(word(ram,0x8021BDEC)),slot=(actor-0x804B34B8u)/0xCC;
        if(slot<pickup_idle_motion_source_periods.size())if(auto& pending=pickup_idle_motion_source_periods[slot];pending) {
            auto& common=pending->common;
            if(reward_source_identity(ram,common)&&common.expected_phase<=common.denominator&&common.source_pass+1==slot09_source_pass)pickup_idle_motion_restore(ram,*pending);
            else pending.reset();
        }
        if(slot<flash_source_periods.size())if(auto& pending=flash_source_periods[slot];pending) {
            auto& common=pending->common;
            if(reward_source_identity(ram,common)&&common.expected_phase<=common.denominator&&common.source_pass+1==slot09_source_pass)flash_source_restore(ram,*pending);
            else pending.reset();
        }
        if(slot<pickup_scatter_source_periods.size())if(auto& pending=pickup_scatter_source_periods[slot];pending) {
            auto& common=pending->common;
            if(reward_source_identity(ram,common)&&common.expected_phase<=common.denominator&&common.source_pass+1==slot09_source_pass)pickup_scatter_restore(ram,*pending);
            else pending.reset();
        }
        if(slot<pickup_settle_source_periods.size())if(auto& pending=pickup_settle_source_periods[slot];pending) {
            auto& common=pending->common;
            if(reward_source_identity(ram,common)&&common.expected_phase<=common.denominator&&common.source_pass+1==slot09_source_pass)pickup_settle_restore(ram,*pending);
            else pending.reset();
        }
        if(slot<pickup_reveal_source_periods.size())if(auto& pending=pickup_reveal_source_periods[slot];pending) {
            auto& common=pending->common;
            if(reward_source_identity(ram,common)&&common.expected_phase<=common.denominator&&common.source_pass+1==slot09_source_pass)reward_source_restore(ram,common);
            else pending.reset();
        }
        if(slot<pickup_arc_source_periods.size())if(auto& pending=pickup_arc_source_periods[slot];pending) {
            auto& common=pending->common;
            if(reward_source_identity(ram,common)&&common.expected_phase<=common.denominator&&common.source_pass+1==slot09_source_pass)pickup_arc_restore(ram,*pending);
            else pending.reset();
        }
        if(slot<pickup_spin_source_periods.size())if(auto& pending=pickup_spin_source_periods[slot];pending) {
            auto& common=pending->common;
            if(reward_source_identity(ram,common)&&common.expected_phase<=common.denominator&&common.source_pass+1==slot09_source_pass)pickup_spin_restore(ram,*pending);
            else pending.reset();
        }
        if(slot<reward_source_periods.size())if(auto& pending=reward_source_periods[slot];pending) {
            if(reward_source_identity(ram,*pending)&&pending->expected_phase<=pending->denominator&&
               pending->source_pass+1==slot09_source_pass)reward_source_restore(ram,*pending);
            else pending.reset();
        }
    }
    if((site==0x801E8E00||site==0x801E8E10||site==0x801E8E20)&&object_call&&active_simulation_slice()) {
        const auto actor=uint32_t(word(ram,0x8021BDEC)),slot=(actor-0x804B34B8u)/0xCC;
        if(slot<pickup_scatter_source_periods.size()&&pickup_scatter_source_periods[slot]&&word(ram,actor+0x94)==3) {
            const auto& period=*pickup_scatter_source_periods[slot];const auto& slice=*active_simulation_slice();
            if(reward_source_identity(ram,period.common)&&period.common.tick==slice.tick&&period.common.denominator==slice.denominator) {
                const double age=double(slice.phase+slice.numerator-period.common.start_phase)/double(slice.denominator);
                if(site==0x801E8E10) {
                    const auto first=std::bit_cast<float>(uint32_t(period.common.initial[0])),last=std::bit_cast<float>(uint32_t(period.proposal[2]));
                    ctx->f0.fl=float(double(first)+(double(last)-first)*age);
                } else {
                    const bool x=site==0x801E8E00;const auto first=std::bit_cast<float>(uint32_t(period.initial[x?1:2]));
                    const auto velocity=std::bit_cast<float>(uint32_t(period.initial[x?4:6])),last=std::bit_cast<float>(uint32_t(period.proposal[x?6:8]));
                    ctx->f0.fl=float(double(first)+legacy_kinematic_step(velocity,double(last)-velocity,age).displacement);
                }
                ctx->f2.fl=0;return 1;
            }
        }
    }
    if((site==0x801E8E00||site==0x801E8E10||site==0x801E8E20)&&object_call&&active_simulation_slice()) {
        const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,0x8021BCDC)),slot=(actor-0x804B34B8u)/0xCC;
        if(slot<pickup_idle_motion_source_periods.size()&&pickup_idle_motion_source_periods[slot]&&word(ram,actor+0x94)==1) {
            const auto& common=pickup_idle_motion_source_periods[slot]->common;
            if(reward_source_identity(ram,common)) {
                const unsigned axis=site==0x801E8E00?0:site==0x801E8E10?1:2;
                const auto position=actor+0x9C+4*axis,velocity=physical+0x10+4*axis;
                ctx->f0.fl=game_model_animation().scalar_linear(ram,common.model,position,
                    std::bit_cast<float>(uint32_t(word(ram,position))),std::bit_cast<float>(uint32_t(word(ram,velocity))));
                ctx->f2.fl=0;return 1;
            }
        }
    }
    if(site==0x801E8E10&&object_call&&active_simulation_slice()) {
        uint8_t* rdram=ram;
        const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,0x8021BCDC)),slot=(actor-0x804B34B8u)/0xCC;
        if(pointer(physical,0xF4)&&standard_pickup_kind(MEM_H(0x20,int32_t(physical)))&&word(ram,actor+0x94)==2&&owns_common_object(ram,actor)) {
            if(slot>=pickup_settle_source_periods.size()||!pickup_settle_source_periods[slot])throw std::logic_error("Missing Native pickup settle geometry period");
            const auto& period=*pickup_settle_source_periods[slot];const auto& slice=*active_simulation_slice();
            if(period.parent)return 0; // Attached Source requires zero velocities; keep the genuine common XYZ stores.
            const auto initial=std::bit_cast<float>(uint32_t(period.initial_velocity)),height=std::bit_cast<float>(uint32_t(period.common.initial[0]));
            const double age=double(slice.phase+slice.numerator-period.common.start_phase)/double(slice.denominator);
            const auto delta=period.end_velocity==0?0.:legacy_kinematic_step(initial,double(period.end_velocity)-initial,age).displacement;
            ctx->f0.fl=float(double(height)+delta);ctx->f2.fl=0;return 1;
        }
        if(pointer(physical,0xF4)&&reward_sequence_state(ram,actor,physical)&&owns_common_object(ram,actor)) {
            if(slot>=reward_source_periods.size()||!reward_source_periods[slot])throw std::logic_error("Missing periodic effect geometry period");
            const auto& period=*reward_source_periods[slot];const auto& slice=*active_simulation_slice();
            const auto y=std::bit_cast<float>(uint32_t(period.initial[0]));
            ctx->f0.fl=float(double(y)+period.velocity*double(slice.phase+slice.numerator-period.start_phase)/double(slice.denominator));
            ctx->f2.fl=0;return 1;
        }
    }


    if(site==0x801EBAB4&&object_call&&live(ram)&&active_simulation_slice()) {
        uint8_t* rdram=ram;const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,0x8021BCDC));
        if(!pointer(physical,0xF4)||!standard_pickup_kind(MEM_H(0x20,int32_t(physical)))||word(ram,actor+0x94)!=1||!owns_common_object(ram,actor))
            throw std::logic_error("Invalid stationary pickup idle turn");
        const auto model=uint32_t(word(ram,actor+0x74));
        const float yaw=std::bit_cast<float>(word(ram,actor+0x10));
        ctx->f12.fl=yaw;ctx->f14.fl=4;ctx->f2.fl=360;
        ctx->f0.fl=game_model_animation().scalar_latched_turn(ram,model,actor+0x10,yaw,4);
        const auto slot=(actor-0x804B34B8u)/0xCC;
        if(slot<pickup_idle_motion_source_periods.size()&&pickup_idle_motion_source_periods[slot]&&
           reward_source_identity(ram,pickup_idle_motion_source_periods[slot]->common))return 1;
        if(slot>=pickup_idle_source_periods.size()||!pickup_idle_source_periods[slot])
            throw std::logic_error("Missing stationary pickup Source period");
        auto& period=*pickup_idle_source_periods[slot];
        period.expected=pickup_idle_source_fields(ram,actor,physical);
        period.expected[0]=std::bit_cast<int32_t>(ctx->f0.fl);
        period.expected_phase=active_simulation_slice()->phase+active_simulation_slice()->numerator;
        return 1;
    }
    if((site==0x801E8CB0||site==0x801E8CE8||site==0x801E8CD4)&&object_call&&live(ram)&&active_simulation_slice()) {
        uint8_t* rdram=ram;const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,0x8021BCDC));
        if(pointer(physical,0xF4)&&(standard_pickup_kind(MEM_H(0x20,int32_t(physical)))||MEM_H(0x20,int32_t(physical))==130)&&owns_common_object(ram,actor)) {
            // Source-born flash130 uses this same opacity command. Accumulating
            // individually rounded Native increments loses Original endpoints.
            // Guest float rounding can reach opacity1 before this Original
            // period ends. Keep the fade command C8 through that last piece;
            // only the real Source end clears it. Otherwise another piece
            // enters E8D38 and clears publication marker2A one period early.
            if(site==0x801E8CD4)return !active_simulation_slice()->ends_tick();
            const auto model=uint32_t(word(ram,actor+0x74));
            const auto original_delta=std::bit_cast<double>(uint64_t(LD(int32_t(0x802115A8u),0)));
            ctx->f0.d=double(game_model_animation().scalar_linear(ram,model,physical+0xC4,float(ctx->f0.d),
                site==0x801E8CB0?original_delta:-original_delta));
            return 1;
        }
        return 0;
    }
    if(pickup_motion_site(site)||site==0x801E8E10) {
        const int result=pickup_motion_hook(ram,ctx,site);
        if(pickup_motion_site(site)||result)return result;
    }
    if(flash_motion_site(site)||site==0x801E8E00||site==0x801E8E10||site==0x801E8E20) {
        const int result=flash_motion_hook(ram,ctx,site);
        if(flash_motion_site(site)||result)return result;
    }
    if((site==0x801ED520||site==0x801ED5E8)&&active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();uint8_t* rdram=ram;
        const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,0x8021BCDC));
        if(actor<0x804B34B8u||(actor-0x804B34B8u)%0xCC||!pointer(actor,0xCC)||!pointer(physical,0xF4)||
           uint32_t(word(ram,actor+0x98))!=physical||MEM_H(0x20,int32_t(physical))!=130||
           uint32_t(word(ram,0x80202060u+130u*84u+4))!=0x801ED898u||
           uint32_t(word(ram,0x80202060u+130u*84u+8))!=0x801ED4A4u)
            throw std::logic_error("Invalid flash delay actor or callback identity");
        const auto slot=(actor-0x804B34B8u)/0xCC,model=uint32_t(word(ram,actor+0x74));
        const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model);
        if(slot>=flash_delay_commits.size()||!lifetime)throw std::logic_error("Unregistered flash delay model");
        const std::array<int32_t,3> fields{MEM_H(0x34,int32_t(physical)),MEM_H(0x44,int32_t(physical)),word(ram,actor+0x94)};
        auto& previous=flash_delay_commits[slot];
        const FlashDelayCommit observed{actor,physical,model,lifetime,run,epoch,slice.tick,slice.denominator,fields};
        if(site==0x801ED5E8) {if(slice.ends_tick())previous=observed;return 0;}
        if(!slice.ends_tick())return 1;
        if(previous) {const auto& v=*previous;
            if(v.actor==actor&&v.physical==physical&&v.model==model&&v.lifetime==lifetime&&v.run==run&&
               v.epoch==epoch&&v.tick==slice.tick&&v.denominator==slice.denominator&&v.fields==fields)return 1;}
        if(object_call)object_command_at_end=true;
        if(word(ram,actor+0x94)!=1)throw std::logic_error("Flash delay invoked outside state1");
        return 0;
    }
    if(site==0x801E8BDC&&!object_call) {
        ++slot09_source_pass;
        for(auto& handoff:scene_source_handoffs)if(handoff&&(handoff->source_pass<slot09_source_pass||!scene_completed_handoff_valid(ram,*handoff)))handoff.reset();
        for(auto& handoff:slot09_scene_handoffs)if(handoff&&(handoff->source_pass<slot09_source_pass||!slot09_scene_handoff_valid(ram,*handoff)))handoff.reset();
    }
    if(site==0x801E8C9C&&!object_call) {
        const auto actor=uint32_t(word(ram,0x8021BDEC)),slot=(actor-0x804B34B8u)/0xCC;
        // A Source mode3 contact replaces the callback for this boundary:
        // Original executes collection here, not another idle turn. Native
        // has already shown the preceding idle period; restore only its
        // proven callback/opacity baselines before the real Source initializer.
        uint8_t* idle_ram=ram;const auto idle_physical=uint32_t(word(ram,actor+0x98)),idle_model=uint32_t(word(ram,actor+0x74));
        if(slot<pickup_idle_source_periods.size()&&pickup_idle_source_periods[slot]&&pointer(idle_physical,0xF4)&&
           ([&] {uint8_t* rdram=idle_ram;return standard_pickup_kind(MEM_H(0x20,int32_t(idle_physical)))&&MEM_BU(0x28,int32_t(idle_physical))==0;})()&&word(ram,actor+0x94)==10) {
            const auto period=*pickup_idle_source_periods[slot];
            if(period.actor==actor&&period.physical==idle_physical&&period.model==idle_model&&period.epoch==epoch&&
               period.lifetime==game_model_animation().lifetime_token(idle_model)&&period.run==game_model_animation().run_token(idle_model)&&
               period.start_phase==0&&period.expected_phase==period.denominator&&period.source_pass+1==slot09_source_pass&&reward_pool_claim_matches(ram,actor)&&
               period.expected==pickup_idle_source_fields(ram,actor,idle_physical)) {
                uint8_t* rdram=ram;
                MEM_W(0,int32_t(actor+0x10))=period.initial[0];
                MEM_W(0,int32_t(idle_physical+0xC4))=period.initial[1];
                MEM_B(0xC8,int32_t(idle_physical))=int8_t(period.initial[2]);
                MEM_B(0x2A,int32_t(idle_physical))=int8_t(period.initial[3]);
                pickup_idle_source_periods[slot].reset();
            }
        }

        if(slot<scene_source_handoffs.size())if(auto& handoff=scene_source_handoffs[slot];handoff) {
            const bool completed=handoff->source_pass==slot09_source_pass&&scene_completed_handoff_valid(ram,*handoff);
            handoff.reset();if(completed) {slot09_scene_handoffs[slot].reset();return 1;}
        }
        if(slot<slot09_scene_handoffs.size())if(auto& handoff=slot09_scene_handoffs[slot];handoff) {
            const bool completed=handoff->source_pass==slot09_source_pass&&slot09_scene_handoff_valid(ram,*handoff);
            handoff.reset();if(completed)return 1;
        }
    }
    if(site==0x8022F684&&!object_call&&!active_simulation_slice()) {
        const auto actor=uint32_t(word(ram,0x8021BDEC)),slot=(actor-0x804B34B8u)/0xCC;
        if(slot<slot09_scene_publications.size())slot09_scene_publications[slot].reset();
    }
    if(site==0x8022F380&&object_call&&live(ram)&&active_simulation_slice()&&dora3_overlay_loaded(0x001398A0u)) {
        const auto& slice=*active_simulation_slice();const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,actor+0x98)),model=uint32_t(word(ram,actor+0x74)),slot=(actor-0x804B34B8u)/0xCC;
        const auto lifetime=game_model_animation().lifetime_token(model);
        if(slot>=slot09_scene_publications.size()||!slot09_scene_identity(ram,actor,physical,model,lifetime))throw std::logic_error("Invalid sceneFD publication owner");
        slot09_scene_publications[slot]=Slot09ScenePublication{actor,physical,model,lifetime,slice.tick,slice.denominator};
        return 1; // Common actor tail must finish before eligibility changes.
    }


    if(site==0x8022FE60&&!object_call&&!active_simulation_slice()) {
        const auto actor=uint32_t(word(ram,0x8021BDEC)),slot=(actor-0x804B34B8u)/0xCC;
        if(slot<slot09_special_turns.size())cancel_slot09_special_turn(ram,slot);
    }

    if(site==0x8022DFB4&&!object_call&&!active_simulation_slice()) {
        const auto actor=uint32_t(word(ram,0x8021BDEC)),slot=(actor-0x804B34B8u)/0xCC;
        if(slot<slot09_story_starts.size())cancel_slot09_story_start(ram,slot);
    }

    if((site==0x801C6FCC || site==0x801C7160 || site==0x801C7338 ||
        site==0x801C6FC0 || site==0x801C7154 || site==0x801C732C || site==0x801C7494 ||
        site==0x801C7528 || site==0x801C75BC || site==0x801C7648) &&
       motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74));
        const PlayerDeathPeriod value{actor,physical,model,game_model_animation().lifetime_token(model),
            game_model_animation().run_token(model),slice.tick,slice.denominator,word(ram,actor+0x94)};
        if(site!=0x801C6FCC && site!=0x801C7160 && site!=0x801C7338) {
            // Original calls the old first and then the new second body.
            // Its newly constructed death's first timer starts next period.
            player_death_period=value;return 0;
        }
        if(player_death_period) {
            const auto& old=*player_death_period;
            if(old.actor==value.actor && old.physical==value.physical && old.model==value.model && old.lifetime==value.lifetime &&
               old.run==value.run && old.tick==value.tick && old.denominator==value.denominator && old.state==value.state)return 1;
        }
        // This closure owns only Source-counted state/fade/scene commands;
        // animation and the actual second body/collision remain Native.
        player_death_period=value;return 0;
    }
    if((site==0x801C2FC8 || site==0x801C7CB4 || site==0x801C7DE4 || site==0x801C7F74 ||
        site==0x801C7C6C || site==0x801C7D9C || site==0x801C7F2C || site==0x801C7AB8 || site==0x801C7AAC) &&
       motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74));
        const PlayerDeathPeriod value{actor,physical,model,game_model_animation().lifetime_token(model),
            game_model_animation().run_token(model),slice.tick,slice.denominator,word(ram,actor+0x94)};
        const bool constructor=site==0x801C7C6C || site==0x801C7D9C || site==0x801C7F2C || site==0x801C7AAC;
        if(constructor){player_surface_action_period=value;return 0;}
        // The actual F4 producer re-enters61 unconditionally even while it
        // already owns61: clip, counter and air reset once per Source period.
        // Preserve that restart without resetting the clip each Native dt.
        if(site==0x801C2FC8 && value.state!=61)return 0;
        if(player_surface_action_period) {
            const auto& old=*player_surface_action_period;
            if(old.actor==value.actor && old.physical==value.physical && old.model==value.model && old.lifetime==value.lifetime &&
               old.run==value.run && old.tick==value.tick && old.denominator==value.denominator && old.state==value.state)return 1;
        }
        if(site==0x801C2FC8)return 0; // The real ctor records its new clip run.
        // Common Native damage/input/weapon callbacks run before this gate.
        // Only the surface exit/Source incoming vertical condition is latched;
        // Native second bodies, impulses, spin and collision still run each dt.
        player_surface_action_period=value;return 0;
    }
    if(site==0x801C6E98 && motion_call && active_simulation_slice()) {player_hover_counter.reset();player_hover_threshold.reset();}
    if(site==0x801C6EDC && motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74));const auto count=int16_t(ctx->r2);
        const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model);
        if(player_hover_threshold) {
            const auto& old=player_hover_threshold->owner;
            if(old.actor==actor && old.physical==physical && old.model==model && old.lifetime==lifetime && old.run==run &&
               old.tick==slice.tick && old.denominator==slice.denominator &&
               (uint16_t(count)==old.expected || count==player_hover_threshold->initial)) {
                // The first callback sees the incoming count throughout its
                // Source period, not the increment of its own second body.
                ctx->r2=player_hover_threshold->initial;return 0;
            }
        }
        player_hover_threshold=PlayerHoverThreshold{{actor,physical,model,lifetime,run,slice.tick,slice.denominator,uint16_t(count+1)},count};return 0;
    }
    if(site==0x801C6F38 && motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();uint8_t* rdram=ram;
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74));const auto count=MEM_HU(0x44,int32_t(physical));
        const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model);
        if(player_hover_counter) {
            const auto& old=*player_hover_counter;
            if(old.actor==actor && old.physical==physical && old.model==model && old.lifetime==lifetime && old.run==run &&
               old.tick==slice.tick && old.denominator==slice.denominator && old.expected==count) {
                // The skipped load supplies a0 to both zero-angle stores.
                ctx->r4=int32_t(actor);return 1;
            }
        }
        player_hover_counter=PlayerActionCounter{actor,physical,model,lifetime,run,slice.tick,slice.denominator,uint16_t(count+1)};
        return 0;
    }
    if((site==0x801C8118 || site==0x801C0EA8) && motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();uint8_t* rdram=ram;
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto control=uint32_t(word(ram,0x8021D6C0));
        if(word(ram,actor+0x94)!=63 || MEM_BU(0x2C,int32_t(control))!=1)return 0;
        auto read=[&](uint32_t p){return std::bit_cast<float>(word(ram,p));};
        auto write=[&](uint32_t p,float f){MEM_W(0,int32_t(p))=std::bit_cast<int32_t>(f);};
        if(site==0x801C8118) {
            const auto model=uint32_t(word(ram,actor+0x74)),params=uint32_t(word(ram,0x8021D648));
            const std::array<int32_t,7> command{MEM_BU(2,int32_t(control)),MEM_HU(0,int32_t(0x8021D644))&0x8000,
                word(ram,0x8021D4C4),MEM_H(0xC,int32_t(params)),MEM_BU(0x2C,int32_t(control)),0,0};
            const auto observation=flight_observation(ram,actor,physical);
            if(!player_hover_clock || !same_flight_owner(ram,player_hover_clock->motion) ||
               player_hover_clock->motion.tick!=slice.tick || player_hover_clock->motion.denominator!=slice.denominator ||
               player_hover_clock->motion.expected_phase!=slice.phase || player_hover_clock->motion.expected!=observation ||
               player_hover_clock->motion.command!=command || player_hover_clock->expected_vertical_speed!=word(ram,physical+8)) {
                PlayerHoverClock hover{};auto& clock=hover.motion;
                clock.actor=actor;clock.physical=physical;clock.model=model;
                clock.lifetime=game_model_animation().lifetime_token(model);clock.run=game_model_animation().run_token(model);
                if(!clock.lifetime || !clock.run)throw std::logic_error("Unregistered hover model");
                clock.tick=slice.tick;clock.denominator=slice.denominator;clock.start_phase=slice.phase;
                clock.expected_phase=slice.phase;clock.expected=observation;clock.command=command;
                const auto fields=flight_fields(actor,physical);const auto vertical_speed=word(ram,physical+8);
                for(unsigned k=0;k<6;++k)clock.initial[k]=read(fields[k]);
                recomp_context source;dora3::copy_recomp_context(source,*ctx);
                // Actual Source geometry only: no input polling, allocation,
                // RNG, audio or state commands. Restore every touched field.
                {SimulationSuspension geometry;
                 dora3_s3_801C0E3C(ram,&source);
                 for(unsigned k=0;k<3;++k)clock.velocity[k]=read(physical+0x10+4*k);
                 dora3_s3_core_801C8118(ram,&source);
                 for(unsigned k=0;k<6;++k)clock.target[k]=read(fields[k]);
                 hover.expected_vertical_speed=word(ram,physical+8);
                 // Original's clipped scalar kick drifts immediately at the
                 // target. Preserve the previous yaw for the paired bend.
                 const float target=command[0]?float(command[3]):0.f;
                 const float raw=clock.initial[0]<=target?clock.initial[0]+6.f:clock.initial[0]-6.f;
                 if(clock.target[0]!=raw) {
                     write(physical,clock.target[0]);write(physical+4,clock.initial[1]);
                     dora3_s3_801C0E3C(ram,&source);
                     for(unsigned k=0;k<3;++k)clock.velocity[k]=read(physical+0x10+4*k);
                     write(physical+4,clock.target[1]);
                 }
                 dora3_s3_801C0E3C(ram,&source);dora3_s3_801C0EC0(ram,&source);
                 for(unsigned k=0;k<3;++k)clock.change[k]=read(physical+0x10+4*k)-clock.velocity[k];
                 for(unsigned k=0;k<fields.size();++k)MEM_W(0,int32_t(fields[k]))=observation[k];
                 MEM_W(0,int32_t(physical+8))=vertical_speed;}
                player_hover_clock=hover;
            }
            auto& hover=*player_hover_clock;auto& clock=hover.motion;clock.open=true;
            const double age=double(slice.phase+slice.numerator-clock.start_phase)/slice.denominator;
            const auto fields=flight_fields(actor,physical);
            for(unsigned k=0;k<6;++k) {
                // Mode90/270 and zero pitch/roll are instantaneous commands;
                // only scalar speed and yaw advance continuously.
                if(k==2 || k==3 || k==5) {write(fields[k],clock.target[k]);continue;}
                const double delta=k?std::remainder(double(clock.target[k])-clock.initial[k],360.):double(clock.target[k])-clock.initial[k];
                double value=clock.initial[k]+delta*age;
                if(k && delta!=0.) {value=std::fmod(value,360.);if(value<0)value+=360.;}
                write(fields[k],age==1.?clock.target[k]:float(value));
            }
            MEM_W(0,int32_t(physical+8))=hover.expected_vertical_speed;return 1;
        }
        if(player_hover_clock && player_hover_clock->motion.open && same_flight_owner(ram,player_hover_clock->motion)) {
            const auto& clock=player_hover_clock->motion;
            const double a=double(slice.phase-clock.start_phase)/slice.denominator;
            const double b=double(slice.phase+slice.numerator-clock.start_phase)/slice.denominator;
            for(unsigned k:{0u,2u}) {
                const float velocity=float(clock.velocity[k]+clock.change[k]*b);
                const double distance=clock.velocity[k]*(b-a)+0.5*clock.change[k]*(b*b+b-a*a-a);
                write(physical+0x10+4*k,velocity);publish_simulation_motion(physical+0x10+4*k,{velocity,distance});
            }
        }
        return 0;
    }
    if(site==0x801C8E58 && motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto control=uint32_t(word(ram,0x8021D6C0)),model=uint32_t(word(ram,actor+0x74));
        const PlayerControlPeriod value{actor,physical,control,model,game_model_animation().lifetime_token(model),slice.tick,slice.denominator};
        if(player_idle_period) {
            const auto& old=*player_idle_period;
            // Its own clip/state change does not repeat the pre-command
            // idle increment/reset. A new actor/control lifetime still does.
            if(old.actor==value.actor && old.physical==value.physical && old.control==value.control &&
               old.model==value.model && old.lifetime==value.lifetime && old.tick==value.tick && old.denominator==value.denominator)
                return 1;
        }
        player_idle_period=value;return 0;
    }
    if((site==0x801C8550 || site==0x801C7D04 || site==0x801C7FB0) && motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74));
        const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model);
        uint8_t* rdram=ram;
        const bool spin=site!=0x801C8550;
        const std::array<int32_t,2> command=spin?std::array<int32_t,2>{word(ram,actor+0x94),int32_t(site)}:
            std::array<int32_t,2>{MEM_H(0xD4,int32_t(physical)),word(ram,physical+0xEC)};
        // Both spin JALs load f12 in their delay slots, which the guarded
        // replacement skips together with the helper. Read its actual operand.
        const float current=spin?std::bit_cast<float>(word(ram,actor+0x10)):ctx->f12.fl,delta=spin?30.f:float(int32_t(ctx->r3));
        if(!std::isfinite(current)||(!spin&&!std::isfinite(std::bit_cast<float>(command[1])))||std::abs(delta)>30)
            throw std::invalid_argument("Invalid scripted turn command");
        if(!player_turn_clock || player_turn_clock->actor!=actor || player_turn_clock->physical!=physical ||
           player_turn_clock->model!=model || player_turn_clock->lifetime!=lifetime || player_turn_clock->run!=run ||
           player_turn_clock->tick!=slice.tick || player_turn_clock->denominator!=slice.denominator ||
           player_turn_clock->expected_phase!=slice.phase || player_turn_clock->command!=command ||
           std::bit_cast<int32_t>(player_turn_clock->expected)!=std::bit_cast<int32_t>(current))
            player_turn_clock=PlayerTurnClock{actor,physical,model,lifetime,run,slice.tick,slice.denominator,
                slice.phase,slice.phase,command,current,delta,current};
        auto& clock=*player_turn_clock;
        const double elapsed=double(slice.phase+slice.numerator-clock.start_phase)/slice.denominator;
        float next=clock.initial+float(double(clock.delta)*elapsed);
        if(next>=360.f)next-=360.f;else if(next<0)next+=360.f;
        // Match the Original helper's endpoint and register results; its
        // bang-bang sign is retained until the next Source command period.
        ctx->f12.fl=ctx->f0.fl=next;ctx->f2.fl=360.f;ctx->f14.fl=clock.delta;
        clock.expected=next;clock.expected_phase=slice.phase+slice.numerator;return 1;
    }
    if((site==0x801C8574 || site==0x801C8774) && motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74));
        const PlayerStopClock value{actor,physical,model,game_model_animation().lifetime_token(model),
            game_model_animation().run_token(model),slice.tick,slice.denominator,word(ram,actor+0x94),
            site==0x801C8574?0:int32_t(ctx->f0.u32l),word(ram,physical)};
        if(player_stop_clock) {
            const auto& old=*player_stop_clock;
            if(old.actor==value.actor && old.physical==value.physical && old.model==value.model &&
               old.lifetime==value.lifetime && old.run==value.run && old.tick==value.tick && old.denominator==value.denominator &&
               old.state==value.state && old.command==value.command && old.expected==value.expected)return 1;
        }
        player_stop_clock=value;return 0;
    }
    if((site==0x801C85BC || site==0x801C85D8 || site==0x801C86DC) &&
       motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74));
        const PlayerDamageCounter value{actor,physical,model,game_model_animation().lifetime_token(model),
            game_model_animation().run_token(model),slice.tick,slice.denominator};
        if(site==0x801C85BC) {
            // The old first callback enters67; only its new second body
            // runs now. Setup and the recovery count start next period.
            player_surface_init=player_surface_counter=value;return 0;
        }
        auto& guard=site==0x801C85D8?player_surface_init:player_surface_counter;
        if(guard) {
            const auto& old=*guard;
            if(old.actor==value.actor && old.physical==value.physical && old.model==value.model &&
               old.lifetime==value.lifetime && old.run==value.run && old.tick==value.tick && old.denominator==value.denominator)
                return 1;
        }
        guard=value;return 0;
    }
    if(site==0x801C6A9C && motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74));
        // Original enters46 during the old first callback, then runs46's
        // second body. Its first decrement starts next Source period.
        player_wait_damage_counter=PlayerDamageCounter{actor,physical,model,
            game_model_animation().lifetime_token(model),game_model_animation().run_token(model),slice.tick,slice.denominator};
        return 0;
    }
    if(site==0x801C6BCC && motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74));
        // The kick's own unpause must not reopen either command phase.
        // A subsequent external bind/run change still invalidates both.
        for(auto* guard:{&player_wait_damage_counter,&player_wait_damage_impulse})if(*guard) {
            auto& clock=**guard;
            if(clock.actor==actor && clock.physical==physical && clock.model==model &&
               clock.lifetime==game_model_animation().lifetime_token(model) && clock.tick==slice.tick && clock.denominator==slice.denominator)
                clock.run=game_model_animation().run_token(model);
        }
        return 0;
    }
    if((site==0x801C6AC8 || site==0x801C6B20) && motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74));
        const PlayerDamageCounter value{actor,physical,model,game_model_animation().lifetime_token(model),
            game_model_animation().run_token(model),slice.tick,slice.denominator};
        auto& guard=site==0x801C6AC8?player_wait_damage_counter:player_wait_damage_impulse;
        if(guard) {
            const auto& old=*guard;
            if(old.actor==value.actor && old.physical==value.physical && old.model==value.model &&
               old.lifetime==value.lifetime && old.run==value.run && old.tick==value.tick && old.denominator==value.denominator)
                return 1;
        }
        guard=value;return 0;
    }
    if(site==0x801C6500 && motion_call && live(ram) && active_simulation_slice() && player_damage_counter) {
        auto& clock=*player_damage_counter;const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74));
        if(clock.actor==actor && clock.physical==physical && clock.model==model &&
           clock.lifetime==game_model_animation().lifetime_token(model) && clock.tick==slice.tick && clock.denominator==slice.denominator)
            clock.run=game_model_animation().run_token(model);
        return 0;
    }
    if((site==0x801C6094 || site==0x801C6418 || site==0x801C67D8) && motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74));
        const PlayerDamageCounter value{actor,physical,model,game_model_animation().lifetime_token(model),
            game_model_animation().run_token(model),slice.tick,slice.denominator};
        if(player_damage_counter) {
            const auto& old=*player_damage_counter;
            if(old.actor==value.actor && old.physical==value.physical && old.model==value.model &&
               old.lifetime==value.lifetime && old.run==value.run && old.tick==value.tick && old.denominator==value.denominator)
                {uint8_t* rdram=ram;return site==0x801C6418 && MEM_H(0x44,int32_t(physical))>16?2:1;}
        }
        player_damage_counter=value;return 0;
    }
    if(site==0x801C0FA8 && motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        if(uint32_t(ctx->r2)!=physical)return 0;
        const auto model=uint32_t(word(ram,actor+0x74));
        const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model);
        const float velocity=ctx->f0.fl,gravity=ctx->f2.fl;
        if(!std::isfinite(velocity)||!std::isfinite(gravity))throw std::invalid_argument("Nonfinite player gravity");
        const auto command=std::bit_cast<int32_t>(gravity);
        if(!player_gravity_clock || player_gravity_clock->actor!=actor || player_gravity_clock->physical!=physical ||
           player_gravity_clock->model!=model || player_gravity_clock->lifetime!=lifetime || player_gravity_clock->run!=run ||
           player_gravity_clock->tick!=slice.tick || player_gravity_clock->denominator!=slice.denominator ||
           player_gravity_clock->expected_phase!=slice.phase || player_gravity_clock->gravity!=command ||
           std::bit_cast<int32_t>(player_gravity_clock->expected)!=std::bit_cast<int32_t>(velocity)) {
            const float raw=velocity-gravity,target=std::max(-55.f,raw);
            player_gravity_clock=PlayerGravityClock{actor,physical,model,lifetime,run,slice.tick,slice.denominator,
                slice.phase,slice.phase,command,velocity,target,velocity,target!=raw};
        }
        auto& clock=*player_gravity_clock;
        const double a=double(slice.phase-clock.start_phase)/slice.denominator;
        const double b=double(slice.phase+slice.numerator-clock.start_phase)/slice.denominator;
        const float next=b==1?clock.target:float(std::max(-55.,double(clock.initial)-double(gravity)*b));
        // Compute from the guarded Source baseline, never from a rounded
        // previous substep. A clipped Source kick drifts at its endpoint,
        // just as Original's terminal clamp does for the complete update.
        const double distance=clock.clamped?double(clock.target)*(b-a):
            double(clock.initial)*(b-a)-0.5*double(gravity)*(b*b+b-a*a-a);
        ctx->f0.fl=next;publish_simulation_motion(physical+0x14,{next,distance});
        clock.expected=next;clock.expected_phase=slice.phase+slice.numerator;
        return 1;
    }
    if(site==0x801BF8A8 && motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74));
        const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model);
        const auto control=uint32_t(word(ram,0x8021D6C0));uint8_t* rdram=ram;
        if((word(ram,actor+0x94)==49 || word(ram,actor+0x94)==63 || word(ram,actor+0x94)==56) && MEM_BU(0x2C,int32_t(control))==1) {
            const auto velocity=word(ram,physical+0x14);const float position=ctx->f24.fl;
            if(!std::isfinite(position)||!std::isfinite(std::bit_cast<float>(velocity)))throw std::invalid_argument("Nonfinite hover height");
            if(!player_hover_height || player_hover_height->actor!=actor || player_hover_height->physical!=physical ||
               player_hover_height->model!=model || player_hover_height->lifetime!=lifetime || player_hover_height->run!=run ||
               player_hover_height->tick!=slice.tick || player_hover_height->denominator!=slice.denominator ||
               player_hover_height->expected_phase!=slice.phase || player_hover_height->velocity!=velocity ||
               std::bit_cast<int32_t>(player_hover_height->expected)!=std::bit_cast<int32_t>(position))
                player_hover_height=PlayerHoverHeightClock{actor,physical,model,lifetime,run,slice.tick,slice.denominator,slice.phase,slice.phase,velocity,position,position};
            auto& clock=*player_hover_height;
            const double age=double(slice.phase+slice.numerator-clock.start_phase)/slice.denominator;
            const float next=clock.initial+float(double(std::bit_cast<float>(velocity))*age);
            ctx->f24.fl=next;ctx->f0.fl=0;clock.expected=next;clock.expected_phase=slice.phase+slice.numerator;return 0;
        }
        player_hover_height.reset();
        if(!player_gravity_clock) {player_height_clock.reset();return 0;}
        const auto& gravity=*player_gravity_clock;
        if(gravity.actor!=actor || gravity.physical!=physical || gravity.model!=model || gravity.lifetime!=lifetime || gravity.run!=run ||
           gravity.tick!=slice.tick || gravity.denominator!=slice.denominator || gravity.expected_phase!=slice.phase+slice.numerator ||
           std::bit_cast<int32_t>(gravity.expected)!=word(ram,physical+0x14)) {player_height_clock.reset();return 0;}
        const float position=ctx->f24.fl;
        if(!std::isfinite(position))throw std::invalid_argument("Nonfinite player height");
        if(!player_height_clock || player_height_clock->actor!=actor || player_height_clock->physical!=physical ||
           player_height_clock->model!=model || player_height_clock->lifetime!=lifetime || player_height_clock->run!=run ||
           player_height_clock->tick!=slice.tick || player_height_clock->denominator!=slice.denominator ||
           player_height_clock->expected_phase!=slice.phase || player_height_clock->gravity_start!=gravity.start_phase ||
           player_height_clock->initial_velocity!=std::bit_cast<int32_t>(gravity.initial) || player_height_clock->gravity!=gravity.gravity ||
           std::bit_cast<int32_t>(player_height_clock->expected)!=std::bit_cast<int32_t>(position)) {
            // Real collision corrections, external position/velocity writes,
            // and new model runs rebase before the next solver query.
            player_height_clock=PlayerHeightClock{actor,physical,model,lifetime,run,slice.tick,slice.denominator,
                slice.phase,slice.phase,gravity.start_phase,std::bit_cast<int32_t>(gravity.initial),gravity.gravity,position,position};
        }
        auto& clock=*player_height_clock;
        const double a=double(clock.start_phase-gravity.start_phase)/slice.denominator;
        const double b=double(slice.phase+slice.numerator-gravity.start_phase)/slice.denominator;
        const double distance=gravity.clamped?double(gravity.target)*(b-a):
            double(gravity.initial)*(b-a)-0.5*double(std::bit_cast<float>(gravity.gravity))*(b*b+b-a*a-a);
        // Original rounds the full displacement and coordinate add once.
        // Re-evaluate from that baseline, instead of accumulating rounded
        // native deltas. Preserve the exact zero which defines floor contact.
        const float next=clock.initial+float(distance);
        ctx->f24.fl=next;ctx->f0.fl=0;
        clock.expected=next;clock.expected_phase=slice.phase+slice.numerator;
        return 0;
    }
    if(site==0x801BFE18 && motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74));
        for(unsigned axis=0;axis<2;++axis)if(player_translation_clocks[axis]) {
            auto& clock=*player_translation_clocks[axis];
            if(clock.actor==actor && clock.physical==physical && clock.model==model &&
               clock.lifetime==game_model_animation().lifetime_token(model) && clock.run==game_model_animation().run_token(model) &&
               clock.tick==slice.tick && clock.denominator==slice.denominator && clock.expected_phase==slice.phase+slice.numerator) {
                const float actual=std::bit_cast<float>(word(ram,actor+(axis?0xA4:0x9C)));
                clock.collision=std::bit_cast<int32_t>(actual)!=std::bit_cast<int32_t>(clock.expected);
                clock.expected=actual;
            }
        }
        return 0;
    }
    if((site==0x801BFA18 || site==0x801BFA24) && motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74));
        const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model);
        const unsigned axis=site==0x801BFA18?1:0;
        const float position=axis?ctx->f22.fl:ctx->f26.fl,delta=ctx->f4.fl;
        if(!std::isfinite(position)||!std::isfinite(delta))throw std::invalid_argument("Nonfinite player translation");
        const std::array<int32_t,2> velocity{word(ram,physical+0x10),word(ram,physical+0x18)};
        auto& sample=player_translation_clocks[axis];
        if(!sample || sample->actor!=actor || sample->physical!=physical || sample->model!=model ||
           sample->lifetime!=lifetime || sample->run!=run || sample->tick!=slice.tick || sample->denominator!=slice.denominator ||
           sample->expected_phase!=slice.phase || std::bit_cast<int32_t>(sample->expected)!=std::bit_cast<int32_t>(position) ||
           (sample->collision && sample->velocity!=velocity)) {
            // External position writes or a velocity command after contact
            // rebase immediately. For unchanged velocity retain the Source
            // candidate across the solver's integer pushouts; querying that
            // candidate at each native time avoids accumulating its rounding.
            sample=PlayerTranslationClock{actor,physical,model,lifetime,run,slice.tick,slice.denominator,slice.phase,double(position),position,velocity,false};
        }
        auto& clock=*sample;clock.position+=double(delta);
        const float next=float(clock.position);
        ctx->f4.fl=next-position;
        clock.expected=next;clock.expected_phase=slice.phase+slice.numerator;clock.velocity=velocity;clock.collision=false;
        return 0;
    }
    if(site==0x801C1754 && motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74)),control=uint32_t(word(ram,0x8021D6C0));
        const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model);
        const float velocity=ctx->f0.fl,target=ctx->f4.fl,amount=ctx->f2.fl;
        const float slope=std::bit_cast<float>(word(ram,control+0x24));uint8_t* rdram=ram;
        const float maximum=float(MEM_H(8,int32_t(word(ram,0x8021D648))));
        const std::array<int32_t,4> command{std::bit_cast<int32_t>(target),std::bit_cast<int32_t>(amount),
            std::bit_cast<int32_t>(slope),std::bit_cast<int32_t>(maximum)};
        if(!player_drive_clock || player_drive_clock->actor!=actor || player_drive_clock->physical!=physical ||
           player_drive_clock->model!=model || player_drive_clock->lifetime!=lifetime || player_drive_clock->run!=run ||
           player_drive_clock->tick!=slice.tick || player_drive_clock->denominator!=slice.denominator ||
           player_drive_clock->expected_phase!=slice.phase || player_drive_clock->command!=command ||
           std::bit_cast<int32_t>(player_drive_clock->expected)!=std::bit_cast<int32_t>(velocity)) {
            // Use the reviewed Original single-step arithmetic and clamps.
            // The endpoint is shared by every native subdivision of this
            // drive phase; a collision, command or resource change rebases it.
            const float raw=float((velocity<=target?float(velocity+amount):float(velocity-amount))+slope);
            float next=velocity<=target?(raw>=target?target:raw):(raw<=target?target:raw);
            next=std::clamp(next,-30.f,maximum);
            player_drive_clock=PlayerDriveClock{actor,physical,model,lifetime,run,slice.tick,slice.denominator,
                slice.phase,slice.phase,command,velocity,next,velocity,next!=raw};
        }
        auto& clock=*player_drive_clock;
        const double a=double(slice.phase-clock.start_phase)/slice.denominator;
        const double b=double(slice.phase+slice.numerator-clock.start_phase)/slice.denominator;
        const double change=double(clock.target)-clock.initial;
        const float next=b==1?clock.target:float(clock.initial+change*b);
        // A clipped kick drifts at its clipped endpoint. This preserves
        // Original's immediate stop and never creates a backwards correction.
        // Unclipped acceleration retains the continuous kick/drift extension.
        const double distance=clock.clamped?double(clock.target)*(b-a):
            clock.initial*(b-a)+0.5*change*(b*b+b-a*a-a);
        ctx->f0.fl=next;publish_simulation_drive(physical,{next,distance});
        clock.expected=next;clock.expected_phase=slice.phase+slice.numerator;
        return 1;
    }
    if((site==0x801C5DB0 || site==0x801C6064 || site==0x801C5A58 || site==0x801C63D8 || site==0x801C67A8 || site==0x801C6AE8 || site==0x801C5950 || site==0x801C38D8) && motion_call && live(ram) && active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74));
        const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model);
        const auto state=word(ram,actor+0x94);
        if(!player_source_contact || player_source_contact->actor!=actor || player_source_contact->physical!=physical ||
           player_source_contact->model!=model || player_source_contact->lifetime!=lifetime || player_source_contact->run!=run ||
           player_source_contact->tick!=slice.tick || player_source_contact->denominator!=slice.denominator || player_source_contact->state!=state) {
            // C0C7C has just probed this action's current position. Preserve
            // that Source command observation; the previous interval's
            // pre-movement flags can disagree with its completed collision.
            player_source_contact=PlayerSourceContact{actor,physical,model,lifetime,run,slice.tick,slice.denominator,state,int32_t(ctx->r2)};
        } else ctx->r2=player_source_contact->flags;
        return 0;
    }
    if((site==0x801C6E00 || site==0x801C5F7C || site==0x801C62C8 || site==0x801C5D04 || site==0x801C66B0 || site==0x801C6A0C || site==0x801C6CF4 || site==0x801C6F24 || site==0x801C8D18) && motion_call && live(ram)) {
        const auto* slice=active_simulation_slice();
        if(!slice)return 0;
        const auto actor=uint32_t(word(ram,0x8021D49C)),physical=uint32_t(word(ram,0x8021D4B0));
        const auto model=uint32_t(word(ram,actor+0x74));uint8_t* rdram=ram;
        if(((site==0x801C6E00 && word(ram,actor+0x94)==57) || (site==0x801C6F24 && word(ram,actor+0x94)==63) ||
            ((site==0x801C5F7C || site==0x801C62C8 || site==0x801C5D04 || site==0x801C66B0 || site==0x801C6A0C || site==0x801C6CF4) && word(ram,actor+0x94)>=1 && word(ram,actor+0x94)<=5)) &&
           MEM_H(0xD8,int32_t(physical))==30) {
            // Original decremented the incoming damage timer before the
            // clip command rearmed it. The new30 must not be decremented or
            // blinked by the tail of that same Source interval.
            player_cooldown_rearm=PlayerCooldownRearm{actor,physical,model,
                game_model_animation().lifetime_token(model),slice->tick,slice->denominator};
        } else if(site==0x801C8D18 && player_cooldown_rearm) {
            const auto& value=*player_cooldown_rearm;
            if(value.actor==actor && value.physical==physical && value.model==model && value.lifetime &&
               value.lifetime==game_model_animation().lifetime_token(model) && value.tick==slice->tick &&
               value.denominator==slice->denominator && MEM_H(0xD8,int32_t(physical))==30)
                ctx->r6=10000; // Reviewed incoming value before the rearm.
        }
        return 0;
    }
    if(site==0x801CC0B0 && projectile_call && live(ram) && active_simulation_slice()) {
        // The bounce impulse is a new motion phase at this committed end.
        // Commands arriving between Source ticks must not borrow the old
        // phase grid for the subsequent ten-tick ascent/conversion.
        register_projectile_birth(ram,uint32_t(ctx->r17),true);
        return 0;
    }
    if(site==0x801D0924 && interaction_after_pose && !interaction_geometry_only && active_simulation_slice())
        return game_model_animation().ended_this_tick(uint32_t(word(ram,uint32_t(ctx->r17)+0x74)));
    if(site==0x801C12D0 && motion_call && live(ram) && active_simulation_slice() && gameplay_slice) {
        const auto state=word(ram,uint32_t(word(ram,0x8021D49C))+0x94);
        if(state==24 || state==25 || state==27) {
            zero_shot_before.emplace();zero_shot_epoch=epoch;
            for(unsigned i=0;i<zero_shot_before->size();++i) {
                const auto actor=0x804B24C8u+i*0xCC;
                (*zero_shot_before)[i]=word(ram,actor+0x94)&&owns_projectile(ram,actor)?projectile_identity(ram,actor,0):ProjectileClock{};
            }
        }
        return 0;
    }
    if(site==0x801C12D8 && zero_shot_before) {
        const auto before=*zero_shot_before;zero_shot_before.reset();
        if(zero_shot_epoch!=epoch || !motion_call || !live(ram) || !active_simulation_slice() || !gameplay_slice)return 0;
        for(unsigned i=0;i<before.size();++i) {
            const auto actor=0x804B24C8u+i*0xCC;
            if(!word(ram,actor+0x94)||!owns_projectile(ram,actor))continue;
            const auto identity=projectile_identity(ram,actor,0);
            if(same_projectile(before[i],identity))continue;
            register_projectile_birth(ram,actor,true,true);
            projectile_start_births[i]=identity.lifetime;
        }
        return 0;
    }
    if(player_projectile_birth) {
        if(site==0x801D10D8)return 0;
        if(site==0x801D1130) {
            const auto actor=uint32_t(ctx->r17),physical=uint32_t(word(ram,actor+0x98));
            const auto slot=(actor-0x804B24C8u)/0xCC;uint8_t* rdram=ram;
            const auto lifetime=game_model_animation().lifetime_token(uint32_t(word(ram,actor+0x74)));
            if(slot>=projectile_before.size() || (physical==projectile_before[slot] && lifetime==projectile_before_lifetimes[slot]) ||
               (physical==projectile_birth_done[slot] && lifetime==projectile_birth_lifetimes[slot]) || !owns_projectile(ram,actor))return 1;
            const auto kind=MEM_H(0x20,int32_t(physical));
            register_projectile_birth(ram,actor);
            projectile_birth_done[slot]=physical;
            projectile_birth_lifetimes[slot]=lifetime;
            return kind==101||kind==103||kind==105; // Those ctor hooks already initialize their commands.
        }
    }
    if((site==0x801CFA80 || site==0x801D0074 || site==0x801D0594) && live(ram) && ctx->r2==1) {
        // Source executes this charge's first command in its birth pass. It
        // initializes phase21, radius10, alpha and pose before any subsequent
        // elapsed interval; the radius is attachment geometry, not free flight.
        const auto actor=uint32_t(site==0x801D0594?ctx->r17:ctx->r16),physical=uint32_t(site==0x801D0594?ctx->r16:ctx->r17);
        register_projectile_birth(ram,actor,true);
        recomp_context command;dora3::copy_recomp_context(command,*ctx);
        auto initialize=[&] {
            command.r4=int32_t(actor);command.r5=int32_t(physical);
            if(site==0x801D0594)update_interaction(ram,&command,actor,physical);
            else (site==0x801CFA80?dora3_s3_core_801CFA94:dora3_s3_core_801D0088)(ram,&command);
            command.r4=int32_t(actor);command.r5=0;command.r6=std::bit_cast<int32_t>(3.f);dora3_s0_8002A1B8(ram,&command);
            command.r4=int32_t(actor);command.r5=1;dora3_s0_80028C74(ram,&command);
        };
        if(active_simulation_slice()) {SimulationSuspension birth_commands;initialize();}
        else initialize();
        const auto model=uint32_t(word(ram,actor+0x74));
        game_model_animation().acquire_simulation(ram,[&](uint32_t candidate)->std::optional<float> {return candidate==model?std::optional<float>{3}:std::nullopt;},false);
        if(site==0x801D0594) {
            const auto target=uint32_t(word(ram,physical+0x48));
            if(capture_actor(ram,target))game_model_animation().bind_actor(ram,model,physical+0x48,target);
        }
        return 0;
    }
    if(site==0x801D117C && projectile_call) {
        if(!live(ram)||!active_simulation_slice())return 2;
        const auto actor=uint32_t(ctx->r17),physical=uint32_t(ctx->r16);
        uint8_t* rdram=ram;
        if(MEM_H(0x20,int32_t(physical))==105) {
            update_interaction(ram,ctx,actor,physical);
            if(!live(ram)||!active_simulation_slice())return 2;
            if(!word(ram,actor+0x94) && active_simulation_slice()->ends_tick())initialize_player_projectiles(ram,ctx);
            if(!live(ram)||!active_simulation_slice())return 2;
            return !word(ram,actor+0x94)?1:0;
        }
        const auto kind=MEM_H(0x20,int32_t(physical));
        const bool terminal=kind==92||kind==94||kind==96||kind==98||kind==100||kind==102||kind==104||kind==106||kind==108;
        if(kind==89 || terminal) {
            // Original consumes the preceding model counter. Native birth
            // acquisition restores the initial pose; consume its end pulse
            // after this interval's advance to preserve the same lifetime.
            if(!active_simulation_slice()->ends_tick())return 0;
            recomp_context command;dora3::copy_recomp_context(command,*ctx);
            command.r4=int32_t(actor);command.r5=0;dora3_s0_menu_8002A3B0(ram,&command);
            if(!command.r2 && !game_model_animation().ended_this_tick(uint32_t(word(ram,actor+0x74))))return 0;
            if(!terminal && (MEM_H(0x36,int32_t(physical))!=10 || MEM_H(0x38,int32_t(physical))==4))
                dora3_s3_801C05F0(ram,&command);
            command.r4=int32_t(actor);command.r5=0xFFFF;dora3_s0_8002EA4C(ram,&command);
            return !live(ram)||!active_simulation_slice()?2:1;
        }
        if(MEM_H(0x20,int32_t(physical))!=83)return 0;
        if(!active_simulation_slice()->ends_tick())return 0;
        // This callback's clip commands belong to the end of the interval
        // just advanced. Rebinding here gives the newborn model zero elapsed
        // time, then the next slice uses its new phase and opacity immediately.
        recomp_context command;dora3::copy_recomp_context(command,*ctx);
        command.r4=int32_t(actor);command.r5=0;dora3_s0_menu_8002A3B0(ram,&command);
        if(!command.r2 && !game_model_animation().ended_this_tick(uint32_t(word(ram,actor+0x74))))return 0;
        const auto phase=int16_t(MEM_H(0x34,int32_t(physical))+1);
        MEM_H(0x34,int32_t(physical))=phase;
        command.r4=int32_t(actor);command.r5=phase<4?1:0xFFFF;
        dora3_s0_8002EA4C(ram,&command);
        if(phase>=4)return 1;
        command.r4=int32_t(actor);command.r5=0;command.r6=phase;dora3_s0_8002A2E8(ram,&command);
        command.r4=int32_t(actor);command.r5=0;command.r6=phase;dora3_s0_8002A328(ram,&command);
        MEM_H(0x22,int32_t(physical))=phase;
        if(phase==2)dora3_s3_801C05F0(ram,&command);
        return !live(ram)||!active_simulation_slice()?2:0;
    }
    if(site==0x801C9048 && motion_call) {
        finish_player_flight(ram);
        if(player_hover_clock && active_simulation_slice()) {
            auto& hover=*player_hover_clock;auto& clock=hover.motion;const auto& slice=*active_simulation_slice();
            if(same_flight_owner(ram,clock) && word(ram,clock.actor+0x94)==63) {
                clock.expected=flight_observation(ram,clock.actor,clock.physical);
                hover.expected_vertical_speed=word(ram,clock.physical+8);
                clock.expected_phase=slice.phase+slice.numerator;clock.open=false;
            } else player_hover_clock.reset();
        }
        if(player_stop_clock && active_simulation_slice()) {
            auto& clock=*player_stop_clock;const auto& slice=*active_simulation_slice();
            if(clock.actor==uint32_t(word(ram,0x8021D49C)) && clock.physical==uint32_t(word(ram,0x8021D4B0)) &&
               clock.model==uint32_t(word(ram,clock.actor+0x74)) && clock.lifetime==game_model_animation().lifetime_token(clock.model) &&
               clock.run==game_model_animation().run_token(clock.model) && clock.tick==slice.tick && clock.denominator==slice.denominator)
                clock.expected=word(ram,clock.physical);
        }
    }
    // Stop at the next dispatcher boundary after an action or scene replacement.
    if((site==0x801C8F10 || site==0x801C8F34 || site==0x801C9048) && active_player_epoch)
        return *active_player_epoch!=epoch || !live(ram) || !active_simulation_slice();
    if((site==0x80230F70||site==0x80231030||site==0x8023098C||site==0x802309B4||site==0x80230D0C||site==0x80230F0C)&&
       object_call&&live(ram)&&active_simulation_slice()) {
        uint8_t* rdram=ram;const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,0x8021BCDC));
        const auto model=uint32_t(word(ram,actor+0x74)),slot=(actor-0x804B34B8u)/0xCC;
        const auto lifetime=game_model_animation().lifetime_token(model);const auto kind=MEM_H(0x20,int32_t(physical));
        if(slot>=linked_city_periods.size()||!linked_city_owner(ram,actor))throw std::logic_error("Invalid linked city owner");
        const auto other_index=MEM_H(kind==18?0x80:0x7C,int32_t(physical));
        std::array<uint64_t,5> link{};
        if(other_index!=-1) {
            const auto other=0x804B1CD0u+unsigned(other_index)*0xCC,other_model=uint32_t(word(ram,other+0x74));
            link={other,uint32_t(word(ram,other+0x98)),other_model,game_model_animation().lifetime_token(other_model),0};
            if(kind==19)link[4]=linked_city_publications[(other-0x804B34B8u)/0xCC];
        }
        const std::array<int32_t,10> fields{word(ram,actor+0x94),MEM_B(0x28,int32_t(physical)),
            MEM_H(0x34,int32_t(physical)),MEM_H(0x36,int32_t(physical)),MEM_H(0x40,int32_t(physical)),
            MEM_H(0x42,int32_t(physical)),MEM_H(0x44,int32_t(physical)),MEM_H(0x7C,int32_t(physical)),MEM_H(0x80,int32_t(physical)),kind==18?MEM_B(0,int32_t(0x8021710D)):0};
        auto& old=linked_city_periods[slot];
        const auto yaw=[&]{return std::array<int32_t,2>{word(ram,actor+0x10),word(ram,physical+4)};};
        const auto advance=[&](bool first) {
            if(old->geometry) {
                const auto address=old->geometry==1?physical+4:actor+0x10;
                const float current=first?old->start:std::bit_cast<float>(word(ram,address));
                // The first Original FSM sees the complete Source proposal,
                // including the yaw used to compute its return rate. Restore
                // only its visible geometry before evaluating the fraction.
                if(first)MEM_W(0,int32_t(address))=std::bit_cast<int32_t>(current);
                const float value=game_model_animation().scalar_latched_turn(ram,model,address,current,old->delta);
                MEM_W(0x10,int32_t(actor))=std::bit_cast<int32_t>(value);
                if(old->geometry==1)MEM_W(4,int32_t(physical))=std::bit_cast<int32_t>(value);
            }
            old->expected_yaw=yaw();
        };
        if(site==0x80230F70||site==0x8023098C) {
            if(old&&old->actor==actor&&old->physical==physical&&old->model==model&&old->lifetime==lifetime&&
               old->tick==slice.tick&&old->denominator==slice.denominator&&old->expected==fields&&old->link==link&&
               (kind==19||old->expected_yaw==yaw())) {advance(false);return 1;}
            old=LinkedCityPeriod{actor,physical,model,lifetime,slice.tick,slice.denominator,fields,link,yaw(),0,0,0};
            return 0;
        }
        if(!old)throw std::logic_error("Linked city callback missing period entry");
        if(site==0x80230D0C) {
            const float current=std::bit_cast<float>(word(ram,physical+4));
            recomp_context source;dora3::copy_recomp_context(source,*ctx);source.r5=int32_t(0x41F00000u);
            {SimulationSuspension geometry;dora3_s3_demo_801E88C4(ram,&source);}
            old->geometry=1;old->start=current;old->delta=float(std::remainder(double(source.f0.fl)-current,360.));
            ctx->r5=source.r5;ctx->f0.fl=source.f0.fl;return 1;
        }
        if(site==0x80230F0C) {old->geometry=2;old->start=std::bit_cast<float>(word(ram,actor+0x10));old->delta=ctx->f14.fl;return 0; /* f12 is loaded in the JAL delay slot, after this hook. */}
        old->expected=fields;
        if(kind==18&&MEM_H(0x40,int32_t(physical))!=-1)++linked_city_publications[slot];
        if(kind==18&&word(ram,actor+0x94)!=2)old->geometry=0; // Original terminal snap is immediate.
        advance(true);return 0;
    }
    if((site==0x80232070||site==0x802320A4||site==0x802320B0||site==0x802322E0||site==0x8023229C)&&object_call&&live(ram)&&active_simulation_slice()) {
        uint8_t* rdram=ram;const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,0x8021BCDC));
        const auto model=uint32_t(word(ram,actor+0x74)),slot=(actor-0x804B34B8u)/0xCC;
        const auto lifetime=game_model_animation().lifetime_token(model);
        if(slot>=city_kaminari_periods.size()||!lifetime||!owns_common_object(ram,actor)||MEM_H(0x20,int32_t(physical))!=21)
            throw std::logic_error("Invalid special city dialogue owner");
        const std::array<int32_t,11> fields{word(ram,actor+0x94),MEM_B(0x28,int32_t(physical)),MEM_H(0x34,int32_t(physical)),
            MEM_H(0x36,int32_t(physical)),MEM_H(0x42,int32_t(physical)),MEM_H(0x44,int32_t(physical)),
            MEM_B(0,int32_t(0x8021710D)),word(ram,physical+0x58),word(ram,physical+0x5C),word(ram,actor+0x10),word(ram,physical+4)};
        auto& old=city_kaminari_periods[slot];auto& counter=city_kaminari_counters[slot];
        const auto same_counter=[&] {return counter&&counter->actor==actor&&counter->physical==physical&&counter->model==model&&
            counter->lifetime==lifetime&&counter->tick==slice.tick&&counter->denominator==slice.denominator&&counter->expected==fields[3];};
        if(site==0x8023229C) {
            // Busy or yaw writes can re-evaluate the proposal within a period,
            // but cannot consume a second discrete release tick.
            if(same_counter()) {ctx->r2=counter->expected;return 1;}
            counter=CityKaminariCounter{actor,physical,model,lifetime,slice.tick,slice.denominator,int16_t(ctx->r2)};return 0;
        }
        if(site==0x80232070||site==0x802320B0) {
            if(old&&old->actor==actor&&old->physical==physical&&old->model==model&&old->lifetime==lifetime&&
               old->tick==slice.tick&&old->denominator==slice.denominator&&old->expected==fields)return 1;
            return 0;
        }
        old=CityKaminariPeriod{actor,physical,model,lifetime,slice.tick,slice.denominator,fields};
        if(fields[0]==201&&fields[3]>0)counter=CityKaminariCounter{actor,physical,model,lifetime,slice.tick,slice.denominator,fields[3]};
        return 0;
    }
    if((site==0x8022FC1C||site==0x8022FC38||site==0x8022FE40)&&object_call&&live(ram)&&active_simulation_slice()&&dora3_overlay_loaded(0x001398A0u)) {
        uint8_t* rdram=ram;const auto actor=uint32_t(word(ram,0x8021BDEC)),slot=(actor-0x804B34B8u)/0xCC;
        if(slot>=slot09_special_turns.size()||!slot09_special_turns[slot]||!slot09_special_turns[slot]->executing)
            throw std::logic_error("Special28 Source decision missing geometry proposal");
        const auto v=*slot09_special_turns[slot];
        if(site==0x8022FE40) {slot09_special_turns[slot].reset();return 0;}
        if(site==0x8022FC1C) {ctx->r5=int32_t(0x41F00000u);ctx->f0.fl=v.proposal;return 1;}
        // FC3C is the real JAL delay-slot write, before the target result.
        MEM_W(0x10,ctx->r4)=ctx->f0.u32l;ctx->f0.fl=v.target;return 1;
    }
    const auto slot09_dialogue_site=[&]() {
        switch(site) {
        case 0x8022DBB8:return std::pair{24,0};case 0x8022DEA8:return std::pair{24,1};case 0x8022DC48:return std::pair{24,2};
        case 0x8022E4F8:return std::pair{25,0};case 0x8022E6F4:return std::pair{25,1};case 0x8022E574:return std::pair{25,2};
        case 0x8022E938:return std::pair{25,0};case 0x8022EB88:return std::pair{25,1};case 0x8022E9B4:return std::pair{25,2};
        case 0x8022F254:return std::pair{26,0};case 0x8022F668:return std::pair{26,1};case 0x8022F2D0:return std::pair{26,2};
        case 0x802300C0:return std::pair{27,0};case 0x80230384:return std::pair{27,1};case 0x80230144:return std::pair{27,2};
        case 0x8022F920:return std::pair{28,0};case 0x8022FBA0:return std::pair{28,1};case 0x8022F984:return std::pair{28,2};
        default:return std::pair{0,-1};
        }
    }();
    if(slot09_dialogue_site.first&&object_call&&live(ram)&&active_simulation_slice()&&dora3_overlay_loaded(0x001398A0u)) {
        uint8_t* rdram=ram;const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,0x8021BCDC));
        const auto model=uint32_t(word(ram,actor+0x74)),slot=(actor-0x804B34B8u)/0xCC;
        const auto lifetime=game_model_animation().lifetime_token(model);
        if(slot>=slot09_dialogue_periods.size()||!owns_common_object(ram,actor)||MEM_H(0x20,int32_t(physical))!=slot09_dialogue_site.first)
            throw std::logic_error("Invalid slot09 dialogue owner");
        const std::array<int32_t,9> fields{word(ram,actor+0x94),MEM_B(0x28,int32_t(physical)),MEM_H(0x34,int32_t(physical)),
            MEM_H(0x36,int32_t(physical)),MEM_H(0x42,int32_t(physical)),MEM_H(0x44,int32_t(physical)),
            MEM_B(0,int32_t(0x8021710D)),word(ram,physical+0x58),word(ram,physical+0x5C)};
        const auto yaw=[&]{return std::array<int32_t,2>{word(ram,actor+0x10),word(ram,physical+4)};};
        auto& old=slot09_dialogue_periods[slot];
        const auto advance=[&](bool first) {
            if(old->continuous)for(unsigned i=0;i<2;++i) {
                const auto address=i?physical+4:actor+0x10;
                const float current=first?old->start[i]:std::bit_cast<float>(word(ram,address));
                if(first)MEM_W(0,int32_t(address))=std::bit_cast<int32_t>(current);
                const float value=game_model_animation().scalar_latched_turn(ram,model,address,current,old->delta[i]);
                MEM_W(0,int32_t(address))=std::bit_cast<int32_t>(value);
            }
            old->expected_yaw=yaw();
        };
        if(slot09_dialogue_site.second==0) {
            if(old&&old->actor==actor&&old->physical==physical&&old->model==model&&old->lifetime==lifetime&&
               old->tick==slice.tick&&old->denominator==slice.denominator&&old->expected==fields&&old->expected_yaw==yaw()) {
                advance(false);return 1;
            }
            const std::array<float,2> start{std::bit_cast<float>(word(ram,actor+0x10)),std::bit_cast<float>(word(ram,physical+4))};
            old=CityDialoguePeriod{actor,physical,model,lifetime,slice.tick,slice.denominator,fields,yaw(),start,{},false};return 0;
        }
        if(!old)throw std::logic_error("Slot09 dialogue missing proposal entry");
        if(slot09_dialogue_site.second==2) {
            recomp_context source;dora3::copy_recomp_context(source,*ctx);source.r5=int32_t(0x41F00000u);
            {SimulationSuspension geometry;dora3_s3_demo_801E88C4(ram,&source);}
            ctx->r5=source.r5;ctx->f0.fl=source.f0.fl;return 1;
        }
        old->expected=fields;old->continuous=slot09_dialogue_state(slot09_dialogue_site.first,word(ram,actor+0x94));
        const auto proposed=yaw();
        for(unsigned i=0;i<2;++i)old->delta[i]=float(std::remainder(double(std::bit_cast<float>(proposed[i]))-old->start[i],360.));
        // A completed dialogue cannot execute the newly selected walk callback
        // twice inside this same Original decision period.
        slot09_decision_periods[slot]=CityDecisionPeriod{actor,physical,model,lifetime,slice.tick,slice.denominator,
            {word(ram,actor+0x94),MEM_B(0x28,int32_t(physical)),MEM_H(0x34,int32_t(physical))}};
        advance(true);return 0;
    }
    if((site==0x8022F684||site==0x8022F700)&&!object_call&&live(ram)&&!active_simulation_slice()&&dora3_overlay_loaded(0x001398A0u)) {
        uint8_t* rdram=ram;const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,0x8021BCDC));
        const auto slot=(actor-0x804B34B8u)/0xCC;
        if(slot<slot09_source_dialogues.size()&&pointer(physical,0xF4)&&MEM_H(0x20,int32_t(physical))==26&&
           uint32_t(word(ram,0x80202064+26*84))==slot09_callback(26)) {
            const auto model=uint32_t(word(ram,actor+0x74));const auto state=word(ram,actor+0x94);
            const auto lifetime=game_model_animation().lifetime_token(model);
            auto& before=slot09_source_dialogues[slot];
            if(site==0x8022F684) {
                before.reset();slot09_native_reentries[slot].reset();
                if(state==102&&lifetime&&!game_model_animation().native_birth(model)&&timeline->phase()<60)
                    before=Slot09DialogueReentry{actor,physical,model,lifetime,timeline->ticks(),timeline->phase(),uint64_t(timeline->divider())*second,state};
            } else if(before) {
                auto handoff=*before;before.reset();
                if(handoff.actor==actor&&handoff.physical==physical&&handoff.model==model&&handoff.lifetime==lifetime&&
                   (state==1||state==101)&&MEM_BU(0x28,int32_t(physical))==0&&timeline->ticks()==handoff.tick&&timeline->phase()==handoff.phase) {
                    handoff.state=state;slot09_native_reentries[slot]=handoff;
                }
            }
        }
    }
    const auto slot09_site=[&]() -> std::pair<int,bool> {
        switch(site) {
        case 0x8022DFB4:return {24,true};case 0x8022E0A4:return {24,false};
        case 0x8022EBA4:return {25,true};case 0x8022EC38:return {25,false};
        case 0x8022F684:return {26,true};case 0x8022F700:return {26,false};
        case 0x80230458:return {27,true};case 0x802304BC:return {27,false};
        case 0x8022FE60:return {28,true};case 0x8022FEF8:return {28,false};
        default:return {0,false};
        }
    }();
    if(slot09_site.first&&object_call&&live(ram)&&active_simulation_slice()&&dora3_overlay_loaded(0x001398A0u)) {
        uint8_t* rdram=ram;const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,0x8021BCDC));
        const auto slot=(actor-0x804B34B8u)/0xCC,model=uint32_t(word(ram,actor+0x74));
        const auto lifetime=game_model_animation().lifetime_token(model);
        if(slot>=slot09_decision_periods.size()||!lifetime||MEM_H(0x20,int32_t(physical))!=slot09_site.first||
           uint32_t(word(ram,0x80202064+unsigned(slot09_site.first)*84))!=slot09_callback(slot09_site.first))
            throw std::logic_error("Invalid slot09 decision owner");
        const std::array<int32_t,3> fields{word(ram,actor+0x94),MEM_B(0x28,int32_t(physical)),MEM_H(0x34,int32_t(physical))};
        auto& old=slot09_decision_periods[slot];
        // Dialogue proposals have their own yaw/phase cache; the dispatcher
        // must reach that gate even on the second fraction of this period.
        if(slot09_site.second&&slot09_dialogue_state(slot09_site.first,fields[0]))return 0;
        if(slot09_site.second&&slot09_site.first==28&&fields[0]==200) {
            auto& v=slot09_special_turns[slot];
            const auto yaw=[&] {return std::array<int32_t,2>{word(ram,actor+0x10),word(ram,physical+4)};};
            const std::array<int32_t,9> discrete{fields[0],fields[1],fields[2],MEM_H(0x36,int32_t(physical)),
                MEM_H(0x42,int32_t(physical)),MEM_H(0x44,int32_t(physical)),MEM_B(0,int32_t(0x8021710D)),word(ram,physical+0x58),word(ram,physical+0x5C)};
            if(v&&(!slot09_special_turn_identity(ram,*v)||v->tick!=slice.tick||v->denominator!=slice.denominator||v->fields!=discrete||v->expected!=yaw()))
                cancel_slot09_special_turn(ram,slot);
            if(!v) {
                const auto start=yaw();std::array<float,2> proposed{std::bit_cast<float>(start[0]),std::bit_cast<float>(start[1])};
                float target=0;
                recomp_context source;dora3::copy_recomp_context(source,*ctx);source.r4=int32_t(actor);
                if(discrete[1]==0||discrete[2]==0) {
                    // Original initialization copies actor yaw into physics
                    // before E88C4 reads it. Do not publish that init early.
                    if(discrete[1]==0)MEM_W(4,int32_t(physical))=start[0];
                    source.r5=int32_t(0x41F00000u);
                    {SimulationSuspension geometry;dora3_s3_demo_801E88C4(ram,&source);}
                    proposed={source.f0.fl,source.f0.fl};MEM_W(4,int32_t(physical))=start[1];
                    source.r4=int32_t(actor);
                    {SimulationSuspension geometry;dora3_s3_demo_801E8590(ram,&source);}
                    target=source.f0.fl;
                }
                if(discrete[1]!=0&&discrete[3]!=0) {
                    source.f12.fl=std::bit_cast<float>(start[0]);source.f14.fl=std::bit_cast<float>(discrete[8]);
                    {SimulationSuspension geometry;dora3_s0_80031A0C(ram,&source);}
                    proposed[0]=source.f0.fl;
                }
                std::array<float,2> delta{};
                for(unsigned axis=0;axis<2;++axis)delta[axis]=float(std::remainder(double(proposed[axis])-std::bit_cast<float>(start[axis]),360.));
                v=Slot09SpecialTurn{actor,physical,model,lifetime,slice.tick,slice.denominator,discrete,start,start,delta,proposed[1],target,false};
            }
            for(unsigned axis=0;axis<2;++axis) {
                const auto address=axis?physical+4:actor+0x10;
                const auto value=game_model_animation().scalar_latched_turn(ram,model,address,std::bit_cast<float>(word(ram,address)),v->delta[axis]);
                MEM_W(0,int32_t(address))=std::bit_cast<int32_t>(value);
            }
            v->expected=yaw();
            if(!slice.ends_tick())return 1;
            // Original consumes its initial pose exactly once. Geometry hooks
            // supply the interval-start proposal, preserving command timing.
            MEM_W(0x10,int32_t(actor))=v->start[0];MEM_W(4,int32_t(physical))=v->start[1];
            v->executing=true;object_command_at_end=true;return 0;
        }
        if(slot09_site.second&&slot09_site.first==24&&fields[0]==200) {
            auto& pending=slot09_story_starts[slot];
            if(pending&&(!slot09_story_start_identity(ram,*pending)||pending->tick!=slice.tick||pending->denominator!=slice.denominator))
                cancel_slot09_story_start(ram,slot);
            if(MEM_BU(0x28,int32_t(physical))==0) {
                recomp_context command;dora3::copy_recomp_context(command,*ctx);
                MEM_B(0x28,int32_t(physical))=1;
                command.r4=int32_t(actor);command.r5=0;command.r6=6;
                dora3_s0_8002A328(ram,&command);
                pending=Slot09StoryStart{actor,physical,model,lifetime,slice.tick,slice.denominator};
            }
            if(!slice.ends_tick())return 1;
            object_command_at_end=true;
            if(pending) {
                if(!slot09_story_start_identity(ram,*pending))throw std::logic_error("Invalid pending special24 dialog publication");
                recomp_context command;dora3::copy_recomp_context(command,*ctx);command.r4=int32_t(0x80232A40u);
                dora3_s3_core_801B91D4(ram,&command);pending.reset();
            }
            // Execute the real initialized Original body: busy query and
            // once-only quest51/counter/sound/completion, in original order.
            return 0;
        }
        // Forced starts24/28 and special24 contain only discrete commands.
        // Keep the exact Source completion boundary even for Native-born
        // models; common geometry/model time still advances each fraction.
        if(slot09_site.second&&((slot09_site.first==24&&fields[0]==100)||
                               (slot09_site.first==28&&fields[0]==100))) {
            if(!slice.ends_tick())return 1;
            object_command_at_end=true;
            return 0;
        }
        // Kind27 state100 is a pure fifty-count wait which starts a real
        // dialogue. Its completed counter and command belong to the end
        // of the Source period, not the first fractional callback within it.
        // Model time/common geometry still advance on each Native slice.
        if(slot09_site.second&&slot09_site.first==27&&fields[0]==100) {
            if(!slice.ends_tick())return 1;
            object_command_at_end=true;
            slot09_wait_decision=Slot09SourceHandoff{actor,physical,model,lifetime};
            // The post gate also records skipped fractional callbacks.
            // That cache is not evidence that this completed wait count
            // has executed. Each positive slice ends a given period once.
            return 0;
        }
        if(slot09_site.second)return old&&old->actor==actor&&old->physical==physical&&old->model==model&&old->lifetime==lifetime&&
            old->tick==slice.tick&&old->denominator==slice.denominator&&old->expected==fields;
        if(!slot09_site.second&&slot09_wait_decision) {
            const auto decision=*slot09_wait_decision;slot09_wait_decision.reset();
            if(decision.actor==actor&&decision.physical==physical&&decision.model==model&&
               decision.lifetime==lifetime&&fields[0]==2)slot09_source_handoffs[slot]=decision;
        }
        old=CityDecisionPeriod{actor,physical,model,lifetime,slice.tick,slice.denominator,fields};return 0;
    }
    const auto dialogue_site=[&]() {
        switch(site) {
        case 0x8022E0A8:return std::pair{13,0};
        case 0x8022E3E4:return std::pair{13,1};
        case 0x8022E164:return std::pair{13,2};
        case 0x8022EA08:return std::pair{14,0};
        case 0x8022ECE0:return std::pair{14,1};
        case 0x8022EA8C:return std::pair{14,2};
        case 0x8022F2D8:return std::pair{15,0};
        case 0x8022F5B0:return std::pair{15,1};
        case 0x8022F35C:return std::pair{15,2};
        case 0x8022FBC0:return std::pair{16,0};
        case 0x8022FEA8:return std::pair{16,1};
        case 0x8022FC40:return std::pair{16,2};
        case 0x802304A4:return std::pair{17,0};
        case 0x8023077C:return std::pair{17,1};
        case 0x80230528:return std::pair{17,2};
        case 0x8023147C:return std::pair{20,0};
        case 0x80231734:return std::pair{20,1};
        case 0x802314F8:return std::pair{20,2};
        case 0x80231D54:return std::pair{21,0};
        case 0x80231FD8:return std::pair{21,1};
        case 0x80231DD0:return std::pair{21,2};
        case 0x80232860:return std::pair{22,0};
        case 0x80232B38:return std::pair{22,1};
        case 0x802328E4:return std::pair{22,2};
        default:return std::pair{0,-1};
        }
    }();
    if(dialogue_site.first&&object_call&&live(ram)&&active_simulation_slice()) {
        uint8_t* rdram=ram;const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,0x8021BCDC));
        const auto model=uint32_t(word(ram,actor+0x74)),slot=(actor-0x804B34B8u)/0xCC;
        const auto lifetime=game_model_animation().lifetime_token(model);
        if(slot>=city_dialogue_periods.size()||!owns_common_object(ram,actor)||MEM_H(0x20,int32_t(physical))!=dialogue_site.first)
            throw std::logic_error("Invalid city dialogue owner");
        const std::array<int32_t,9> fields{word(ram,actor+0x94),MEM_B(0x28,int32_t(physical)),MEM_H(0x34,int32_t(physical)),
            MEM_H(0x36,int32_t(physical)),MEM_H(0x42,int32_t(physical)),MEM_H(0x44,int32_t(physical)),
            MEM_B(0,int32_t(0x8021710D)),word(ram,physical+0x58),word(ram,physical+0x5C)};
        const auto yaw=[&]{return std::array<int32_t,2>{word(ram,actor+0x10),word(ram,physical+4)};};
        auto& old=city_dialogue_periods[slot];
        const auto advance=[&](bool first) {
            if(old->continuous)for(unsigned i=0;i<2;++i) {
                const auto address=i?physical+4:actor+0x10;
                const float current=first?old->start[i]:std::bit_cast<float>(word(ram,address));
                if(first)MEM_W(0,int32_t(address))=std::bit_cast<int32_t>(current);
                const float value=game_model_animation().scalar_latched_turn(ram,model,address,current,old->delta[i]);
                MEM_W(0,int32_t(address))=std::bit_cast<int32_t>(value);
            }
            old->expected_yaw=yaw();
        };
        if(dialogue_site.second==0) {
            if(old&&old->actor==actor&&old->physical==physical&&old->model==model&&old->lifetime==lifetime&&
               old->tick==slice.tick&&old->denominator==slice.denominator&&old->expected==fields&&old->expected_yaw==yaw()) {
                advance(false);return 1;
            }
            const std::array<float,2> start{std::bit_cast<float>(word(ram,actor+0x10)),std::bit_cast<float>(word(ram,physical+4))};
            old=CityDialoguePeriod{actor,physical,model,lifetime,slice.tick,slice.denominator,fields,yaw(),start,{},false};return 0;
        }
        if(!old)throw std::logic_error("City dialogue missing proposal entry");
        if(dialogue_site.second==2) {
            recomp_context source;dora3::copy_recomp_context(source,*ctx);source.r5=int32_t(0x41F00000u);
            {SimulationSuspension geometry;dora3_s3_demo_801E88C4(ram,&source);}
            ctx->r5=source.r5;ctx->f0.fl=source.f0.fl;return 1;
        }
        old->expected=fields;old->continuous=word(ram,actor+0x94)==3;
        const auto proposed=yaw();
        for(unsigned i=0;i<2;++i)old->delta[i]=float(std::remainder(double(std::bit_cast<float>(proposed[i]))-old->start[i],360.));
        // A completed dialogue cannot execute the newly selected walk callback
        // twice inside this same Original decision period.
        city_decision_periods[slot]=CityDecisionPeriod{actor,physical,model,lifetime,slice.tick,slice.denominator,
            {word(ram,actor+0x94),MEM_B(0x28,int32_t(physical)),MEM_H(0x34,int32_t(physical))}};
        advance(true);return 0;
    }
    const auto city_decision_site=[&]() -> std::pair<int,bool> {
        switch(site) {
        case 0x8022DE70:return {13,true};
        case 0x8022DF44:return {13,false};
        case 0x8022DF50:return {13,true};
        case 0x8022E09C:return {13,false};
        case 0x8022E7D0:return {14,true};
        case 0x8022E8A4:return {14,false};
        case 0x8022E8B0:return {14,true};
        case 0x8022E9FC:return {14,false};
        case 0x8022F0A0:return {15,true};
        case 0x8022F174:return {15,false};
        case 0x8022F180:return {15,true};
        case 0x8022F2CC:return {15,false};
        case 0x8022F988:return {16,true};
        case 0x8022FA5C:return {16,false};
        case 0x8022FA68:return {16,true};
        case 0x8022FBB4:return {16,false};
        case 0x8023026C:return {17,true};
        case 0x80230340:return {17,false};
        case 0x8023034C:return {17,true};
        case 0x80230498:return {17,false};
        case 0x80231270:return {20,true};
        case 0x80231344:return {20,false};
        case 0x80231350:return {20,true};
        case 0x8023146C:return {20,false};
        case 0x80231B1C:return {21,true};
        case 0x80231BF0:return {21,false};
        case 0x80231BFC:return {21,true};
        case 0x80231D48:return {21,false};
        case 0x80232654:return {22,true};
        case 0x80232728:return {22,false};
        case 0x80232734:return {22,true};
        case 0x80232850:return {22,false};
        default:return {0,false};
        }
    }();
    if(city_decision_site.first&&object_call&&live(ram)&&active_simulation_slice()) {
        uint8_t* rdram=ram;const auto& slice=*active_simulation_slice();
        const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,0x8021BCDC));
        const auto model=uint32_t(word(ram,actor+0x74)),slot=(actor-0x804B34B8u)/0xCC;
        const auto lifetime=game_model_animation().lifetime_token(model);
        if(slot>=city_decision_periods.size()||MEM_H(0x20,int32_t(physical))!=city_decision_site.first||!owns_common_object(ram,actor))
            throw std::logic_error("Invalid city patrol decision owner");
        const std::array<int32_t,3> fields{word(ram,actor+0x94),MEM_B(0x28,int32_t(physical)),MEM_H(0x34,int32_t(physical))};
        auto& old=city_decision_periods[slot];
        if(city_decision_site.second) {
            // Preserve the state produced by this period's Original command:
            // a1->2 transition must not bind clip2 again in the same period.
            // A clip rebind changes run, not actor lifetime or private count.
            return old&&old->actor==actor&&old->physical==physical&&old->model==model&&old->lifetime==lifetime&&
                old->tick==slice.tick&&old->denominator==slice.denominator&&old->expected==fields;
        }
        old=CityDecisionPeriod{actor,physical,model,lifetime,slice.tick,slice.denominator,fields};
        return 0;
    }
    if((site==0x801E8E00||site==0x801E8E10||site==0x801E8E20)&&object_call&&live(ram)&&active_simulation_slice()) {
        uint8_t* rdram=ram;const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,0x8021BCDC));
        const auto kind=MEM_H(0x20,int32_t(physical));
        // Absolute callback coordinates retain Original's velocity offset at
        // each evaluated pose. Incremental axes still receive fractional dt.
        const auto slot=(actor-0x804B34B8u)/0xCC;const auto& slice=*active_simulation_slice();
        if((city_patrol_callback(kind)||slot09_callback(kind))&&site!=0x801E8E10&&owns_common_object(ram,actor)&&slot<city_axis_clocks.size()) {
            // Evaluate one Original float addition from the period baseline,
            // avoiding repeated float-roundoff changing a patrol boundary.
            // Explicit velocity/position writes begin a new fractional segment.
            const auto axis=site==0x801E8E00?0u:1u,model=uint32_t(word(ram,actor+0x74));
            const auto lifetime=game_model_animation().lifetime_token(model);
            auto& old=city_axis_clocks[slot][axis];
            const auto current=std::bit_cast<int32_t>(ctx->f0.fl),velocity=std::bit_cast<int32_t>(ctx->f2.fl);
            if(!old||old->actor!=actor||old->physical!=physical||old->model!=model||old->lifetime!=lifetime||
               old->tick!=slice.tick||old->denominator!=slice.denominator||old->expected_phase!=slice.phase||
               old->expected!=current||old->velocity!=velocity)
                old=CityAxisClock{actor,physical,model,lifetime,slice.tick,slice.denominator,slice.phase,slice.phase,current,velocity,ctx->f0.fl};
            const auto end=slice.phase+slice.numerator;
            ctx->f0.fl=float(double(old->initial)+double(ctx->f2.fl)*double(end-old->start_phase)/double(slice.denominator));
            ctx->f2.fl=0;old->expected=std::bit_cast<int32_t>(ctx->f0.fl);old->expected_phase=end;
            return 1;
        }
        if(kind<71||kind>76||slot>=object_periodic_poses.size()||!owns_common_object(ram,actor))return 0;
        const auto& birth=object_periodic_poses[slot];
        return birth&&birth->actor==actor&&birth->physical==physical&&birth->update_tick==slice.tick&&
            birth->update_phase==slice.phase+slice.numerator&&(site==0x801E8E10||birth->reset_xz);
    }
    if((site==0x80223FBC||site==0x8022402C||site==0x80224074)&&object_call&&live(ram)&&active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();uint8_t* rdram=ram;
        const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,0x8021BCDC));
        const auto model=uint32_t(word(ram,actor+0x74)),slot=(actor-0x804B34B8u)/0xCC;
        const auto lifetime=game_model_animation().lifetime_token(model);
        if(slot>=object_periodic_poses.size()||!owns_common_object(ram,actor))throw std::logic_error("Invalid floating dialogue owner");
        auto& birth=object_periodic_poses[slot];
        if(site==0x80223FBC) {
            if(!birth||birth->actor!=actor||birth->physical!=physical||birth->model!=model||birth->lifetime!=lifetime)
                birth=ObjectPeriodicPose{actor,physical,model,lifetime,float(MEM_H(0x42,int32_t(physical))),
                    MEM_BU(0x28,int32_t(physical))==0};
            birth->update_tick=slice.tick;birth->update_phase=slice.phase+slice.numerator;birth->reset_xz=false;
            return 0;
        }
        if(site==0x80224074)return 1; // The fractional clock already wrote the end angle once.
        if(!birth)throw std::logic_error("Missing floating dialogue birth sample");
        const float end=game_model_animation().angle_short(ram,model,physical+0x42,10.f);
        float angle=birth->first_period?birth->angle:float(std::fmod(double(end)+350.,360.));
        union {uint64_t bits;double value;} radians;radians.bits=LD(0,int32_t(0x80224B10));
        ctx->f12.d=double(angle)*radians.value;
        if(slice.ends_tick())birth->first_period=false;
        return 1;
    }
    // These three Original dialogue actors share the same30-degree look/return
    // geometry and branch-delay yaw store as the scene's orbital actors.
    const auto turn_site=[&] {
        switch(site) {
        case 0x80223E5C:case 0x8023DE6C:return 0x801EEEF8u;
        case 0x80223E80:case 0x8023DE90:return 0x801EEF1Cu;
        case 0x80223F44:case 0x8023DEC4:return 0x801EEF50u;
        case 0x80223F64:case 0x8023DEE4:return 0x801EEF70u;
        case 0x80223F84:case 0x8023DF04:return 0x801EEF90u;
        case 0x80223FA8:case 0x8023DF28:return 0x801EEFB4u;
        default:return site;
        }
    }();
    if((site==0x801EEFC8||site==0x801EED78||site==0x801EEE00||site==0x801EEE40||site==0x801EF038||site==0x801EF080||
        turn_site==0x801EEEF8||turn_site==0x801EEF1C||turn_site==0x801EEF50||turn_site==0x801EEF70||turn_site==0x801EEF90||turn_site==0x801EEFB4)&&object_call&&live(ram)&&active_simulation_slice()) {
        const auto& slice=*active_simulation_slice();uint8_t* rdram=ram;
        const auto actor=uint32_t(word(ram,0x8021BDEC)),physical=uint32_t(word(ram,0x8021BCDC));
        const auto model=uint32_t(word(ram,actor+0x74));const auto slot=(actor-0x804B34B8u)/0xCC;
        if(slot>=object_periodic_poses.size()||!owns_common_object(ram,actor))throw std::logic_error("Invalid orbit owner");
        const auto lifetime=game_model_animation().lifetime_token(model);
        auto normalized=[](double value){value=std::fmod(value,360.);if(value<0)value+=360.;return float(value);};
        auto radians=[&](uint32_t address){union {uint64_t bits;double value;} angle;angle.bits=LD(0,int32_t(address));return angle.value;};
        if(site==0x801EEFC8) {
            auto& birth=object_periodic_poses[slot];
            if(!birth||birth->actor!=actor||birth->physical!=physical||birth->model!=model||birth->lifetime!=lifetime)
                birth=ObjectPeriodicPose{actor,physical,model,lifetime,float(MEM_H(0x36,int32_t(physical))),
                    MEM_BU(0x28,int32_t(physical))==0&&(game_model_animation().native_birth(model)||word(ram,actor+0x94)==1||word(ram,actor+0x94)==2)};
            birth->update_tick=slice.tick;birth->update_phase=slice.phase+slice.numerator;
            birth->reset_xz=word(ram,actor+0x94)==1&&MEM_H(0x42,int32_t(physical))==0;
            return 0;
        }
        if(site==0x801EED78) {
            const float angle=game_model_animation().angle_short(ram,model,physical+0x34,3.f);
            ctx->r3=MEM_H(0x34,int32_t(physical));ctx->r2=int32_t(physical);ctx->r16=int32_t(actor);
            ctx->f20.d=radians(0x802151E8);ctx->f22.fl=50.f;ctx->f12.d=double(angle)*ctx->f20.d;return 1;
        }
        if(site==0x801EEE00) {
            ctx->f12.d=double(game_model_animation().angle_short_sample(ram,model,physical+0x34))*ctx->f20.d;
            ctx->r16=int32_t(actor);return 1;
        }
        if(site==0x801EEE40) {
            ctx->f0.fl=normalized(game_model_animation().angle_short_sample(ram,model,physical+0x34)+90.);return 1;
        }
        if(site==0x801EF038) {
            const float angle=game_model_animation().angle_short(ram,model,physical+0x36,10.f);
            auto& birth=object_periodic_poses[slot];if(!birth)throw std::logic_error("Missing orbit birth sample");
            const float pose_angle=birth->first_period?birth->angle:normalized(double(angle)-10.);
            ctx->f12.d=double(pose_angle)*radians(0x802151F0);
            if(slice.ends_tick())birth->first_period=false;
            return 1;
        }
        if(site==0x801EF080)return 1; // Short angle was already advanced exactly once before the sine.
        if(turn_site==0x801EEEF8) {
            const float current=std::bit_cast<float>(word(ram,physical+4));
            recomp_context source;dora3::copy_recomp_context(source,*ctx);source.r5=int32_t(0x41F00000u);
            {SimulationSuspension geometry;dora3_s3_demo_801E88C4(ram,&source);}
            ctx->r5=source.r5;ctx->f0.fl=game_model_animation().scalar_latched_turn(ram,model,physical+4,current,
                float(std::remainder(double(source.f0.fl)-current,360.)));return 1;
        }
        if(turn_site==0x801EEF1C) {
            if(!slice.ends_tick())ctx->r2=1;
            else if(site!=turn_site&&!ctx->r2)object_command_at_end=true;
            return 0;
        }
        if(turn_site==0x801EEF50) {
            auto& turn=object_return_turns[slot];const auto run=game_model_animation().run_token(model);
            const auto target=word(ram,physical+0x58),yaw=word(ram,actor+0x10);
            if(!turn||turn->actor!=actor||turn->physical!=physical||turn->model!=model||turn->lifetime!=lifetime||
               turn->run!=run||turn->tick!=slice.tick||turn->denominator!=slice.denominator||turn->expected_phase!=slice.phase||
               turn->target!=target||turn->expected_yaw!=yaw) {
                recomp_context source;dora3::copy_recomp_context(source,*ctx);
                {SimulationSuspension geometry;dora3_s3_demo_801E8838(ram,&source);}
                turn=ObjectReturnTurn{actor,physical,model,lifetime,run,slice.tick,slice.denominator,slice.phase,target,yaw,int32_t(source.r2)};
            }
            ctx->r2=turn->direction;return 1;
        }
        if(turn_site==0x801EEF70) {
            ctx->f0.fl=game_model_animation().scalar_latched_turn(ram,model,actor+0x10,ctx->f12.fl,ctx->f22.fl*ctx->f20.fl);return 1;
        }
        if(turn_site==0x801EEF90) {
            if(!slice.ends_tick()) {
                // The skipped BC1F executes SWC1 f0,0x10(a0) in its delay
                // slot on both branch paths. Keep that continuous yaw store;
                // only the following state1 commit waits for the period end.
                MEM_W(0x10,int32_t(actor))=ctx->f0.u32l;
                return 1;
            }
            return 0;
        }
        if(turn_site==0x801EEFB4)if(auto& turn=object_return_turns[slot];turn) {
            turn->expected_yaw=word(ram,actor+0x10);turn->expected_phase=slice.phase+slice.numerator;
        }
        return 0;
    }
    if((site==0x8022D8F0||site==0x8022DB88)&&object_call&&live(ram)&&active_simulation_slice()) {
        // State2 keeps moving with its old velocity until the Source command
        // completes. Releasing the sprite and entering state1 earlier would
        // let the next fractional callback stop it within this same period.
        if(site==0x8022D8F0) {
            if(!active_simulation_slice()->ends_tick())ctx->r2=1;
            else if(!ctx->r2)object_command_at_end=true;
        } else object_command_at_end=active_simulation_slice()->ends_tick();
        return 0;
    }
    if(site==0x801DBD7C && camera_call)return 1;
    if(site==0x801E8C08 && object_call)return 1;
    if(site==0x801E8D88 && object_call)return !live(ram)||!active_simulation_slice();
    if(site==0x801E8C9C && object_call) {
        const auto actor=uint32_t(word(ram,0x8021BDEC));
        if(!live(ram)||!active_simulation_slice()||!owns_common_object(ram,actor))return 1;
        const auto flash_physical=uint32_t(word(ram,actor+0x98));uint8_t* flash_ram=ram;
        if(([&] {uint8_t* rdram=flash_ram;const auto kind=MEM_H(0x20,int32_t(flash_physical));return kind==129||kind==130||kind==131||standard_pickup_kind(kind);})()&&
           active_simulation_slice()->phase==0&&active_simulation_slice()->numerator<60) {
            // The Source submission ceiling is not a new discrete callback.
            // Its time is included once in this lifetime's next real interval.
            const auto slot=(actor-0x804B34B8u)/0xCC,model=uint32_t(word(ram,actor+0x74));
            const auto& slice=*active_simulation_slice();
            if(slot<reward_source_ceilings.size())reward_source_ceilings[slot]=RewardSourceCeiling{actor,flash_physical,model,
                game_model_animation().lifetime_token(model),epoch,slice.tick,slice.numerator,slice.denominator};
            return 1;
        }
        const auto birth_slot=(actor-0x804B34B8u)/0xCC;
        if(birth_slot<deferred_enemy_births.size())if(auto& pending=deferred_enemy_births[birth_slot];pending) {
            const auto birth=*pending;const auto& slice=*gameplay_slice;
            if(birth.epoch!=epoch||birth.denominator!=slice.denominator||
               !same_object(birth.owner,object_identity(ram,actor,0)))pending.reset();
            else {
                // An earlier reused slot starts at the next real Source pass,
                // not at a later Native substep of the current pass.
                if(slice_position(slice,false)<birth.deadline)return 1;
                object_clocks[birth_slot]=birth.owner;object_clocks[birth_slot].local=true;pending.reset();
            }
        }
        begin_object_scope(ram,actor);
        const auto slot=(actor-0x804B34B8u)/0xCC;uint8_t* rdram=ram;
        const auto physical=uint32_t(word(ram,actor+0x98)),model=uint32_t(word(ram,actor+0x74));
        if(slot<reward_pool_claims.size()&&(MEM_H(0x20,int32_t(physical))==129||MEM_H(0x20,int32_t(physical))==130||MEM_H(0x20,int32_t(physical))==131||standard_pickup_kind(MEM_H(0x20,int32_t(physical)))))
            reward_pool_claims[slot]=RewardPoolClaim{actor,physical,model,game_model_animation().lifetime_token(model),epoch};
        if(slot<flash_source_periods.size()&&MEM_H(0x20,int32_t(physical))==130)flash_source_begin(ram,actor,physical,model,slot,slot09_source_pass);
        if(slot<pickup_scatter_source_periods.size()&&standard_pickup_kind(MEM_H(0x20,int32_t(physical)))&&word(ram,actor+0x94)==3)
            pickup_scatter_begin(ram,actor,physical,model,slot,slot09_source_pass);
        if(slot<pickup_settle_source_periods.size()&&standard_pickup_kind(MEM_H(0x20,int32_t(physical)))&&word(ram,actor+0x94)==2)
            pickup_settle_begin(ram,actor,physical,model,slot,slot09_source_pass);
        if(slot<pickup_reveal_source_periods.size()&&standard_pickup_kind(MEM_H(0x20,int32_t(physical)))&&word(ram,actor+0x94)==4)
            pickup_reveal_begin(ram,actor,physical,model,slot,slot09_source_pass);
        if(slot<pickup_arc_source_periods.size()&&standard_pickup_kind(MEM_H(0x20,int32_t(physical)))&&word(ram,actor+0x94)==8)
            pickup_arc_begin(ram,actor,physical,model,slot,slot09_source_pass);
        if(slot<pickup_spin_source_periods.size()&&standard_pickup_kind(MEM_H(0x20,int32_t(physical)))&&word(ram,actor+0x94)==7)
            pickup_spin_begin(ram,actor,physical,model,slot,slot09_source_pass);
        if(slot<reward_source_periods.size()&&reward_sequence_state(ram,actor,physical)) {
            const auto& slice=*active_simulation_slice();const auto fields=reward_source_fields(ram,actor,physical);
            const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model),
                       time_command=game_model_animation().time_command_token(model);
            const auto model_time=word(ram,model+0x64);const auto model_flags=MEM_BU(0x7C,int32_t(model));
            auto& prior=reward_source_periods[slot];
            if(!prior||prior->actor!=actor||prior->physical!=physical||prior->model!=model||prior->lifetime!=lifetime||
               prior->epoch!=epoch||prior->tick!=slice.tick||prior->denominator!=slice.denominator||prior->expected_phase!=slice.phase)
                prior=RewardSourcePeriod{actor,physical,model,MEM_H(0x20,int32_t(physical)),word(ram,actor+0x94),
                    std::bit_cast<float>(uint32_t(word(ram,physical+0x14))),lifetime,run,time_command,epoch,slice.tick,slice.denominator,slice.phase,slice.phase,slice.phase,
                    slot09_source_pass,fields,fields,model_time,model_time,word(ram,model+0x78),word(ram,model+0x78),
                    word(ram,model+0x74),word(ram,model+0x74),model_flags,model_flags,false};
            else {
                for(unsigned i=0;i<fields.size();++i)if(prior->expected[i]!=fields[i]) {
                    prior->initial[i]=fields[i];if(i==0)prior->start_phase=slice.phase;if(i==4)prior->start_scale_phase=slice.phase;
                }
                if(prior->time_command!=time_command||prior->expected_model_time!=model_time)prior->initial_model_time=model_time;
                if(prior->time_command!=time_command||prior->expected_model_flags!=model_flags)prior->initial_model_flags=model_flags;
                if(prior->expected_model_opacity!=word(ram,model+0x78))prior->initial_model_opacity=word(ram,model+0x78);
                if(prior->expected_model_render_flag!=word(ram,model+0x74))prior->initial_model_render_flag=word(ram,model+0x74);
                prior->run=run;prior->time_command=time_command;
            }
            // Original changes vertical velocity before the common tail.
            // Predict its pure motion without publishing the ordered command.
            if(standard_pickup_kind(prior->kind)) {
                const auto y=std::bit_cast<float>(uint32_t(prior->initial[0]));
                if(prior->state==5&&MEM_H(0x34,int32_t(physical))==0&&y>=250.f)prior->velocity=0;
                if(prior->state==6) {
                    const auto scale=std::bit_cast<float>(uint32_t(prior->initial[4]));
                    const auto decrement=std::bit_cast<double>(uint64_t(LD(int32_t(0x802116B0u),0)));
                    if(float(double(scale)-decrement)<=2.f)prior->velocity=0;
                }
            }
        }
        if(slot<pickup_idle_motion_source_periods.size()&&pickup_idle_motion_eligible(rdram,actor,physical))
            pickup_idle_motion_begin(ram,actor,physical,model,slot,slot09_source_pass);
        if(slot<pickup_idle_source_periods.size()&&standard_pickup_kind(MEM_H(0x20,int32_t(physical)))&&word(ram,actor+0x94)==1&&!pickup_idle_motion_eligible(rdram,actor,physical)) {
            const auto& slice=*active_simulation_slice();
            const auto fields=pickup_idle_source_fields(ram,actor,physical);
            const auto lifetime=game_model_animation().lifetime_token(model),run=game_model_animation().run_token(model);
            auto& prior=pickup_idle_source_periods[slot];
            if(!prior||prior->actor!=actor||prior->physical!=physical||prior->model!=model||prior->lifetime!=lifetime||
               prior->run!=run||prior->epoch!=epoch||prior->tick!=slice.tick||prior->denominator!=slice.denominator||
               prior->expected_phase!=slice.phase||prior->expected!=fields)
                prior=PickupIdleSourcePeriod{actor,physical,model,lifetime,run,epoch,slice.tick,slice.denominator,slice.phase,slice.phase,slot09_source_pass,fields,fields};
        }
        if(slot<scene_frame_entered.size())scene_frame_entered[slot]=true;
        return 0;
    }
    if(site==0x801D1160 && projectile_call) {
        if(!live(ram)||!active_simulation_slice())return 1;
        // These Source-boundary conversions create a successor which Original
        // dispatches later in this same pool pass. Apply its initial command
        // once at this timestamp, without replaying the preceding interval.
        const auto actor=uint32_t(ctx->r17),physical=uint32_t(ctx->r16);uint8_t* rdram=ram;
        const auto kind=MEM_H(0x20,int32_t(physical));
        if(active_simulation_slice()->ends_tick() && (kind==89 || !word(ram,actor+0x94)))
            initialize_player_projectiles(ram,ctx);
        return !live(ram)||!active_simulation_slice();
    }
    if(site==0x801D1130 && projectile_call) {
        const auto actor=uint32_t(ctx->r17);
        const auto slot=(actor-0x804B24C8u)/0xCC;
        // A hit creates its effect at the end of the movement slice. Never
        // advance that newborn over the interval preceding its creation,
        // even when its free pool slot follows the parent in this pass.
        if(!live(ram)||!active_simulation_slice()||slot>=projectile_before.size()||!owns_projectile(ram,actor))return 1;
        register_projectile_birth(ram,actor);
        if(!projectile_before[slot] || projectile_before[slot]!=uint32_t(word(ram,actor+0x98)) ||
           projectile_before_lifetimes[slot]!=game_model_animation().lifetime_token(uint32_t(word(ram,actor+0x74))))return 1;
        begin_projectile_scope(ram,actor);
        return 0;
    }
    if(!live(ram))return 0;
    switch(site) {
    case 0x801BB080:
        // Commit elapsed native time before source-rate scripts change shared
        // positions, velocities, ownership, or contact flags. Those commands
        // take effect at their timestamp, never retroactively over the gap
        // since the last variable frame. The update preserves caller context.
        native_gameplay_preview_update(ram,ctx,monotonic_time());
        motion_allowed=camera_allowed=object_allowed=projectile_allowed=false;break;
    case 0x801D10D8:if(!projectile_call) {
        projectile_allowed=true;
        // Original AI commands have just run. Reassert a held attachment at
        // this same timestamp without advancing its clip or clocks again.
        auto reassert=[&] {
            struct GeometryOnly {
                bool saved=interaction_geometry_only;
                GeometryOnly(){interaction_geometry_only=true;}
                ~GeometryOnly(){interaction_geometry_only=saved;}
            } geometry;
            for(unsigned i=10;i<30;++i) {
                const auto actor=0x804B1CD0u+i*0xCC;
                if(!word(ram,actor+0x94)||!owns_projectile(ram,actor))continue;
                const auto physical=uint32_t(word(ram,actor+0x98));uint8_t* rdram=ram;
                if(MEM_H(0x20,int32_t(physical))==105&&MEM_H(0x34,int32_t(physical))==2)
                    update_interaction(ram,ctx,actor,physical);
            }
        };
        if(active_simulation_slice()){SimulationSuspension commands;reassert();}
        else reassert();
    }break;
    case 0x801D1130:return owns_projectile(ram,uint32_t(ctx->r17));
    case 0x801E8BDC:if(!object_call)object_allowed=true;break;
    case 0x801E8C9C: {
        uint8_t* rdram=ram;
        const auto actor=uint32_t(word(ram,0x8021BDEC));
        const auto slot=(actor-0x804B34B8u)/0xCC;
        if(slot<slot09_source_handoffs.size())if(auto& pending=slot09_source_handoffs[slot];pending) {
            const auto handoff=*pending;pending.reset();
            const auto physical=uint32_t(word(ram,actor+0x98)),model=uint32_t(word(ram,actor+0x74));
            if(handoff.actor==actor&&handoff.physical==physical&&handoff.model==model&&
               game_model_animation().lifetime_token(model)==handoff.lifetime&&word(ram,actor+0x94)==2&&
               dora3_overlay_loaded(0x001398A0u)&&pointer(physical,0xF4)&&
               MEM_H(0x20,int32_t(physical))==27&&uint32_t(word(ram,0x80202064u+27u*84))==slot09_callback(27)) {
                // Native already executed the wait's Source decision and
                // dialogue command. Consume exactly that next Source actor
                // pass, then its unreviewed state2 starts on the following
                // Source update. A new lifetime never inherits the handoff.
                return 1;
            }
        }
        const auto physical=uint32_t(word(ram,actor+0x98));
        if(pointer(physical,0xF4)&&pickup_idle_motion_eligible(rdram,actor,physical))return 0;
        if(slot<pickup_idle_motion_source_periods.size()&&pickup_idle_motion_source_periods[slot]&&pickup_idle_motion_source_periods[slot]->common.replayed)return 0;
        if(pointer(physical,0xF4)&&standard_pickup_kind(MEM_H(0x20,int32_t(physical)))&&word(ram,actor+0x94)==3)return 0;
        if(slot<pickup_scatter_source_periods.size()&&pickup_scatter_source_periods[slot]&&pickup_scatter_source_periods[slot]->common.replayed)return 0;
        if(pointer(physical,0xF4)&&standard_pickup_kind(MEM_H(0x20,int32_t(physical)))&&word(ram,actor+0x94)==2)return 0;
        if(slot<pickup_settle_source_periods.size()&&pickup_settle_source_periods[slot]&&pickup_settle_source_periods[slot]->common.replayed)return 0;
        if(pointer(physical,0xF4)&&standard_pickup_kind(MEM_H(0x20,int32_t(physical)))&&word(ram,actor+0x94)==4)return 0;
        if(slot<pickup_reveal_source_periods.size()&&pickup_reveal_source_periods[slot]&&pickup_reveal_source_periods[slot]->common.replayed)return 0;
        if(pointer(physical,0xF4)&&standard_pickup_kind(MEM_H(0x20,int32_t(physical)))&&word(ram,actor+0x94)==8)return 0;
        if(slot<pickup_arc_source_periods.size()&&pickup_arc_source_periods[slot]&&pickup_arc_source_periods[slot]->common.replayed)return 0;
        if(pointer(physical,0xF4)&&standard_pickup_kind(MEM_H(0x20,int32_t(physical)))&&word(ram,actor+0x94)==7)return 0;
        if(slot<pickup_spin_source_periods.size()&&pickup_spin_source_periods[slot]&&pickup_spin_source_periods[slot]->common.replayed)return 0;
        if(pointer(physical,0xF4)&&reward_sequence_state(ram,actor,physical))return 0;
        if(slot<reward_source_periods.size()&&reward_source_periods[slot]&&reward_source_periods[slot]->replayed)return 0;
        if(pointer(physical,0xF4)&&MEM_H(0x20,int32_t(physical))==130)return 0; // Replayed Source callback owns actual query/retirement.
        if(pointer(physical,0xF4)&&standard_pickup_kind(MEM_H(0x20,int32_t(physical))))
            return owns_common_object(ram,actor)&&reward_pool_claim_matches(ram,actor);
        return owns_common_object(ram,actor);
    }
    case 0x801C8B28:if(!motion_call) {
        // Original BB080 has just selected/dispatched NPC contacts. Only now
        // may any Native weapon consume this Source B sample, including
        // held-fire and release/charge modes and secret weapons.
        native_input_commit_source_b(ram);motion_allowed=true;return 1;
    }break;
    // Both the fallthrough and swimming BE5C branch reach BE84.
    // BE78 alone is bypassed when control+28==1 and mode!=4.
    case 0x801DBE84:if(!camera_call) {camera_allowed=true;return 1;}break;
    default:break;
    }
    return 0;
}
namespace {
// Original BB080 runs the player contact selector and DD8FC before C8B28.
// Native B edges arrive between those Source updates. Resolve that same
// contact before the weapon callback consumes the edge, using the Original
// collider and pool order rather than a separate talk radius or NPC list.
void dispatch_native_npc_action(uint8_t* rdram,recomp_context& context) {
    if(!(MEM_HU(0,int32_t(0x801A48EC))&0x4000))return;
    recomp_context command;dora3::copy_recomp_context(command,context);
    SimulationSuspension commands;
    dora3_s3_801C0624(rdram,&command);if(!command.r2)return;
    dora3_s3_801B922C(rdram,&command);if(command.r2)return;
    const auto player=uint32_t(word(rdram,0x8021D49C)),physical=uint32_t(word(rdram,player+0x98));
    // DCB78 clears D4 before Original's search. Scope each eligibility probe
    // so a new input edge does not clear unrelated Source collision records.
    struct FreshContactProbe {
        uint8_t* ram;uint32_t physical;int16_t previous;
        FreshContactProbe(uint8_t* buffer,uint32_t owner):ram(buffer),physical(owner) {
            uint8_t* rdram=ram;previous=MEM_H(0xD4,int32_t(physical));MEM_H(0xD4,int32_t(physical))=-1;
        }
        ~FreshContactProbe(){uint8_t* rdram=ram;MEM_H(0xD4,int32_t(physical))=previous;}
    };
    command.r4=int32_t(player);
    {FreshContactProbe fresh(rdram,physical);dora3_s3_801DCC1C(rdram,&command);}
    if(!command.r2)return;
    uint32_t selected=0;
    for(unsigned slot=30;slot<dora3::distance::common_end;++slot) {
        const auto actor=0x804B1CD0u+slot*0xCC;
        if(!word(rdram,actor+0x94))continue;
        const auto owner=uint32_t(word(rdram,actor+0x98));
        if(!pointer(owner,0xF4))continue;
        command.r4=int32_t(actor);
        {FreshContactProbe fresh(rdram,owner);dora3_s3_801DCC50(rdram,&command);}
        if(!command.r2)continue;
        const auto stack=command.r29;command.r29-=16;
        command.r4=int32_t(player);command.r5=int32_t(actor);
        command.r6=command.r29+8;command.r7=command.r29+12;
        dora3_s3_801DC880(rdram,&command);command.r29=stack;
        if(!command.r2)continue;
        selected=actor;
        const auto kind=MEM_H(0x20,int32_t(owner));
        if(kind<0||!pointer(0x80202060u+uint32_t(kind)*84,84))return;
        // Original DD17C ends the player search when this flag differs
        // from one. NPCs have flag zero: the first overlapping slot wins,
        // including a non-talk obstacle before another NPC.
        if(MEM_BU(0,int32_t(0x8020208C+kind*84))!=1)break;
    }
    if(!selected)return;
    const auto owner=uint32_t(word(rdram,selected+0x98));
    const auto parent_slot=MEM_H(0x7C,int32_t(owner));
    if(parent_slot<-1||parent_slot>=dora3::distance::common_end)return;
    const auto parent=parent_slot==-1?selected:0x804B1CD0u+uint32_t(parent_slot)*0xCC;
    if(!word(rdram,parent+0x94))return;
    const auto parent_physical=uint32_t(word(rdram,parent+0x98));
    if(!pointer(parent_physical,0xF4))return;
    const auto parent_kind=MEM_H(0x20,int32_t(parent_physical));
    if(parent_kind<0||!pointer(0x80202060u+uint32_t(parent_kind)*84,84)||
       !MEM_BU(0,int32_t(0x80202091+parent_kind*84)))return;
    const auto kind=MEM_H(0x20,int32_t(owner));
    if(kind>=111&&kind<=128)return; // Original D6=16 dispatches a pickup.
    command.r4=int32_t(player);command.r5=int32_t(selected);
    dora3_s3_801DCDF0(rdram,&command);if(!command.r2)return;
    struct SelectedBEdge {
        uint8_t* ram;int16_t previous;
        explicit SelectedBEdge(uint8_t* buffer):ram(buffer) {
            uint8_t* rdram=ram;previous=MEM_H(0,int32_t(0x801AC69C));
            MEM_H(0,int32_t(0x801AC69C))=int16_t(uint16_t(previous)|0x4000);
        }
        ~SelectedBEdge(){uint8_t* rdram=ram;MEM_H(0,int32_t(0x801AC69C))=previous;}
    } edge(rdram);
    // DD8FC/ DD510 keep the Original state gates, attachment parent,
    // overlay callback, dialogue initializer and player/world pause command.
    dora3_s3_801DD8FC(rdram,&command);
}
}

void native_gameplay_preview_update(uint8_t* rdram,recomp_context* original,Nanoseconds now) {
    dora3::graphics_profile::SampleScope graphics_sample(dora3::graphics_profile::Stage::ObjectTimelineSample); dora3::profile::Scope profile(dora3::profile::Tag::ObjectTimeline);
    if(!live(rdram))return;
    const auto elapsed=now-previous;if(elapsed<0)throw std::logic_error("Gameplay preview time moved backwards");
    dora3::profile::timeline_gap(uint64_t(elapsed));
    auto cursor=previous;previous=now;
    const auto clock=timeline;const auto generation=epoch;
    auto current=[&] {return generation==epoch && live(rdram);};
    // Only the timeline calls this predicate, outside the continuous closure:
    // input, simulation and temporary actor/camera scopes have all unwound.
    // Source BB080 also advances this timeline, outside FrameBridge::dispatch.
    // Deliver completed SP/DP work between whole slices, not after catch-up.
    bool checkpoint_due=true;
    auto current_at_boundary=[&] {
        if(!current())return false;
        if(checkpoint_due) {
            checkpoint_due=false;
            dora3_native_scheduler_checkpoint(rdram);
            return current(); // A resumed guest thread may revoke the session.
        }
        return true;
    };
    while(cursor<now && current()) {
    const auto end=native_input_next_sample(cursor,now);uint64_t advanced=0;
    dora3::profile::timeline_input_segment(end<now);
    clock->advance_while(end-cursor,[&](const SimulationSlice& slice) {
        checkpoint_due=true;
        dora3::profile::timeline_slice(slice.numerator);
        SimulationScope step(slice);recomp_context ctx;dora3::copy_recomp_context(ctx,*original);
        struct GameplaySliceScope {
            std::optional<SimulationSlice> saved=gameplay_slice;
            explicit GameplaySliceScope(const SimulationSlice& value) {
                scene_completion_slice.reset();
                reset_slots(scene_frame_completed);
                reset_slots(scene_frame_entered);
                gameplay_slice=value;
            }
            ~GameplaySliceScope(){scene_completion_slice.reset();gameplay_slice=saved;}
        } global_time(slice);
        NativeInputScope input(rdram,cursor+Nanoseconds(advanced/60));advanced+=slice.numerator;
        try {
            // Snapshot before all producers, including player actions. An
            // actor created during this interval cannot receive its past dt.
            projectile_birth_done.fill(0);
            projectile_birth_lifetimes.fill(0);
            projectile_start_births.fill(0);
            if(projectile_allowed)for(unsigned i=0;i<projectile_before.size();++i) {
                const auto actor=0x804B24C8u+i*0xCC;
                projectile_before[i]=word(rdram,actor+0x94)?uint32_t(word(rdram,actor+0x98)):0;
                projectile_before_lifetimes[i]=projectile_before[i]?game_model_animation().lifetime_token(uint32_t(word(rdram,actor+0x74))):0;
                if(projectile_before[i] && owns_projectile(rdram,actor)) {
                    const auto identity=projectile_identity(rdram,actor,slice_position(slice,false));
                    if(!same_projectile(projectile_clocks[i],identity))projectile_clocks[i]=identity;
                    if(MEM_BU(0xC9,int32_t(identity.physical))==1)projectile_clocks[i].origin+=slice.numerator;
                }
            }
            if(object_allowed) {
                for(auto& clock:object_clocks) {
                    if(!clock.local||!clock.lifetime)continue;
                    if(MEM_BU(0xC9,int32_t(clock.physical))==1)clock.origin+=slice.numerator;
                }
                game_model_animation().begin_source_pool();object_call=true;
                {dora3::graphics_profile::Scope common_scope(dora3::graphics_profile::Stage::NativeCommon);dora3_s3_801E8BDC(rdram,&ctx);}
                native_enemy_source_pool_end(rdram);object_call=false;
                if(!current() || !active_simulation_slice())return;
            }
            // Original BB080 skips player C8B28 while B8ED0's D494
            // fade is nonzero; the common object pool still runs.
            if(motion_allowed&&word(rdram,0x8021D494)==0) {
                if(slice.phase==0)player_source_positions=capture_shot_attachment(rdram);
                motion_call=true;
                ctx.r4=1; // Original controller index is one-based.
                {
                    struct PlayerInvocation {
                        std::optional<uint64_t> saved=active_player_epoch;
                        std::optional<SourceTimestamp> saved_stamp=active_player_command_stamp;
                        PlayerInvocation(){active_player_command_stamp=native_gameplay_preview_command_time();active_player_epoch=epoch;}
                        ~PlayerInvocation(){active_player_epoch=saved;active_player_command_stamp=saved_stamp;}
                    } invocation;
                    dispatch_native_npc_action(rdram,ctx);
                    if(current() && active_simulation_slice()) {dora3::graphics_profile::Scope player_scope(dora3::graphics_profile::Stage::NativePlayer);ctx.r4=1;dora3_s3_801C8B28(rdram,&ctx);}
                }
                motion_call=false;
                if(!current() || !active_simulation_slice())return;
                bool born=false;
                for(unsigned i=0;i<projectile_before.size();++i) {
                    const auto actor=0x804B24C8u+i*0xCC;
                    if(word(rdram,actor+0x94) && owns_projectile(rdram,actor) &&
                       (uint32_t(word(rdram,actor+0x98))!=projectile_before[i] ||
                        game_model_animation().lifetime_token(uint32_t(word(rdram,actor+0x74)))!=projectile_before_lifetimes[i]))born=true;
                }
                if(born) {
                    initialize_player_projectiles(rdram,&ctx);
                    if(!current() || !active_simulation_slice())return;
                    for(unsigned i=0;i<projectile_start_births.size();++i) {
                        if(!projectile_start_births[i])continue;
                        const auto actor=0x804B24C8u+i*0xCC;
                        if(!word(rdram,actor+0x94)||!owns_projectile(rdram,actor))continue;
                        const auto identity=projectile_identity(rdram,actor,0);
                        if(identity.lifetime!=projectile_start_births[i])continue;
                        // The zero cue was dispatched before player motion.
                        // Its initial command has now been applied; admit only
                        // that same lifetime to this interval's elapsed pass.
                        projectile_before[i]=identity.physical;
                        projectile_before_lifetimes[i]=identity.lifetime;
                    }
                }
                const auto player=uint32_t(word(rdram,0x8021D49C));const auto state=word(rdram,player+0x94);
                const auto model=uint32_t(word(rdram,player+0x74));
                const auto cue=player_shot_cue(state);
                if(cue && uses_source_attachment(rdram) && game_model_animation().frame_event(rdram,model,
                   std::bit_cast<float>(word(rdram,model+0x64)),cue->frame-3.f,cue->site^0x4000000000ull)) {
                    // The Original ctor reads the previous action interval's
                    // position. Use the action clock, including off-grid B
                    // entry, rather than the unrelated global tick grid.
                    player_action_positions=capture_shot_attachment(rdram);
                }
                if((state==24 || state==25 || state==27) && game_model_animation().frame_event(rdram,model,
                   std::bit_cast<float>(word(rdram,model+0x64)),18.f,state==27?0x20801C52F0ull:0x20801C4F2Cull)) {
                    // The reviewed first callback permits repetition from
                    // frame18. Consume its boundary after this pose advance,
                    // so an update ending exactly there includes the new shot.
                    // Later presses still follow the ordinary first callback.
                    dora3_s3_core_801C2798(rdram,&ctx);
                    if(!current() || !active_simulation_slice())return;
                    if(ctx.r2) {
                        const auto rebound=uint32_t(word(rdram,player+0x74));
                        const auto next_state=word(rdram,player+0x94);
                        // Share the generated .scene1 zero-cue cursor. A
                        // separate private key would let the next ordinary
                        // second callback execute this fresh frame0 again.
                        if((next_state==24 || next_state==25 || next_state==27) && game_model_animation().frame_event(rdram,rebound,
                           std::bit_cast<float>(word(rdram,rebound+0x64)),0.f,next_state==27?0x03801C535Cull:0x03801C4F9Cull,true)) {
                            dora3_s3_core_801C120C(rdram,&ctx);
                            if(!current() || !active_simulation_slice())return;
                            initialize_player_projectiles(rdram,&ctx);
                            if(!current() || !active_simulation_slice())return;
                            const auto mirror=uint32_t(word(rdram,0x8021D640));
                            MEM_W(0x10,int32_t(mirror))=word(rdram,player+0x10);
                        }
                    }
                }
                // Weapon13 deliberately holds frame18 while its interaction
                // owns the player. This is a Source-rate level poll, not a
                // second crossing or a new shot. The real handler releases
                // the pause and restores its original morph when busy clears.
                const bool paused_weapon_poll=(state==28 || state==29) && word(rdram,0x801B36E0)==13 &&
                    (MEM_BU(0x7C,int32_t(model))&8) && std::bit_cast<float>(word(rdram,model+0x64))==18.f && slice.ends_tick();
                if(cue && (paused_weapon_poll || game_model_animation().frame_event(rdram,model,
                   std::bit_cast<float>(word(rdram,model+0x64)),cue->frame,cue->site))) {
                    struct ShotPositions {
                        uint8_t* ram;std::optional<ShotAttachment> saved;
                        explicit ShotPositions(uint8_t* buffer):ram(buffer) {
                            if(!uses_source_attachment(ram))return;
                            const auto* source=player_action_positions && same_shot_attachment(ram,*player_action_positions)?&*player_action_positions:
                                player_source_positions && same_shot_attachment(ram,*player_source_positions)?&*player_source_positions:nullptr;
                            if(!source)return;
                            saved=capture_shot_attachment(ram);if(!saved)return;
                            uint8_t* rdram=ram;
                            for(unsigned i=0;i<2;++i)for(unsigned axis=0;axis<3;++axis)
                                MEM_W(0,int32_t(source->owners[i].actor+0x9C+4*axis))=source->positions[3*i+axis];
                        }
                        ~ShotPositions() {
                            if(!saved || !same_shot_attachment(ram,*saved,false))return;
                            uint8_t* rdram=ram;
                            for(unsigned i=0;i<2;++i)for(unsigned axis=0;axis<3;++axis)
                                MEM_W(0,int32_t(saved->owners[i].actor+0x9C+4*axis))=saved->positions[3*i+axis];
                        }
                    };
                    {ShotPositions source_attachment(rdram);
                    if(state==28 || state==29) {
                        struct CommandOnly {
                            bool saved=player_weapon_command_only;
                            CommandOnly(){player_weapon_command_only=true;}
                            ~CommandOnly(){player_weapon_command_only=saved;}
                        } commands;
                        dora3_s3_core_801C554C(rdram,&ctx);
                    } else dora3_s3_core_801C120C(rdram,&ctx);
                    }
                    if(!current() || !active_simulation_slice())return;
                    initialize_player_projectiles(rdram,&ctx);
                    if(!current() || !active_simulation_slice())return;
                    const auto mirror=uint32_t(word(rdram,0x8021D640));
                    MEM_W(0x10,int32_t(mirror))=word(rdram,player+0x10);
                }
            }
            if(projectile_allowed) {
                projectile_call=true;dora3_s3_801D10D8(rdram,&ctx);projectile_call=false;
                if(!current() || !active_simulation_slice())return;
            }
            if(motion_allowed) {
                // Ordinary cameras are expressed relative to this world origin.
                // The original world loop publishes it after player movement;
                // updating the player alone leaves a source-rate sawtooth in
                // both the actor and static-world view transforms.
                const auto player=uint32_t(word(rdram,0x8021D49C));
                ctx.f12.fl=std::bit_cast<float>(word(rdram,player+0x9C));
                ctx.f14.fl=std::bit_cast<float>(word(rdram,player+0xA0));
                ctx.r6=word(rdram,player+0xA4);
                dora3_s3_801BBDDC(rdram,&ctx);
            }
            if(camera_allowed&&live(rdram)) {
                camera_call=true;dora3_s3_801DBD58(rdram,&ctx);camera_call=false;
                if(!current() || !active_simulation_slice())return;
                for(unsigned i=0;i<2;++i) {ctx.r4=int32_t(0x801A99F8+i*0x34);dora3_s0_8002F064(rdram,&ctx);}
            }
        } catch(...) {dora3_native_object_scope_end();dora3_native_projectile_scope_end();motion_call=camera_call=object_call=projectile_call=false;throw;}
    },[](uint64_t){},current_at_boundary,[&](const SimulationSlice& slice) {
        if(object_allowed)register_object_clocks(rdram,slice);
        const auto rates=boundary_model_rates(rdram,slice);
        auto limit=game_model_animation().clip_boundary_limit(rdram,slice,[&](uint32_t model)->std::optional<float> {
            const auto found=rates.find(model);
            return found==rates.end()?std::nullopt:std::optional<float>{found->second};
        });
        unsigned split_reason=limit<slice.numerator?1:0;
        const auto before_object_periods=limit;
        if(object_allowed)for(const auto& clock:object_clocks) {
            if(!clock.local||!clock.lifetime||MEM_BU(0xC9,int32_t(clock.physical))==1)continue;
            const auto start=slice_position(slice,false);
            if(start>=clock.origin)limit=std::min(limit,slice.denominator-(start-clock.origin)%slice.denominator);
        }
        if(limit<before_object_periods)split_reason=2;
        const auto player=uint32_t(word(rdram,0x8021D49C));const auto state=word(rdram,player+0x94);
        const auto cue=player_shot_cue(state);
        const auto before_projectile_periods=limit;
        if(projectile_allowed)for(unsigned i=0;i<projectile_clocks.size();++i) {
            const auto actor=0x804B24C8u+i*0xCC;const auto& owner=projectile_clocks[i];
            if(!owner.physical || !word(rdram,actor+0x94) || !owns_projectile(rdram,actor) ||
               !same_projectile(owner,projectile_identity(rdram,actor,0)) || MEM_BU(0xC9,int32_t(owner.physical))==1)continue;
            const auto start=slice_position(slice,false);
            if(start>=owner.origin)limit=std::min(limit,slice.denominator-(start-owner.origin)%slice.denominator);
        }
        if(limit<before_projectile_periods)split_reason=3;
        const auto before_player_cues=limit;
        if(motion_allowed && cue)limit=std::min(limit,game_model_animation().frame_boundary_limit(
            rdram,slice,uint32_t(word(rdram,player+0x74)),cue->frame,3.f));
        if(motion_allowed && cue && uses_source_attachment(rdram))limit=std::min(limit,game_model_animation().frame_boundary_limit(
            rdram,slice,uint32_t(word(rdram,player+0x74)),cue->frame-3.f,3.f));
        if(motion_allowed && (state==24 || state==25 || state==27))limit=std::min(limit,game_model_animation().frame_boundary_limit(
            rdram,slice,uint32_t(word(rdram,player+0x74)),18.f,3.f));
        if(motion_allowed && state==64) {
            const auto character=MEM_BU(0,int32_t(0x801B374A));
            if(character==0 || character==5)for(float target:character==0?
                std::array<float,2>{48.f,318.f}:std::array<float,2>{111.f,336.f})
                limit=std::min(limit,game_model_animation().frame_boundary_limit(
                    rdram,slice,uint32_t(word(rdram,player+0x74)),target,3.f));
        }
        if(limit<before_player_cues)split_reason=4;
        dora3::profile::timeline_limit(split_reason,limit);
        return limit;
    });
    cursor=end;
    }
}
