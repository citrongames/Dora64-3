// Diagnostic CPU-only room handoff. Synthetic assets/presentation endpoints;
// the real first resource barrier drains both banks before this test stops.
#include "native_simulation_time.hpp"
#include "native_gameplay_preview.hpp"
#include "native_input_timeline.hpp"
#include "native_camera_timeline.hpp"
#include "native_model_animation.hpp"
#include "native_frame_bridge.hpp"
#include "native_frame_session.hpp"
#include "native_game_loop.hpp"
#include "native_present.hpp"
#include "frame_lights.hpp"
#include "player-update-functions.h"
#include <array>
#include <bit>
#include <cassert>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <set>
#include <vector>
using namespace dora3::native_time;
#include "player-fixture.hpp"
constexpr uint32_t other=0x80254000,asset=0x80100000,clip=0x80101000;
static bool preview_live=false;
static bool scene_loop_watch=false;
static unsigned scene_requested_fps=60,scene_builds=0,scene_receives=0,scene_barriers=0;
static std::array<FrameTicket,2> scene_tickets;
struct SceneResourceBoundary {};
extern "C" int fixture_real_loop_gameplay_active();
extern "C" int fixture_real_loop_owns_frame_banks();
extern "C" int fixture_real_loop_owns_fade();
extern "C" void fixture_real_loop_fade_command(uint8_t*,recomp_context*);
extern "C" void fixture_real_loop_resource_barrier(uint8_t*,recomp_context*);
extern "C" void dora3_s3_801BB080(uint8_t*,recomp_context*);
namespace dora3::native_time {const FrameBinding* fixture_real_active_frame_binding();}
unsigned dora3_requested_native_fps(){return scene_requested_fps;}
extern "C" void dora3_graphics_frame_build(uint8_t* rdram,recomp_context*) {
    assert(scene_loop_watch&&scene_builds<2);
    const auto* b=active_frame_binding();assert(b&&b->owner);
    scene_tickets[scene_builds++]=b->ticket;
    const auto task=dora3::frame::bank0+b->ticket.bank*dora3::frame::bank_stride;
    native_present_ledger().bind(b->ticket,*b->owner,task,b->ticket.bank?0x803DA800:0x803B5000,task+0xC0);
    recomp_context submit{};submit.r8=int32_t(task);dora3_native_loop_submit(rdram,&submit);
}
static void scene_retire(uint8_t* rdram,unsigned i) {
    const auto& ticket=scene_tickets[i];const auto task=dora3::frame::bank0+ticket.bank*dora3::frame::bank_stride;
    recomp_context completion{};completion.r16=int32_t(task);MEM_W(4,completion.r16)=0;
    dora3_native_loop_retire(rdram,&completion);assert(native_present_ledger().retire(task));
}
extern "C" void osRecvMesg_recomp(uint8_t* rdram,recomp_context* c) {
    assert(scene_loop_watch&&uint32_t(c->r4)==game_queue&&c->f_odd==&c->f0.u32h);
    c->f_odd[0]=0x12345678;uint32_t message=0;
    const auto task0=dora3::frame::bank0+scene_tickets[0].bank*dora3::frame::bank_stride;
    const auto task1=dora3::frame::bank0+scene_tickets[1].bank*dora3::frame::bank_stride;
    switch(scene_receives++) {
    case 0:message=0x802F0010;break;
    case 1:message=WakeMailbox::message_value;break;
    case 2:message=task0+0xC0;break;
    case 3:assert(MEM_W(0,int32_t(0x801AA618))==1&&!game_frame_bridge().drained());scene_retire(rdram,0);assert(native_present_ledger().host_read_done(scene_tickets[0]));message=0x802F0020;break;
    case 4:scene_retire(rdram,1);message=task1+0xC0;break;
    case 5:assert(MEM_W(0,int32_t(0x801AA618))==0&&!game_frame_bridge().drained());assert(native_present_ledger().host_read_done(scene_tickets[1]));message=0x802F0030;break;
    default:assert(false);
    }
    MEM_W(0,c->r5)=int32_t(message);c->r2=0;
}
struct DeathHandoff {};
struct DeathCommand {
    uint32_t site;SourceTimestamp stamp;int32_t a0,a1;
    bool operator==(const DeathCommand&) const=default;
};
struct DoorCommand {
    uint32_t site;SourceTimestamp stamp;std::array<int32_t,3> args;
    bool operator==(const DoorCommand&) const=default;
};
static bool door_watch=false;
static SourceTimestamp door_source_stamp{};
static std::vector<DoorCommand> door_commands;
static bool death_watch=false;
static std::optional<FadeTimeline> death_fade;
static std::vector<DeathCommand> death_commands;
static unsigned death_barriers=0,death_stream_frees=0;
static uint32_t death_transition_site=0;
static bool respawn_watch=false;
static std::set<std::pair<unsigned,int32_t>> respawn_resource_ids;
static std::array<uint32_t,2> death_owned_models{};
static recomp_func_t* death_handler=nullptr;
static SourceTimestamp death_source_stamp{};
static FadeState death_read_fade(uint8_t* rdram) {
    return {MEM_H(0,int32_t(0x801AA660)),MEM_B(0,int32_t(0x801A99A0)),MEM_BU(0,int32_t(0x80171580))};
}
static void death_write_fade(uint8_t* rdram,FadeState value) {
    MEM_H(0,int32_t(0x801AA660))=int16_t(value.alpha);MEM_B(0,int32_t(0x801A99A0))=int8_t(value.direction);
}
static unsigned flight_random_calls=0;
static bool flight_damage_watch=false;
static unsigned flight_damage_commands=0;
static bool native_damage_rumble_watch=false;
static std::array<unsigned,3> native_damage_rumble{};
static unsigned native_damage_rumble_commands=0;
static unsigned native_damage_effect_commands=0;
static unsigned decisions=0,actions=0,footsteps=0,sprite_commands=0;
static unsigned jumps=0,falls=0,landings=0;
static unsigned source_object_calls=0,object_blinks=0;
static bool script_objects=false;
static bool world_init_watch=false;
static bool city_loaded=false,city_watch=false,city_identity_verified=true;
static bool dialogue_actor_loaded=false,dialogue_actor_watch=false;
static bool npc_action_watch=false;
static uint32_t npc_action_actor=0;
static unsigned npc_action_sounds=0;

static bool patrol_watch=false;
static bool city_dialogue_watch=false;
static unsigned city_actual_constructor_cases=0;
static unsigned slot09_constructor_cases=0,slot09_constructor_skips=0;
static bool slot09_loaded=false;
static unsigned slot09_source_cases=0,slot09_native_cases=0,slot09_rejections=0,slot09_original_cases=0;
static unsigned slot09_yaw_samples=0;
static bool scene_frame_watch=false;
static std::optional<std::array<double,27>> scene_end_fields;
static std::array<double,27> scene_frame_fields(uint8_t* rdram) {
    const auto a=uint32_t(MEM_W(0,int32_t(0x8021D49C))),p=uint32_t(MEM_W(0x98,int32_t(a)));
    const auto b=uint32_t(MEM_W(0,int32_t(0x8021D640)));
    std::array<double,27> v{};
    v[0]=MEM_W(0x94,int32_t(a));v[1]=rf(rdram,a+0x9C);v[2]=rf(rdram,a+0xA0);v[3]=rf(rdram,a+0xA4);
    v[4]=rf(rdram,a+0x10);v[5]=rf(rdram,p);v[6]=rf(rdram,p+4);v[7]=rf(rdram,p+0x14);
    v[8]=rf(rdram,uint32_t(MEM_W(0x74,int32_t(a)))+0x64);v[9]=rf(rdram,uint32_t(MEM_W(0x78,int32_t(a)))+0x64);v[10]=rf(rdram,b+0x10);
    v[11]=rf(rdram,0x800DAE54);v[12]=rf(rdram,0x800FE878);v[13]=rf(rdram,0x800DD3F0);
    for(unsigned n=0;n<13;++n)v[14+n]=rf(rdram,0x801A99F8+4*n);
    return v;
}
static std::vector<uint32_t> scene_pending_passes;
static unsigned city_story_source_cases=0,city_story_native_cases=0,city_story_rejections=0,city_story_original_cases=0;
static unsigned city_special_source_cases=0,city_special_native_cases=0,city_special_rejections=0,city_special_original_cases=0,city_special_counter_samples=0;
static unsigned city_dialogue_contact_cases=0;
static unsigned city_dialogue_source_cases=0,city_dialogue_native_cases=0,city_dialogue_rejections=0,city_dialogue_yaw_samples=0,city_dialogue_original_ram_cases=0;
static bool linked_construct_watch=false;
static unsigned linked_constructor_cases=0;
static bool linked_watch=false,linked_original_reference=false;
static unsigned linked_source_cases=0,linked_native_cases=0,linked_rejections=0,linked_yaw_samples=0,linked_original_ram_cases=0;
static unsigned linked_contact_cases=0;
static const std::set<std::pair<unsigned,int32_t>> linked_resource_ids{{1,906},{1,917},{1,964},{1,997},{1,1001},{2,354},{2,907},{2,908},{2,909},{2,910},{2,911},{2,912},{2,913},{2,914},{2,915},{2,916},{2,918},{2,919},{2,920},{2,921},{2,922},{2,923},{2,924},{2,925},{2,965},{2,966},{2,967},{2,968},{2,969},{2,970},{2,998},{2,1002},{2,1003},{2,1004},{2,1005},{2,1006},{2,1007},{3,983},{3,984},{3,999},{3,1000},{3,1009},{3,1010},{4,802},{1,777},{1,803},{1,804},{1,810},{1,816},{1,817},{1,830},{1,831},{1,846},{1,880},{1,893},{2,97},{2,99},{2,778},{2,779},{2,780},{2,781},{2,782},{2,783},{2,784},{2,785},{2,786},{2,787},{2,788},{2,789},{2,790},{2,791},{2,792},{2,793},{2,794},{2,795},{2,818},{2,819},{2,820},{2,821},{2,822},{2,823},{2,824},{2,832},{2,833},{2,834},{2,835},{2,836},{2,837},{2,838},{2,839},{2,840},{2,841},{2,847},{2,848},{2,849},{2,850},{2,851},{2,852},{2,853},{2,881},{2,882},{2,883},{2,884},{2,885},{2,886},{2,887},{2,888},{2,894},{2,895},{2,896},{2,897},{2,898},{2,899},{2,900},{2,933},{3,796},{3,797},{3,798},{3,799},{3,800},{3,801},{3,805},{3,806},{3,807},{3,808},{3,809},{3,811},{3,812},{3,813},{3,814},{3,815},{3,825},{3,826},{3,827},{3,828},{3,829},{3,842},{3,843},{3,844},{3,845},{3,854},{3,855},{3,856},{3,857},{3,858},{3,889},{3,890},{3,891},{3,892},{3,901},{3,902},{3,903},{3,904},{3,905},{4,802},{1,859},{1,869},{2,860},{2,861},{2,862},{2,863},{2,864},{2,865},{2,866},{2,867},{2,868},{2,870},{2,871},{2,872},{2,873},{3,874},{3,875},{3,876},{3,877},{3,878},{4,879},{1,0x476},{2,0x477}};
static bool patrol_original_reference=false;
static unsigned patrol_original_ram_cases=0;
static unsigned patrol_source_cases=0,patrol_native_cases=0,patrol_rejections=0,patrol_position_samples=0;
static unsigned dialogue_actor_source_cases=0,dialogue_actor_native_cases=0,dialogue_actor_rejections=0,dialogue_actor_return_samples=0;
struct CityCommand {
    uint32_t site;SourceTimestamp stamp;std::array<int32_t,3> args;
    bool operator==(const CityCommand&) const=default;
};
static std::vector<CityCommand> city_commands;
static std::vector<CityCommand> linked_publications;
static SourceTimestamp city_source_stamp{};
static std::optional<SourceTimestamp> city_source_initial_stamp;

static unsigned city_source_cases=0,city_native_cases=0,city_sprite_commands=0,city_overlay_rejections=0;
static unsigned world_init_commands=0,world_source_cases=0,world_native_cases=0,orbit_source_cases=0,orbit_native_cases=0,orbit_return_samples=0;
static uint32_t terminal_model=0;
static unsigned terminal_deletes=0;
static double terminal_deleted_at=0;
static bool terminal_end_commit=false;
static unsigned impact_spawns=0,resource_kind=0;
static unsigned impact_commands=0;
static uint32_t bounce_physical=0;
static double bounced_at=-1,bounce_y=0;
static unsigned spawn_charge=0;
static bool capture_birth=false;
static float interaction_initial_pose=0;
static uint8_t interaction_initial_flags=0;
extern "C" int dora3_native_defer_player_shot();
static bool repeat_watch=false;
static std::vector<double> repeat_shots;
static bool weapon_watch=false;
static unsigned weapon_fires=0;
static double weapon_fired_at=0;
static bool initial_geometry_watch=false;
static std::optional<std::array<float,3>> initial_shot_position;
static std::array<int32_t,0xF4/4> initial_shot_physics{};
static double charge_born_at=0;
static double impact_born_at=0;
static std::array<unsigned,5> resource_references{};
static unsigned shot_clip_changes=0;
constexpr uint32_t prop=0x801734B8,prop_physics=0x80258000;
static std::array<unsigned,17> entered{};
static double landed_at=0;
static std::array<double,2> animations{};
static Nanoseconds input_now=0;
static uint32_t next_address=0x802A0000;
static std::set<uint32_t> allocations;
static const FrameBinding* binding=nullptr;
namespace dora3::native_time {
Nanoseconds monotonic_time(){return input_now;}
const FrameBinding* active_frame_binding(){return binding?binding:fixture_real_active_frame_binding();}
}
double simulation_stamp(bool end) {
    const auto slice=native_gameplay_preview_slice();
    return slice?double(slice->tick*slice->denominator+slice->phase+(end?slice->numerator:0))/(60.*second):double(input_now)/second;
}
extern "C" {
int dora3_native_loop_gameplay_active(){return scene_loop_watch?fixture_real_loop_gameplay_active():preview_live;}
int dora3_overlay_loaded(uint32_t rom) {
    if(rom==0x1398A0u)return slot09_loaded&&city_identity_verified;
    if(rom==0x132F80u)return city_loaded&&city_identity_verified;
    assert(rom==0xEDDB0u||rom==0x15E440u);return dialogue_actor_loaded&&city_identity_verified;
}
int dora3_native_loop_owns_frame_banks(){return scene_loop_watch?fixture_real_loop_owns_frame_banks():preview_live;}
int dora3_native_loop_owns_fade(){return scene_loop_watch?fixture_real_loop_owns_fade():preview_live;}
void dora3_native_loop_fade_command(uint8_t* rdram,recomp_context* c) {
    if(scene_loop_watch){fixture_real_loop_fade_command(rdram,c);return;}
    if(death_fade) {
        const auto stamp=native_gameplay_preview_command_time();assert(stamp&&stamp->nanoseconds<=input_now);
        death_write_fade(rdram,death_fade->before_command(death_read_fade(rdram),*stamp));
    }
}
void dora3_native_loop_resource_barrier(uint8_t* rdram,recomp_context* c) {
    if(scene_loop_watch) {
        assert(MEM_W(0,int32_t(0x802170EC))==2&&scene_builds==2&&MEM_W(0,int32_t(0x801AA618))==2);
        const auto before=*c;const auto scratch=MEM_W(-0x10,c->r29);
        fixture_real_loop_resource_barrier(rdram,c);
        assert(std::memcmp(&before,c,sizeof(*c))==0&&MEM_W(-0x10,c->r29)==scratch);
        assert(scene_receives==6&&game_frame_session().phase()==FrameSession::Phase::Original&&game_frame_bridge().drained()&&!native_present_ledger().occupied());
        ++scene_barriers;throw SceneResourceBoundary{};
    }
    // Graphics-bank/FIFO/context draining executes in the full loop fixture.
    // Here verify its timing before any of the real guest destructors.
    assert(death_watch);
    if(death_transition_site==0x801EF5B0)assert(c->r4==2&&(c->r5==1||c->r5==2));
    else assert(respawn_watch&&death_transition_site==0x801C9BE0);
    ++death_barriers;
    for(auto model:death_owned_models)assert(allocations.count(model));
    if(preview_live) {
        assert(active_simulation_slice());
        if(death_fade)death_write_fade(rdram,death_fade->sample(death_read_fade(rdram),input_now));
        cancel_active_simulation();native_gameplay_preview_reset(rdram);death_fade.reset();
    }
}
void dora3_s0_80030C44(uint8_t*,recomp_context* c) {
    assert(death_watch&&(uint32_t(c->r4)==0x802F0100u||c->r4==0));
    if(c->r4)++death_stream_frees;
}
bool hover_command_watch=false;
std::array<unsigned,2> hover_commands{};
bool special_command_watch=false;
std::array<unsigned,3> special_commands{};
void dora3_trace(uint8_t* rdram,recomp_context* c,uint32_t site,uint32_t kind) {
    // div2's thirteenth end is a rational timestamp. The integer-ns
    // flush may continue for40/60ns in the no-request control. Capture
    // the completed frame before that following interval begins.
    if(scene_frame_watch&&site==0x801E8BDC)if(const auto* s=active_simulation_slice();s&&s->tick==13&&s->phase==0) {
        assert(!scene_end_fields);scene_end_fields=scene_frame_fields(rdram);
    }
    if(scene_frame_watch&&active_simulation_slice()&&MEM_W(0,int32_t(0x802170EC))==1&&
        (site==0x801C8B28||site==0x801D10D8||site==0x801DBD58))scene_pending_passes.push_back(site);
    // D1498 is the actual Source script-command publication, including the
    // kind24 close path whose physics78 marker requests exactly one release.
    if(slot09_loaded&&linked_watch&&site==0x801D1498) {
        city_commands.push_back({site,native_gameplay_preview_command_time().value_or(city_source_initial_stamp.value_or(city_source_stamp)),{int32_t(c->r4),0,0}});
    }

    if((city_watch||(slot09_loaded&&linked_watch))&&((site==0x80028BC8&&city_watch&&uint32_t(c->r4)==prop)||site==0x801BA0C4||site==0x801BBD5C||site==0x801E6E6C||site==0x80028878||site==0x800288EC||site==0x801B91D4||site==0x801B713C||site==0x801BBD40)) {
        std::array<int32_t,3> args{};
        if(site==0x80028BC8||site==0x80028878)args={int32_t(c->r4),int32_t(c->r5),int32_t(c->r6)};
        else if(site==0x801BA0C4||site==0x801BBD40)args={int32_t(c->r4),int32_t(c->r5),0};
        else args={int32_t(c->r4),0,0};
        // Original evaluates its state2 first-command before the same
        // period's sprite advance. The CE0 bind initializes that interval;
        // the CD4 release and countdown events commit its completed end.
        const auto source_stamp=site==0x80028BC8&&uint32_t(c->r6)==0x80232CE0u&&city_source_initial_stamp?
            *city_source_initial_stamp:city_source_stamp;
        city_commands.push_back({site,native_gameplay_preview_command_time().value_or(source_stamp),args});
    }
    if((dialogue_actor_watch||patrol_watch)&&(site==0x801B50F4||site==0x80042838||(site==0x8002A328&&uint32_t(c->r4)==prop))) {
        const auto stamp=site==0x8002A328&&city_source_initial_stamp?*city_source_initial_stamp:city_source_stamp;
        city_commands.push_back({site,native_gameplay_preview_command_time().value_or(stamp),{int32_t(c->r4),int32_t(c->r5),int32_t(c->r6)}});
    }
    if(city_dialogue_watch&&site==0x8002A454)city_commands.push_back({site,native_gameplay_preview_command_time().value_or(city_source_initial_stamp.value_or(city_source_stamp)),{int32_t(c->r4),int32_t(c->r5),int32_t(c->r6)}});
    if(linked_watch&&(site==0x8002A328||site==0x8002A454)) {
        city_commands.push_back({site,native_gameplay_preview_command_time().value_or(city_source_initial_stamp.value_or(city_source_stamp)),{int32_t(c->r4),int32_t(c->r5),int32_t(c->r6)}});
    }
    if(world_init_watch&&kind==0x1000&&site==0x8002A328&&uint32_t(c->r4)==prop)++world_init_commands;
    if(door_watch&&kind==0x70A50 && (site==0x801BBCD8 || site==0x801BE5C8 || site==0x801BE568 ||
        site==0x801BA05C || site==0x801BA0C4 || site==0x801B8ED0 || site==0x80028878 || site==0x800288EC)) {
        std::array<int32_t,3> args{};
        if(site==0x801BBCD8 || site==0x80028878)args={int32_t(c->r4),int32_t(c->r5),int32_t(c->r6)};
        else if(site==0x801BE568 || site==0x801BA05C || site==0x801BA0C4)args={int32_t(c->r4),int32_t(c->r5),0};
        else if(site==0x801B8ED0)args={int32_t(c->r4),0,0};
        door_commands.push_back({site,native_gameplay_preview_command_time().value_or(door_source_stamp),args});
    }
    if(death_watch&&death_barriers&&(site==0x801C70C8||site==0x801C729C||site==0x801C73F0||site==0x801C875C)) {
        std::cerr<<"Old player second callback after resource barrier site="<<std::hex<<site<<std::dec<<" Native="<<preview_live<<'\n';std::abort();
    }
    if(death_watch&&(site==0x80028878 || site==0x800288EC || site==0x801B8ED0 || site==0x801EF5B0 || site==0x801C9BE0 || site==0x801BA084)) {
        const auto stamp=native_gameplay_preview_command_time().value_or(death_source_stamp);
        const auto a0=site==0x801C9BE0?0:int32_t(c->r4);
        const auto a1=site==0x801C9BE0||site==0x801BA084?0:int32_t(c->r5);
        death_commands.push_back({site,stamp,a0,a1});
        if(site==0x801EF5B0||site==0x801C9BE0)death_transition_site=site;
    }
    if(hover_command_watch && kind==0x70A50) {
        if(site==0x801C6E28)++hover_commands[0];
        if(site==0x801C8028)++hover_commands[1];
    }
    if(special_command_watch && kind==0x70A50) {
        if(site==0x801C02A8)++special_commands[0];
        if(site==0x801C0398)++special_commands[1];
        if(site==0x801C12F0)++special_commands[2];
    }
    if(site==0x8003196C)++flight_random_calls;
    if(kind==0x70A50 && (site==0x801CF7E4 || site==0x801CFDEC || site==0x801CC420 || site==0x801CD394 || site==0x801CD6C4 || site==0x801CC8F4 || site==0x801CDB1C || site==0x801CE0AC || site==0x801CE658 || site==0x801CBE40 || site==0x801D03D0 || site==0x801CEBE0 || site==0x801CF228) && weapon_watch) {
        ++weapon_fires;const auto* slice=active_simulation_slice();
        const auto actor=uint32_t(MEM_W(0,int32_t(0x8021D49C)));
        const auto state=MEM_W(0x94,int32_t(actor));
        const bool zero_start=dora3_native_defer_player_shot()&&(state==24||state==25||state==27);
        weapon_fired_at=slice?simulation_stamp(!zero_start):double(input_now)/second;
        if(repeat_watch)repeat_shots.push_back(weapon_fired_at);
    }
    if(kind==0x70A50 && site==0x801C8B28 && spawn_charge && active_simulation_slice() && active_simulation_slice()->ends_tick()) {
        const auto kind=spawn_charge;spawn_charge=0;
        charge_born_at=simulation_stamp(true);
        auto creation=*c;creation.f_odd=&creation.f0.u32h;
        (kind==101?dora3_s3_core_801CF7E4:kind==103?dora3_s3_core_801CFDEC:dora3_s3_core_801D03D0)(rdram,&creation);assert(creation.r2==1);
        if(kind==105) {const auto model=uint32_t(MEM_W(0x74,int32_t(0x801724C8)));interaction_initial_pose=rf(rdram,model+0x64);interaction_initial_flags=MEM_BU(0x7C,int32_t(model));}
    }
    if(kind==0x70A50 && site==0x801D05A8 && capture_birth) {
        capture_birth=false;MEM_H(0xD6,c->r5)=16;MEM_H(0xD4,c->r5)=30;
    }
    if(kind==0x70A50 && site==0x801CB400 && uint32_t(c->r4)==bounce_physical && bounced_at<0 && active_simulation_slice() && MEM_H(0x34,int32_t(bounce_physical))==2) {
        assert(rf(rdram,bounce_physical+0x14)==50);
        bounced_at=simulation_stamp(false);
        bounce_y=rf(rdram,0x801724C8+0xA0);
    }
    if(kind==0x70A50 && site==0x801BBE58 && active_simulation_slice()) {
        if(native_damage_rumble_watch) {
            if(c->r4!=native_damage_rumble[0] || c->r5!=native_damage_rumble[1] || c->r6!=native_damage_rumble[2]) {
                const auto actor=uint32_t(MEM_W(0,int32_t(0x8021D49C)));
                std::cerr<<"Damage rumble actual="<<c->r4<<","<<c->r5<<","<<c->r6<<" expected="<<native_damage_rumble[0]<<","<<native_damage_rumble[1]<<","<<native_damage_rumble[2]<<" state="<<MEM_W(0x94,int32_t(actor))<<" time="<<simulation_stamp(false)<<'\n';std::abort();
            }
            ++native_damage_rumble_commands;
        }
        else if(flight_damage_watch) {assert(c->r4==8 && c->r5==3 && c->r6==1);++flight_damage_commands;}
        else {assert(c->r4==8 && c->r5==2 && c->r6==2);++impact_commands;}
    }
    if(kind==0x1000 && site==0x8002E214)resource_kind=unsigned(c->r5);
    if(kind==0x1000 && site==0x8002A328 && uint32_t(c->r4)==0x801724C8 && c->r6==1)++shot_clip_changes;
    for(auto entry:{0x801CD994u,0x801CDDA8u,0x801CE43Cu,0x801CEA58u,0x801CEE80u,0x801D0964u,0x801CC180u,0x801CCE4Cu,0x801CC6BCu,0x801CF638u,0x801CFC54u})
        if(kind==0x70A50 && site==entry) {
            ++impact_spawns;const auto* slice=active_simulation_slice();
            impact_born_at=slice?simulation_stamp(true):double(input_now)/second;
        }
    if(kind==0x70A50 && site==0x801CBA4C && active_simulation_slice()) {
        ++impact_spawns;
        impact_born_at=simulation_stamp(true);
    }
    if(kind==0x1000 && site==0x8002A1B8) {
        if(initial_geometry_watch && preview_live && !active_simulation_slice() && uint32_t(c->r4)==0x801724C8u) {
            initial_shot_position=std::array<float,3>{rf(rdram,0x801724C8+0x9C),rf(rdram,0x801724C8+0xA0),rf(rdram,0x801724C8+0xA4)};
            const auto physical=uint32_t(MEM_W(0x98,int32_t(0x801724C8)));
            for(unsigned i=0;i<initial_shot_physics.size();++i)initial_shot_physics[i]=MEM_W(int32_t(4*i),int32_t(physical));
        }
        ++actions;
        if(uint32_t(c->r4)==player||uint32_t(c->r4)==other)
            animations[uint32_t(c->r4)==player?0:1]+=std::bit_cast<float>(uint32_t(c->r6))*simulation_ticks();
    }
    if(site==0x801C3638)++jumps;
    if(site==0x801C383C)++falls;
    if(site==0x8002A544 && uint32_t(c->r4)==prop)++object_blinks;
    if(site==0x8003B77C && uint32_t(c->r4)==terminal_model) {
        ++terminal_deletes;
        const auto* slice=active_simulation_slice();
        assert(bool(slice)==preview_live);
        if(slice && terminal_end_commit)assert(slice->ends_tick());
        terminal_deleted_at=slice ? simulation_stamp(terminal_end_commit) : double(input_now)/second;
    }
    if(site==0x801C3934) {
        ++landings;
        const auto* s=active_simulation_slice();
        landed_at=s?simulation_stamp(false):double(input_now)/second;
    }
    const std::array<uint32_t,6> starts{0x801C3BA8,0x801C3D14,0x801C3ED8,0x801C4038,0x801C41A4,0x801C436C};
    for(unsigned i=0;i<starts.size();++i)if(site==starts[i])++entered[i+11];
}
void fixture_linked_parent_publish_A328(uint8_t* rdram,recomp_context* c) {
    if(linked_watch)linked_publications.push_back({0x8002A328,native_gameplay_preview_command_time().value_or(city_source_initial_stamp.value_or(city_source_stamp)),{int32_t(c->r4),int32_t(c->r5),int32_t(c->r6)}});
    dora3_s0_8002A328(rdram,c);
}
void dora3_game_heap_alloc(uint8_t*,recomp_context* c) {
    const uint32_t size=uint32_t(c->r4);assert(size && size<4096);
    c->r2=int32_t(next_address);assert(allocations.insert(next_address).second);
    next_address+=(size+15)&~15u;
}
void dora3_s0_80033C50(uint8_t*,recomp_context* c) {assert(allocations.erase(uint32_t(c->r4))==1);}
void dora3_s0_80038B10(uint8_t* ram,recomp_context* c) {dora3_s0_80033C50(ram,c);}
void do_break(uint32_t) {std::abort();}
void switch_error(const char*,uint32_t,uint32_t) {std::abort();}
void dora3_s0_80028BC8(uint8_t* rdram,recomp_context* c) {
    ++sprite_commands;
    if(linked_watch||city_dialogue_watch)city_commands.push_back({0x80028BC8,native_gameplay_preview_command_time().value_or(city_source_initial_stamp.value_or(city_source_stamp)),{int32_t(c->r4),int32_t(c->r5),int32_t(c->r6)}});
    if(city_watch&&uint32_t(c->r4)==prop) {++city_sprite_commands;fixture_city_sprite_bind_80028BC8(rdram,c);}
}
void dora3_s0_80042838(uint8_t* rdram,recomp_context* c) {
    if(c->r6==0xCB)++footsteps;
    // The common-contact regression also executes the Original non-talk
    // damage/player sound. Only F6 is the reviewed kind77 NPC command here;
    // its actor/arguments and exactly-once count are asserted separately.
    if(npc_action_watch&&c->r6==0xF6) {
        assert(uint32_t(c->r4)==npc_action_actor&&c->r5==0&&c->r7==0&&MEM_W(0x10,c->r29)==0);
        ++npc_action_sounds;
    }
    else if(dialogue_actor_watch)assert(uint32_t(c->r4)==prop&&c->r5==0&&(c->r6==0xCF||c->r6==0xF6)&&c->r7==0&&MEM_W(0x10,c->r29)==0);
}
void dora3_s0_800428C8(uint8_t*,recomp_context*) {}
void dora3_s0_800444C0(uint8_t*,recomp_context* c) {
    if(slot09_loaded&&linked_watch)assert(c->r4==0xE6&&c->r5==1);
    else assert(city_watch&&c->r4==0xE6&&c->r5==1);
    c->r2=-1; // Checked sound-queue failure; real dialogue setup ignores the return.
}
void dora3_s0_80042894(uint8_t*,recomp_context*) {}
// Synthetic resource lookup only: real guest constructors, resource lists,
// model/channel binding and all actor/physics retirement stay in the closure.
void dora3_resource_by_id(uint8_t*,recomp_context* c) {
    if(linked_construct_watch) {
        if(c->r4!=0||!linked_resource_ids.count({resource_kind,int32_t(c->r5)})) {std::cerr<<"Linked unexpected resource group="<<c->r4<<" kind="<<resource_kind<<" id="<<c->r5<<'\n';std::abort();}
        c->r2=resource_kind==4?3:resource_kind==3?4:resource_kind==2?2:1;++resource_references[unsigned(c->r2)];return;
    }
    if(respawn_watch) {
        assert(c->r4==0&&respawn_resource_ids.count({resource_kind,int32_t(c->r5)}));
        c->r2=resource_kind==4?3:resource_kind==3?4:resource_kind==2?2:1;
        ++resource_references[unsigned(c->r2)];return;
    }
    if(c->r4!=0 || !((c->r5>=0x200 && c->r5<=0x300) || (resource_kind==3 && c->r5>=0x4F7 && c->r5<=0x506))) {
        std::cerr<<"Synthetic lookup group="<<c->r4<<" id="<<std::hex<<c->r5<<" kind="<<resource_kind<<'\n';std::abort();
    }
    assert(resource_kind>=1 && resource_kind<=4);
    c->r2=resource_kind==4?3:resource_kind==3?4:resource_kind==2?2:1;++resource_references[unsigned(c->r2)];
}
void dora3_s0_80030CC8(uint8_t*,recomp_context* c) {
    if(c->r4<1||c->r4>4)std::cerr<<"Unexpected resource release id="<<c->r4<<" ra="<<std::hex<<c->r31<<std::dec<<" death="<<death_watch<<" time="<<input_now<<'\n';
    assert(c->r4>=1 && c->r4<=4);
    auto& references=resource_references[unsigned(c->r4)];assert(references);--references;
}
// Actual E59BC/E5894 and contact-list reset execute in the closure.
// Unported visual effect allocation is an explicit failed-allocation boundary;
// its location, single command and gameplay recovery are still checked.
void dora3_s3_801E6968(uint8_t* rdram,recomp_context* c) {
    if(linked_construct_watch) {actual_dora3_s3_801E6968(rdram,c);return;}
    const auto actor=uint32_t(MEM_W(0,int32_t(0x8021D49C)));
    assert(uint32_t(c->r4)==0x801FC39Cu || uint32_t(c->r4)==0x801FC3E0u || uint32_t(c->r4)==0x801FC400u);
    assert(uint32_t(c->r5)==uint32_t(MEM_W(0x9C,int32_t(actor))));
    assert(uint32_t(c->r6)==uint32_t(MEM_W(0xA0,int32_t(actor))));
    assert(uint32_t(c->r7)==uint32_t(MEM_W(0xA4,int32_t(actor))));
    if(native_damage_rumble_watch)++native_damage_effect_commands;
    c->r2=0;
}
void dora3_s3_801E5A1C(uint8_t*,recomp_context*) {}
void dora3_s3_801E5B98(uint8_t*,recomp_context*) {}
void dora3_s3_801E5D10(uint8_t*,recomp_context*) {}
recomp_func_t* get_function(int32_t address) {
    if(uint32_t(address)==0x8022DD94u) {assert(city_loaded);return dora3_s8_level1_8022DD94;}
    if(uint32_t(address)==0x8022E6F4u) {assert(city_loaded);return dora3_s8_level1_8022E6F4;}
    if(uint32_t(address)==0x8022EFC4u) {assert(city_loaded);return dora3_s8_level1_8022EFC4;}
    if(uint32_t(address)==0x8022F8ACu) {assert(city_loaded);return dora3_s8_level1_8022F8AC;}
    if(uint32_t(address)==0x80230190u) {assert(city_loaded);return dora3_s8_level1_80230190;}
    if(uint32_t(address)==0x80230BB0u) {assert(city_loaded);return dora3_s8_level1_80230BB0;}
    if(uint32_t(address)==0x80231194u) {assert(city_loaded);return dora3_s8_level1_80231194;}
    if(uint32_t(address)==0x80231A18u) {assert(city_loaded);return dora3_s8_level1_80231A18;}
    if(uint32_t(address)==0x80232578u) {assert(city_loaded);return dora3_s8_level1_80232578;}
    if(uint32_t(address)==0x80223CD4u) {assert(dialogue_actor_loaded);return dora3_s7_level1_80223CD4;}
    if(uint32_t(address)==0x8023DD00u) {assert(dialogue_actor_loaded);return dora3_s9_level1_8023DD00;}
    if(uint32_t(address)==0x801E5D7Cu)return dora3_s3_core_801E5D7C;

    if(slot09_loaded&&uint32_t(address)==0x8022DFA4u)return patrol_original_reference?reference_dora3_s44_remaining_8022DFA4:dora3_s44_remaining_8022DFA4;
    if(slot09_loaded&&uint32_t(address)==0x8022EB94u)return patrol_original_reference?reference_dora3_s44_remaining_8022EB94:dora3_s44_remaining_8022EB94;
    if(slot09_loaded&&uint32_t(address)==0x8022F674u)return patrol_original_reference?reference_dora3_s44_remaining_8022F674:dora3_s44_remaining_8022F674;
    if(slot09_loaded&&uint32_t(address)==0x80230448u)return patrol_original_reference?reference_dora3_s44_remaining_80230448:dora3_s44_remaining_80230448;
    if(slot09_loaded&&uint32_t(address)==0x8022FE4Cu)return patrol_original_reference?reference_dora3_s44_remaining_8022FE4C:dora3_s44_remaining_8022FE4C;
    if(uint32_t(address)==0x801C489Cu)return dora3_s3_core_801C489C;
    if(uint32_t(address)==0x801C5254u)return dora3_s3_core_801C5254;
    if(uint32_t(address)==0x801CEBE0u)return dora3_s3_core_801CEBE0;
    if(uint32_t(address)==0x801CF228u)return dora3_s3_core_801CF228;
    if(uint32_t(address)==0x801C5390u)return dora3_s3_core_801C5390;
    if(uint32_t(address)==0x801C5680u)return dora3_s3_core_801C5680;
    if(uint32_t(address)==0x801CBE40u)return dora3_s3_core_801CBE40;
    if(uint32_t(address)==0x801D03D0u)return dora3_s3_core_801D03D0;
    if(uint32_t(address)==0x801C4DA8u)return dora3_s3_core_801C4DA8;
    if(uint32_t(address)==0x801C5024u)return dora3_s3_core_801C5024;
    if(uint32_t(address)==0x801CC8F4u)return dora3_s3_core_801CC8F4;
    if(uint32_t(address)==0x801CDB1Cu)return dora3_s3_core_801CDB1C;
    if(uint32_t(address)==0x801CE0ACu)return dora3_s3_core_801CE0AC;
    if(uint32_t(address)==0x801CE658u)return dora3_s3_core_801CE658;
    if(uint32_t(address)==0x801C49A0u)return dora3_s3_core_801C49A0;
    if(uint32_t(address)==0x801C4BBCu)return dora3_s3_core_801C4BBC;
    if(uint32_t(address)==0x801CC420u)return dora3_s3_core_801CC420;
    if(uint32_t(address)==0x801CD394u)return dora3_s3_core_801CD394;
    if(uint32_t(address)==0x801CD6C4u)return dora3_s3_core_801CD6C4;
    if(uint32_t(address)==0x801C44C0u)return dora3_s3_core_801C44C0;
    if(uint32_t(address)==0x801C46B0u)return dora3_s3_core_801C46B0;
    if(uint32_t(address)==0x801CF7E4u)return dora3_s3_core_801CF7E4;
    if(uint32_t(address)==0x801CFDECu)return dora3_s3_core_801CFDEC;
    if(uint32_t(address)==0x80223FBCu) {assert(dialogue_actor_loaded);return dora3_s7_level1_80223FBC;}
    if(uint32_t(address)==0x8023DF3Cu) {assert(dialogue_actor_loaded);return dora3_s9_level1_8023DF3C;}
    if(uint32_t(address)==0x8022E480u) {assert(city_loaded);return patrol_original_reference?reference_dora3_s8_level1_8022E480:dora3_s8_level1_8022E480;}
    if(uint32_t(address)==0x8022ED78u) {assert(city_loaded);return patrol_original_reference?reference_dora3_s8_level1_8022ED78:dora3_s8_level1_8022ED78;}
    if(uint32_t(address)==0x8022F648u) {assert(city_loaded);return patrol_original_reference?reference_dora3_s8_level1_8022F648:dora3_s8_level1_8022F648;}
    if(uint32_t(address)==0x8022FF44u) {assert(city_loaded);return patrol_original_reference?reference_dora3_s8_level1_8022FF44:dora3_s8_level1_8022FF44;}
    if(uint32_t(address)==0x80230830u) {assert(city_loaded);return patrol_original_reference?reference_dora3_s8_level1_80230830:dora3_s8_level1_80230830;}
    if(uint32_t(address)==0x802317CCu) {assert(city_loaded);return patrol_original_reference?reference_dora3_s8_level1_802317CC:dora3_s8_level1_802317CC;}
    if(uint32_t(address)==0x802322ECu) {assert(city_loaded);return patrol_original_reference?reference_dora3_s8_level1_802322EC:dora3_s8_level1_802322EC;}
    if(uint32_t(address)==0x80232BD0u) {assert(city_loaded);return patrol_original_reference?reference_dora3_s8_level1_80232BD0:dora3_s8_level1_80232BD0;}
    if(uint32_t(address)==0x80230F70u) {assert(city_loaded);return linked_original_reference?reference_dora3_s8_level1_80230F70:dora3_s8_level1_80230F70;}
    if(uint32_t(address)==0x8023098Cu) {assert(city_loaded);return linked_original_reference?reference_dora3_s8_level1_8023098C:dora3_s8_level1_8023098C;}
    if(uint32_t(address)==0x8022D938u) {assert(city_loaded);return dora3_s8_level1_8022D938;}
    if(uint32_t(address)==0x8022DBB0u) {assert(city_loaded);return dora3_s8_level1_8022DBB0;}
    if(uint32_t(address)==0x801E5D84u)return dora3_s3_core_801E5D84;
    if(uint32_t(address)==0x801EEFC8u)return dora3_s3_core_801EEFC8;
    if(uint32_t(address)==0x801EF2ECu)return dora3_s3_core_801EF2EC;
    if(uint32_t(address)==0x801EB864u&&(world_init_watch||linked_watch))return dora3_s3_core_801EB864;
    if(uint32_t(address)==0x801EB864u)return +[](uint8_t* rdram,recomp_context*) {
        assert(!active_simulation_slice());++source_object_calls;
        if(script_objects && input_now==second/2)wf(rdram,prop_physics+0x10,-4);
        if(script_objects && input_now==7*second/10)MEM_W(0x94,int32_t(prop+3*0xCC))=2;
        if(script_objects && input_now==13*second/10)MEM_W(0x94,int32_t(prop+3*0xCC))=0;
    };
    struct Handler {uint32_t address;recomp_func_t* function;bool first;};
#include "player-update-dispatch.hpp"
#include "terminal-effect-dispatch.hpp"
#include "flight-dispatch.hpp"
#include "growth-dispatch.hpp"
#include "ballistic-dispatch.hpp"
    for(const auto& h:terminals)if(uint32_t(address)==h.address)return h.function;
    for(const auto& h:flights)if(uint32_t(address)==h.address)return h.function;
    for(const auto& h:growths)if(uint32_t(address)==h.address)return h.function;
    for(const auto& h:ballistics)if(uint32_t(address)==h.address)return h.function;
    for(const auto& h:handlers)if(uint32_t(address)==h.address) {
        if(h.first)++decisions;
        if((death_watch||door_watch)&&h.first) {
            death_handler=h.function;
            return +[](uint8_t* rdram,recomp_context* c) {
                death_handler(rdram,c);
                // Source reference stops after actual scene commands. The
                // production Native C8F10 hook checks this before the body.
                if(!preview_live&&(!native_gameplay_preview_eligible(rdram)||(death_watch&&death_barriers)))throw DeathHandoff{};
            };
        }
        return h.function;
    }
    std::cerr<<"Unexpected player handler "<<std::hex<<uint32_t(address)<<'\n';std::abort();
}
}
std::array<uint32_t,2> models(uint8_t* rdram,recomp_context& c) {
    auto W=[&](uint32_t a)->int32_t& {return MEM_W(0,int32_t(a));};
    auto F=[&](uint32_t a,float f){wf(rdram,a,f);};
    auto B=[&](uint32_t a)->int8_t& {return MEM_B(0,int32_t(a));};
    auto H=[&](uint32_t a)->int16_t& {return MEM_H(0,int32_t(a));};
    B(asset+0x57)=0;B(asset+0x56)=3;B(asset+0x55)=3;
    W(asset+0x14)=0x100;W(asset+0x2C)=0x200;H(asset+0x2A)=1;W(asset+0x34)=0x280;W(asset+0x3C)=1;
    F(asset+0x100+0x1C,1);F(asset+0x100+0x20,1);F(asset+0x100+0x24,1);
    W(asset+0x280)=0x300;H(asset+0x300)=0;H(asset+0x302)=100;H(asset+0x304)=50;H(asset+0x306)=30;H(asset+0x30C)=-1;
    W(clip+0x14)=0x100;H(clip+0x1A)=2;W(clip+0x1C)=0x200;W(clip+0x24)=0x300;
    F(clip+0x100+0x1C,1);F(clip+0x100+0x20,1);F(clip+0x100+0x24,1);
    const uint32_t ch=clip+0x200,key=clip+0x300;
    W(ch)=4;F(ch+4,30);W(ch+0x10)=0;W(ch+0x14)=0;W(ch+0x18)=0;
    W(ch+28)=10;F(ch+32,30);W(ch+28+0x10)=0;W(ch+28+0x14)=1;
    W(key)=0;W(key+8)=2;W(key+12)=0x400;
    W(key+16)=0;W(key+24)=2;W(key+28)=0x440;
    F(clip+0x400,0);F(clip+0x404,0);F(clip+0x408,30);F(clip+0x40C,300);
    F(clip+0x440,0);F(clip+0x444,0);F(clip+0x448,30);F(clip+0x44C,1);
    W(0x801AEC00)=int32_t(clip);
    std::array<uint32_t,2> result{};
    for(unsigned i=0;i<2;++i) {
        c.r4=int32_t(asset);dora3_s0_8003B560(rdram,&c);result[i]=uint32_t(c.r2);
        c.r4=int32_t(result[i]);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
        W((i?other:player)+0x74)=int32_t(result[i]);W((i?other:player)+0xB4)=int32_t(0x80257000);
    }
    // Actor model slot1 supplies the original resume-morph source for13.
    W(player+0x78)=int32_t(result[1]);
    return result;
}
void poses(uint8_t* rdram,recomp_context& c,const std::array<uint32_t,2>& owners,Nanoseconds stamp,unsigned bank) {
    std::array<int32_t,2> saved{},times{};
    for(unsigned i=0;i<2;++i) {const auto bone=uint32_t(MEM_W(0x5C,int32_t(owners[i])));saved[i]=MEM_W(0x14,int32_t(bone));times[i]=MEM_W(0x64,int32_t(owners[i]));}
    MEM_W(0,int32_t(0x801A9544))=int(bank);FrameBinding frame{{{},bank,{0,73}},nullptr};binding=&frame;
    const auto context=c;
    {ModelFrameScope pose(rdram,&c,stamp,bank);
     for(unsigned i=0;i<2;++i) {
        const auto bone=uint32_t(MEM_W(0x5C,int32_t(owners[i])));
        assert(std::abs(rf(rdram,bone+0x14)-10*rf(rdram,owners[i]+0x64))<0.001f);
        assert(MEM_W(0x64,int32_t(owners[i]))==times[i]);
     }}
    binding=nullptr;assert(std::memcmp(&c,&context,sizeof(c))==0);
    for(unsigned i=0;i<2;++i)assert(MEM_W(0x14,MEM_W(0x5C,int32_t(owners[i])))==saved[i]);
}
#include "native-npc-interaction-check.inc"
#include "slot09-overlay-fixture.hpp"
int main(int argc,char** argv) {
    assert(argc==2);std::vector<uint32_t> image(0x400000/4);std::ifstream in(argv[1],std::ios::binary);
    in.read(reinterpret_cast<char*>(image.data()),0x400000);assert(in.gcount()==0x400000);
    unsigned native_moving_edge_cases=0,repeat_weapon_cases=0,player_flight_cases=0,flight_weapon_cases=0,flight_native_input_cases=0,flight_repeat_cases=0,flight_edge_cases=0,flight_damage_cases=0,ground_damage_cases=0,heavy_damage_cases=0,native_damage_cases=0,air_damage_cases=0,hazard_damage_cases=0,waiting_damage_cases=0,recovery31_cases=0,special_player_cases=0,native_special_cases=0,special_recovery_cases=0,native_special_recovery_cases=0,hover_source_cases=0,hover_native_cases=0,death_source_cases=0,respawn_recovery_cases=0,surface_source_cases=0,door_source_cases=0,surface_native_cases=0;
    unsigned cases=0,terminal_cases=0,flight_cases=0,growth_cases=0,ballistic_cases=0,phase_cases=0,bounce_cases=0,homing_cases=0,charge_cases=0,charge_birth_cases=0,interaction_cases=0,interaction_birth_cases=0,weapon_cases=0,moving_weapon_cases=0,native_weapon_cases=0;
    for(unsigned camera_mode:{12u,2u})for(unsigned divider:{2u,3u}) {
        std::array<double,6> baseline{};double baseline_peak=0,baseline_landed=0;
        std::array<std::array<double,27>,2> turn_baseline{};
        std::array<double,17> flight_edge_baseline{};
        for(unsigned fps:{60u,73u}) {
            auto memory=image;auto* rdram=reinterpret_cast<uint8_t*>(memory.data());seed(rdram,0,true);
            std::memset(rdram+(0x80171CD0&0xFFFFFF),0,230*0xCC);
            MEM_W(0x94,int32_t(player))=1;MEM_W(0,int32_t(0x8021D640))=int32_t(other);
            wf(rdram,player+0xA0,0);wf(rdram,physics+0x14,0);MEM_W(0x30,int32_t(control))=-1;
            wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;MEM_H(0x18,int32_t(control))=0;
            MEM_W(0,int32_t(0x801B36E0))=0;MEM_B(0,int32_t(0x801B36EC))=0;
            for(unsigned i=0;i<8;++i) {
                const auto slot=int32_t(0x801A44F0+i*20);MEM_W(0,slot)=i<2?2:0;MEM_W(12,slot)=i;MEM_W(16,slot)=int32_t(i?0x801BB848:0x801BB730);
            }
            for(auto p:{0x8021D484u,0x8021D494u,0x80218AD8u,0x801A9C88u})MEM_W(0,int32_t(p))=0;
            MEM_W(0,int32_t(0x801AA648))=MEM_W(0,int32_t(0x801A9500))=1;
            MEM_B(0,int32_t(0x801A48B4))=1;MEM_H(0,int32_t(0x80171560))=0;
            MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;MEM_B(4,int32_t(0x80171560))=0;
            recomp_context c{};c.r29=int32_t(0x80300000);c.f_odd=&c.f0.u32h;c.r4=1;
            game_model_animation().reset();allocations.clear();next_address=0x802A0000;
            const auto owners=models(rdram,c);c.r4=1;wf(rdram,params+0x10,15);
            MEM_W(0,int32_t(0x801A39DC))=int(divider);
            MEM_B(0,int32_t(0x801B361A))=int8_t(camera_mode);
            c.f12.fl=rf(rdram,player+0x9C);c.f14.fl=rf(rdram,player+0xA0);c.r6=MEM_W(0xA4,int32_t(player));
            dora3_s3_801BBDDC(rdram,&c);
            if(camera_mode==2)dora3_s3_801D7134(rdram,&c);
            c.r4=1;
            assert(native_gameplay_preview_eligible(rdram));preview_live=true;
            decisions=actions=footsteps=sprite_commands=jumps=falls=landings=0;animations={};dora3_native_camera_dispatch_reset();
            input_now=0;native_gameplay_preview_begin(rdram,0,divider);
            unsigned vi=0;const auto saved=c;
            // Source scripts, native props, source-owned AI, flipbooks and
            // player/camera share one timestamp. Source commands are applied
            // after flushing the previous native interval, without backdating.
            native_gameplay_preview_reset(rdram);preview_live=false;
            MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;
            MEM_B(3,int32_t(0x80171560))=0;input_now=0;
            dora3_controller_accumulate(rdram,&c);dora3_controller_latch(rdram,&c);
            wf(rdram,physics,0);wf(rdram,physics+0x14,0);
            MEM_W(0x94,int32_t(player))=1;
            c.r4=int32_t(owners[0]);dora3_s0_8003B744(rdram,&c);
            c.r4=int32_t(owners[0]);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
            // Controlled unreviewed AI uses kind7 in this synthetic table.
            // It retains the existing observed callback, while the reviewed
            // kind6 can now be tested against its actual Original body below.
            const auto unreviewed_callback=MEM_W(0,int32_t(0x80202064+7*84));
            MEM_W(0,int32_t(0x80202064+7*84))=int32_t(0x801EB864);
            std::array<uint32_t,4> object_models{};
            for(unsigned n=0;n<4;++n) {
                const auto actor=prop+n*0xCC,physical=prop_physics+n*0x100;
                std::memset(rdram+(actor&0xFFFFFF),0,0xCC);
                std::memset(rdram+(physical&0xFFFFFF),0,0x100);
                c.r4=int32_t(asset);dora3_s0_8003B560(rdram,&c);object_models[n]=uint32_t(c.r2);
                c.r4=int32_t(object_models[n]);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                MEM_W(0x74,int32_t(actor))=int32_t(object_models[n]);
                MEM_W(0xB4,int32_t(actor))=int32_t(0x80257000);
                MEM_W(0x94,int32_t(actor))=n==3?0:2;MEM_W(0x98,int32_t(actor))=int32_t(physical);
                MEM_H(0x20,int32_t(physical))=n==1?7:0;
                wf(rdram,actor+0x9C,float(500+n*200));wf(rdram,actor+0xA0,100);
                wf(rdram,physical+0x10,n==0?8:3);wf(rdram,physical+0x14,0);
                wf(rdram,physical+0x18,-2);wf(rdram,physical+0xCC,n==2?1.5f:0);
                MEM_B(0xC8,int32_t(physical))=n==0?1:(n==2?2:0);
                wf(rdram,physical+0xC4,n==2?1:0);
                MEM_H(0xD8,int32_t(physical))=n==0?9:0;
            }
            // Real sprite flipbook lives in slot2 of the first native prop.
            constexpr uint32_t sprite=0x801A9CB0,descriptor=0x80102000;
            MEM_B(0x6D,int32_t(prop))=5;MEM_W(0x7C,int32_t(prop))=0;
            MEM_W(0,int32_t(sprite))=int32_t(descriptor);
            MEM_B(4,int32_t(sprite))=2;MEM_B(5,int32_t(sprite))=0;MEM_B(6,int32_t(sprite))=1;
            MEM_W(0,int32_t(descriptor))=int32_t(descriptor+0x10);
            MEM_W(4,int32_t(descriptor))=int32_t(descriptor+0x20);
            for(unsigned n=0;n<4;++n) {
                MEM_B(int32_t(n),int32_t(descriptor+0x10))=int8_t(n==3?0xFF:n);
                MEM_B(int32_t(n),int32_t(descriptor+0x20))=int8_t(n==0?2:(n==1?3:1));
            }
            auto legacy_sprite=memory;auto* sprite_ram=reinterpret_cast<uint8_t*>(legacy_sprite.data());
            for(unsigned tick=0;tick<120/divider;++tick) {
                c.r4=int32_t(prop);c.r5=2;dora3_s0_80028C74(sprite_ram,&c);
            }
            preview_live=true;native_gameplay_preview_begin(rdram,0,divider);
            source_object_calls=object_blinks=0;script_objects=true;unsigned world_vi=0;
            for(unsigned frame=1;frame<=2*fps;++frame) {
                while(uint64_t(world_vi+1)*fps<=uint64_t(frame)*60) {
                    ++world_vi;input_now=second*world_vi/60;
                    dora3_controller_accumulate(rdram,&c);
                    if(world_vi%divider==0) {
                        dora3_controller_latch(rdram,&c);
                        const auto context=c;
                        dora3_native_preview_hook(rdram,&c,0x801BB080);
                        assert(std::memcmp(&c,&context,sizeof(c))==0);
                        dora3_s3_801E8BDC(rdram,&c);c.r4=1;dora3_s3_801C8B28(rdram,&c);
                        MEM_H(0,int32_t(0x8021BE00))=0x123; // Source-owned contact list must survive native passes.
                        dora3_s3_801DBD58(rdram,&c);
                    }
                }
                c=saved;c.f_odd=&c.f0.u32h;
                input_now=second*frame/fps;
                native_gameplay_preview_update(rdram,&c,input_now);
                assert(std::memcmp(&c,&saved,sizeof(c))==0);
                assert(MEM_HU(0,int32_t(0x8021BE00))==0x123 || world_vi<divider);
                poses(rdram,c,owners,input_now,frame&1);
            }
            script_objects=false;
            // Float RAM positions accumulate at variable partitions; bound
            // their roundoff separately from discrete counts and commands.
            const auto near=[](float a,double b) {return std::abs(double(a)-b)<0.03;};
            const double ticks=120./divider;
            if(!near(rf(rdram,prop+0x9C),500-ticks)||!near(rf(rdram,prop+0xA4),-2*ticks)||
               !near(rf(rdram,prop+0xCC+0x9C),700+3*ticks)||source_object_calls!=120/divider||object_blinks!=8) {
                std::cerr<<"World ownership/command partition mismatch divider="<<divider<<" fps="<<fps
                         <<" x="<<rf(rdram,prop+0x9C)<<" expected="<<500-ticks
                         <<" source_x="<<rf(rdram,prop+0xCC+0x9C)<<" source_calls="<<source_object_calls
                         <<" blink="<<object_blinks<<'\n';return 1;
            }
            assert(near(rf(rdram,prop_physics+0xC4),1) && near(rf(rdram,prop_physics+0x200+0xC4),0));
            assert(MEM_H(0xD8,int32_t(prop_physics))==1);
            assert(MEM_W(0x94,int32_t(prop+3*0xCC))==0);
            assert(near(rf(rdram,prop+3*0xCC+0x9C),1100+1.8*60/divider));
            assert(near(rf(rdram,object_models[3]+0x64),std::fmod(1.8*60/divider,30.)));
            // Compare with the isolated real Original flipbook at source ticks.
            for(unsigned n=4;n<=6;++n) {
                const auto actual=MEM_BU(int32_t(n),int32_t(sprite));
                auto* saved_ram=rdram;rdram=sprite_ram;
                const auto expected=MEM_BU(int32_t(n),int32_t(sprite));rdram=saved_ram;
                assert(actual==expected);
            }
            native_gameplay_preview_reset(rdram);preview_live=false;
            // Differential Original check for every reviewed common no-op
            // kind, against the frozen unpatched world dispatcher. Source AI
            // is a controlled observation boundary, with real common model,
            // opacity, blink and sprite functions on both sides.
            std::vector<unsigned> noop_kinds{0,1,2,3,4,5,8,10,11,12,23};
            for(unsigned kind=82;kind<=108;++kind)noop_kinds.push_back(kind);
            for(auto kind:noop_kinds) {
                MEM_H(0x20,int32_t(prop_physics))=int16_t(kind);
                MEM_H(0xD8,int32_t(prop_physics))=3;
                MEM_B(0xC8,int32_t(prop_physics))=1;wf(rdram,prop_physics+0xC4,0.25f);
                auto legacy_world=memory;auto* old_ram=reinterpret_cast<uint8_t*>(legacy_world.data());
                c=saved;c.f_odd=&c.f0.u32h;
                auto old_context=c;old_context.f_odd=&old_context.f0.u32h;
                reference_dora3_s3_801E8BDC(old_ram,&old_context);
                dora3_s3_801E8BDC(rdram,&c);
                if(memory!=legacy_world) {std::cerr<<"Original common world mismatch kind="<<kind<<'\n';return 1;}
            }
            MEM_W(0,int32_t(0x80202064+7*84))=unreviewed_callback;
            // Load only the checked slot09 ROM data for this block. Its VRAM
            // overlaps city slot1; restore the city bytes before city checks.
            const auto overlay_begin=memory.begin()+slot09_overlay_vram_offset/4;
            const std::vector<uint32_t> saved_slot09_image(overlay_begin,overlay_begin+slot09_overlay_words.size());
            std::copy(slot09_overlay_words.begin(),slot09_overlay_words.end(),overlay_begin);
            // Slot09 actual constructors: every reviewed room/quest branch,
            // including skipped spawns, compared with canonical frozen189.
            struct Slot09Case {unsigned kind;int scene;std::vector<unsigned> flags;};
            const std::vector<Slot09Case> slot09_cases{
                {24,0,{}},{24,0xCF,{}},{24,0xCF,{0x7B}},{24,0xF0,{}},{24,0xF0,{0x7B}},
                {25,0,{}},{25,0xA1,{}},{25,0xA1,{0x7A}},{25,0xC0,{}},
                {26,0,{}},{26,0xC1,{}},{26,0xC1,{0x7B}},{26,0xC1,{0x5C}},{26,0xC1,{0x7B,0x5C}},
                {26,0xAC,{}},{26,0xAC,{0x10}},{26,0xFD,{}},{26,0xFD,{0x2C}},
                {27,0,{}},{27,0,{0x2B}},{28,0,{}}
            };
            for(const auto& test:slot09_cases)for(bool gap:{false,true}) {
                const auto ctor_memory=memory;const auto ctor_heap=allocations;const auto ctor_next=next_address;const auto ctor_refs=resource_references;
                native_gameplay_preview_reset(rdram);preview_live=false;room(rdram);MEM_H(0,int32_t(0x801B36B8))=int16_t(test.scene);
                linked_construct_watch=linked_watch=true;city_loaded=false;city_watch=patrol_watch=city_dialogue_watch=false;
                city_source_stamp={0,0};city_source_initial_stamp.reset();city_commands.clear();
                for(unsigned slot=30;slot<230;++slot) {c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(0x80171CD0u+slot*0xCC);dora3_scene_object_init(rdram,&c);MEM_H(0xA8,int32_t(0x80171CD0u+slot*0xCC))=-1;}
                for(unsigned n=0;n<4;++n)MEM_W(0,int32_t(0x801B3628+4*n))=0;
                for(auto flag:test.flags) {c.r4=flag;dora3_s1_801B4FBC(rdram,&c);}
                if(gap)MEM_W(0x94,int32_t(prop))=99;
                MEM_H(0,int32_t(0x8021BE02))=-500;MEM_H(2,int32_t(0x8021BE02))=80;MEM_H(4,int32_t(0x8021BE02))=-300;MEM_H(6,int32_t(0x8021BE02))=359;
                MEM_W(0,int32_t(0x801AEC00+20))=int32_t(asset);MEM_W(0,int32_t(0x801AEC00+40))=int32_t(clip);MEM_W(0,int32_t(0x801AEC00+80))=int32_t(descriptor);
                const auto actor=gap?prop+0xCC:prop;const auto kind=test.kind;c=saved;c.f_odd=&c.f0.u32h;
                const auto before=memory;const auto before_context=c;const auto before_heap=allocations;const auto before_next=next_address;const auto before_refs=resource_references;
                auto construct=[&](bool reference) {switch(kind) {
                    case 24:if(reference)reference_dora3_s44_remaining_8022D6D0(rdram,&c);else dora3_s44_remaining_8022D6D0(rdram,&c);break;
                    case 25:if(reference)reference_dora3_s44_remaining_8022E0FC(rdram,&c);else dora3_s44_remaining_8022E0FC(rdram,&c);break;
                    case 26:if(reference)reference_dora3_s44_remaining_8022EC90(rdram,&c);else dora3_s44_remaining_8022EC90(rdram,&c);break;
                    case 27:if(reference)reference_dora3_s44_remaining_8022FF08(rdram,&c);else dora3_s44_remaining_8022FF08(rdram,&c);break;
                    case 28:if(reference)reference_dora3_s44_remaining_8022F758(rdram,&c);else dora3_s44_remaining_8022F758(rdram,&c);break;
                    default:std::abort();}};
                auto retire=[&] {if(MEM_W(0x94,int32_t(actor))) {c.r4=int32_t(actor);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);c.r4=int32_t(actor);dora3_scene_object_init(rdram,&c);}};
                construct(true);const auto expected=memory;const auto expected_context=c;const auto expected_heap=allocations;const auto expected_next=next_address;const auto expected_refs=resource_references;
                const auto expected_models=game_model_animation().model_count();const auto expected_commands=city_commands;
                retire();assert(allocations==before_heap&&resource_references==before_refs&&game_model_animation().model_count()==6);
                memory=before;c=before_context;allocations=before_heap;next_address=before_next;resource_references=before_refs;city_commands.clear();
                construct(false);assert(memory==expected&&std::memcmp(&c,&expected_context,sizeof(c))==0&&allocations==expected_heap&&next_address==expected_next&&resource_references==expected_refs&&game_model_animation().model_count()==expected_models&&city_commands==expected_commands);
                if(MEM_W(0x94,int32_t(actor))) {const auto pp=uint32_t(MEM_W(0x98,int32_t(actor)));assert(MEM_H(0x20,int32_t(pp))==int16_t(kind)&&game_model_animation().model_count()==7);}
                else {assert(allocations==before_heap&&resource_references==before_refs&&game_model_animation().model_count()==6);++slot09_constructor_skips;}
                retire();assert(allocations==before_heap&&resource_references==before_refs&&game_model_animation().model_count()==6);
                linked_construct_watch=linked_watch=false;memory=ctor_memory;allocations=ctor_heap;next_address=ctor_next;resource_references=ctor_refs;++slot09_constructor_cases;
            }

            // Slot09 actual idle/wait/patrol constructors and Source-only rejection states.
            for(unsigned kind:{24u,25u,26u,27u,28u})for(bool gap:{false,true})for(unsigned scenario=0;scenario<59;++scenario) {
                if((scenario!=20&&scenario!=21)||kind!=26)continue;
                if(scenario>=10&&scenario<20&&kind!=27&&kind!=28)continue;
                if(scenario>=20&&scenario<22&&kind!=26)continue;
                if(scenario>=22&&scenario<27&&kind!=24)continue;
                if(scenario>=27&&scenario<32&&kind!=25)continue;
                if(scenario>=32&&scenario<45&&kind!=26)continue;
                if(scenario>=45&&scenario<52&&kind!=24)continue;
                if(scenario>=52&&kind!=28)continue;
                if(scenario>=17&&kind==27)continue;
                const auto special_memory=memory;const auto special_heap=allocations;const auto special_next=next_address;const auto special_refs=resource_references;
                struct SpecialOriginal {std::vector<uint32_t> ram;recomp_context context;std::set<uint32_t> heap;uint32_t next;decltype(resource_references) refs;std::vector<CityCommand> commands;std::size_t models;};
                struct SpecialResult {bool first_continuous=false;std::array<double,2> initial{},first_end{};std::vector<std::array<double,27>> samples;std::vector<CityCommand> commands;std::vector<SpecialOriginal> original;};
                auto run_story=[&](bool native,unsigned render_fps,unsigned edge_mode=0,bool frozen=true,const SpecialResult* oracle=nullptr) {
                    scene_loop_watch=false;scene_frame_watch=false;scene_end_fields.reset();scene_pending_passes.clear();native_gameplay_preview_reset(rdram);preview_live=false;memory=special_memory;allocations=special_heap;next_address=special_next;resource_references=special_refs;input_now=0;
                    room(rdram);city_loaded=false;slot09_loaded=linked_construct_watch=linked_watch=true;MEM_H(0,int32_t(0x801B36B8))=0;city_identity_verified=scenario!=4;patrol_original_reference=frozen&&!native;linked_original_reference=false;
                    city_watch=patrol_watch=city_dialogue_watch=false;city_commands.clear();city_source_initial_stamp.reset();
                    for(unsigned slot=30;slot<230;++slot) {c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(0x80171CD0u+slot*0xCC);dora3_scene_object_init(rdram,&c);MEM_H(0xA8,int32_t(0x80171CD0u+slot*0xCC))=-1;}
                    MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;wf(rdram,physics,0);wf(rdram,physics+0x14,0);MEM_W(0x94,int32_t(player))=1;
                    wf(rdram,player+0x9C,-700);wf(rdram,player+0xA0,80);wf(rdram,player+0xA4,-50);
                    const Nanoseconds birth=scenario>=45?(scenario==46||scenario==48||scenario==53||scenario==55?173'000'000:73'000'000):scenario==5?73'000'000:scenario==6?173'000'000:0;
                    if(native&&birth) {preview_live=true;native_gameplay_preview_begin(rdram,0,divider);dora3_native_preview_hook(rdram,&c,0x801BB080);dora3_native_preview_hook(rdram,&c,0x801E8BDC);}
                    unsigned frame=0,edge=0;uint32_t parent=gap?prop+0xCC:prop,pp=0,pm=0,sm=0,child=0,cp=0,cm=0;SpecialResult result;
                    auto check_yaw=[&](Nanoseconds stamp) {
                        if(!oracle||!oracle->first_continuous||stamp<=birth||uint64_t(stamp-birth)*60>=uint64_t(second)*divider)return;
                        const double elapsed=double(uint64_t(stamp-birth)*60)/double(uint64_t(second)*divider);
                        for(unsigned axis=0;axis<2;++axis) {
                            const double delta=std::remainder(oracle->first_end[axis]-oracle->initial[axis],360.);
                            double expected=std::fmod(oracle->initial[axis]+delta*elapsed,360.);if(expected<0)expected+=360.;
                            const double actual=rf(rdram,axis?pp+4:parent+0x10);
                            if(std::abs(std::remainder(actual-expected,360.))>0.003) {
                                std::cerr<<"Slot09 intermediate yaw kind="<<kind<<" gap="<<gap<<" scenario="<<scenario<<" fps="<<fps<<" divider="<<divider<<" stamp="<<stamp<<" axis="<<axis<<" actual="<<actual<<" expected="<<expected<<'\n';std::abort();
                            }
                            ++slot09_yaw_samples;
                        }
                    };
                    auto advance=[&](Nanoseconds stamp) {while(second*(frame+1)/render_fps<stamp) {input_now=second*++frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);assert(std::memcmp(&c,&saved,sizeof(c))==0);check_yaw(input_now);}input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);assert(std::memcmp(&c,&saved,sizeof(c))==0);check_yaw(stamp);};
                    if(native&&birth)advance(birth);
                    MEM_W(0,int32_t(0x801B3628))=0;MEM_W(12,int32_t(0x801B3628))=0;
                    MEM_H(0,int32_t(0x8021BE02))=-500;MEM_H(2,int32_t(0x8021BE02))=80;MEM_H(4,int32_t(0x8021BE02))=-300;MEM_H(6,int32_t(0x8021BE02))=359;
                    MEM_W(0,int32_t(0x801AEC00+20))=int32_t(asset);MEM_W(0,int32_t(0x801AEC00+40))=int32_t(clip);MEM_W(0,int32_t(0x801AEC00+80))=int32_t(descriptor);
                    MEM_H(0x2A,int32_t(asset))=20;MEM_W(0x2C,int32_t(asset))=0x700;
                    for(unsigned n=0;n<20;++n) {const auto bind=asset+0x100+48*n;for(unsigned f=0;f<48;f+=4)MEM_W(0,int32_t(bind+f))=0;wf(rdram,bind+0x1C,1);wf(rdram,bind+0x20,1);wf(rdram,bind+0x24,1);MEM_W(0x28,int32_t(bind))=n&&n<19?1:0;MEM_W(0x2C,int32_t(bind))=n==0?1:0;}
                    MEM_W(0x34,int32_t(asset))=0x900;MEM_W(0,int32_t(asset+0x900))=0x980;MEM_H(0,int32_t(asset+0x980))=0;MEM_H(2,int32_t(asset+0x980))=100;MEM_H(4,int32_t(asset+0x980))=50;MEM_H(6,int32_t(asset+0x980))=30;MEM_H(12,int32_t(asset+0x980))=-1;
                    auto links=[&] {pp=uint32_t(MEM_W(0x98,int32_t(parent)));pm=uint32_t(MEM_W(0x74,int32_t(parent)));sm=uint32_t(MEM_W(0x78,int32_t(parent)));const int index=MEM_H(0x80,int32_t(pp));child=index>=30&&index<230?0x80171CD0u+unsigned(index)*0xCC:0;cp=child?uint32_t(MEM_W(0x98,int32_t(child))):0;cm=child?uint32_t(MEM_W(0x74,int32_t(child))):0;};
                    auto create=[&] {if(gap)MEM_W(0x94,int32_t(prop))=99;c=saved;c.f_odd=&c.f0.u32h;switch(kind) {
                    case 24:if(frozen&&!native)reference_dora3_s44_remaining_8022D6D0(rdram,&c);else dora3_s44_remaining_8022D6D0(rdram,&c);break;
                    case 25:if(frozen&&!native)reference_dora3_s44_remaining_8022E0FC(rdram,&c);else dora3_s44_remaining_8022E0FC(rdram,&c);break;
                    case 26:if(frozen&&!native)reference_dora3_s44_remaining_8022EC90(rdram,&c);else dora3_s44_remaining_8022EC90(rdram,&c);break;
                    case 27:if(frozen&&!native)reference_dora3_s44_remaining_8022FF08(rdram,&c);else dora3_s44_remaining_8022FF08(rdram,&c);break;
                    case 28:if(frozen&&!native)reference_dora3_s44_remaining_8022F758(rdram,&c);else dora3_s44_remaining_8022F758(rdram,&c);break;
default:std::abort();}if(gap)MEM_W(0x94,int32_t(prop))=0;links();assert(MEM_W(0x94,int32_t(parent))!=0&&MEM_H(0x20,int32_t(pp))==int16_t(kind));};
                    city_source_stamp={birth,0};input_now=birth;if(birth) {const auto units=uint64_t(birth)*60,den=uint64_t(divider)*second;const SimulationSlice born{1,den,units/den,units%den};SimulationScope born_scope(born);create();}else create();
                    auto retire=[&] {links();if(child) {c.r4=int32_t(child);dora3_s3_801E656C(rdram,&c);c.r4=int32_t(child);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);c.r4=int32_t(child);dora3_scene_object_init(rdram,&c);}c.r4=int32_t(parent);dora3_s3_801E656C(rdram,&c);c.r4=int32_t(parent);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);c.r4=int32_t(parent);dora3_scene_object_init(rdram,&c);child=cp=cm=0;};
                    const auto patrol_state=kind==24?2:kind==25||kind==26?101:kind==27?100:1;
                    if(scenario==1||scenario==8)MEM_W(0x94,int32_t(parent))=kind==24?1:kind==25||kind==26?100:kind==27?100:1;
                    if(scenario==2||scenario==9)MEM_W(0x94,int32_t(parent))=patrol_state;
                    if(scenario==3)MEM_W(0x94,int32_t(parent))=99;
                    if(scenario==7)MEM_W(0x94,int32_t(parent))=kind==24?3:kind==25||kind==27||kind==28?2:102;
                    if(scenario==8) {MEM_B(0x28,int32_t(pp))=1;MEM_H(0x34,int32_t(pp))=kind==27?49:39;}
                    if(scenario==9) {if(kind==24)wf(rdram,parent+0xA4,float(MEM_H(0xC2,int32_t(pp)))+120);else if(kind==25||kind==26)wf(rdram,parent+0x9C,float(MEM_H(0xBE,int32_t(pp)))+120);}
                    if(scenario>=10&&scenario<14) {
                        MEM_W(0x94,int32_t(parent))=2;MEM_B(0x28,int32_t(pp))=0;
                        MEM_H(0x44,int32_t(pp))=1;MEM_H(0x42,int32_t(pp))=1;
                        if(scenario==12||scenario==13) {
                            MEM_B(0x28,int32_t(pp))=1;MEM_H(0x34,int32_t(pp))=1;MEM_H(0x36,int32_t(pp))=1;
                            wf(rdram,parent+0x10,scenario==12?350:10);wf(rdram,pp+4,scenario==12?350:10);
                            wf(rdram,pp+0x58,scenario==12?120:240);wf(rdram,pp+0x5C,scenario==12?30:-30);
                            c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(parent);c.r5=0;c.r6=3;
                            dora3_s0_8002A328(rdram,&c);
                        }
                    }
                    MEM_B(0,int32_t(0x8021710D))=scenario==11||scenario==12||scenario==13?0:1;
                    if(scenario>=14&&scenario<20) {
                        for(unsigned off=0;off<16;off+=4)MEM_W(int32_t(off),int32_t(0x801B3628))=0;
                        auto quest=[&](unsigned flag) {const auto a=int32_t(0x801B3628+4*(flag/32));MEM_W(0,a)|=int32_t(1u<<(flag%32));};
                        uint32_t expected_descriptor=0;
                        if(kind==27) {
                            if(scenario==15||scenario==16)quest(0x2A);
                            if(scenario==16)quest(0x2B);
                            expected_descriptor=scenario==14?0x80232E34:scenario==15?0x80232E40:0x80232E5C;
                        } else {
                            if(scenario==15)quest(0x4B);
                            if(scenario==16||scenario==17)quest(0x4D);
                            if(scenario==17)for(unsigned flag:{0x5Du,0x54u,0x2Cu})quest(flag);
                            if(scenario==18)quest(0x4E);
                            if(scenario==19)quest(0x57);
                            expected_descriptor=scenario==14?0x80232CFC:scenario==15?0x80232D08:scenario==16?0x80232D6C:scenario==17?0x80232D90:scenario==18?0x80232DC4:0x80232DE4;
                        }
                        MEM_W(0,int32_t(0x8021BDEC))=int32_t(parent);MEM_W(0,int32_t(0x8021BCDC))=int32_t(pp);
                        MEM_W(0,int32_t(0x8021BCE4))=4;MEM_B(0,int32_t(0x8021710D))=0;
                        c=saved;c.f_odd=&c.f0.u32h;
                        if(kind==27) {if(frozen&&!native)reference_dora3_s44_remaining_8022FFE8(rdram,&c);else dora3_s44_remaining_8022FFE8(rdram,&c);}
                        else {if(frozen&&!native)reference_dora3_s44_remaining_8022F80C(rdram,&c);else dora3_s44_remaining_8022F80C(rdram,&c);}
                        assert(MEM_W(0x94,int32_t(parent))==2&&MEM_BU(0,int32_t(0x8021710D))==1);
                        unsigned contact_commands=0;
                        for(const auto& command:city_commands)if(command.site==0x801B91D4) {assert(uint32_t(command.args[0])==expected_descriptor);++contact_commands;}
                        assert(contact_commands==1);
                    }
                    if(scenario>=20&&scenario<22) {
                        MEM_H(0,int32_t(0x801B36B8))=0xFD;
                        for(unsigned off=0;off<16;off+=4)MEM_W(int32_t(off),int32_t(0x801B3628))=0;
                        if(scenario==21)MEM_W(8,int32_t(0x801B3628))=int32_t(1u<<(0x5B%32));
                        MEM_W(0x94,int32_t(parent))=102;MEM_B(0x28,int32_t(pp))=0;MEM_H(0x44,int32_t(pp))=0;
                        MEM_B(0,int32_t(0x8021710D))=1;
                    }
                    if(scenario==22||scenario==27||scenario==28) {
                        for(unsigned off=0;off<16;off+=4)MEM_W(int32_t(off),int32_t(0x801B3628))=0;
                        MEM_W(0x94,int32_t(parent))=scenario==22?2:scenario==27?1:101;
                        MEM_H(0,int32_t(0x801B36B8))=scenario==22?0:scenario==27?0xA1:0xC0;
                        if(kind==24)MEM_H(0x44,int32_t(pp))=3;
                        MEM_W(0,int32_t(0x8021BDEC))=int32_t(parent);MEM_W(0,int32_t(0x8021BCDC))=int32_t(pp);
                        MEM_W(0,int32_t(0x8021BCE4))=4;MEM_B(0,int32_t(0x8021710D))=0;
                        c=saved;c.f_odd=&c.f0.u32h;
                        if(kind==24) {if(frozen&&!native)reference_dora3_s44_remaining_8022D800(rdram,&c);else dora3_s44_remaining_8022D800(rdram,&c);}
                        else {if(frozen&&!native)reference_dora3_s44_remaining_8022E200(rdram,&c);else dora3_s44_remaining_8022E200(rdram,&c);}
                        assert(MEM_W(0x94,int32_t(parent))==(scenario==22?3:scenario==27?2:102)&&MEM_BU(0x28,int32_t(pp))==0&&MEM_BU(0,int32_t(0x8021710D))==1);
                        const auto descriptor=scenario==22?0x80232990u:scenario==27?0x80232A94u:0x80232B14u;
                        unsigned contact_commands=0;
                        for(const auto& command:city_commands)if(command.site==0x801B91D4) {assert(uint32_t(command.args[0])==descriptor);++contact_commands;}
                        assert(contact_commands==1);
                        if(scenario==22)assert(uint32_t(MEM_W(4,int32_t(0x801B3628)))&(1u<<(0x39%32)));
                        if(scenario==27)assert(uint32_t(MEM_W(0,int32_t(0x801B3628)))&(1u<<0x1A));
                    }
                    if(scenario>=22&&scenario<32) {
                        MEM_W(0x94,int32_t(parent))=kind==24?3:scenario>=28&&scenario!=29?102:2;
                        MEM_B(0x28,int32_t(pp))=0;MEM_H(0x34,int32_t(pp))=0;MEM_H(0x36,int32_t(pp))=0;
                        MEM_H(0x44,int32_t(pp))=scenario==23?6:3;
                        MEM_B(0,int32_t(0x8021710D))=scenario==24||scenario==25||scenario==26||scenario>=29?0:1;
                        if(kind==24)wf(rdram,pp+0x78,1);
                        if(scenario==25||scenario==26||scenario>=29) {
                            const bool negative=scenario==26||scenario==30;
                            MEM_B(0x28,int32_t(pp))=1;MEM_H(0x34,int32_t(pp))=1;MEM_H(0x36,int32_t(pp))=1;
                            wf(rdram,parent+0x10,negative?10:350);wf(rdram,pp+4,negative?10:350);
                            wf(rdram,pp+0x58,negative?240:120);wf(rdram,pp+0x5C,negative?-30:30);
                            c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(parent);c.r5=0;c.r6=kind==24?5:scenario==29?4:7;
                            dora3_s0_8002A328(rdram,&c);
                            c.r4=int32_t(parent);c.r5=0;c.r6=1;dora3_s0_8002A454(rdram,&c);
                        }
                    }
                    if(scenario>=32&&scenario<45) {
                        MEM_W(0x94,int32_t(parent))=102;MEM_B(0x28,int32_t(pp))=0;
                        MEM_H(0x34,int32_t(pp))=0;MEM_H(0x36,int32_t(pp))=0;MEM_H(0x44,int32_t(pp))=scenario==33?1:0;
                        for(unsigned off=0;off<16;off+=4)MEM_W(int32_t(off),int32_t(0x801B3628))=0;
                        auto quest=[&](unsigned flag) {MEM_W(0,int32_t(0x801B3628+4*(flag/32)))|=int32_t(1u<<(flag%32));};
                        MEM_H(0,int32_t(0x801B36B8))=scenario==37||scenario==42?0xAC:scenario==38||scenario==39||scenario==43?0xC1:scenario==40||scenario==41||scenario==44?0xFD:0;
                        if(scenario==37)quest(0x10);
                        if(scenario==38) {quest(0x7B);quest(0x2F);}
                        if(scenario==40||scenario==41)quest(0x5B);
                        if(scenario==41)quest(0x4D);
                        if(scenario==43)quest(0x7B);
                        MEM_B(0,int32_t(0x8021710D))=scenario==34||scenario==35||scenario==36?0:1;
                        if(scenario>=37&&scenario<42) {
                            MEM_W(0x94,int32_t(parent))=1;
                            MEM_W(0,int32_t(0x8021BDEC))=int32_t(parent);MEM_W(0,int32_t(0x8021BCDC))=int32_t(pp);
                            MEM_W(0,int32_t(0x8021BCE4))=4;MEM_B(0,int32_t(0x8021710D))=0;
                            c=saved;c.f_odd=&c.f0.u32h;
                            if(frozen&&!native)reference_dora3_s44_remaining_8022EDF4(rdram,&c);else dora3_s44_remaining_8022EDF4(rdram,&c);
                            assert(MEM_W(0x94,int32_t(parent))==102&&MEM_BU(0x28,int32_t(pp))==0&&MEM_H(0x44,int32_t(pp))==0&&MEM_BU(0,int32_t(0x8021710D))==1);
                            const auto desc=scenario==37?0x80232C1Cu:scenario==38?0x80232CA0u:scenario==39?0x80232C30u:scenario==40?0x80232C58u:0x80232C64u;
                            unsigned contacts=0;for(const auto& cmd:city_commands)if(cmd.site==0x801B91D4) {assert(uint32_t(cmd.args[0])==desc);++contacts;}assert(contacts==1);
                            if(scenario==41)assert(uint32_t(MEM_W(8,int32_t(0x801B3628)))&(1u<<(0x5C%32)));
                        }
                        if(scenario==35||scenario==36) {
                            const bool negative=scenario==36;
                            MEM_B(0x28,int32_t(pp))=1;MEM_H(0x34,int32_t(pp))=1;MEM_H(0x36,int32_t(pp))=1;
                            wf(rdram,parent+0x10,negative?10:350);wf(rdram,pp+4,negative?10:350);
                            wf(rdram,pp+0x58,negative?240:120);wf(rdram,pp+0x5C,negative?-30:30);
                            c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(parent);c.r5=0;c.r6=5;dora3_s0_8002A328(rdram,&c);
                            c.r4=int32_t(parent);c.r5=0;c.r6=1;dora3_s0_8002A454(rdram,&c);
                        }
                    }
                    if(scenario>=45) {
                        for(unsigned off=0;off<16;off+=4)MEM_W(int32_t(off),int32_t(0x801B3628))=0;
                        MEM_W(0,int32_t(0x802181DC))=0;
                        MEM_W(0x94,int32_t(parent))=scenario<=46||(scenario>=52&&scenario<=53)?100:200;
                        MEM_B(0x28,int32_t(pp))=scenario>=49&&scenario<=51?1:0;
                        MEM_B(0,int32_t(0x8021710D))=1;
                        MEM_B(14,int32_t(0x801B3678))=scenario==49?98:scenario==50?127:scenario==51?-1:0;
                        if(scenario>=49&&scenario<=51) {
                            c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(parent);c.r5=0;c.r6=6;
                            dora3_s0_8002A328(rdram,&c);
                        }
                    }
                    if(scenario>=54) {
                        MEM_B(15,int32_t(0x801B3700))=scenario==58?-1:0;
                        if(scenario>=56) {
                            MEM_B(0x28,int32_t(pp))=1;MEM_H(0x34,int32_t(pp))=1;MEM_H(0x36,int32_t(pp))=1;
                            wf(rdram,parent+0x10,scenario==57?10:350);wf(rdram,pp+4,scenario==57?10:350);
                            wf(rdram,pp+0x58,scenario==57?240:120);wf(rdram,pp+0x5C,scenario==57?-30:30);
                        }
                    }
                    auto source_end_story=[&] {
                        const auto state=MEM_W(0x94,int32_t(parent));
                        return (kind==24&&(state==100||(state==200&&MEM_BU(0x28,int32_t(pp))!=0)))||(kind==28&&(state==100||state==200));
                    };
                    auto dialogue26_non_event=[&] {
                        const auto scene=MEM_H(0,int32_t(0x801B36B8));
                        const auto flag=[&](unsigned n) {return (uint32_t(MEM_W(0,int32_t(0x801B3628+4*(n/32))))&(1u<<(n%32)))!=0;};
                        return scene==0xFD?true:scene==0xAC?flag(0x10):scene==0xC1?(!flag(0x7B)||flag(0x2F)):true;
                    };
                    auto owned=[&] {const auto state=MEM_W(0x94,int32_t(parent));return city_identity_verified&&
                        (kind==24?(state==1||state==2||state==3||state==4||state==100||state==200):kind==25?(state==1||state==2||state==100||state==101||state==102):kind==26?(state==1||state==100||state==101||(state==102&&dialogue26_non_event())):kind==27?(state==1||state==2||state==100):(state==1||state==2||state==100||state==200));};
                    auto record=[&] {if(!native) {auto context=c;context.f_odd=nullptr;result.original.push_back({memory,context,allocations,next_address,resource_references,city_commands,game_model_animation().model_count()});}};
                    if(!birth) {city_source_stamp={0,0};c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);links();record();}
                    result.initial={rf(rdram,parent+0x10),rf(rdram,pp+4)};
                    city_commands.clear();preview_live=native;if(native&&!birth) {
                        if(scenario==20) {
                            scene_loop_watch=true;scene_requested_fps=render_fps;scene_builds=scene_receives=scene_barriers=0;
                            MEM_W(0,int32_t(0x801AA618))=MEM_W(0,int32_t(0x801A48C0))=MEM_W(0,int32_t(0x801B1430))=MEM_W(0,int32_t(0x801AC694))=MEM_W(0,int32_t(0x801A9544))=0;
                            assert(dora3_native_loop_after_vi(rdram,&c));assert(game_frame_session().phase()==FrameSession::Phase::Native);
                        } else native_gameplay_preview_begin(rdram,0,divider);
                        dora3_native_preview_hook(rdram,&c,0x801BB080);dora3_native_preview_hook(rdram,&c,0x801E8BDC);
                        dora3_native_preview_hook(rdram,&c,0x801C8B28);dora3_native_preview_hook(rdram,&c,0x801D10D8);dora3_native_preview_hook(rdram,&c,0x801DBE78);
                        scene_frame_watch=true;
                    }
                    const std::array<Nanoseconds,5> edges{73'000'000,173'000'000,273'000'000,373'000'000,473'000'000};
                    auto to=[&](Nanoseconds stamp) {while(edge_mode&&edge<edges.size()&&edges[edge]<=stamp) {advance(edges[edge]);links();city_source_stamp={edges[edge],0};
                        if(edge_mode==1&&edge<2) {MEM_B(0xC9,int32_t(pp))=edge==0?1:0;if(child)MEM_B(0xC9,int32_t(cp))=edge==0?1:0;}
                        else if(edge_mode==2&&edge==0) {MEM_W(0x94,int32_t(parent))=99;c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);}
                        else if(edge_mode==2&&edge==1) {MEM_W(0x94,int32_t(parent))=patrol_state;MEM_B(0x28,int32_t(pp))=0;}
                        else if(edge_mode==3&&edge==1) {retire();const auto units=uint64_t(edges[edge])*60,den=uint64_t(divider)*second;const SimulationSlice born{1,den,units/den,units%den};SimulationScope born_scope(born);create();}
                        else if(edge==2) {MEM_W(0x94,int32_t(parent))=patrol_state;MEM_B(0x28,int32_t(pp))=0;if(child)MEM_B(0x28,int32_t(cp))=0;}
                        else if(edge==3) {wf(rdram,parent+0x10,33);wf(rdram,pp+4,33);}
                        else if(edge==4) {c.r4=int32_t(pm);dora3_s0_8003B744(rdram,&c);c.r4=int32_t(pm);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);}
                        ++edge;
                    }advance(stamp);links();};
                    for(unsigned tick=1;tick<=13;++tick) {
                        if((scenario==10||(scenario>=14&&scenario<42)||scenario>=45)&&tick==13) {
                            const auto boundary=uint64_t(birth)*60+uint64_t(second)*divider*(tick-1);
                            if(native)to(Nanoseconds((boundary+59)/60));
                            MEM_B(0,int32_t(0x8021710D))=0;
                        }
                        const auto units=uint64_t(birth)*60+uint64_t(second)*divider*tick;input_now=Nanoseconds(units/60);city_source_stamp={input_now,unsigned(units%60)};
                        if(native) {
                            if(scene_loop_watch&&tick==13) {
                                // Two real published banks remain owned. The
                                // Source BB080 entry flushes the final Native
                                // interval before BA0F4 consumes EC=1.
                                const auto start=Nanoseconds((units-uint64_t(second)*divider+59)/60);
                                for(unsigned i=0;i<2;++i) {
                                    input_now=start+second*(i+1)/render_fps;c=saved;c.f_odd=&c.f0.u32h;
                                    assert(game_frame_bridge().post_due(input_now,[] {return true;}));
                                    assert(game_frame_bridge().dispatch(WakeMailbox::message_value,input_now,rdram,&c));
                                }
                                assert(scene_builds==2&&!game_frame_bridge().drained());
                                input_now=Nanoseconds((units+59)/60);c=saved;c.f_odd=&c.f0.u32h;
                                try {dora3_s3_801BB080(rdram,&c);assert(false);}catch(const SceneResourceBoundary&) {}
                                assert(scene_barriers==1);c=saved;c.f_odd=&c.f0.u32h;
                                c.r4=int32_t(game_queue);c.r5=int32_t(0x802F0040);
                                for(auto m:{0x802F0010u,0x802F0020u,0x802F0030u}) {assert(dora3_native_loop_receive_deferred(rdram,&c));assert(uint32_t(MEM_W(0,c.r5))==m);}
                                assert(!dora3_native_loop_receive_deferred(rdram,&c));c=saved;c.f_odd=&c.f0.u32h;
                            } else {to(Nanoseconds((units+59)/60));c=saved;c.f_odd=&c.f0.u32h;dora3_s3_801E8BDC(rdram,&c);links();}
                        }
                        else {const auto start=uint64_t(birth)*60+uint64_t(second)*divider*(tick-1);// Original BA05C publishes pending scene request at802170EC.
                        // Such a world remains on Source; an idle callback after
                        // dialogue closure must keep its actual Source timestamp.
                        if(owned()&&!source_end_story()&&MEM_W(0,int32_t(0x802170EC))==0) {city_source_initial_stamp=SourceTimestamp{Nanoseconds(start/60),unsigned(start%60)};}
                        c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);links();city_source_initial_stamp.reset();if(tick==1||tick==13||tick==14||tick==30||tick==31||tick==96)record();}
                        if(tick==1) {result.first_end={rf(rdram,parent+0x10),rf(rdram,pp+4)};result.first_continuous=owned()&&(kind==24?MEM_W(0x94,int32_t(parent))==3:kind==25?(MEM_W(0x94,int32_t(parent))==2||MEM_W(0x94,int32_t(parent))==102):kind==26?MEM_W(0x94,int32_t(parent))==102:(kind==27||kind==28)&&(MEM_W(0x94,int32_t(parent))==2||(kind==28&&MEM_W(0x94,int32_t(parent))==200)));}
                        if(tick==13) {
                            const auto v=scene_end_fields.value_or(scene_frame_fields(rdram));
                            for(const auto value:v) {assert(std::isfinite(value));}
                            result.samples.push_back(v);
                        }
                    }
                    if(kind==24&&scenario>=22&&scenario<=26) {
                        unsigned releases=0;
                        for(const auto& command:city_commands)if(command.site==0x801D1498) {assert(command.args[0]==1);++releases;}
                        assert(releases==1&&rf(rdram,pp+0x78)==0&&MEM_W(0,int32_t(0x802181DC))==1);
                    }
                    if(scenario>=45&&scenario<=46) {
                        assert(uint32_t(MEM_W(8,int32_t(0x801B3628)))&(1u<<(0x4F%32)));
                        unsigned starts=0;for(const auto& cmd:city_commands)if(cmd.site==0x801B91D4) {assert(uint32_t(cmd.args[0])==0x80232A1Cu);++starts;}
                        assert(starts==1);
                    }
                    if(scenario>=47&&scenario<=51) {
                        assert(uint32_t(MEM_W(8,int32_t(0x801B3628)))&(1u<<(0x51%32)));
                        assert(MEM_W(0,int32_t(0x802181DC))==1&&MEM_W(0x94,int32_t(parent))==4);
                        const auto count=MEM_BU(14,int32_t(0x801B3678));
                        assert(count==(scenario==49?99:scenario==50||scenario==51?0:1));
                        unsigned releases=0;for(const auto& cmd:city_commands)if(cmd.site==0x801D1498) {assert(cmd.args[0]==1);++releases;}
                        assert(releases==1);
                    }
                    if(scenario>=52&&scenario<=53) {
                        unsigned starts=0;for(const auto& cmd:city_commands)if(cmd.site==0x801B91D4) {assert(uint32_t(cmd.args[0])==0x80232D2Cu);++starts;}
                        assert(starts==1&&MEM_H(0x44,int32_t(pp))==0);
                        unsigned releases=0;for(const auto& cmd:city_commands)if(cmd.site==0x801D1498) {assert(cmd.args[0]==1);++releases;}
                        assert(releases==1&&MEM_W(0,int32_t(0x802181DC))==1);
                    }
                    if(scenario>=54) {
                        assert(uint32_t(MEM_W(8,int32_t(0x801B3628)))&(1u<<(0x4E%32)));
                        assert(MEM_W(0,int32_t(0x802181DC))==1&&MEM_W(0x94,int32_t(parent))==1);
                        // Real B50F4 normalizes sentinel -1 to0 before adding1.
                        assert(MEM_BU(15,int32_t(0x801B3700))==1);
                        unsigned starts=0,releases=0;
                        for(const auto& cmd:city_commands) {
                            if(cmd.site==0x801B91D4) {assert(uint32_t(cmd.args[0])==0x80232D9Cu);++starts;}
                            if(cmd.site==0x801D1498) {assert(cmd.args[0]==1);++releases;}
                        }
                        assert(starts==(scenario<=55?1u:0u)&&releases==1);
                    }
                    if(scenario==20) {
                        assert(MEM_W(0,int32_t(0x802170EC))==2);
                        assert(scene_pending_passes==std::vector<uint32_t>({0x801C8B28,0x801D10D8,0x801DBD58}));
                    } else {assert(MEM_W(0,int32_t(0x802170EC))==0&&scene_pending_passes.empty());}
                    scene_loop_watch=false;scene_frame_watch=false;
                    result.commands=city_commands;linked_watch=patrol_original_reference=false;native_gameplay_preview_reset(rdram);preview_live=false;retire();
                    // Enabling the actual player binds its idle animation. The
                    // Original B744/B6BC pair frees/replaces only model+60's
                    // channel allocation; all other addresses must be exact.
                    const auto old_channel=special_memory[(owners[0]+0x60-0x80000000u)/4];
                    const auto current_channel=uint32_t(MEM_W(0x60,int32_t(owners[0])));
                    auto expected_heap=special_heap;
                    assert(old_channel&&current_channel&&expected_heap.erase(old_channel)==1);
                    assert(expected_heap.insert(current_channel).second);
                    assert(allocations==expected_heap&&resource_references==special_refs&&game_model_animation().model_count()==6);
                    c.r4=int32_t(owners[0]);dora3_s0_8003B744(rdram,&c);
                    assert(expected_heap.erase(current_channel)==1);
                    assert(allocations==expected_heap&&MEM_W(0x60,int32_t(owners[0]))==0&&resource_references==special_refs&&game_model_animation().model_count()==6);linked_construct_watch=slot09_loaded=false;memory=special_memory;allocations=special_heap;next_address=special_next;resource_references=special_refs;return result;
                };
                auto compare=[&](const SpecialResult& a,const SpecialResult& b,unsigned mode) {assert(a.samples.size()==b.samples.size());for(unsigned n=0;n<a.samples.size();++n)for(unsigned k=0;k<a.samples[n].size();++k) {const bool geometry=k==1||k==2||(k>=4&&k<=8)||k==11||(k>=15&&k<=17)||k==24||k==25;const double tolerance=geometry?0.03:0;if(std::abs(a.samples[n][k]-b.samples[n][k])>tolerance) {std::cerr<<"Slot09 patrol mismatch kind="<<kind<<" gap="<<gap<<" scenario="<<scenario<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<" tick="<<3*(n+1)<<" field="<<k<<" actual="<<a.samples[n][k]<<" expected="<<b.samples[n][k]<<'\n';return false;}}if(a.commands!=b.commands) {std::cerr<<"Slot09 patrol commands kind="<<kind<<" gap="<<gap<<" scenario="<<scenario<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<'\n';
                    for(const auto& list:{b.commands,a.commands}) {std::cerr<<"count="<<list.size()<<" ";for(const auto& command:list)std::cerr<<std::hex<<command.site<<std::dec<<":"<<command.stamp.nanoseconds<<"+"<<command.stamp.sixtieths<<"/60 "<<command.args[0]<<","<<command.args[1]<<","<<command.args[2]<<"; ";std::cerr<<'\n';}
                    return false;}return true;};
                const auto expected=run_story(true,60),actual=run_story(true,fps);
                assert(expected.samples.size()==1&&actual.samples.size()==1);
                for(unsigned field=0;field<27;++field)if(std::abs(expected.samples[0][field]-actual.samples[0][field])>0.02) {
                    std::cerr<<"Scene frame partition fps="<<fps<<" divider="<<divider<<" camera="<<camera_mode<<" scenario="<<scenario<<" field="<<field<<" actual="<<actual.samples[0][field]<<" expected="<<expected.samples[0][field]<<std::endl;return 1;
                }
                if(scenario==20)turn_baseline[gap?1:0]=actual.samples[0];
                else {
                    const auto& event=turn_baseline[gap?1:0];const auto& control=actual.samples[0];
                    for(unsigned field=0;field<27;++field)if(std::abs(event[field]-control[field])>=0.02) {std::cerr<<"Scene no-request control gap="<<gap<<" fps="<<fps<<" divider="<<divider<<" camera="<<camera_mode<<" field="<<field<<" event="<<event[field]<<" control="<<control[field]<<std::endl;return 1;}
                }
                ++slot09_native_cases;
            }

            std::copy(saved_slot09_image.begin(),saved_slot09_image.end(),memory.begin()+slot09_overlay_vram_offset/4);
            assert(!slot09_loaded);
            for(auto model:object_models) {c.r4=int32_t(model);dora3_s0_8003B77C(rdram,&c);}
            // An unreviewed action cancels ownership before a subsequent step.
            MEM_W(0x94,int32_t(player))=19;assert(!native_gameplay_preview_eligible(rdram));
            native_gameplay_preview_reset(rdram);preview_live=false;
            dora3_controller_latch(rdram,&c);assert(MEM_HU(0,int32_t(0x801A48EC))==0);
            for(auto model:owners) {c.r4=int32_t(model);dora3_s0_8003B77C(rdram,&c);}
            assert(game_model_animation().model_count()==0 && allocations.empty());
            ++cases;
        }
    }
    assert(check_npc_action_contacts(image)==3192);
    assert(slot09_constructor_cases==cases*42&&slot09_constructor_skips==cases*12);
    assert(slot09_source_cases==0&&slot09_native_cases==cases*4&&slot09_rejections==0&&slot09_original_cases==0);
    assert(slot09_yaw_samples==0);
    std::cout<<"PASS: "<<slot09_constructor_cases<<" actual slot09 constructors; "<<slot09_source_cases<<" slot09 Source schedules; "<<slot09_native_cases<<" slot09 Native edge schedules; "<<slot09_rejections<<" slot09 rejections; "<<slot09_original_cases<<" slot09 frozen189 full-RAM/context/heap/resource/command/model-count comparisons; "<<slot09_yaw_samples<<" slot09 intermediate yaw samples; settings="<<cases<<"; ASan/UBSan/LSan; no game/GPU.\n";
}
