#include "native_simulation_time.hpp"
#include "native_gameplay_preview.hpp"
#include "native_input_timeline.hpp"
#include "native_camera_timeline.hpp"
#include "native_model_animation.hpp"
#include "native_frame_bridge.hpp"
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
const FrameBinding* active_frame_binding(){return binding;}
}
double simulation_stamp(bool end) {
    const auto slice=native_gameplay_preview_slice();
    return slice?double(slice->tick*slice->denominator+slice->phase+(end?slice->numerator:0))/(60.*second):double(input_now)/second;
}
extern "C" {
int dora3_native_loop_gameplay_active(){return preview_live;}
int dora3_overlay_loaded(uint32_t rom) {
    if(rom==0x1398A0u)return slot09_loaded&&city_identity_verified;
    if(rom==0x132F80u)return city_loaded&&city_identity_verified;
    assert(rom==0xEDDB0u||rom==0x15E440u);return dialogue_actor_loaded&&city_identity_verified;
}
int dora3_native_loop_owns_frame_banks(){return preview_live;}
int dora3_native_loop_owns_fade(){return preview_live;}
void dora3_native_loop_fade_command(uint8_t* rdram,recomp_context*) {
    if(death_fade) {
        const auto stamp=native_gameplay_preview_command_time();assert(stamp&&stamp->nanoseconds<=input_now);
        death_write_fade(rdram,death_fade->before_command(death_read_fade(rdram),*stamp));
    }
}
void dora3_native_loop_resource_barrier(uint8_t* rdram,recomp_context* c) {
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
        std::array<std::array<double,5>,2> turn_baseline{};
        std::array<double,17> flight_edge_baseline{};
        for(unsigned fps:{60u,73u,120u,144u,165u,360u}) {
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
            assert(native_gameplay_preview_eligible(rdram));
            // Exact ROM-backed reserved handlers must remain no-ops while the
            // rest of the world keeps its Native policy. Full RAM/context equality
            // detects accidental handler commands, stores or register mutations.
            for(unsigned reserved:{19u,23u,26u,30u}) {
                MEM_W(0x94,int32_t(player))=int(reserved);assert(native_gameplay_preview_eligible(rdram));
                const auto untouched=memory;const auto registers=c;
                const uint32_t handler=reserved==19?0x801C488Cu:reserved==23?0x801C4D98u:reserved==26?0x801C5244u:0x801C58B8u;
                for(unsigned phase=0;phase<2;++phase){get_function(int32_t(handler+phase*8))(rdram,&c);assert(memory==untouched&&std::memcmp(&registers,&c,sizeof(c))==0);}
            }
            MEM_W(0x94,int32_t(player))=1;preview_live=true;
            decisions=actions=footsteps=sprite_commands=jumps=falls=landings=0;animations={};dora3_native_camera_dispatch_reset();
            input_now=0;native_gameplay_preview_begin(rdram,0,divider);
            unsigned vi=0;const auto saved=c;
            for(unsigned frame=1;frame<=fps;++frame) {
                while(uint64_t(vi+1)*fps<=uint64_t(frame)*60) {
                    ++vi;input_now=second*vi/60;MEM_B(2,int32_t(0x80171560))=vi>=12?60:0;
                    dora3_controller_accumulate(rdram,&c);
                    if(vi%divider==0) {
                        dora3_controller_latch(rdram,&c);c.r4=1;
                        const auto before=decisions;dora3_s3_801C8B28(rdram,&c);assert(decisions==before);
                        // Real original world updates its shared render origin
                        // here even though the player callback is suppressed.
                        c.f12.fl=rf(rdram,player+0x9C);c.f14.fl=rf(rdram,player+0xA0);c.r6=MEM_W(0xA4,int32_t(player));
                        dora3_s3_801BBDDC(rdram,&c);
                    }
                }
                c=saved;c.f_odd=&c.f0.u32h;
                native_gameplay_preview_update(rdram,&c,second*frame/fps);
                assert(std::memcmp(&c,&saved,sizeof(c))==0);
                assert(native_gameplay_preview_eligible(rdram));
                for(auto pair:std::array<std::array<uint32_t,2>,3>{{{player+0x9C,0x800DAE54},{player+0xA0,0x800FE878},{player+0xA4,0x800DD3F0}}}) {
                    if(rf(rdram,pair[0])!=rf(rdram,pair[1])) {
                        std::cerr<<"Stale world render origin camera="<<camera_mode<<" fps="<<fps<<" frame="<<frame<<" delta="<<rf(rdram,pair[0])-rf(rdram,pair[1])<<'\n';return 1;
                    }
                }
                poses(rdram,c,owners,second*frame/fps,frame&1);
            }
            assert(decisions>=fps && actions==decisions*2);
            assert(std::abs(animations[0]-3.*(60/divider))<1e-6 && animations[0]==animations[1]);
            assert(MEM_HU(0x14,int32_t(control))==0); // Continuous input left idle.
            std::array<double,6> values{rf(rdram,player+0x9C),rf(rdram,player+0xA0),rf(rdram,player+0xA4),rf(rdram,physics),double(footsteps),double(sprite_commands)};
            if(fps==60)baseline=values;
            else for(unsigned i=0;i<values.size();++i)if(std::abs(values[i]-baseline[i])>(i<4?0.12:1)) {
                std::cerr<<"Full player partition mismatch divider="<<divider<<" fps="<<fps<<" field="<<i<<" actual="<<values[i]<<" expected="<<baseline[i]<<'\n';return 1;
            }
            // Hold A throughout a complete jump: the impulse, clip bind and
            // landing event must each happen once, without Original handoff.
            MEM_H(0,int32_t(0x80171560))=int16_t(0x8000);dora3_controller_accumulate(rdram,&c);
            double peak=0;
            for(unsigned frame=fps+1;frame<=4*fps;++frame) {
                input_now=second*frame/fps;c=saved;c.f_odd=&c.f0.u32h;
                native_gameplay_preview_update(rdram,&c,input_now);
                assert(native_gameplay_preview_eligible(rdram));
                peak=std::max(peak,double(rf(rdram,player+0xA0)));
                poses(rdram,c,owners,input_now,frame&1);
            }
            if(jumps!=1 || falls!=1 || landings!=1) {
                std::cerr<<"Action count mismatch camera="<<camera_mode<<" divider="<<divider<<" fps="<<fps<<" jump="<<jumps<<" fall="<<falls<<" land="<<landings<<" state="<<MEM_W(0x94,int32_t(player))<<" y="<<rf(rdram,player+0xA0)<<" peak="<<peak<<" model="<<rf(rdram,owners[0]+0x64)<<'\n';return 1;
            }
            assert(MEM_W(0x94,int32_t(player))>=1 && MEM_W(0x94,int32_t(player))<=5);
            assert(rf(rdram,player+0xA0)==0);
            if(fps==60) {baseline_peak=peak;baseline_landed=landed_at;}
            else if(std::abs(peak-baseline_peak)>0.12 || std::abs(landed_at-baseline_landed)>1./60+1e-8) {
                std::cerr<<"Jump partition mismatch divider="<<divider<<" fps="<<fps<<" peak="<<peak<<" expected="<<baseline_peak<<" landed="<<landed_at<<" expected="<<baseline_landed<<'\n';return 1;
            }
            // Both reversal families use real init/clip commands and the full
            // dispatcher on a flat non-sliding surface. The negative speed
            // stops before a one-shot end pulse returns to ground movement.
            for(unsigned family=0;family<2;++family) {
                native_gameplay_preview_reset(rdram);preview_live=false;
                MEM_H(0,int32_t(0x80171560))=0;
                MEM_B(2,int32_t(0x80171560))=60;
                MEM_B(3,int32_t(0x80171560))=0;
                input_now=0;dora3_controller_accumulate(rdram,&c);dora3_controller_latch(rdram,&c);
                wf(rdram,player+0x9C,20);wf(rdram,player+0xA0,0);wf(rdram,player+0xA4,10);
                wf(rdram,physics,-12);wf(rdram,physics+4,0);wf(rdram,physics+0x14,0);
                entered={};c.r4=1;
                (family?dora3_s3_core_801C4038:dora3_s3_core_801C3BA8)(rdram,&c);
                preview_live=true;dora3_native_camera_dispatch_reset();
                native_gameplay_preview_begin(rdram,0,divider);
                unsigned input_vi=0;
                for(unsigned frame=1;frame<=2*fps;++frame) {
                    while(uint64_t(input_vi+1)*fps<=uint64_t(frame)*60) {
                        ++input_vi;input_now=second*input_vi/60;
                        MEM_B(2,int32_t(0x80171560))=input_vi<=45?60:0;
                        dora3_controller_accumulate(rdram,&c);
                        if(input_vi%divider==0) {
                            dora3_controller_latch(rdram,&c);c.r4=1;
                            const auto before=decisions;dora3_s3_801C8B28(rdram,&c);assert(decisions==before);
                        }
                    }
                    c=saved;c.f_odd=&c.f0.u32h;
                    native_gameplay_preview_update(rdram,&c,second*frame/fps);
                    assert(native_gameplay_preview_eligible(rdram));
                    poses(rdram,c,owners,second*frame/fps,frame&1);
                }
                for(unsigned i=11+3*family;i<=13+3*family;++i)if(entered[i]!=1) {
                    std::cerr<<"Reverse transition mismatch divider="<<divider<<" fps="<<fps<<" family="<<family<<" state="<<i<<" count="<<entered[i]<<" current="<<MEM_W(0x94,int32_t(player))<<'\n';return 1;
                }
                assert(MEM_W(0x94,int32_t(player))>=1 && MEM_W(0x94,int32_t(player))<=5);
                const std::array<double,5> value{rf(rdram,player+0x9C),rf(rdram,player+0xA0),rf(rdram,player+0xA4),rf(rdram,physics),rf(rdram,physics+4)};
                if(fps==60)turn_baseline[family]=value;
                else for(unsigned field=0;field<value.size();++field)if(std::abs(value[field]-turn_baseline[family][field])>0.12) {
                    std::cerr<<"Reverse partition mismatch divider="<<divider<<" fps="<<fps<<" family="<<family<<" field="<<field<<" actual="<<value[field]<<" expected="<<turn_baseline[family][field]<<'\n';return 1;
                }
            }
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
                    native_gameplay_preview_reset(rdram);preview_live=false;memory=special_memory;allocations=special_heap;next_address=special_next;resource_references=special_refs;input_now=0;
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
                    city_commands.clear();preview_live=native;if(native&&!birth) {native_gameplay_preview_begin(rdram,0,divider);dora3_native_preview_hook(rdram,&c,0x801BB080);dora3_native_preview_hook(rdram,&c,0x801E8BDC);}
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
                    for(unsigned tick=1;tick<=96;++tick) {
                        if((scenario==10||(scenario>=14&&scenario<42)||scenario>=45)&&tick==13) {
                            const auto boundary=uint64_t(birth)*60+uint64_t(second)*divider*(tick-1);
                            if(native)to(Nanoseconds((boundary+59)/60));
                            MEM_B(0,int32_t(0x8021710D))=0;
                        }
                        const auto units=uint64_t(birth)*60+uint64_t(second)*divider*tick;input_now=Nanoseconds(units/60);city_source_stamp={input_now,unsigned(units%60)};
                        if(native) {to(Nanoseconds((units+59)/60));c=saved;c.f_odd=&c.f0.u32h;dora3_s3_801E8BDC(rdram,&c);links();}
                        else {const auto start=uint64_t(birth)*60+uint64_t(second)*divider*(tick-1);// Original BA05C publishes pending scene request at802170EC.
                        // Such a world remains on Source; an idle callback after
                        // dialogue closure must keep its actual Source timestamp.
                        if(owned()&&!source_end_story()&&MEM_W(0,int32_t(0x802170EC))==0) {city_source_initial_stamp=SourceTimestamp{Nanoseconds(start/60),unsigned(start%60)};}
                        c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);links();city_source_initial_stamp.reset();if(tick==1||tick==13||tick==14||tick==30||tick==31||tick==96)record();}
                        if(tick==1) {result.first_end={rf(rdram,parent+0x10),rf(rdram,pp+4)};result.first_continuous=owned()&&(kind==24?MEM_W(0x94,int32_t(parent))==3:kind==25?(MEM_W(0x94,int32_t(parent))==2||MEM_W(0x94,int32_t(parent))==102):kind==26?MEM_W(0x94,int32_t(parent))==102:(kind==27||kind==28)&&(MEM_W(0x94,int32_t(parent))==2||(kind==28&&MEM_W(0x94,int32_t(parent))==200)));}
                        if(tick%3==0) {
                            std::array<double,27> v{};v[0]=MEM_W(0x94,int32_t(parent));v[1]=rf(rdram,parent+0x10);v[2]=rf(rdram,pp+4);v[3]=MEM_BU(0x28,int32_t(pp));
                            for(unsigned a=0;a<3;++a)v[4+a]=rf(rdram,parent+0x9C+4*a);
                            v[7]=rf(rdram,pm+0x64);v[8]=rf(rdram,pp+0x5C);v[9]=MEM_H(0x34,int32_t(pp));v[10]=MEM_H(0x36,int32_t(pp));v[11]=rf(rdram,pp+0x58);
                            v[12]=MEM_H(0x42,int32_t(pp));v[13]=MEM_H(0x44,int32_t(pp));v[14]=MEM_BU(0,int32_t(0x8021710D));
                            for(unsigned a=0;a<3;++a)v[15+a]=rf(rdram,parent+0x18+4*a);
                            v[18]=MEM_H(0xD8,int32_t(pp));v[19]=MEM_BU(0xC9,int32_t(pp));v[20]=game_model_animation().model_count();v[21]=allocations.size();v[22]=resource_references[2];v[23]=sm?1:0;
                            v[24]=rf(rdram,pp);v[25]=rf(rdram,pp+0x14);v[26]=resource_references[1];result.samples.push_back(v);
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
                    if(scenario==20||scenario==21) {
                        unsigned transition_commands=0;
                        for(const auto& command:city_commands)if(command.site==0x801BA0C4) {
                            assert(command.args[0]==0x175&&command.args[1]==1);++transition_commands;
                        }
                        const bool requested=scenario==20;
                        assert(transition_commands==(requested?1u:0u));
                        assert((uint32_t(MEM_W(8,int32_t(0x801B3628)))&(1u<<(0x5B%32)))!=0);
                        assert(MEM_W(0x94,int32_t(parent))==1&&MEM_BU(0x28,int32_t(pp))==1);
                        assert(MEM_W(0,int32_t(0x802170EC))==(requested?1:0));
                        if(requested) {
                            assert(MEM_W(0,int32_t(0x80217104))==0x175&&MEM_W(0,int32_t(0x802170FC))==1);
                            assert(uint32_t(MEM_W(0,int32_t(0x80217100)))==0x80217104u);
                            assert(!native_gameplay_preview_eligible(rdram));
                        } else assert(native_gameplay_preview_eligible(rdram));
                    }
                    result.commands=city_commands;linked_watch=patrol_original_reference=false;native_gameplay_preview_reset(rdram);preview_live=false;retire();assert(allocations==special_heap&&resource_references==special_refs&&game_model_animation().model_count()==6);linked_construct_watch=slot09_loaded=false;memory=special_memory;allocations=special_heap;next_address=special_next;resource_references=special_refs;return result;
                };
                auto compare=[&](const SpecialResult& a,const SpecialResult& b,unsigned mode) {assert(a.samples.size()==b.samples.size());for(unsigned n=0;n<a.samples.size();++n)for(unsigned k=0;k<a.samples[n].size();++k) {const bool geometry=k==1||k==2||(k>=4&&k<=8)||k==11||(k>=15&&k<=17)||k==24||k==25;const double tolerance=geometry?0.03:0;if(std::abs(a.samples[n][k]-b.samples[n][k])>tolerance) {std::cerr<<"Slot09 patrol mismatch kind="<<kind<<" gap="<<gap<<" scenario="<<scenario<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<" tick="<<3*(n+1)<<" field="<<k<<" actual="<<a.samples[n][k]<<" expected="<<b.samples[n][k]<<'\n';return false;}}if(a.commands!=b.commands) {std::cerr<<"Slot09 patrol commands kind="<<kind<<" gap="<<gap<<" scenario="<<scenario<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<'\n';
                    for(const auto& list:{b.commands,a.commands}) {std::cerr<<"count="<<list.size()<<" ";for(const auto& command:list)std::cerr<<std::hex<<command.site<<std::dec<<":"<<command.stamp.nanoseconds<<"+"<<command.stamp.sixtieths<<"/60 "<<command.args[0]<<","<<command.args[1]<<","<<command.args[2]<<"; ";std::cerr<<'\n';}
                    return false;}return true;};
                const auto expected=run_story(false,60),original=run_story(false,60,0,false);
                assert(expected.original.size()==original.original.size());for(unsigned n=0;n<expected.original.size();++n) {const auto& a=expected.original[n];const auto& b=original.original[n];if(a.ram!=b.ram||std::memcmp(&a.context,&b.context,sizeof(c))!=0||a.heap!=b.heap||a.next!=b.next||a.refs!=b.refs||a.commands!=b.commands||a.models!=b.models) {
                    std::cerr<<"Slot09 Original mismatch kind="<<kind<<" gap="<<gap<<" scenario="<<scenario<<" checkpoint="<<n<<" fps="<<fps<<" divider="<<divider<<" ram="<<(a.ram==b.ram)<<" context="<<(std::memcmp(&a.context,&b.context,sizeof(c))==0)<<" heap="<<(a.heap==b.heap)<<" next="<<(a.next==b.next)<<" refs="<<(a.refs==b.refs)<<" commands="<<(a.commands==b.commands)<<" models="<<(a.models==b.models)<<'\n';
                    for(unsigned i=0,shown=0;i<a.ram.size()&&shown<12;++i)if(a.ram[i]!=b.ram[i]) {std::cerr<<" ram "<<std::hex<<0x80000000u+4*i<<" expected="<<a.ram[i]<<" actual="<<b.ram[i]<<std::dec<<'\n';++shown;}
                    return 1;
                }++slot09_original_cases;}
                if(!compare(original,expected,0))return 1;
                const auto actual=run_story(true,fps,0,true,&expected);if(!compare(actual,expected,0))return 1;
                if(scenario==2)for(unsigned mode:{1u,2u,3u}) {const auto baseline=run_story(true,60,mode),varied=run_story(true,fps,mode);if(!compare(varied,baseline,mode))return 1;++slot09_native_cases;}
                if(scenario==3||scenario==4||(scenario>=42&&scenario<44))++slot09_rejections;
                ++slot09_source_cases;
            }

            std::copy(saved_slot09_image.begin(),saved_slot09_image.end(),memory.begin()+slot09_overlay_vram_offset/4);
            assert(!slot09_loaded);
            // Real constructors for all city patrol/dialogue/story profiles.
            // Compare every guest byte and context with frozen189, including
            // actual descriptor/resource/model lists; decoded assets are synthetic.
            for(unsigned kind:{13u,14u,15u,16u,17u,20u,21u,22u})for(unsigned profile:{0u,1u,2u})for(bool earlier_gap:{false,true}) {
                const auto constructor_memory=memory;const auto constructor_heap=allocations;
                const auto constructor_next=next_address;const auto constructor_refs=resource_references;
                native_gameplay_preview_reset(rdram);preview_live=false;room(rdram);
                city_loaded=linked_construct_watch=true;city_identity_verified=true;patrol_watch=city_dialogue_watch=city_watch=false;
                for(unsigned slot=30;slot<230;++slot) {
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(0x80171CD0u+slot*0xCC);dora3_scene_object_init(rdram,&c);
                    MEM_H(0xA8,int32_t(0x80171CD0u+slot*0xCC))=-1;
                }
                MEM_W(0,int32_t(0x801B3628))=0;MEM_W(12,int32_t(0x801B3628))=0;
                if(profile) {c.r4=profile==2?0x7E:1;dora3_s1_801B4FBC(rdram,&c);}
                if(earlier_gap)MEM_W(0x94,int32_t(prop))=99;
                MEM_H(0,int32_t(0x8021BE02))=-500;MEM_H(2,int32_t(0x8021BE02))=80;
                MEM_H(4,int32_t(0x8021BE02))=-300;MEM_H(6,int32_t(0x8021BE02))=359;
                // A5F0 attaches model group1 to bone19 of model group0;
                // it does not seek animation frame19. Build a real20-bone
                // star hierarchy using Original's48-byte bind-pose records.
                MEM_H(0x2A,int32_t(asset))=20;MEM_W(0x2C,int32_t(asset))=0x700;
                for(unsigned n=0;n<20;++n) {
                    const auto bind=asset+0x100+48*n;
                    for(unsigned field=0;field<48;field+=4)MEM_W(0,int32_t(bind+field))=0;
                    wf(rdram,bind+0x1C,1);wf(rdram,bind+0x20,1);wf(rdram,bind+0x24,1);
                    MEM_W(0x28,int32_t(bind))=n&&n<19?1:0;MEM_W(0x2C,int32_t(bind))=n==0?1:0;
                }
                // Move synthetic geometry/bounds clear of the expanded bones.
                MEM_W(0x34,int32_t(asset))=0x900;MEM_W(0,int32_t(asset+0x900))=0x980;
                MEM_H(0,int32_t(asset+0x980))=0;MEM_H(2,int32_t(asset+0x980))=100;
                MEM_H(4,int32_t(asset+0x980))=50;MEM_H(6,int32_t(asset+0x980))=30;MEM_H(12,int32_t(asset+0x980))=-1;
                MEM_W(0,int32_t(0x801AEC00+20))=int32_t(asset);MEM_W(0,int32_t(0x801AEC00+40))=int32_t(clip);
                MEM_W(0,int32_t(0x801AEC00+80))=int32_t(descriptor);
                c=saved;c.f_odd=&c.f0.u32h;
                const auto before=memory;const auto before_context=c;const auto before_heap=allocations;
                const auto before_next=next_address;const auto before_refs=resource_references;
                auto construct=[&](bool reference) {
                    switch(kind) {
                    case 13:if(reference)reference_dora3_s8_level1_8022DC04(rdram,&c);else dora3_s8_level1_8022DC04(rdram,&c);break;
                    case 14:if(reference)reference_dora3_s8_level1_8022E564(rdram,&c);else dora3_s8_level1_8022E564(rdram,&c);break;
                    case 15:if(reference)reference_dora3_s8_level1_8022EE5C(rdram,&c);else dora3_s8_level1_8022EE5C(rdram,&c);break;
                    case 16:if(reference)reference_dora3_s8_level1_8022F72C(rdram,&c);else dora3_s8_level1_8022F72C(rdram,&c);break;
                    case 17:if(reference)reference_dora3_s8_level1_80230028(rdram,&c);else dora3_s8_level1_80230028(rdram,&c);break;
                    case 20:if(reference)reference_dora3_s8_level1_8023103C(rdram,&c);else dora3_s8_level1_8023103C(rdram,&c);break;
                    case 21:if(reference)reference_dora3_s8_level1_802318B0(rdram,&c);else dora3_s8_level1_802318B0(rdram,&c);break;
                    case 22:if(reference)reference_dora3_s8_level1_80232414(rdram,&c);else dora3_s8_level1_80232414(rdram,&c);break;
                    default:std::abort();
                    }
                };
                const auto actor=earlier_gap?prop+0xCC:prop;
                construct(true);const auto expected=memory;const auto expected_context=c;const auto expected_heap=allocations;
                const auto expected_next=next_address;const auto expected_refs=resource_references;const auto expected_models=game_model_animation().model_count();
                c.r4=int32_t(actor);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);
                assert(allocations==before_heap&&resource_references==before_refs&&game_model_animation().model_count()==6);
                memory=before;c=before_context;allocations=before_heap;next_address=before_next;resource_references=before_refs;
                construct(false);
                assert(memory==expected&&std::memcmp(&c,&expected_context,sizeof(c))==0&&allocations==expected_heap&&next_address==expected_next&&resource_references==expected_refs&&game_model_animation().model_count()==expected_models);
                const auto physical=uint32_t(MEM_W(0x98,int32_t(actor)));
                assert(MEM_H(0x20,int32_t(physical))==int16_t(kind));
                assert(MEM_W(0x94,int32_t(actor))==(profile==1?100:profile==2&&kind==21?200:2));
                if(profile!=1) {
                    const int offset=kind==13||kind==14||kind==15?(profile==0?6:3):kind==17||kind==22?(profile==0?3:6):0;
                    const int sprite=kind==13?(profile==0?2:4):kind==16?(profile==0?1:3):(profile==0?0:2);
                    if(MEM_H(0x44,int32_t(physical))!=offset||MEM_H(0x42,int32_t(physical))!=sprite) {
                        std::cerr<<"Actual constructor fields kind="<<kind<<" profile="<<profile<<" gap="<<earlier_gap<<" offset44="<<MEM_H(0x44,int32_t(physical))<<" expected="<<offset<<" sprite42="<<MEM_H(0x42,int32_t(physical))<<" expected="<<sprite<<'\n';return 1;
                    }
                }
                c.r4=int32_t(actor);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);c.r4=int32_t(actor);dora3_scene_object_init(rdram,&c);
                assert(allocations==constructor_heap&&resource_references==constructor_refs&&game_model_animation().model_count()==6);
                linked_construct_watch=city_loaded=false;memory=constructor_memory;allocations=constructor_heap;next_address=constructor_next;resource_references=constructor_refs;
                ++city_actual_constructor_cases;
            }

            // Story100 families use actual constructors, including secondary
            // model groups and the real wrapped-slime attachment allocator.
            for(unsigned kind:{13u,14u,15u,16u,17u,20u,21u,22u})for(bool gap:{false,true})for(unsigned scenario=0;scenario<6;++scenario) {
                const auto story_memory=memory;const auto story_heap=allocations;const auto story_next=next_address;const auto story_refs=resource_references;
                struct StoryOriginal {std::vector<uint32_t> ram;recomp_context context;std::set<uint32_t> heap;uint32_t next;decltype(resource_references) refs;std::vector<CityCommand> commands;std::size_t models;};
                struct StoryResult {std::vector<std::array<double,27>> samples;std::vector<CityCommand> commands;std::vector<StoryOriginal> original;};
                auto run_story=[&](bool native,unsigned render_fps,unsigned edge_mode=0,bool frozen=true) {
                    native_gameplay_preview_reset(rdram);preview_live=false;memory=story_memory;allocations=story_heap;next_address=story_next;resource_references=story_refs;input_now=0;
                    room(rdram);city_loaded=linked_construct_watch=linked_watch=true;city_identity_verified=scenario!=1;patrol_original_reference=frozen&&!native;linked_original_reference=false;
                    city_watch=patrol_watch=city_dialogue_watch=false;city_commands.clear();city_source_initial_stamp.reset();
                    for(unsigned slot=30;slot<230;++slot) {c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(0x80171CD0u+slot*0xCC);dora3_scene_object_init(rdram,&c);MEM_H(0xA8,int32_t(0x80171CD0u+slot*0xCC))=-1;}
                    MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;wf(rdram,physics,0);wf(rdram,physics+0x14,0);MEM_W(0x94,int32_t(player))=1;
                    wf(rdram,player+0x9C,-700);wf(rdram,player+0xA0,80);wf(rdram,player+0xA4,-50);
                    const Nanoseconds birth=scenario==2?73'000'000:scenario==3?173'000'000:0;
                    if(native&&birth) {preview_live=true;native_gameplay_preview_begin(rdram,0,divider);dora3_native_preview_hook(rdram,&c,0x801BB080);dora3_native_preview_hook(rdram,&c,0x801E8BDC);}
                    unsigned frame=0,edge=0;uint32_t parent=gap?prop+0xCC:prop,pp=0,pm=0,sm=0,child=0,cp=0,cm=0;StoryResult result;
                    auto advance=[&](Nanoseconds stamp) {while(second*(frame+1)/render_fps<stamp) {input_now=second*++frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);assert(std::memcmp(&c,&saved,sizeof(c))==0);}input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);assert(std::memcmp(&c,&saved,sizeof(c))==0);};
                    if(native&&birth)advance(birth);
                    MEM_W(0,int32_t(0x801B3628))=0;MEM_W(12,int32_t(0x801B3628))=0;c.r4=1;dora3_s1_801B4FBC(rdram,&c);
                    MEM_H(0,int32_t(0x8021BE02))=-500;MEM_H(2,int32_t(0x8021BE02))=80;MEM_H(4,int32_t(0x8021BE02))=-300;MEM_H(6,int32_t(0x8021BE02))=359;
                    MEM_W(0,int32_t(0x801AEC00+20))=int32_t(asset);MEM_W(0,int32_t(0x801AEC00+40))=int32_t(clip);MEM_W(0,int32_t(0x801AEC00+80))=int32_t(descriptor);
                    MEM_H(0x2A,int32_t(asset))=20;MEM_W(0x2C,int32_t(asset))=0x700;
                    for(unsigned n=0;n<20;++n) {const auto bind=asset+0x100+48*n;for(unsigned f=0;f<48;f+=4)MEM_W(0,int32_t(bind+f))=0;wf(rdram,bind+0x1C,1);wf(rdram,bind+0x20,1);wf(rdram,bind+0x24,1);MEM_W(0x28,int32_t(bind))=n&&n<19?1:0;MEM_W(0x2C,int32_t(bind))=n==0?1:0;}
                    MEM_W(0x34,int32_t(asset))=0x900;MEM_W(0,int32_t(asset+0x900))=0x980;MEM_H(0,int32_t(asset+0x980))=0;MEM_H(2,int32_t(asset+0x980))=100;MEM_H(4,int32_t(asset+0x980))=50;MEM_H(6,int32_t(asset+0x980))=30;MEM_H(12,int32_t(asset+0x980))=-1;
                    auto links=[&] {pp=uint32_t(MEM_W(0x98,int32_t(parent)));pm=uint32_t(MEM_W(0x74,int32_t(parent)));sm=uint32_t(MEM_W(0x78,int32_t(parent)));const int index=MEM_H(0x80,int32_t(pp));child=index>=30&&index<230?0x80171CD0u+unsigned(index)*0xCC:0;cp=child?uint32_t(MEM_W(0x98,int32_t(child))):0;cm=child?uint32_t(MEM_W(0x74,int32_t(child))):0;};
                    auto create=[&] {if(gap)MEM_W(0x94,int32_t(prop))=99;c=saved;c.f_odd=&c.f0.u32h;switch(kind) {
                    case 13:if(frozen&&!native)reference_dora3_s8_level1_8022DC04(rdram,&c);else dora3_s8_level1_8022DC04(rdram,&c);break;
                    case 14:if(frozen&&!native)reference_dora3_s8_level1_8022E564(rdram,&c);else dora3_s8_level1_8022E564(rdram,&c);break;
                    case 15:if(frozen&&!native)reference_dora3_s8_level1_8022EE5C(rdram,&c);else dora3_s8_level1_8022EE5C(rdram,&c);break;
                    case 16:if(frozen&&!native)reference_dora3_s8_level1_8022F72C(rdram,&c);else dora3_s8_level1_8022F72C(rdram,&c);break;
                    case 17:if(frozen&&!native)reference_dora3_s8_level1_80230028(rdram,&c);else dora3_s8_level1_80230028(rdram,&c);break;
                    case 20:if(frozen&&!native)reference_dora3_s8_level1_8023103C(rdram,&c);else dora3_s8_level1_8023103C(rdram,&c);break;
                    case 21:if(frozen&&!native)reference_dora3_s8_level1_802318B0(rdram,&c);else dora3_s8_level1_802318B0(rdram,&c);break;
                    case 22:if(frozen&&!native)reference_dora3_s8_level1_80232414(rdram,&c);else dora3_s8_level1_80232414(rdram,&c);break;
                    default:std::abort();}if(gap)MEM_W(0x94,int32_t(prop))=0;links();assert(MEM_W(0x94,int32_t(parent))==100&&MEM_H(0x20,int32_t(pp))==int16_t(kind));};
                    city_source_stamp={birth,0};input_now=birth;if(birth) {const auto units=uint64_t(birth)*60,den=uint64_t(divider)*second;const SimulationSlice born{1,den,units/den,units%den};SimulationScope born_scope(born);create();}else create();
                    auto retire=[&] {links();if(child) {c.r4=int32_t(child);dora3_s3_801E656C(rdram,&c);c.r4=int32_t(child);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);c.r4=int32_t(child);dora3_scene_object_init(rdram,&c);}c.r4=int32_t(parent);dora3_s3_801E656C(rdram,&c);c.r4=int32_t(parent);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);c.r4=int32_t(parent);dora3_scene_object_init(rdram,&c);child=cp=cm=0;};
                    if(scenario==4)MEM_W(0x94,int32_t(parent))=99;
                    if(scenario==5) {MEM_W(0,int32_t(0x8021BDEC))=int32_t(parent);MEM_W(0,int32_t(0x8021BCDC))=int32_t(pp);MEM_W(0,int32_t(0x8021BCE4))=4;MEM_B(0,int32_t(0x8021710D))=0;city_watch=true;c=saved;c.f_odd=&c.f0.u32h;switch(kind) {
                    case 13:dora3_s8_level1_8022DD94(rdram,&c);break;case 14:dora3_s8_level1_8022E6F4(rdram,&c);break;case 15:dora3_s8_level1_8022EFC4(rdram,&c);break;case 16:dora3_s8_level1_8022F8AC(rdram,&c);break;case 17:dora3_s8_level1_80230190(rdram,&c);break;case 20:dora3_s8_level1_80231194(rdram,&c);break;case 21:dora3_s8_level1_80231A18(rdram,&c);break;case 22:dora3_s8_level1_80232578(rdram,&c);break;default:std::abort();}assert(MEM_W(0x94,int32_t(parent))==100&&MEM_B(0,int32_t(0x8021710D))==0);city_watch=false;}
                    auto record=[&] {if(!native) {auto context=c;context.f_odd=nullptr;result.original.push_back({memory,context,allocations,next_address,resource_references,city_commands,game_model_animation().model_count()});}};
                    if(!birth) {city_source_stamp={0,0};c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);links();record();}
                    city_commands.clear();preview_live=native;if(native&&!birth) {native_gameplay_preview_begin(rdram,0,divider);dora3_native_preview_hook(rdram,&c,0x801BB080);dora3_native_preview_hook(rdram,&c,0x801E8BDC);}
                    const std::array<Nanoseconds,5> edges{73'000'000,173'000'000,273'000'000,373'000'000,473'000'000};
                    auto to=[&](Nanoseconds stamp) {while(edge_mode&&edge<edges.size()&&edges[edge]<=stamp) {advance(edges[edge]);links();city_source_stamp={edges[edge],0};
                        if(edge_mode==1&&edge<2) {MEM_B(0xC9,int32_t(pp))=edge==0?1:0;if(child)MEM_B(0xC9,int32_t(cp))=edge==0?1:0;}
                        else if(edge_mode==2&&edge==0) {MEM_W(0x94,int32_t(parent))=99;c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);}
                        else if(edge_mode==2&&edge==1) {MEM_W(0x94,int32_t(parent))=100;MEM_B(0x28,int32_t(pp))=0;}
                        else if(edge_mode==3&&edge==1) {retire();const auto units=uint64_t(edges[edge])*60,den=uint64_t(divider)*second;const SimulationSlice born{1,den,units/den,units%den};SimulationScope born_scope(born);create();}
                        else if(edge==2) {MEM_W(0x94,int32_t(parent))=100;MEM_B(0x28,int32_t(pp))=0;if(child)MEM_B(0x28,int32_t(cp))=0;}
                        else if(edge==3) {wf(rdram,parent+0x10,33);wf(rdram,pp+4,33);}
                        else if(edge==4) {c.r4=int32_t(pm);dora3_s0_8003B744(rdram,&c);c.r4=int32_t(pm);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);}
                        ++edge;
                    }advance(stamp);links();};
                    for(unsigned tick=1;tick<=96;++tick) {const auto units=uint64_t(birth)*60+uint64_t(second)*divider*tick;input_now=Nanoseconds(units/60);city_source_stamp={input_now,unsigned(units%60)};
                        if(native) {const auto state=MEM_W(0x94,int32_t(parent));const auto latch=MEM_B(0x28,int32_t(pp));const auto count=city_commands.size();to(input_now);if(scenario==1||scenario==4) {assert(MEM_W(0x94,int32_t(parent))==state&&MEM_B(0x28,int32_t(pp))==latch&&city_commands.size()==count);c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);links();}}
                        else {const auto start=uint64_t(birth)*60+uint64_t(second)*divider*(tick-1);if(scenario!=1&&scenario!=4)city_source_initial_stamp=SourceTimestamp{Nanoseconds(start/60),unsigned(start%60)};c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);links();city_source_initial_stamp.reset();if(tick==1||tick==13||tick==14||tick==30||tick==31||tick==96)record();}
                        if(tick%3==0) {std::array<double,27> v{};v[0]=MEM_W(0x94,int32_t(parent));v[1]=rf(rdram,parent+0x10);v[2]=rf(rdram,pp+4);v[3]=MEM_BU(0x28,int32_t(pp));for(unsigned a=0;a<3;++a)v[4+a]=rf(rdram,parent+0x9C+4*a);v[7]=rf(rdram,pm+0x64);v[8]=sm?rf(rdram,sm+0x64):-1;v[9]=child?1:0;if(child) {v[10]=MEM_W(0x94,int32_t(child));v[11]=rf(rdram,child+0x10);for(unsigned a=0;a<3;++a)v[12+a]=rf(rdram,child+0x9C+4*a);v[15]=rf(rdram,cm+0x64);v[16]=MEM_BU(0x28,int32_t(cp));for(unsigned a=0;a<3;++a)v[17+a]=rf(rdram,child+0x18+4*a);v[21]=MEM_BU(0xC9,int32_t(cp));}v[20]=MEM_BU(0xC9,int32_t(pp));v[22]=allocations.size();v[23]=resource_references[2];v[24]=game_model_animation().model_count();v[25]=MEM_H(0x9E,int32_t(pp));v[26]=child?MEM_H(0x7C,int32_t(cp)):-1;result.samples.push_back(v);}
                    }
                    result.commands=city_commands;linked_watch=patrol_original_reference=false;native_gameplay_preview_reset(rdram);preview_live=false;retire();assert(allocations==story_heap&&resource_references==story_refs&&game_model_animation().model_count()==6);linked_construct_watch=city_loaded=false;memory=story_memory;allocations=story_heap;next_address=story_next;resource_references=story_refs;return result;
                };
                auto compare=[&](const StoryResult& a,const StoryResult& b,unsigned mode) {assert(a.samples.size()==b.samples.size());for(unsigned n=0;n<a.samples.size();++n)for(unsigned k=0;k<a.samples[n].size();++k) {const bool geometry=k==1||k==2||(k>=4&&k<=8)||(k>=11&&k<=15)||(k>=17&&k<=19);const double tolerance=geometry?0.03:0;if(std::abs(a.samples[n][k]-b.samples[n][k])>tolerance) {std::cerr<<"Story family mismatch kind="<<kind<<" gap="<<gap<<" scenario="<<scenario<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<" tick="<<3*(n+1)<<" field="<<k<<" actual="<<a.samples[n][k]<<" expected="<<b.samples[n][k]<<'\n';return false;}}if(a.commands!=b.commands) {std::cerr<<"Story family commands kind="<<kind<<" gap="<<gap<<" scenario="<<scenario<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<'\n';return false;}return true;};
                const auto expected=run_story(false,60),original=run_story(false,60,0,false);
                assert(expected.original.size()==original.original.size());for(unsigned n=0;n<expected.original.size();++n) {const auto& a=expected.original[n];const auto& b=original.original[n];if(a.ram!=b.ram||std::memcmp(&a.context,&b.context,sizeof(c))!=0||a.heap!=b.heap||a.next!=b.next||a.refs!=b.refs||a.commands!=b.commands||a.models!=b.models) {
                    std::cerr<<"Story Original mismatch kind="<<kind<<" gap="<<gap<<" scenario="<<scenario<<" checkpoint="<<n<<" fps="<<fps<<" divider="<<divider<<" ram="<<(a.ram==b.ram)<<" context="<<(std::memcmp(&a.context,&b.context,sizeof(c))==0)<<" heap="<<(a.heap==b.heap)<<" next="<<(a.next==b.next)<<" refs="<<(a.refs==b.refs)<<" commands="<<(a.commands==b.commands)<<" models="<<(a.models==b.models)<<'\n';
                    for(unsigned i=0,shown=0;i<a.ram.size()&&shown<12;++i)if(a.ram[i]!=b.ram[i]) {std::cerr<<" ram "<<std::hex<<0x80000000u+4*i<<" expected="<<a.ram[i]<<" actual="<<b.ram[i]<<std::dec<<'\n';++shown;}
                    return 1;
                }++city_story_original_cases;}
                if(!compare(original,expected,0))return 1;
                const auto actual=run_story(true,fps);if(!compare(actual,expected,0))return 1;
                if(scenario==0)for(unsigned mode:{1u,2u,3u}) {const auto baseline=run_story(true,60,mode),varied=run_story(true,fps,mode);if(!compare(varied,baseline,mode))return 1;++city_story_native_cases;}
                if(scenario==1||scenario==4)++city_story_rejections;
                ++city_story_source_cases;
            }
            // Special200/201 uses the actual profile2 constructor and contact
            // producer, preserving the six-tick release and Original yaw snaps.
            for(unsigned kind:{21u})for(bool gap:{false,true})for(unsigned scenario=0;scenario<16;++scenario) {
                const auto special_memory=memory;const auto special_heap=allocations;const auto special_next=next_address;const auto special_refs=resource_references;
                struct SpecialOriginal {std::vector<uint32_t> ram;recomp_context context;std::set<uint32_t> heap;uint32_t next;decltype(resource_references) refs;std::vector<CityCommand> commands;std::size_t models;};
                struct SpecialResult {std::vector<std::array<double,27>> samples;std::vector<CityCommand> commands;std::vector<SpecialOriginal> original;};
                auto run_story=[&](bool native,unsigned render_fps,unsigned edge_mode=0,bool frozen=true) {
                    native_gameplay_preview_reset(rdram);preview_live=false;memory=special_memory;allocations=special_heap;next_address=special_next;resource_references=special_refs;input_now=0;
                    room(rdram);city_loaded=linked_construct_watch=linked_watch=true;city_identity_verified=scenario!=7;patrol_original_reference=frozen&&!native;linked_original_reference=false;
                    city_watch=patrol_watch=city_dialogue_watch=false;city_commands.clear();city_source_initial_stamp.reset();
                    for(unsigned slot=30;slot<230;++slot) {c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(0x80171CD0u+slot*0xCC);dora3_scene_object_init(rdram,&c);MEM_H(0xA8,int32_t(0x80171CD0u+slot*0xCC))=-1;}
                    MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;wf(rdram,physics,0);wf(rdram,physics+0x14,0);MEM_W(0x94,int32_t(player))=1;
                    wf(rdram,player+0x9C,-700);wf(rdram,player+0xA0,80);wf(rdram,player+0xA4,-50);
                    const Nanoseconds birth=scenario==5?73'000'000:scenario==6?173'000'000:0;
                    if(native&&birth) {preview_live=true;native_gameplay_preview_begin(rdram,0,divider);dora3_native_preview_hook(rdram,&c,0x801BB080);dora3_native_preview_hook(rdram,&c,0x801E8BDC);}
                    unsigned frame=0,edge=0;uint32_t parent=gap?prop+0xCC:prop,pp=0,pm=0,sm=0,child=0,cp=0,cm=0;SpecialResult result;
                    auto advance=[&](Nanoseconds stamp) {while(second*(frame+1)/render_fps<stamp) {input_now=second*++frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);assert(std::memcmp(&c,&saved,sizeof(c))==0);}input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);assert(std::memcmp(&c,&saved,sizeof(c))==0);};
                    if(native&&birth)advance(birth);
                    MEM_W(0,int32_t(0x801B3628))=0;MEM_W(12,int32_t(0x801B3628))=0;c.r4=0x7E;dora3_s1_801B4FBC(rdram,&c);
                    MEM_H(0,int32_t(0x8021BE02))=-500;MEM_H(2,int32_t(0x8021BE02))=80;MEM_H(4,int32_t(0x8021BE02))=-300;MEM_H(6,int32_t(0x8021BE02))=359;
                    MEM_W(0,int32_t(0x801AEC00+20))=int32_t(asset);MEM_W(0,int32_t(0x801AEC00+40))=int32_t(clip);MEM_W(0,int32_t(0x801AEC00+80))=int32_t(descriptor);
                    MEM_H(0x2A,int32_t(asset))=20;MEM_W(0x2C,int32_t(asset))=0x700;
                    for(unsigned n=0;n<20;++n) {const auto bind=asset+0x100+48*n;for(unsigned f=0;f<48;f+=4)MEM_W(0,int32_t(bind+f))=0;wf(rdram,bind+0x1C,1);wf(rdram,bind+0x20,1);wf(rdram,bind+0x24,1);MEM_W(0x28,int32_t(bind))=n&&n<19?1:0;MEM_W(0x2C,int32_t(bind))=n==0?1:0;}
                    MEM_W(0x34,int32_t(asset))=0x900;MEM_W(0,int32_t(asset+0x900))=0x980;MEM_H(0,int32_t(asset+0x980))=0;MEM_H(2,int32_t(asset+0x980))=100;MEM_H(4,int32_t(asset+0x980))=50;MEM_H(6,int32_t(asset+0x980))=30;MEM_H(12,int32_t(asset+0x980))=-1;
                    auto links=[&] {pp=uint32_t(MEM_W(0x98,int32_t(parent)));pm=uint32_t(MEM_W(0x74,int32_t(parent)));sm=uint32_t(MEM_W(0x78,int32_t(parent)));const int index=MEM_H(0x80,int32_t(pp));child=index>=30&&index<230?0x80171CD0u+unsigned(index)*0xCC:0;cp=child?uint32_t(MEM_W(0x98,int32_t(child))):0;cm=child?uint32_t(MEM_W(0x74,int32_t(child))):0;};
                    auto create=[&] {if(gap)MEM_W(0x94,int32_t(prop))=99;c=saved;c.f_odd=&c.f0.u32h;switch(kind) {
                    case 13:if(frozen&&!native)reference_dora3_s8_level1_8022DC04(rdram,&c);else dora3_s8_level1_8022DC04(rdram,&c);break;
                    case 14:if(frozen&&!native)reference_dora3_s8_level1_8022E564(rdram,&c);else dora3_s8_level1_8022E564(rdram,&c);break;
                    case 15:if(frozen&&!native)reference_dora3_s8_level1_8022EE5C(rdram,&c);else dora3_s8_level1_8022EE5C(rdram,&c);break;
                    case 16:if(frozen&&!native)reference_dora3_s8_level1_8022F72C(rdram,&c);else dora3_s8_level1_8022F72C(rdram,&c);break;
                    case 17:if(frozen&&!native)reference_dora3_s8_level1_80230028(rdram,&c);else dora3_s8_level1_80230028(rdram,&c);break;
                    case 20:if(frozen&&!native)reference_dora3_s8_level1_8023103C(rdram,&c);else dora3_s8_level1_8023103C(rdram,&c);break;
                    case 21:if(frozen&&!native)reference_dora3_s8_level1_802318B0(rdram,&c);else dora3_s8_level1_802318B0(rdram,&c);break;
                    case 22:if(frozen&&!native)reference_dora3_s8_level1_80232414(rdram,&c);else dora3_s8_level1_80232414(rdram,&c);break;
                    default:std::abort();}if(gap)MEM_W(0x94,int32_t(prop))=0;links();assert(MEM_W(0x94,int32_t(parent))==200&&MEM_H(0x20,int32_t(pp))==int16_t(kind));};
                    city_source_stamp={birth,0};input_now=birth;if(birth) {const auto units=uint64_t(birth)*60,den=uint64_t(divider)*second;const SimulationSlice born{1,den,units/den,units%den};SimulationScope born_scope(born);create();}else create();
                    auto retire=[&] {links();if(child) {c.r4=int32_t(child);dora3_s3_801E656C(rdram,&c);c.r4=int32_t(child);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);c.r4=int32_t(child);dora3_scene_object_init(rdram,&c);}c.r4=int32_t(parent);dora3_s3_801E656C(rdram,&c);c.r4=int32_t(parent);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);c.r4=int32_t(parent);dora3_scene_object_init(rdram,&c);child=cp=cm=0;};
                    if(scenario==9)MEM_W(0x94,int32_t(parent))=300;
                    if(scenario==8)MEM_W(0x94,int32_t(parent))=299;
                    if(scenario==3) {
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(parent);dora3_s3_demo_801E8590(rdram,&c);
                        wf(rdram,pp+4,float(std::fmod(double(c.f0.fl)+5.,360.)));wf(rdram,parent+0x10,350);
                    }
                    if(scenario!=0&&scenario!=4&&scenario!=9&&scenario!=10&&scenario!=13&&scenario!=14) {
                        MEM_W(0,int32_t(0x8021BDEC))=int32_t(parent);MEM_W(0,int32_t(0x8021BCDC))=int32_t(pp);
                        MEM_W(0,int32_t(0x8021BCE4))=scenario==15?3:4;MEM_B(0,int32_t(0x8021710D))=0;
                        city_watch=true;c=saved;c.f_odd=&c.f0.u32h;
                        if(frozen&&!native)reference_dora3_s8_level1_80231A18(rdram,&c);else dora3_s8_level1_80231A18(rdram,&c);
                        assert(MEM_W(0x94,int32_t(parent))==(scenario==15?200:201));
                        assert(MEM_B(0,int32_t(0x8021710D))==(scenario==15?0:1));city_watch=false;
                    }
                    if(scenario==4||scenario==10||scenario==13||scenario==14) {
                        MEM_W(0x94,int32_t(parent))=201;MEM_B(0x28,int32_t(pp))=1;
                        MEM_H(0x34,int32_t(pp))=1;MEM_H(0x36,int32_t(pp))=scenario==10?5:1;
                        wf(rdram,parent+0x10,scenario==13?10:350);wf(rdram,pp+4,scenario==13?10:350);
                        wf(rdram,pp+0x58,scenario==13?240:120);wf(rdram,pp+0x5C,scenario==13?-5:5);
                    }
                    MEM_B(0,int32_t(0x8021710D))=scenario==1||scenario==3||scenario==5||scenario==6||scenario==7||scenario==8||scenario==11||scenario==12?1:0;
                    auto record=[&] {if(!native) {auto context=c;context.f_odd=nullptr;result.original.push_back({memory,context,allocations,next_address,resource_references,city_commands,game_model_animation().model_count()});}};
                    if(!birth) {city_source_stamp={0,0};c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);links();record();}
                    city_commands.clear();preview_live=native;if(native&&!birth) {native_gameplay_preview_begin(rdram,0,divider);dora3_native_preview_hook(rdram,&c,0x801BB080);dora3_native_preview_hook(rdram,&c,0x801E8BDC);}
                    const std::array<Nanoseconds,5> edges=edge_mode==4?std::array<Nanoseconds,5>{73'000'000,80'000'000,87'000'000,173'000'000,273'000'000}:std::array<Nanoseconds,5>{73'000'000,173'000'000,273'000'000,373'000'000,473'000'000};
                    auto to=[&](Nanoseconds stamp) {while(edge_mode&&edge<edges.size()&&edges[edge]<=stamp) {advance(edges[edge]);links();city_source_stamp={edges[edge],0};
                        if(edge_mode==4&&edge<3) {
                            const int expected=1+int(uint64_t(edges[edge])*60/(uint64_t(divider)*second))+1;
                            assert(MEM_W(0x94,int32_t(parent))==201&&MEM_H(0x36,int32_t(pp))==expected);
                            wf(rdram,parent+0x10,33.f+edge);wf(rdram,pp+4,33.f+edge);
                            advance(edges[edge]+1);links();assert(MEM_H(0x36,int32_t(pp))==expected);++city_special_counter_samples;
                        }
                        else if(edge_mode==1&&edge<2) {MEM_B(0xC9,int32_t(pp))=edge==0?1:0;if(child)MEM_B(0xC9,int32_t(cp))=edge==0?1:0;}
                        else if(edge_mode==2&&edge==0) {MEM_W(0x94,int32_t(parent))=99;c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);}
                        else if(edge_mode==2&&edge==1) {MEM_W(0x94,int32_t(parent))=201;MEM_B(0x28,int32_t(pp))=0;}
                        else if(edge_mode==3&&edge==1) {retire();const auto units=uint64_t(edges[edge])*60,den=uint64_t(divider)*second;const SimulationSlice born{1,den,units/den,units%den};SimulationScope born_scope(born);create();}
                        else if(edge==2) {MEM_W(0x94,int32_t(parent))=201;MEM_B(0x28,int32_t(pp))=0;if(child)MEM_B(0x28,int32_t(cp))=0;}
                        else if(edge==3) {wf(rdram,parent+0x10,33);wf(rdram,pp+4,33);}
                        else if(edge==4) {c.r4=int32_t(pm);dora3_s0_8003B744(rdram,&c);c.r4=int32_t(pm);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);}
                        ++edge;
                    }advance(stamp);links();};
                    for(unsigned tick=1;tick<=96;++tick) {
                        if(tick==13&&!edge_mode&&(scenario==1||scenario==3||scenario==5||scenario==6||scenario==8||scenario==12)) {
                            if(native)to(birth+second*divider*(tick-1)/60);
                            if(scenario==12) {wf(rdram,player+0x9C,-900);wf(rdram,player+0xA4,-300);}
                            else MEM_B(0,int32_t(0x8021710D))=0;
                        }
                        const auto units=uint64_t(birth)*60+uint64_t(second)*divider*tick;input_now=Nanoseconds(units/60);city_source_stamp={input_now,unsigned(units%60)};
                        if(native) {const auto state=MEM_W(0x94,int32_t(parent));const auto latch=MEM_B(0x28,int32_t(pp));const auto count=city_commands.size();to(input_now);if(scenario==7||scenario==9) {assert(MEM_W(0x94,int32_t(parent))==state&&MEM_B(0x28,int32_t(pp))==latch&&city_commands.size()==count);c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);links();}}
                        else {const auto start=uint64_t(birth)*60+uint64_t(second)*divider*(tick-1);if(scenario!=7&&scenario!=9)city_source_initial_stamp=SourceTimestamp{Nanoseconds(start/60),unsigned(start%60)};c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);links();city_source_initial_stamp.reset();if(tick==1||tick==13||tick==14||tick==30||tick==31||tick==96)record();}
                        if(tick%3==0) {
                            std::array<double,27> v{};v[0]=MEM_W(0x94,int32_t(parent));v[1]=rf(rdram,parent+0x10);v[2]=rf(rdram,pp+4);v[3]=MEM_BU(0x28,int32_t(pp));
                            for(unsigned a=0;a<3;++a)v[4+a]=rf(rdram,parent+0x9C+4*a);
                            v[7]=rf(rdram,pm+0x64);v[8]=rf(rdram,pp+0x5C);v[9]=MEM_H(0x34,int32_t(pp));v[10]=MEM_H(0x36,int32_t(pp));v[11]=rf(rdram,pp+0x58);
                            v[12]=MEM_H(0x42,int32_t(pp));v[13]=MEM_H(0x44,int32_t(pp));v[14]=MEM_BU(0,int32_t(0x8021710D));
                            for(unsigned a=0;a<3;++a)v[15+a]=rf(rdram,parent+0x18+4*a);
                            v[18]=MEM_H(0xD8,int32_t(pp));v[19]=MEM_BU(0xC9,int32_t(pp));v[20]=game_model_animation().model_count();v[21]=allocations.size();v[22]=resource_references[2];v[23]=sm?1:0;
                            v[24]=rf(rdram,pp);v[25]=rf(rdram,pp+0x14);v[26]=resource_references[1];result.samples.push_back(v);
                        }
                    }
                    result.commands=city_commands;linked_watch=patrol_original_reference=false;native_gameplay_preview_reset(rdram);preview_live=false;retire();assert(allocations==special_heap&&resource_references==special_refs&&game_model_animation().model_count()==6);linked_construct_watch=city_loaded=false;memory=special_memory;allocations=special_heap;next_address=special_next;resource_references=special_refs;return result;
                };
                auto compare=[&](const SpecialResult& a,const SpecialResult& b,unsigned mode) {assert(a.samples.size()==b.samples.size());for(unsigned n=0;n<a.samples.size();++n)for(unsigned k=0;k<a.samples[n].size();++k) {const bool geometry=k==1||k==2||(k>=4&&k<=8)||k==11||(k>=15&&k<=17)||k==24||k==25;const double tolerance=geometry?0.03:0;if(std::abs(a.samples[n][k]-b.samples[n][k])>tolerance) {std::cerr<<"Special family mismatch kind="<<kind<<" gap="<<gap<<" scenario="<<scenario<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<" tick="<<3*(n+1)<<" field="<<k<<" actual="<<a.samples[n][k]<<" expected="<<b.samples[n][k]<<'\n';return false;}}if(a.commands!=b.commands) {std::cerr<<"Special family commands kind="<<kind<<" gap="<<gap<<" scenario="<<scenario<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<'\n';return false;}return true;};
                const auto expected=run_story(false,60),original=run_story(false,60,0,false);
                assert(expected.original.size()==original.original.size());for(unsigned n=0;n<expected.original.size();++n) {const auto& a=expected.original[n];const auto& b=original.original[n];if(a.ram!=b.ram||std::memcmp(&a.context,&b.context,sizeof(c))!=0||a.heap!=b.heap||a.next!=b.next||a.refs!=b.refs||a.commands!=b.commands||a.models!=b.models) {
                    std::cerr<<"Special Original mismatch kind="<<kind<<" gap="<<gap<<" scenario="<<scenario<<" checkpoint="<<n<<" fps="<<fps<<" divider="<<divider<<" ram="<<(a.ram==b.ram)<<" context="<<(std::memcmp(&a.context,&b.context,sizeof(c))==0)<<" heap="<<(a.heap==b.heap)<<" next="<<(a.next==b.next)<<" refs="<<(a.refs==b.refs)<<" commands="<<(a.commands==b.commands)<<" models="<<(a.models==b.models)<<'\n';
                    for(unsigned i=0,shown=0;i<a.ram.size()&&shown<12;++i)if(a.ram[i]!=b.ram[i]) {std::cerr<<" ram "<<std::hex<<0x80000000u+4*i<<" expected="<<a.ram[i]<<" actual="<<b.ram[i]<<std::dec<<'\n';++shown;}
                    return 1;
                }++city_special_original_cases;}
                if(!compare(original,expected,0))return 1;
                const auto actual=run_story(true,fps);if(!compare(actual,expected,0))return 1;
                if(scenario==2)for(unsigned mode:{1u,2u,3u,4u}) {const auto baseline=run_story(true,60,mode),varied=run_story(true,fps,mode);if(!compare(varied,baseline,mode))return 1;++city_special_native_cases;}
                if(scenario==7||scenario==9)++city_special_rejections;
                ++city_special_source_cases;
            }
            // Execute the real descriptor parser, world constructor and linked
            // constructor. Synthetic asset lookup is a checked boundary.
            for(unsigned variant:{0u})for(bool child_first:{false,true}) {
                const auto linked_memory=memory;const auto linked_heap=allocations;
                const auto linked_next=next_address;const auto linked_refs=resource_references;
                native_gameplay_preview_reset(rdram);preview_live=false;room(rdram);
                city_loaded=true;linked_construct_watch=true;
                for(unsigned slot=30;slot<230;++slot) {
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(0x80171CD0u+slot*0xCC);dora3_scene_object_init(rdram,&c);
                    MEM_H(0xA8,int32_t(0x80171CD0u+slot*0xCC))=-1; // Synthetic slots have no separate resource owner.
                }
                // Flags select exactly the documented constructor variants.
                MEM_W(0,int32_t(0x801B3628))=0;MEM_W(12,int32_t(0x801B3628))=0;
                if(variant==2) {c.r4=0x7E;dora3_s1_801B4FBC(rdram,&c);}
                // Leave an earlier free slot only after creating the parent.
                // The real attached allocator starts at the parent and ignores this earlier gap.
                if(child_first)MEM_W(0x94,int32_t(prop))=99;
                MEM_H(0,int32_t(0x8021BE02))=-500;MEM_H(2,int32_t(0x8021BE02))=80;
                MEM_H(4,int32_t(0x8021BE02))=-300;MEM_H(6,int32_t(0x8021BE02))=359;
                MEM_W(0,int32_t(0x801AEC00+20))=int32_t(asset);
                MEM_W(0,int32_t(0x801AEC00+40))=int32_t(clip);
                MEM_W(0,int32_t(0x801AEC00+80))=int32_t(descriptor);
                c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(0x8023310C);
                c.r5=std::bit_cast<int32_t>(-500.f);c.r6=std::bit_cast<int32_t>(80.f);c.r7=std::bit_cast<int32_t>(-300.f);
                MEM_W(0x10,c.r29)=std::bit_cast<int32_t>(359.f);actual_dora3_s3_801E6968(rdram,&c);
                const auto father=uint32_t(c.r2);assert(father==(child_first?prop+0xCC:prop));
                const auto father_physical=uint32_t(MEM_W(0x98,int32_t(father)));
                assert(MEM_H(0x20,int32_t(father_physical))==18);
                if(child_first)MEM_W(0x94,int32_t(prop))=0;
                // Call the actual attachment separately to exercise both slot
                // layouts without replacing its allocator or link bookkeeping.
                c.r4=int32_t(father);c.r5=0;c.r6=int32_t(0x80233080);c.r7=std::bit_cast<int32_t>(-500.f);
                MEM_W(0x10,c.r29)=std::bit_cast<int32_t>(80.f);MEM_W(0x14,c.r29)=std::bit_cast<int32_t>(-300.f);
                MEM_W(0x18,c.r29)=std::bit_cast<int32_t>(359.f);dora3_s3_demo_801E70E0(rdram,&c);
                const auto leg=uint32_t(c.r2),leg_physical=uint32_t(MEM_W(0x98,int32_t(leg)));
                assert(leg==father+0xCC); // Actual E5DF4 starts at parent index, never reuses an earlier gap.
                const auto father_index=(father-0x80171CD0u)/0xCC,leg_index=(leg-0x80171CD0u)/0xCC;
                assert(MEM_H(0x20,int32_t(leg_physical))==19);
                assert(MEM_H(0x7C,int32_t(leg_physical))==int16_t(father_index)&&MEM_H(0x80,int32_t(father_physical))==int16_t(leg_index));
                assert(game_model_animation().model_count()==8);
                wf(rdram,father_physical+4,359);MEM_H(0x44,int32_t(father_physical))=variant==2?4:0;
                MEM_W(0x94,int32_t(father))=1;MEM_W(0x94,int32_t(leg))=1;
                for(unsigned tick=0;tick<24;++tick) {
                    input_now=second*divider*tick/60;city_source_stamp={input_now,0};
                    c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);
                }
                // Original unlink and destruction are separate from pool
                // initialization. They do not recursively retire the child.
                c.r4=int32_t(father);dora3_s3_801E656C(rdram,&c);
                assert(MEM_H(0x9E,int32_t(leg_physical))==-1);
                c.r4=int32_t(father);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);
                assert(MEM_W(0x98,int32_t(father))==0&&MEM_W(0x98,int32_t(leg))==int32_t(leg_physical));
                assert(game_model_animation().model_count()==7);
                c.r4=int32_t(leg);dora3_s3_801E656C(rdram,&c);
                c.r4=int32_t(leg);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);
                c.r4=int32_t(father);dora3_scene_object_init(rdram,&c);
                c.r4=int32_t(leg);dora3_scene_object_init(rdram,&c);
                assert(MEM_W(0x94,int32_t(father))==0&&MEM_W(0x94,int32_t(leg))==0);
                assert(allocations==linked_heap&&resource_references==linked_refs&&game_model_animation().model_count()==6);
                linked_construct_watch=false;city_loaded=false;++linked_constructor_cases;
                memory=linked_memory;allocations=linked_heap;next_address=linked_next;resource_references=linked_refs;
            }

            // Linked18/19 real constructor, command order and private FSM are
            // compared with frozen189. Asset decoding and sprite2 stay checked
            // boundaries; model/resource lists and unlink/free execute Original.
            for(unsigned variant:{0u,1u,2u})for(bool earlier_gap:{false,true})for(unsigned scenario=0;scenario<8;++scenario) {
                // The actual story constructor selects idle100/clip8; its
                // reviewed contact producer forbids dialogue2 in this state.
                if(variant==1&&scenario>=1&&scenario<=3)continue;
                const auto linked_memory=memory;const auto linked_heap=allocations;
                const auto linked_next=next_address;const auto linked_refs=resource_references;
                struct LinkedResult {std::array<double,2> initial{},first_end{};std::vector<std::array<double,25>> samples;std::vector<CityCommand> commands;};
                auto run_linked=[&](bool native,unsigned render_fps,unsigned edge_mode=0,const LinkedResult* oracle=nullptr) {
                    native_gameplay_preview_reset(rdram);preview_live=false;memory=linked_memory;
                    allocations=linked_heap;next_address=linked_next;resource_references=linked_refs;
                    room(rdram);input_now=0;city_loaded=linked_construct_watch=linked_watch=true;
                    city_identity_verified=scenario!=4;linked_original_reference=!native;city_commands.clear();linked_publications.clear();city_source_initial_stamp.reset();
                    for(unsigned slot=30;slot<230;++slot) {
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(0x80171CD0u+slot*0xCC);dora3_scene_object_init(rdram,&c);
                        MEM_H(0xA8,int32_t(0x80171CD0u+slot*0xCC))=-1;
                    }
                    MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                    wf(rdram,physics,0);wf(rdram,physics+0x14,0);MEM_W(0x94,int32_t(player))=1;
                    wf(rdram,player+0x9C,-700);wf(rdram,player+0xA0,80);wf(rdram,player+0xA4,-50);
                    const Nanoseconds birth_stamp=scenario==5?73'000'000:scenario==6?173'000'000:0;
                    if(native&&birth_stamp) {preview_live=true;native_gameplay_preview_begin(rdram,0,divider);
                        dora3_native_preview_hook(rdram,&c,0x801BB080);dora3_native_preview_hook(rdram,&c,0x801E8BDC);}
                    unsigned frame=0,edge=0;LinkedResult result;uint32_t father=0,pp=0,pm=0,leg=0,lp=0,lm=0;
                    auto observe=[&](Nanoseconds stamp) {
                        if(!oracle||variant!=2||scenario!=1||stamp<=birth_stamp||uint64_t(stamp-birth_stamp)*60>=uint64_t(second)*divider)return;
                        const double fraction=double(stamp-birth_stamp)*60/(double(second)*divider);
                        const double expected=std::fmod(oracle->initial[0]+std::remainder(oracle->first_end[0]-oracle->initial[0],360.)*fraction+360.,360.);
                        if(std::abs(std::remainder(double(rf(rdram,father+0x10))-expected,360.))>=0.001) {
                            std::cerr<<"Linked intermediate yaw mismatch fps="<<render_fps<<" divider="<<divider<<" stamp="<<stamp<<" actual="<<rf(rdram,father+0x10)<<" expected="<<expected<<'\n';std::abort();
                        }
                        ++linked_yaw_samples;
                    };
                    auto advance=[&](Nanoseconds stamp) {
                        while(second*(frame+1)/render_fps<stamp) {
                            input_now=second*++frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;
                            native_gameplay_preview_update(rdram,&c,input_now);assert(std::memcmp(&c,&saved,sizeof(c))==0);if(father)observe(input_now);
                        }
                        input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                        assert(std::memcmp(&c,&saved,sizeof(c))==0);if(father)observe(stamp);
                    };
                    if(native&&birth_stamp)advance(birth_stamp);
                    MEM_W(0,int32_t(0x801B3628))=0;MEM_W(12,int32_t(0x801B3628))=0;
                    if(variant==2) {c.r4=0x7E;dora3_s1_801B4FBC(rdram,&c);}
                    else if(variant==1) {c.r4=1;dora3_s1_801B4FBC(rdram,&c);}
                    MEM_H(0,int32_t(0x8021BE02))=-500;MEM_H(2,int32_t(0x8021BE02))=80;
                    MEM_H(4,int32_t(0x8021BE02))=-300;MEM_H(6,int32_t(0x8021BE02))=359;
                    MEM_W(0,int32_t(0x801AEC00+20))=int32_t(asset);MEM_W(0,int32_t(0x801AEC00+40))=int32_t(clip);
                    MEM_W(0,int32_t(0x801AEC00+80))=int32_t(descriptor);
                    if(earlier_gap)MEM_W(0x94,int32_t(prop))=99;
                    input_now=birth_stamp;c=saved;c.f_odd=&c.f0.u32h;
                    auto create=[&] {dora3_s8_level1_802309C0(rdram,&c);};
                    if(birth_stamp) {const auto units=uint64_t(birth_stamp)*60,den=uint64_t(divider)*second;
                        const SimulationSlice born{1,den,units/den,units%den};SimulationScope birth(born);create();}
                    else create();
                    father=earlier_gap?prop+0xCC:prop;pp=uint32_t(MEM_W(0x98,int32_t(father)));pm=uint32_t(MEM_W(0x74,int32_t(father)));
                    if(earlier_gap)MEM_W(0x94,int32_t(prop))=0;
                    assert(MEM_H(0x20,int32_t(pp))==18&&MEM_H(0x44,int32_t(pp))==int16_t(variant==2?4:variant==1?8:0));
                    if(variant!=2) {leg=father+0xCC;lp=uint32_t(MEM_W(0x98,int32_t(leg)));lm=uint32_t(MEM_W(0x74,int32_t(leg)));
                        assert(MEM_H(0x20,int32_t(lp))==(variant==1?6:19)&&MEM_H(0x7C,int32_t(lp))==int16_t((father-0x80171CD0u)/0xCC));}
                    else assert(MEM_H(0x80,int32_t(pp))==-1);
                    assert(game_model_animation().model_count()==(variant!=2?8u:7u));
                    MEM_B(0,int32_t(0x8021710D))=1;
                    if(scenario==1||scenario==5||scenario==6) {
                        // Enter dialogue through actual30BB0/91D4/713C. The
                        // real story100 producer explicitly refuses this input.
                        city_source_stamp={birth_stamp,0};city_source_initial_stamp.reset();city_watch=true;
                        MEM_W(0,int32_t(0x8021BDEC))=int32_t(father);MEM_W(0,int32_t(0x8021BCDC))=int32_t(pp);
                        MEM_W(0,int32_t(0x8021BCE4))=4;c=saved;c.f_odd=&c.f0.u32h;
                        if(!native) {
                            const auto before=memory;const auto before_commands=city_commands;const auto before_context=c;
                            const auto before_heap=allocations;const auto before_next=next_address;const auto before_refs=resource_references;
                            reference_dora3_s8_level1_80230BB0(rdram,&c);
                            const auto expected=memory;const auto expected_commands=city_commands;const auto expected_context=c;
                            const auto expected_heap=allocations;const auto expected_next=next_address;const auto expected_refs=resource_references;
                            memory=before;city_commands=before_commands;c=before_context;allocations=before_heap;next_address=before_next;resource_references=before_refs;
                            dora3_s8_level1_80230BB0(rdram,&c);
                            assert(memory==expected&&city_commands==expected_commands&&std::memcmp(&c,&expected_context,sizeof(c))==0);
                            assert(allocations==expected_heap&&next_address==expected_next&&resource_references==expected_refs);++linked_contact_cases;
                        } else dora3_s8_level1_80230BB0(rdram,&c);
                        assert(MEM_W(0x94,int32_t(father))==(variant==1?100:2));
                        if(variant!=1)assert(MEM_BU(0x28,int32_t(pp))==0&&MEM_B(0,int32_t(0x8021710D))==1);
                        city_watch=false;
                    }
                    if(scenario==7)MEM_W(0x94,int32_t(father))=99;
                    auto source_pass=[&](bool compare_original) {
                        if(!compare_original) {reference_dora3_s3_801E8BDC(rdram,&c);return;}
                        const auto before=memory;const auto before_context=c;const auto before_commands=city_commands;const auto before_publications=linked_publications;
                        const auto before_heap=allocations;const auto before_next=next_address;const auto before_refs=resource_references;
                        assert(linked_original_reference);reference_dora3_s3_801E8BDC(rdram,&c);
                        const auto expected=memory;const auto expected_context=c;const auto expected_commands=city_commands;const auto expected_publications=linked_publications;
                        const auto expected_heap=allocations;const auto expected_next=next_address;const auto expected_refs=resource_references;
                        memory=before;allocations=before_heap;next_address=before_next;resource_references=before_refs;c=before_context;
                        city_commands=before_commands;linked_publications=before_publications;linked_original_reference=false;reference_dora3_s3_801E8BDC(rdram,&c);linked_original_reference=true;
                        assert(memory==expected&&std::memcmp(&c,&expected_context,sizeof(c))==0&&city_commands==expected_commands&&linked_publications==expected_publications);
                        assert(allocations==expected_heap&&next_address==expected_next&&resource_references==expected_refs);++linked_original_ram_cases;
                    };
                    if(!birth_stamp) {city_source_stamp={0,0};source_pass(!native);}
                    if(scenario==2||scenario==3) {
                        MEM_W(0x94,int32_t(father))=2;MEM_B(0x28,int32_t(pp))=1;MEM_H(0x34,int32_t(pp))=1;MEM_H(0x36,int32_t(pp))=1;
                        wf(rdram,father+0x10,scenario==2?350.f:10.f);wf(rdram,pp+0x58,scenario==2?120.f:240.f);
                        wf(rdram,pp+0x5C,scenario==2?5.f:-5.f);
                    }
                    result.initial={rf(rdram,father+0x10),rf(rdram,pp+4)};city_commands.clear();linked_publications.clear();preview_live=native;
                    if(native&&!birth_stamp) {native_gameplay_preview_begin(rdram,0,divider);
                        // Use production Source gates to select only the world
                        // pass. Player idle rebinds otherwise replace an unrelated
                        // baseline heap channel and move the looked-at target.
                        dora3_native_preview_hook(rdram,&c,0x801BB080);dora3_native_preview_hook(rdram,&c,0x801E8BDC);}
                    const std::array<Nanoseconds,5> edges{73'000'000,173'000'000,273'000'000,373'000'000,473'000'000};
                    auto to=[&](Nanoseconds stamp) {
                        while(edge_mode&&edge<edges.size()&&edges[edge]<=stamp) {
                            advance(edges[edge]);city_source_stamp={edges[edge],0};
                            if(edge_mode==1&&edge<2) {MEM_B(0xC9,int32_t(pp))=edge==0?1:0;if(lp)MEM_B(0xC9,int32_t(lp))=edge==0?1:0;}
                            else if(edge_mode==2&&edge==0) {MEM_W(0x94,int32_t(father))=99;c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);}
                            else if(edge_mode==2&&edge==1) {MEM_W(0x94,int32_t(father))=variant==1?100:2;MEM_B(0x28,int32_t(pp))=0;}
                            else if(edge_mode==3&&edge==1) {
                                const auto units=uint64_t(edges[edge])*60,den=uint64_t(divider)*second;const SimulationSlice born{1,den,units/den,units%den};
                                for(auto actor:{father,leg})if(actor) {c.r4=int32_t(MEM_W(0x74,int32_t(actor)));dora3_s0_8003B77C(rdram,&c);
                                    c.r4=int32_t(actor);c.r5=0;c.r6=int32_t(asset);{SimulationScope birth(born);dora3_s0_8002A100(rdram,&c);}
                                    c.r4=int32_t(MEM_W(0x74,int32_t(actor)));c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);}
                                pm=uint32_t(MEM_W(0x74,int32_t(father)));if(leg)lm=uint32_t(MEM_W(0x74,int32_t(leg)));
                            } else if(edge==2) {MEM_W(0x94,int32_t(father))=variant==1?100:2;MEM_B(0x28,int32_t(pp))=0;}
                            else if(edge==3) {wf(rdram,father+0x10,33);wf(rdram,pp+4,33);wf(rdram,father+0x9C,-600);}
                            else if(edge==4) {c.r4=int32_t(pm);dora3_s0_8003B744(rdram,&c);c.r4=int32_t(pm);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);}
                            ++edge;
                        }
                        advance(stamp);
                    };
                    for(unsigned tick=1;tick<=96;++tick) {
                        const auto stamp=birth_stamp+second*divider*tick/60;
                        // Source callback13 first observes the external release.
                        // Publish it before that callback's interval, after12 full
                        // periods (an exact integer-nanosecond boundary).
                        if(tick==13&&!edge_mode) {
                            if(native)to(birth_stamp+second*divider*(tick-1)/60);
                            MEM_B(0,int32_t(0x8021710D))=0;
                        }
                        if(native)to(stamp);
                        input_now=stamp;city_source_stamp={stamp,uint8_t((uint64_t(second)*divider*tick)%60)};
                        city_source_initial_stamp=SourceTimestamp{birth_stamp+second*divider*(tick-1)/60,unsigned((uint64_t(second)*divider*(tick-1))%60)};
                        c=saved;c.f_odd=&c.f0.u32h;
                        if(native) {const auto state=MEM_W(0x94,int32_t(father));const auto phase=MEM_H(0x34,int32_t(pp));const auto count=city_commands.size();
                            dora3_s3_801E8BDC(rdram,&c);
                            if(scenario==4||scenario==7)assert(MEM_W(0x94,int32_t(father))==state&&MEM_H(0x34,int32_t(pp))==phase&&city_commands.size()==count);
                        }
                        else source_pass(tick==1||tick==13||tick==14||tick==30||tick==31||tick==96);
                        if(tick==1)result.first_end={rf(rdram,father+0x10),rf(rdram,pp+4)};
                        if(tick%3==0) {
                            std::array<double,25> v{};
                            v[0]=MEM_W(0x94,int32_t(father));v[1]=rf(rdram,father+0x10);v[2]=rf(rdram,pp+4);
                            v[3]=MEM_BU(0x28,int32_t(pp));v[4]=MEM_H(0x34,int32_t(pp));v[5]=MEM_H(0x36,int32_t(pp));v[6]=MEM_H(0x40,int32_t(pp));
                            v[7]=rf(rdram,pp+0x58);v[8]=rf(rdram,pp+0x5C);v[9]=rf(rdram,pm+0x64);v[10]=MEM_H(0xD8,int32_t(pp));
                            v[11]=rf(rdram,father+0x9C);v[12]=rf(rdram,father+0xA0);v[13]=rf(rdram,father+0xA4);
                            if(leg) {v[14]=MEM_W(0x94,int32_t(leg));v[15]=rf(rdram,lm+0x64);v[16]=MEM_H(0x7C,int32_t(lp));v[17]=rf(rdram,leg+0x10);
                                v[18]=rf(rdram,leg+0x9C);v[19]=rf(rdram,leg+0xA0);v[20]=rf(rdram,leg+0xA4);v[21]=MEM_H(0xD8,int32_t(lp));}
                            v[22]=allocations.size();v[23]=resource_references[2];v[24]=MEM_BU(0xC9,int32_t(pp));result.samples.push_back(v);
                        }
                    }
                    if(variant==0&&scenario!=4&&scenario!=7) {
                        const auto published=linked_publications;std::vector<CityCommand> consumed;
                        for(const auto& command:city_commands)if(command.site==0x8002A328) {
                            if(uint32_t(command.args[0])==leg) {auto value=command;value.args[0]=int32_t(father);consumed.push_back(value);}
                        }
                        // Actual30914 consumes each final30FFC publication in the
                        // same ordered world pass, even a repeated clip index.
                        if(published!=consumed) {
                            std::cerr<<"Linked lost/repeated publication native="<<native<<" scenario="<<scenario<<" mode="<<edge_mode<<" fps="<<render_fps<<" divider="<<divider<<" parent="<<published.size()<<" child="<<consumed.size()<<'\n';
                            for(const auto& list:{published,consumed}) {for(const auto& command:list)std::cerr<<std::hex<<command.site<<std::dec<<":"<<command.stamp.nanoseconds<<"+"<<command.stamp.sixtieths<<"/60("<<command.args[0]<<","<<command.args[1]<<","<<command.args[2]<<"); ";std::cerr<<'\n';}
                            std::abort();
                        }
                    }
                    result.commands=city_commands;linked_watch=false;native_gameplay_preview_reset(rdram);preview_live=false;
                    for(auto actor:{father,leg})if(actor) {c.r4=int32_t(actor);dora3_s3_801E656C(rdram,&c);c.r4=int32_t(actor);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);}
                    if(allocations!=linked_heap||resource_references!=linked_refs||game_model_animation().model_count()!=6) {
                        std::cerr<<"Linked retirement variant="<<variant<<" gap="<<earlier_gap<<" scenario="<<scenario<<" mode="<<edge_mode<<" native="<<native<<" fps="<<render_fps<<" divider="<<divider<<" models="<<game_model_animation().model_count()<<" heap="<<allocations.size()<<" expected="<<linked_heap.size()<<'\n';
                        for(auto value:allocations)if(!linked_heap.count(value))std::cerr<<" extra heap "<<std::hex<<value<<std::dec<<'\n';
                        for(auto value:linked_heap)if(!allocations.count(value))std::cerr<<" missing heap "<<std::hex<<value<<std::dec<<'\n';
                        for(unsigned i=0;i<linked_refs.size();++i)std::cerr<<" ref "<<i<<" actual="<<resource_references[i]<<" expected="<<linked_refs[i]<<'\n';
                    }
                    assert(allocations==linked_heap&&resource_references==linked_refs&&game_model_animation().model_count()==6);
                    linked_construct_watch=city_loaded=false;linked_original_reference=false;return result;
                };
                const auto expected=run_linked(false,60),actual=run_linked(true,fps,0,&expected);
                auto compare=[&](const LinkedResult& a,const LinkedResult& b,unsigned mode) {
                    assert(a.samples.size()==b.samples.size());
                    for(unsigned n=0;n<a.samples.size();++n)for(unsigned k=0;k<a.samples[n].size();++k) {
                        const double tolerance=k==1||k==2||k==7||k==8||k==9||k==15||k==17?0.001:(k>=11&&k<=13)||(k>=18&&k<=20)?0.03:0.;
                        if(std::abs(a.samples[n][k]-b.samples[n][k])>tolerance) {std::cerr<<"Linked mismatch variant="<<variant<<" gap="<<earlier_gap<<" scenario="<<scenario<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<" sample="<<n<<" field="<<k<<" actual="<<a.samples[n][k]<<" expected="<<b.samples[n][k]<<'\n';return false;}
                    }
                    if(a.commands!=b.commands) {std::cerr<<"Linked command mismatch variant="<<variant<<" scenario="<<scenario<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<'\n';
                        for(const auto& list:{b.commands,a.commands}) {for(const auto& command:list)std::cerr<<std::hex<<command.site<<std::dec<<":"<<command.stamp.nanoseconds<<"+"<<command.stamp.sixtieths<<"/60("<<command.args[0]<<","<<command.args[1]<<","<<command.args[2]<<"); ";std::cerr<<'\n';}return false;}
                    return true;
                };
                // Unowned states preserve Native RAM and are exercised by the
                // actual Source pass separately; they cannot match an owned run.
                if(scenario!=4&&scenario!=7&&!compare(actual,expected,0))return 1;
                if(scenario<=1)for(unsigned mode:{1u,2u,3u}) {const auto baseline=run_linked(true,60,mode),varied=run_linked(true,fps,mode);if(!compare(varied,baseline,mode))return 1;++linked_native_cases;}
                if(scenario==4||scenario==7)++linked_rejections;
                memory=linked_memory;allocations=linked_heap;next_address=linked_next;resource_references=linked_refs;++linked_source_cases;
            }

            // Actual kind6 one-shot initialization and the distinct kind80
            // no-op callback share real common movement/model/timer/opacity.
            // Source uses the frozen unpatched world dispatcher as oracle.
            for(unsigned kind:{6u,80u})for(unsigned initial_state:{1u,2u}) {
                const auto world_memory=memory;
                const auto world_heap=allocations;const auto world_next=next_address;const auto world_refs=resource_references;
                auto run_world=[&](bool native,unsigned render_fps,unsigned edge_mode=0) {
                    native_gameplay_preview_reset(rdram);preview_live=false;memory=world_memory;input_now=0;
                    allocations=world_heap;next_address=world_next;resource_references=world_refs;
                    world_init_watch=true;world_init_commands=0;
                    MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                    wf(rdram,physics,0);wf(rdram,physics+0x14,0);MEM_W(0x94,int32_t(player))=1;
                    for(unsigned n=0;n<4;++n)MEM_W(0x94,int32_t(prop+n*0xCC))=0;
                    MEM_W(0x94,int32_t(prop))=int(initial_state);
                    std::memset(rdram+(prop_physics&0xFFFFFF),0,0x100);
                    MEM_H(0x20,int32_t(prop_physics))=int16_t(kind);
                    MEM_B(0x28,int32_t(prop_physics))=0;
                    MEM_H(0xD8,int32_t(prop_physics))=9;
                    MEM_B(0xC8,int32_t(prop_physics))=1;wf(rdram,prop_physics+0xC4,0.25f);
                    wf(rdram,prop_physics+0x10,8);wf(rdram,prop_physics+0x14,-1);wf(rdram,prop_physics+0x18,-2);
                    wf(rdram,prop+0x9C,500);wf(rdram,prop+0xA0,100);wf(rdram,prop+0xA4,0);
                    MEM_B(0x6D,int32_t(prop))=1;MEM_W(0x7C,int32_t(prop))=0;
                    MEM_H(0,int32_t(0x80257000))=0;MEM_W(0,int32_t(0x801AEC00))=int32_t(clip);
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(object_models[0]);dora3_s0_8003B744(rdram,&c);
                    c.r4=int32_t(object_models[0]);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                    reference_dora3_s3_801E8BDC(rdram,&c);
                    assert(world_init_commands==unsigned(kind==6&&initial_state==1));
                    preview_live=native;if(native)native_gameplay_preview_begin(rdram,0,divider);
                    unsigned frame=0,edge=0;std::vector<std::array<double,11>> values;
                    const std::array<Nanoseconds,3> edges{73'000'000,173'000'000,273'000'000};
                    auto advance=[&](Nanoseconds stamp) {
                        while(second*(frame+1)/render_fps<stamp) {
                            input_now=second*++frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;
                            const auto x=MEM_W(0x9C,int32_t(prop));
                            native_gameplay_preview_update(rdram,&c,input_now);
                            if(MEM_BU(0xC9,int32_t(prop_physics))==1)assert(MEM_W(0x9C,int32_t(prop))==x);
                            assert(std::memcmp(&c,&saved,sizeof(c))==0);
                        }
                        input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                    };
                    auto to=[&](Nanoseconds stamp) {
                        while(edge_mode&&edge<edges.size()&&edges[edge]<=stamp) {
                            advance(edges[edge]);
                            if(edge<2)MEM_B(0xC9,int32_t(prop_physics))=edge==0?1:0;
                            else {MEM_W(0x94,int32_t(prop))=1;MEM_B(0x28,int32_t(prop_physics))=0;}
                            ++edge;
                        }
                        advance(stamp);
                    };
                    for(unsigned tick=1;tick<=36;++tick) {
                        if(native)to((second*divider*tick+59)/60);
                        else {input_now=second*divider*tick/60;c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);}
                        if(tick%3==0) {
                            std::array<double,11> v{};
                            for(unsigned axis=0;axis<3;++axis)v[axis]=rf(rdram,prop+0x9C+4*axis);
                            v[3]=rf(rdram,object_models[0]+0x64);v[4]=MEM_BU(0x28,int32_t(prop_physics));
                            v[5]=MEM_H(0xD8,int32_t(prop_physics));v[6]=rf(rdram,prop_physics+0xC4);
                            v[7]=MEM_BU(0xC8,int32_t(prop_physics));v[8]=MEM_W(0x94,int32_t(prop));
                            v[9]=world_init_commands;v[10]=MEM_BU(0xC9,int32_t(prop_physics));values.push_back(v);
                        }
                    }
                    assert(world_init_commands==unsigned(kind==6)*(unsigned(initial_state==1)+unsigned(edge_mode!=0)));
                    native_gameplay_preview_reset(rdram);preview_live=false;world_init_watch=false;return values;
                };
                const auto expected=run_world(false,60),actual=run_world(true,fps);
                for(unsigned n=0;n<actual.size();++n)for(unsigned k=0;k<actual[n].size();++k)
                    if(std::abs(actual[n][k]-expected[n][k])>(k<3?0.03:k==3||k==6?0.001:0.)) {
                        std::cerr<<"World init Source mismatch kind="<<kind<<" state="<<initial_state<<" fps="<<fps<<" divider="<<divider<<" sample="<<n<<" field="<<k<<" actual="<<actual[n][k]<<" expected="<<expected[n][k]<<'\n';return 1;
                    }
                const auto baseline=run_world(true,60,1),varied=run_world(true,fps,1);
                for(unsigned n=0;n<varied.size();++n)for(unsigned k=0;k<varied[n].size();++k)
                    if(std::abs(varied[n][k]-baseline[n][k])>(k<3?0.03:k==3||k==6?0.001:0.)) {
                        std::cerr<<"World init Native mismatch kind="<<kind<<" state="<<initial_state<<" fps="<<fps<<" divider="<<divider<<" sample="<<n<<" field="<<k<<" actual="<<varied[n][k]<<" expected="<<baseline[n][k]<<'\n';return 1;
                    }
                memory=world_memory;allocations=world_heap;next_address=world_next;resource_references=world_refs;
                ++world_source_cases;++world_native_cases;
            }
            // Actual orbital71..74 callback, Source contact entry, turning
            // and return phases; integer angles must retain fractional time.
            for(unsigned kind:{71u,72u,73u,74u})for(unsigned scenario=0;scenario<12;++scenario) {
                const auto orbit_memory=memory;const auto orbit_heap=allocations;
                const auto orbit_next=next_address;const auto orbit_refs=resource_references;
                auto run_orbit=[&](bool native,unsigned render_fps,unsigned edge_mode=0) {
                    native_gameplay_preview_reset(rdram);preview_live=false;memory=orbit_memory;
                    allocations=orbit_heap;next_address=orbit_next;resource_references=orbit_refs;input_now=0;
                    room(rdram);world_init_watch=true;world_init_commands=0;
                    for(unsigned n=0;n<4;++n)MEM_W(0x94,int32_t(prop+n*0xCC))=0;
                    MEM_W(0x94,int32_t(prop))=scenario==4?3:1;
                    std::memset(rdram+(prop_physics&0xFFFFFF),0,0x100);
                    MEM_H(0x20,int32_t(prop_physics))=int16_t(kind);
                    MEM_H(0x34,int32_t(prop_physics))=359;MEM_H(0x36,int32_t(prop_physics))=355;
                    MEM_H(0xBE,int32_t(prop_physics))=-500;MEM_H(0xC0,int32_t(prop_physics))=80;MEM_H(0xC2,int32_t(prop_physics))=-300;
                    MEM_H(0x42,int32_t(prop_physics))=scenario==1?1:0;
                    MEM_B(0x29,int32_t(prop_physics))=scenario==7?1:0;
                    MEM_H(0xD8,int32_t(prop_physics))=9;
                    MEM_B(0,int32_t(0x8021710D))=scenario==3||scenario==6?1:0;
                    wf(rdram,prop_physics+0x10,3);wf(rdram,prop_physics+0x14,-1.5f);wf(rdram,prop_physics+0x18,-2);
                    wf(rdram,prop+0x9C,-500);wf(rdram,prop+0xA0,80);wf(rdram,prop+0xA4,-300);wf(rdram,prop+0x10,359);
                    MEM_B(0x6D,int32_t(prop))=1;MEM_W(0x7C,int32_t(prop))=0;
                    MEM_H(0,int32_t(0x80257000))=0;MEM_W(0,int32_t(0x801AEC00))=int32_t(clip);
                    MEM_H(0,int32_t(0x801B36B8))=1; // Contact dialogue producer rejects this non-story room.
                    MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                    wf(rdram,player+0x9C,400);wf(rdram,player+0xA0,0);wf(rdram,player+0xA4,-300);
                    wf(rdram,physics,0);wf(rdram,physics+0x14,0);MEM_W(0x94,int32_t(player))=1;
                    uint32_t orbit_model=object_models[0];c=saved;c.f_odd=&c.f0.u32h;
                    if(scenario==5||edge_mode==3) {
                        // Real constructor in a Native command scope gives
                        // the first callback no pre-existing Source lookahead.
                        const SimulationSlice born{1,uint64_t(divider)*second,0,0};SimulationScope birth(born);
                        c.r4=int32_t(asset);dora3_s0_8003B560(rdram,&c);orbit_model=uint32_t(c.r2);
                    }
                    MEM_W(0x74,int32_t(prop))=int32_t(orbit_model);
                    if(scenario<8) {
                        c.r4=int32_t(orbit_model);dora3_s0_8003B744(rdram,&c);
                        c.r4=int32_t(orbit_model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                    }
                    const Nanoseconds birth_stamp=scenario==8?73'000'000:scenario==9?173'000'000:0;
                    if(birth_stamp)MEM_W(0x94,int32_t(prop))=0;
                    if(scenario!=5&&!birth_stamp)reference_dora3_s3_801E8BDC(rdram,&c);
                    // Explicit return states exercise both signed turns and the
                    // 360-degree wrap. Earlier dialogue targets could complete
                    // within the tolerance without a nonzero return increment.
                    const float return_start=scenario==10?350.f:10.f;
                    const int return_direction=scenario==10?1:-1;
                    if(scenario>=10) {
                        MEM_W(0x94,int32_t(prop))=2;
                        MEM_B(0x28,int32_t(prop_physics))=1;
                        MEM_H(0x38,int32_t(prop_physics))=1;
                        wf(rdram,prop+0x10,return_start);
                        wf(rdram,prop_physics+0x58,scenario==10?120.f:240.f);
                        c=saved;c.f_odd=&c.f0.u32h;c.f12.fl=return_start;
                        c.f14.fl=rf(rdram,prop_physics+0x58);c.r6=int32_t(std::bit_cast<uint32_t>(30.f));
                        dora3_s3_demo_801E8838(rdram,&c);
                        assert(int32_t(c.r2)==return_direction);
                    }
                    preview_live=native;if(native)native_gameplay_preview_begin(rdram,0,divider);
                    unsigned frame=0,edge=0;std::vector<std::array<double,16>> values;
                    auto contact=[&] {
                        MEM_W(0,int32_t(0x8021BDEC))=int32_t(prop);MEM_W(0,int32_t(0x8021BCDC))=int32_t(prop_physics);
                        MEM_W(0,int32_t(0x8021BCE4))=4;c=saved;c.f_odd=&c.f0.u32h;dora3_s3_core_801EEC70(rdram,&c);
                        assert(MEM_W(0x94,int32_t(prop))==2);
                    };
                    const std::array<Nanoseconds,5> edges{73'000'000,173'000'000,273'000'000,373'000'000,473'000'000};
                    auto verify_return=[&](Nanoseconds stamp) {
                        if(scenario<10||stamp>second*divider/60)return;
                        const double expected=return_start+return_direction*30.*double(stamp)*60./(double(second)*divider);
                        const double error=std::remainder(double(rf(rdram,prop+0x10))-expected,360.);
                        if(std::abs(error)>0.001) {
                            std::cerr<<"Orbit intermediate return mismatch kind="<<kind<<" scenario="<<scenario
                                <<" fps="<<render_fps<<" divider="<<divider<<" ns="<<stamp
                                <<" actual="<<rf(rdram,prop+0x10)<<" expected="<<std::fmod(expected+360.,360.)<<'\n';
                            std::abort();
                        }
                        assert(MEM_W(0x94,int32_t(prop))==2);++orbit_return_samples;
                    };
                    auto advance=[&](Nanoseconds stamp) {
                        while(second*(frame+1)/render_fps<stamp) {
                            input_now=second*++frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;
                            native_gameplay_preview_update(rdram,&c,input_now);
                            assert(std::memcmp(&c,&saved,sizeof(c))==0);
                            verify_return(input_now);
                        }
                        input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                        verify_return(stamp);
                    };
                    auto to=[&](Nanoseconds stamp) {
                        while(edge_mode&&edge<edges.size()&&edges[edge]<=stamp) {
                            advance(edges[edge]);
                            if(edge_mode==1) {
                                if(edge<2)MEM_B(0xC9,int32_t(prop_physics))=edge==0?1:0;
                                else if(edge==2) {MEM_H(0x34,int32_t(prop_physics))=5;MEM_H(0x36,int32_t(prop_physics))=5;wf(rdram,prop+0x10,17);wf(rdram,prop_physics+4,17);}
                                else MEM_H(0x42,int32_t(prop_physics))=edge==3?1:0;
                            } else if(edge_mode==3) {
                                if(edge==0||edge==2) {
                                    const auto reused=orbit_model;
                                    c.r4=int32_t(reused);dora3_s0_8003B77C(rdram,&c);
                                    for(auto address:allocations)assert(address<reused);
                                    next_address=reused;
                                    const SimulationSlice born{1,uint64_t(divider)*second,0,0};SimulationScope birth(born);
                                    c.r4=int32_t(asset);dora3_s0_8003B560(rdram,&c);orbit_model=uint32_t(c.r2);assert(orbit_model==reused);
                                    MEM_W(0x74,int32_t(prop))=int32_t(orbit_model);
                                    c.r4=int32_t(orbit_model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                                } else if(edge==1) {MEM_B(0,int32_t(0x8021710D))=1;contact();}
                                else if(edge==3) {MEM_B(0,int32_t(0x8021710D))=0;wf(rdram,player+0x9C,-900);}
                                else wf(rdram,player+0x9C,400);
                            } else {
                                if(edge==0) {c.r4=int32_t(orbit_model);dora3_s0_8003B744(rdram,&c);c.r4=int32_t(orbit_model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);}
                                else if(edge==1) {MEM_B(0,int32_t(0x8021710D))=1;contact();}
                                else if(edge==2)MEM_B(0,int32_t(0x8021710D))=0;
                                else {MEM_W(0x94,int32_t(prop))=edge==3?0:1;MEM_B(0x28,int32_t(prop_physics))=0;}
                            }
                            ++edge;
                        }
                        advance(stamp);
                    };
                    if(birth_stamp) {
                        if(native)to(birth_stamp);
                        input_now=birth_stamp;c=saved;c.f_odd=&c.f0.u32h;
                        const auto units=uint64_t(birth_stamp)*60,den=uint64_t(divider)*second;
                        const SimulationSlice born{1,den,units/den,units%den};SimulationScope birth(born);
                        c.r4=int32_t(asset);dora3_s0_8003B560(rdram,&c);orbit_model=uint32_t(c.r2);
                        MEM_W(0x74,int32_t(prop))=int32_t(orbit_model);
                        c.r4=int32_t(orbit_model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                        MEM_W(0x94,int32_t(prop))=1;
                    }
                    for(unsigned tick=1;tick<=36;++tick) {
                        if((scenario==2||scenario==3||scenario==6)&&tick==7)contact();
                        if(scenario==6&&(tick==10||tick==17))wf(rdram,player+0x9C,tick==10?-900:400);
                        if(scenario==6&&tick==25)MEM_B(0,int32_t(0x8021710D))=0;
                        if(scenario==7&&tick==13)MEM_B(0x29,int32_t(prop_physics))=0;
                        if(scenario==3&&tick==13)MEM_B(0,int32_t(0x8021710D))=0;
                        // Keep commands before the next exact rational Source boundary.
                        // Ceil would pre-latch that period's target by a fractional ns.
                        if(native)to(birth_stamp+second*divider*tick/60);
                        else {input_now=birth_stamp+second*divider*tick/60;c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);}
                        if(tick%3==0) {
                            std::array<double,16> v{};
                            for(unsigned axis=0;axis<3;++axis)v[axis]=rf(rdram,prop+0x9C+4*axis);
                            v[3]=rf(rdram,prop+0x10);v[4]=MEM_H(0x34,int32_t(prop_physics));v[5]=MEM_H(0x36,int32_t(prop_physics));
                            v[6]=MEM_H(0x38,int32_t(prop_physics));v[7]=MEM_W(0x94,int32_t(prop));v[8]=MEM_BU(0x28,int32_t(prop_physics));
                            v[9]=rf(rdram,prop_physics+4);v[10]=rf(rdram,prop_physics+0x58);v[11]=rf(rdram,orbit_model+0x64);
                            v[12]=MEM_H(0xD8,int32_t(prop_physics));v[13]=world_init_commands;v[14]=MEM_BU(0xC9,int32_t(prop_physics));v[15]=MEM_H(0x42,int32_t(prop_physics));values.push_back(v);
                        }
                    }
                    native_gameplay_preview_reset(rdram);preview_live=false;world_init_watch=false;
                    if(scenario==5||birth_stamp||edge_mode==3) {c.r4=int32_t(orbit_model);dora3_s0_8003B77C(rdram,&c);assert(allocations==orbit_heap);}
                    return values;
                };
                const auto expected=run_orbit(false,60),actual=run_orbit(true,fps);
                for(unsigned n=0;n<actual.size();++n)for(unsigned k=0;k<actual[n].size();++k)
                    if(std::abs(actual[n][k]-expected[n][k])>(k<3?0.035:k==3||k==9||k==10||k==11?0.001:0.)) {
                        std::cerr<<"Orbit Source mismatch kind="<<kind<<" scenario="<<scenario<<" fps="<<fps<<" divider="<<divider<<" sample="<<n<<" field="<<k<<" actual="<<actual[n][k]<<" expected="<<expected[n][k]<<'\n';return 1;
                    }
                if(scenario==0||scenario==2)for(unsigned mode:{1u,2u,3u}) {
                    if(mode==3&&scenario!=0)continue;
                    const auto baseline=run_orbit(true,60,mode),varied=run_orbit(true,fps,mode);
                    for(unsigned n=0;n<varied.size();++n)for(unsigned k=0;k<varied[n].size();++k)
                        if(std::abs(varied[n][k]-baseline[n][k])>(k<3?0.035:k==3||k==9||k==10||k==11?0.001:0.)) {
                            std::cerr<<"Orbit Native mismatch kind="<<kind<<" scenario="<<scenario<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<" sample="<<n<<" field="<<k<<" actual="<<varied[n][k]<<" expected="<<baseline[n][k]<<'\n';return 1;
                        }
                    ++orbit_native_cases;
                }
                memory=orbit_memory;allocations=orbit_heap;next_address=orbit_next;resource_references=orbit_refs;++orbit_source_cases;
            }
            // City7 uses real projection and sprite records; city9 retires
            // through its actual countdown/event/pool-pause/destructor chain.
            for(unsigned kind:{7u,9u})for(unsigned scenario=0;scenario<6;++scenario) {
                const auto city_memory=memory;const auto city_heap=allocations;
                const auto city_next=next_address;const auto city_refs=resource_references;
                auto run_city=[&](bool native,unsigned render_fps,unsigned edge_mode=0) {
                    native_gameplay_preview_reset(rdram);preview_live=false;memory=city_memory;
                    allocations=city_heap;next_address=city_next;resource_references=city_refs;input_now=0;
                    room(rdram);city_loaded=city_watch=true;city_identity_verified=scenario!=4;city_commands.clear();city_sprite_commands=0;city_source_initial_stamp.reset();
                    for(unsigned n=0;n<4;++n)MEM_W(0x94,int32_t(prop+n*0xCC))=0;
                    std::memset(rdram+(prop&0xFFFFFF),0,0xCC); // Fresh pool slot owns no inherited static contact lists.
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(prop);dora3_scene_object_init(rdram,&c);
                    MEM_H(0xA8,int32_t(prop))=-1; // Direct synthetic asset has no resource-id owner.
                    // The static sprite pool is independently owned from models.
                    std::memset(rdram+(0x801A9CB0&0xFFFFFF),0,300*8);MEM_W(0,int32_t(0x801A99A8))=0;
                    MEM_B(0x6B,int32_t(prop))=0;MEM_W(0x74,int32_t(prop))=0;
                    MEM_B(0x6D,int32_t(prop))=0;MEM_W(0x7C,int32_t(prop))=-1;
                    MEM_B(0,int32_t(0x8021710D))=kind==7&&scenario==1?1:0;
                    MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                    wf(rdram,physics,0);wf(rdram,physics+0x14,0);MEM_W(0x94,int32_t(player))=1;
                    const Nanoseconds birth_stamp=scenario==2?73'000'000:scenario==5?173'000'000:0;
                    if(native&&birth_stamp) {preview_live=true;native_gameplay_preview_begin(rdram,0,divider);}
                    unsigned frame=0;
                    auto advance=[&](Nanoseconds stamp) {
                        while(second*(frame+1)/render_fps<stamp) {
                            input_now=second*++frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;
                            native_gameplay_preview_update(rdram,&c,input_now);assert(std::memcmp(&c,&saved,sizeof(c))==0);
                        }
                        input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                        assert(std::memcmp(&c,&saved,sizeof(c))==0);
                    };
                    if(native&&birth_stamp)advance(birth_stamp);
                    input_now=birth_stamp;c=saved;c.f_odd=&c.f0.u32h;
                    c.r4=0xF4;dora3_game_heap_alloc(rdram,&c);const auto city_physical=uint32_t(c.r2);
                    c.r4=int32_t(city_physical);dora3_s0_80042CAC(rdram,&c);
                    auto create=[&] {
                        c.r4=int32_t(prop);c.r5=0;c.r6=int32_t(asset);dora3_s0_8002A100(rdram,&c);
                    };
                    if(birth_stamp) {
                        const auto units=uint64_t(birth_stamp)*60,den=uint64_t(divider)*second;
                        const SimulationSlice born{1,den,units/den,units%den};SimulationScope birth(born);create();
                    } else create();
                    const auto city_model=uint32_t(MEM_W(0x74,int32_t(prop)));
                    c.r4=int32_t(city_model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                    MEM_W(0x98,int32_t(prop))=int32_t(city_physical);MEM_W(0x94,int32_t(prop))=(scenario==0||scenario==3||scenario==4)?1:2;
                    MEM_H(0x20,int32_t(city_physical))=int16_t(kind);MEM_H(0xBC,int32_t(city_physical))=-1;
                    MEM_H(0xD8,int32_t(city_physical))=9;MEM_B(0x28,int32_t(city_physical))=0;
                    wf(rdram,prop+0x9C,-500);wf(rdram,prop+0xA0,80);wf(rdram,prop+0xA4,-300);
                    wf(rdram,city_physical,4);wf(rdram,city_physical+4,90);wf(rdram,city_physical+8,-3);wf(rdram,city_physical+0xC,30);
                    wf(rdram,city_physical+0x10,3);wf(rdram,city_physical+0x14,-1.5f);wf(rdram,city_physical+0x18,-2);
                    MEM_H(0,int32_t(0x80257000))=0;
                    if(birth_stamp&&kind==7) {
                        // Apply the actual state2 initialization command at
                        // birth without borrowing a preceding movement step.
                        MEM_W(0,int32_t(0x8021BDEC))=int32_t(prop);MEM_W(0,int32_t(0x8021BCDC))=int32_t(city_physical);
                        const auto busy=MEM_B(0,int32_t(0x8021710D));MEM_B(0,int32_t(0x8021710D))=1;
                        city_source_stamp={birth_stamp,0};dora3_s8_level1_8022D8B0(rdram,&c);
                        MEM_B(0,int32_t(0x8021710D))=busy;
                    }
                    if(!birth_stamp) {city_source_stamp={0,0};reference_dora3_s3_801E8BDC(rdram,&c);}
                    // Initial Source commands are shared before acquisition.
                    city_commands.clear();city_sprite_commands=0;
                    preview_live=native;if(native&&!birth_stamp)native_gameplay_preview_begin(rdram,0,divider);
                    auto contact=[&](Nanoseconds stamp) {
                        city_source_stamp={stamp,0};c=saved;c.f_odd=&c.f0.u32h;
                        MEM_W(0,int32_t(0x8021BDEC))=int32_t(prop);MEM_W(0,int32_t(0x8021BCDC))=int32_t(city_physical);
                        const auto callback=kind==7?dora3_s8_level1_8022D7DC:dora3_s8_level1_8022DA68;
                        const auto before_state=MEM_W(0x94,int32_t(prop));
                        const auto before_flag=MEM_BU(0x28,int32_t(city_physical));const auto command_count=city_commands.size();
                        for(unsigned mode:{0u,1u,2u,3u,5u}) {
                            MEM_W(0,int32_t(0x8021BCE4))=int32_t(mode);callback(rdram,&c);
                            assert(MEM_W(0x94,int32_t(prop))==before_state&&MEM_BU(0x28,int32_t(city_physical))==before_flag&&city_commands.size()==command_count);
                        }
                        MEM_W(0,int32_t(0x8021BCE4))=4;callback(rdram,&c);
                        if(kind==7&&before_state==2) {assert(city_commands.size()==command_count);return;}
                        assert(MEM_W(0x94,int32_t(prop))==2&&MEM_BU(0x28,int32_t(city_physical))==0);
                        if(kind==7) {
                            assert(MEM_BU(0,int32_t(0x8021710D))==1);
                            assert(uint32_t(MEM_W(0,int32_t(0x801B8D04)))==0x80232CC0u);
                            assert(MEM_H(0,int32_t(0x801B8D50))==1);
                            assert(MEM_W(0,int32_t(0x801B8950))==-9999);
                        } else {
                            assert(MEM_BU(0xC9,int32_t(city_physical))==0);
                            assert(MEM_W(0x94,int32_t(player))==66);
                            assert(MEM_H(0,int32_t(0x801AA660))==0);
                            assert(MEM_BU(0,int32_t(0x80171580))==16);
                        }
                    };
                    unsigned edge=0;const std::array<Nanoseconds,4> edges{73'000'000,173'000'000,273'000'000,373'000'000};
                    auto to=[&](Nanoseconds stamp) {
                        while(edge_mode&&edge<edges.size()&&edges[edge]<=stamp&&MEM_W(0x94,int32_t(prop))) {
                            advance(edges[edge]);
                            if(edge<2)MEM_B(0xC9,int32_t(city_physical))=edge==0?1:0;
                            else if(edge==2)contact(edges[edge]); else {
                                if(edge_mode==2)contact(edges[edge]);
                                MEM_B(0,int32_t(0x8021710D))=0;
                            }
                            ++edge;
                        }
                        advance(stamp);
                    };
                    struct Result {std::vector<std::array<double,16>> samples;std::vector<CityCommand> commands;};Result result;
                    for(unsigned tick=1;tick<=24;++tick) {
                        if(kind==7&&scenario==0&&tick==7) {wf(rdram,city_physical,4);wf(rdram,city_physical+8,-3);}
                        if(kind==7&&scenario==1&&tick==7)MEM_B(0,int32_t(0x8021710D))=0;
                        if(kind==7&&scenario==3&&tick==10)MEM_B(0,int32_t(0x8021710D))=0;
                        const auto units=uint64_t(birth_stamp)*60+uint64_t(second)*divider*tick;
                        city_source_stamp={Nanoseconds(units/60),unsigned(units%60)};
                        if(native) {
                            const auto source_owned=std::array<int32_t,5>{MEM_W(0x94,int32_t(prop)),MEM_W(0x9C,int32_t(prop)),MEM_W(0xA0,int32_t(prop)),MEM_W(0xA4,int32_t(prop)),MEM_W(0x64,int32_t(city_model))};
                            to(Nanoseconds(units/60));
                            if(scenario==4) {
                                const auto unchanged=std::array<int32_t,5>{MEM_W(0x94,int32_t(prop)),MEM_W(0x9C,int32_t(prop)),MEM_W(0xA0,int32_t(prop)),MEM_W(0xA4,int32_t(prop)),MEM_W(0x64,int32_t(city_model))};
                                assert(unchanged==source_owned);
                                c=saved;c.f_odd=&c.f0.u32h;dora3_s3_801E8BDC(rdram,&c);
                            }
                        }
                        else {
                            const auto initial=units-uint64_t(second)*divider;
                            city_source_initial_stamp=SourceTimestamp{Nanoseconds(initial/60),unsigned(initial%60)};
                            input_now=Nanoseconds(units/60);c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);
                            city_source_initial_stamp.reset();
                        }
                        if(scenario==3&&(tick==6||tick==9))contact(input_now);
                        if(tick%3==0) {
                            std::array<double,16> v{};v[0]=MEM_W(0x94,int32_t(prop));
                            for(unsigned axis=0;axis<3;++axis)v[1+axis]=rf(rdram,prop+0x9C+4*axis);
                            if(v[0]) {
                                v[4]=MEM_H(0x34,int32_t(city_physical));v[5]=MEM_BU(0x28,int32_t(city_physical));v[6]=MEM_H(0xD8,int32_t(city_physical));v[7]=rf(rdram,city_model+0x64);v[8]=MEM_BU(0xC9,int32_t(city_physical));
                            }
                            v[9]=MEM_BU(0x6D,int32_t(prop));
                            if(v[9]==5) {const auto record=0x801A9CB0u+8*uint32_t(MEM_W(0x7C,int32_t(prop)));v[10]=MEM_W(0,int32_t(record));v[11]=MEM_BU(4,int32_t(record));v[12]=MEM_BU(5,int32_t(record));v[13]=MEM_BU(6,int32_t(record));}
                            v[14]=city_sprite_commands;v[15]=allocations.size();result.samples.push_back(v);
                        }
                    }
                    result.commands=city_commands;city_watch=false;
                    native_gameplay_preview_reset(rdram);preview_live=false;
                    if(MEM_W(0x94,int32_t(prop))) {c.r4=int32_t(prop);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);}
                    // Real player lock/recovery can replace animation or
                    // morph buffers. Only those exact destructor-owned
                    // fields may change ownership; all city allocations
                    // must still disappear and every other allocation match.
                    auto expected_heap=city_heap;
                    for(auto model:owners) {
                        assert(uint32_t(MEM_W(0x58,int32_t(model)))==city_memory[((model+0x58)&0xFFFFFF)/4]);
                        for(unsigned offset:{0x60u,0x6Cu,0x70u}) {
                            const auto old_buffer=city_memory[((model+offset)&0xFFFFFF)/4];
                            const auto current_buffer=uint32_t(MEM_W(int32_t(offset),int32_t(model)));
                            if(old_buffer==current_buffer)continue;
                            if(old_buffer)assert(expected_heap.erase(old_buffer)==1&&!allocations.count(old_buffer));
                            if(current_buffer)assert(allocations.count(current_buffer)&&expected_heap.insert(current_buffer).second);
                        }
                    }
                    assert(allocations==expected_heap&&resource_references==city_refs&&game_model_animation().model_count()==6);city_loaded=false;
                    return result;
                };
                const auto expected=run_city(false,60),actual=run_city(true,fps);
                for(unsigned n=0;n<actual.samples.size();++n)for(unsigned k=0;k<actual.samples[n].size();++k)
                    if(std::abs(actual.samples[n][k]-expected.samples[n][k])>((k>=1&&k<=3)?0.035:k==7?0.001:0.)) {
                        std::cerr<<"City Source mismatch kind="<<kind<<" scenario="<<scenario<<" fps="<<fps<<" divider="<<divider<<" sample="<<n<<" field="<<k<<" actual="<<actual.samples[n][k]<<" expected="<<expected.samples[n][k]<<'\n';return 1;
                    }
                if(actual.commands!=expected.commands) {
                    std::cerr<<"City Source command mismatch kind="<<kind<<" scenario="<<scenario<<" fps="<<fps<<" divider="<<divider<<'\n';
                    for(const auto& list:{expected.commands,actual.commands}) {
                        for(const auto& event:list)std::cerr<<std::hex<<event.site<<std::dec<<":"<<event.stamp.nanoseconds<<"+"<<event.stamp.sixtieths<<"/60 "<<event.args[0]<<","<<event.args[1]<<","<<event.args[2]<<"; ";
                        std::cerr<<'\n';
                    }
                    return 1;
                }
                if(scenario==0)for(unsigned edge_mode:{1u,2u}) {
                    const auto baseline=run_city(true,60,edge_mode),varied=run_city(true,fps,edge_mode);
                    for(unsigned n=0;n<varied.samples.size();++n)for(unsigned k=0;k<varied.samples[n].size();++k)
                        if(std::abs(varied.samples[n][k]-baseline.samples[n][k])>((k>=1&&k<=3)?0.035:k==7?0.001:0.)) {
                            std::cerr<<"City Native mismatch kind="<<kind<<" fps="<<fps<<" divider="<<divider<<" sample="<<n<<" field="<<k<<" actual="<<varied.samples[n][k]<<" expected="<<baseline.samples[n][k]<<'\n';return 1;
                        }
                    if(varied.commands!=baseline.commands) {std::cerr<<"City Native command mismatch kind="<<kind<<" fps="<<fps<<" divider="<<divider<<'\n';return 1;}
                    ++city_native_cases;
                }
                if(scenario==4)++city_overlay_rejections;
                memory=city_memory;allocations=city_heap;next_address=city_next;resource_references=city_refs;++city_source_cases;
            }
            assert(city_source_cases==(cases+1)*12&&city_native_cases==(cases+1)*4&&city_overlay_rejections==(cases+1)*2);
            city_identity_verified=true;
            // Real floating exchange actors75/76 and character-reply cat77: descriptors,
            // one-shot clips, dialogue outcome, byte counters, bob and yaw.
            for(unsigned kind:{75u,76u,77u})for(unsigned scenario=0;scenario<12;++scenario) {
                const auto dialogue_memory=memory;const auto dialogue_heap=allocations;
                const auto dialogue_next=next_address;const auto dialogue_refs=resource_references;
                struct DialogueResult {std::vector<std::array<double,18>> samples;std::vector<CityCommand> commands;};
                auto run_dialogue=[&](bool native,unsigned render_fps,unsigned edge_mode=0) {
                    native_gameplay_preview_reset(rdram);preview_live=false;memory=dialogue_memory;
                    allocations=dialogue_heap;next_address=dialogue_next;resource_references=dialogue_refs;input_now=0;
                    room(rdram);dialogue_actor_loaded=dialogue_actor_watch=city_watch=true;
                    city_identity_verified=scenario!=5;city_commands.clear();city_source_initial_stamp.reset();
                    for(unsigned n=0;n<4;++n)MEM_W(0x94,int32_t(prop+n*0xCC))=0;
                    std::memset(rdram+(prop&0xFFFFFF),0,0xCC);
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(prop);dora3_scene_object_init(rdram,&c);
                    MEM_H(0xA8,int32_t(prop))=-1;MEM_B(0x6D,int32_t(prop))=0;MEM_W(0x7C,int32_t(prop))=-1;
                    MEM_B(0,int32_t(0x8021710D))=0;MEM_B(0,int32_t(0x801B8D34))=scenario==3?1:0;
                    MEM_B(0,int32_t(0x801B374A))=scenario==3?1:0;
                    MEM_B(1,int32_t(0x801B3700))=7;MEM_B(2,int32_t(0x801B3700))=7;
                    MEM_B(3,int32_t(0x801B3700))=scenario==1?2:12;
                    MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                    wf(rdram,physics,0);wf(rdram,physics+0x14,0);MEM_W(0x94,int32_t(player))=1;
                    wf(rdram,player+0x9C,400);wf(rdram,player+0xA0,0);wf(rdram,player+0xA4,-300);
                    const Nanoseconds birth_stamp=scenario==6?73'000'000:scenario==7?173'000'000:0;
                    if(native&&birth_stamp) {preview_live=true;native_gameplay_preview_begin(rdram,0,divider);}
                    unsigned frame=0,edge=0;
                    auto verify_return=[&](Nanoseconds stamp) {
                        if((scenario!=8&&scenario!=9)||stamp>second*divider/60)return;
                        const double expected=(scenario==8?350.:10.)+(scenario==8?1.:-1.)*30.*double(stamp)*60./(double(second)*divider);
                        assert(std::abs(std::remainder(double(rf(rdram,prop+0x10))-expected,360.))<0.001);
                        assert(MEM_W(0x94,int32_t(prop))==2);++dialogue_actor_return_samples;
                    };
                    auto advance=[&](Nanoseconds stamp) {
                        while(second*(frame+1)/render_fps<stamp) {
                            input_now=second*++frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;
                            native_gameplay_preview_update(rdram,&c,input_now);assert(std::memcmp(&c,&saved,sizeof(c))==0);
                            verify_return(input_now);
                        }
                        input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                        assert(std::memcmp(&c,&saved,sizeof(c))==0);
                        verify_return(stamp);
                    };
                    if(native&&birth_stamp)advance(birth_stamp);
                    input_now=birth_stamp;c=saved;c.f_odd=&c.f0.u32h;
                    c.r4=0xF4;dora3_game_heap_alloc(rdram,&c);const auto dialogue_physical=uint32_t(c.r2);
                    c.r4=int32_t(dialogue_physical);dora3_s0_80042CAC(rdram,&c);
                    auto create=[&] {c.r4=int32_t(prop);c.r5=0;c.r6=int32_t(asset);dora3_s0_8002A100(rdram,&c);};
                    if(birth_stamp) {
                        const auto units=uint64_t(birth_stamp)*60,den=uint64_t(divider)*second;
                        const SimulationSlice born{1,den,units/den,units%den};SimulationScope birth(born);create();
                    } else create();
                    const auto dialogue_model=uint32_t(MEM_W(0x74,int32_t(prop)));
                    c.r4=int32_t(dialogue_model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                    // The real clip command indexes a heap-owned actor B4 list.
                    c.r4=6;dora3_game_heap_alloc(rdram,&c);const auto clip_list=uint32_t(c.r2);
                    for(unsigned n=0;n<3;++n)MEM_H(int32_t(2*n),int32_t(clip_list))=0;
                    MEM_W(0xB4,int32_t(prop))=int32_t(clip_list);MEM_W(0,int32_t(0x801AEC00))=int32_t(clip);
                    MEM_W(0x98,int32_t(prop))=int32_t(dialogue_physical);MEM_W(0x94,int32_t(prop))=1;
                    MEM_H(0x20,int32_t(dialogue_physical))=int16_t(kind);MEM_H(0xBC,int32_t(dialogue_physical))=-1;
                    MEM_H(0xD8,int32_t(dialogue_physical))=9;MEM_H(0xC0,int32_t(dialogue_physical))=80;
                    MEM_H(0x42,int32_t(dialogue_physical))=355;MEM_H(0x44,int32_t(dialogue_physical))=5;
                    MEM_B(0x29,int32_t(dialogue_physical))=scenario==10?1:0;
                    wf(rdram,prop+0x9C,-500);wf(rdram,prop+0xA0,80);wf(rdram,prop+0xA4,-300);wf(rdram,prop+0x10,359);
                    wf(rdram,dialogue_physical+4,359);wf(rdram,dialogue_physical+0x10,3);
                    wf(rdram,dialogue_physical+0x14,-1.5f);wf(rdram,dialogue_physical+0x18,-2);
                    if(!birth_stamp) {city_source_stamp={0,0};reference_dora3_s3_801E8BDC(rdram,&c);}
                    if(scenario==8||scenario==9) {
                        MEM_W(0x94,int32_t(prop))=2;MEM_B(0x28,int32_t(dialogue_physical))=1;
                        MEM_H(0x34,int32_t(dialogue_physical))=1;
                        wf(rdram,prop+0x10,scenario==8?350.f:10.f);
                        wf(rdram,dialogue_physical+0x58,scenario==8?120.f:240.f);
                    }
                    city_commands.clear();preview_live=native;
                    if(native&&!birth_stamp)native_gameplay_preview_begin(rdram,0,divider);
                    auto contact=[&](Nanoseconds stamp) {
                        city_source_stamp={stamp,0};c=saved;c.f_odd=&c.f0.u32h;
                        MEM_W(0,int32_t(0x8021BDEC))=int32_t(prop);MEM_W(0,int32_t(0x8021BCDC))=int32_t(dialogue_physical);
                        const auto callback=kind==77?dora3_s9_level1_8023DD00:dora3_s7_level1_80223CD4;
                        const auto before_state=MEM_W(0x94,int32_t(prop));const auto latch=MEM_BU(0x28,int32_t(dialogue_physical));
                        const auto commands=city_commands.size();
                        for(unsigned mode:{0u,1u,2u,3u,5u}) {
                            MEM_W(0,int32_t(0x8021BCE4))=int32_t(mode);callback(rdram,&c);
                            assert(MEM_W(0x94,int32_t(prop))==before_state&&MEM_BU(0x28,int32_t(dialogue_physical))==latch&&city_commands.size()==commands);
                        }
                        MEM_W(0,int32_t(0x8021BCE4))=4;callback(rdram,&c);
                        assert(MEM_W(0x94,int32_t(prop))==2&&MEM_BU(0x28,int32_t(dialogue_physical))==0);
                        const uint32_t descriptor=kind==77?(scenario==3?0x8023E24Cu:0x8023E248u):
                            kind==75?(scenario==1?0x80224600u:0x802245E8u):(scenario==1?0x80224624u:0x8022460Cu);
                        assert(uint32_t(MEM_W(0,int32_t(0x801B8D04)))==descriptor);
                        if(kind!=77)assert(MEM_H(0x40,int32_t(dialogue_physical))==(scenario==1?1:0));
                    };
                    const std::array<Nanoseconds,5> edges{73'000'000,173'000'000,273'000'000,373'000'000,473'000'000};
                    auto to=[&](Nanoseconds stamp) {
                        while(edge_mode&&edge<edges.size()&&edges[edge]<=stamp) {
                            advance(edges[edge]);
                            if(edge_mode==1&&edge<2)MEM_B(0xC9,int32_t(dialogue_physical))=edge==0?1:0;
                            else if((edge_mode==1&&edge==2)||(edge_mode==2&&(edge==1||edge==2)))contact(edges[edge]);
                            else if(edge==3) {MEM_B(0,int32_t(0x801B8D34))=0;MEM_B(0,int32_t(0x8021710D))=0;}
                            else if(edge==4) {
                                if(edge_mode==2) {c.r4=int32_t(dialogue_model);dora3_s0_8003B744(rdram,&c);c.r4=int32_t(dialogue_model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);}
                                wf(rdram,prop+0x10,17);wf(rdram,dialogue_physical+4,17);
                            }
                            ++edge;
                        }
                        advance(stamp);
                    };
                    DialogueResult result;
                    for(unsigned tick=1;tick<=36;++tick) {
                        if(scenario==10&&tick==13)MEM_B(0x29,int32_t(dialogue_physical))=0;
                        if(scenario==11&&(tick==10||tick==17))wf(rdram,player+0x9C,tick==10?-900.f:400.f);
                        const auto units=uint64_t(birth_stamp)*60+uint64_t(second)*divider*tick;
                        input_now=Nanoseconds(units/60);city_source_stamp={input_now,unsigned(units%60)};
                        if(native) {
                            const auto owned=std::array<int32_t,5>{MEM_W(0x94,int32_t(prop)),MEM_W(0x10,int32_t(prop)),MEM_W(0xA0,int32_t(prop)),MEM_W(0x64,int32_t(dialogue_model)),MEM_W(0xA4,int32_t(prop))};
                            to(input_now);
                            if(scenario==5) {
                                assert((owned==std::array<int32_t,5>{MEM_W(0x94,int32_t(prop)),MEM_W(0x10,int32_t(prop)),MEM_W(0xA0,int32_t(prop)),MEM_W(0x64,int32_t(dialogue_model)),MEM_W(0xA4,int32_t(prop))}));
                                c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);
                            }
                        } else {
                            const auto start=uint64_t(birth_stamp)*60+uint64_t(second)*divider*(tick-1);
                            city_source_initial_stamp=SourceTimestamp{Nanoseconds(start/60),unsigned(start%60)};
                            c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);city_source_initial_stamp.reset();
                        }
                        if(((scenario>=1&&scenario<=4)||scenario==11)&&(tick==6||(scenario==4&&tick==9)))contact(input_now);
                        if(((scenario>=1&&scenario<=4)||scenario==11)&&tick==13) {
                            // The external release occurs after this completed
                            // rational Source boundary, not just before its
                            // sub-nanosecond remainder at floor(timestamp).
                            if(native&&units%60)to(Nanoseconds((units+59)/60));
                            // The actual initializer sets its pending answer.
                            // This observed outcome stands at the untested
                            // dialogue progression/input boundary.
                            MEM_B(0,int32_t(0x801B8D34))=scenario==3?1:0;
                            MEM_B(0,int32_t(0x8021710D))=0;
                        }
                        if(tick%3==0) {
                            std::array<double,18> v{};v[0]=MEM_W(0x94,int32_t(prop));
                            for(unsigned axis=0;axis<3;++axis)v[axis+1]=rf(rdram,prop+0x9C+4*axis);
                            v[4]=rf(rdram,prop+0x10);v[5]=rf(rdram,dialogue_physical+4);v[6]=rf(rdram,dialogue_physical+0x58);
                            v[7]=MEM_H(0x34,int32_t(dialogue_physical));v[8]=MEM_H(0x42,int32_t(dialogue_physical));
                            v[9]=MEM_BU(0x28,int32_t(dialogue_physical));v[10]=MEM_H(0x40,int32_t(dialogue_physical));
                            v[11]=rf(rdram,dialogue_model+0x64);v[12]=MEM_H(0xD8,int32_t(dialogue_physical));
                            v[13]=MEM_B(1,int32_t(0x801B3700));v[14]=MEM_B(2,int32_t(0x801B3700));v[15]=MEM_B(3,int32_t(0x801B3700));
                            v[16]=MEM_BU(0xC9,int32_t(dialogue_physical));v[17]=allocations.size();result.samples.push_back(v);
                        }
                    }
                    if(kind!=77&&scenario>=1&&scenario<=4&&!edge_mode) {
                        if(MEM_B(3,int32_t(0x801B3700))!=(scenario==1?2:scenario==3?12:7))
                            std::cerr<<"Exchange count mismatch kind="<<kind<<" scenario="<<scenario<<" native="<<native<<" fps="<<render_fps<<" divider="<<divider<<" qty="<<int(MEM_B(3,int32_t(0x801B3700)))<<" answer="<<int(MEM_B(0,int32_t(0x801B8D34)))<<" cost="<<MEM_H(0x44,int32_t(dialogue_physical))<<" phase="<<MEM_H(0x34,int32_t(dialogue_physical))<<'\n';
                        assert(MEM_B(3,int32_t(0x801B3700))==(scenario==1?2:scenario==3?12:7));
                        assert(MEM_B(kind==75?1:2,int32_t(0x801B3700))==(scenario==1||scenario==3?7:8));
                    }
                    result.commands=city_commands;city_watch=dialogue_actor_watch=false;
                    native_gameplay_preview_reset(rdram);preview_live=false;
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(prop);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);
                    assert(allocations==dialogue_heap&&resource_references==dialogue_refs&&game_model_animation().model_count()==6);
                    dialogue_actor_loaded=false;return result;
                };
                const auto expected=run_dialogue(false,60),actual=run_dialogue(true,fps);
                auto compare=[&](const DialogueResult& a,const DialogueResult& b,unsigned mode) {
                    for(unsigned n=0;n<a.samples.size();++n)for(unsigned k=0;k<a.samples[n].size();++k)
                        if(std::abs(a.samples[n][k]-b.samples[n][k])>((k>=1&&k<=3)?0.035:k==4||k==5||k==6||k==11?0.001:0.)) {
                            std::cerr<<"Dialogue actor mismatch kind="<<kind<<" scenario="<<scenario<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<" sample="<<n<<" field="<<k<<" actual="<<a.samples[n][k]<<" expected="<<b.samples[n][k]<<'\n';return false;
                        }
                    if(a.commands!=b.commands) {
                        std::cerr<<"Dialogue actor command mismatch kind="<<kind<<" scenario="<<scenario<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<'\n';
                        for(const auto& list:{b.commands,a.commands}) {
                            for(const auto& command:list)std::cerr<<std::hex<<command.site<<std::dec<<":"<<command.stamp.nanoseconds<<"+"<<command.stamp.sixtieths<<"/60("<<command.args[0]<<","<<command.args[1]<<","<<command.args[2]<<"); ";
                            std::cerr<<'\n';
                        }
                        return false;
                    }
                    return true;
                };
                if(!compare(actual,expected,0))return 1;
                if(scenario==0)for(unsigned mode:{1u,2u}) {
                    const auto baseline=run_dialogue(true,60,mode),varied=run_dialogue(true,fps,mode);
                    if(!compare(varied,baseline,mode))return 1;
                    ++dialogue_actor_native_cases;
                }
                if(scenario==5)++dialogue_actor_rejections;
                memory=dialogue_memory;allocations=dialogue_heap;next_address=dialogue_next;resource_references=dialogue_refs;++dialogue_actor_source_cases;
            }
            assert(dialogue_actor_source_cases==(cases+1)*36&&dialogue_actor_native_cases==(cases+1)*6&&dialogue_actor_rejections==(cases+1)*3);
            city_identity_verified=true;
            // Eight Original city wait/walk callbacks: exact command cadence,
            // every distinct patrol edge, real clips and continuous displacement.
            for(unsigned kind:{13u,14u,15u,16u,17u,20u,21u,22u})for(unsigned scenario=0;scenario<8;++scenario) {
                const auto patrol_memory=memory;const auto patrol_heap=allocations;
                const auto patrol_next=next_address;const auto patrol_refs=resource_references;
                struct PatrolResult {
                    std::array<double,3> initial{},first_end{};
                    std::vector<std::array<double,15>> samples;std::vector<CityCommand> commands;
                };
                auto run_patrol=[&](bool native,unsigned render_fps,unsigned edge_mode=0,const PatrolResult* oracle=nullptr) {
                    native_gameplay_preview_reset(rdram);preview_live=false;memory=patrol_memory;
                    allocations=patrol_heap;next_address=patrol_next;resource_references=patrol_refs;input_now=0;
                    room(rdram);city_loaded=patrol_watch=true;city_watch=false;patrol_original_reference=!native;
                    city_identity_verified=scenario!=4;city_commands.clear();city_source_initial_stamp.reset();
                    for(unsigned n=0;n<4;++n)MEM_W(0x94,int32_t(prop+n*0xCC))=0;
                    std::memset(rdram+(prop&0xFFFFFF),0,0xCC);
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(prop);dora3_scene_object_init(rdram,&c);
                    MEM_H(0xA8,int32_t(prop))=-1;MEM_B(0x6D,int32_t(prop))=0;MEM_W(0x7C,int32_t(prop))=-1;
                    MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                    wf(rdram,physics,0);wf(rdram,physics+0x14,0);MEM_W(0x94,int32_t(player))=1;
                    const Nanoseconds birth_stamp=scenario==5?73'000'000:scenario==6?173'000'000:0;
                    if(native&&birth_stamp) {preview_live=true;native_gameplay_preview_begin(rdram,0,divider);}
                    unsigned frame=0,edge=0;PatrolResult result;
                    auto observe=[&](Nanoseconds stamp) {
                        if(!oracle||scenario!=1||stamp<=birth_stamp||uint64_t(stamp-birth_stamp)*60>=uint64_t(second)*divider)return;
                        const double fraction=double(stamp-birth_stamp)*60./(double(second)*divider);
                        for(unsigned axis=0;axis<3;++axis) {
                            const auto expected=oracle->initial[axis]+(oracle->first_end[axis]-oracle->initial[axis])*fraction;
                            if(std::abs(double(rf(rdram,prop+0x9C+4*axis))-expected)>=0.001) {
                                std::cerr<<"City intermediate position mismatch kind="<<kind<<" fps="<<render_fps<<" axis="<<axis<<" stamp="<<stamp<<" actual="<<rf(rdram,prop+0x9C+4*axis)<<" expected="<<expected<<'\n';std::abort();
                            }
                        }
                        ++patrol_position_samples;
                    };
                    auto advance=[&](Nanoseconds stamp) {
                        while(second*(frame+1)/render_fps<stamp) {
                            input_now=second*++frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;
                            native_gameplay_preview_update(rdram,&c,input_now);assert(std::memcmp(&c,&saved,sizeof(c))==0);observe(input_now);
                        }
                        input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                        assert(std::memcmp(&c,&saved,sizeof(c))==0);observe(stamp);
                    };
                    if(native&&birth_stamp)advance(birth_stamp);
                    input_now=birth_stamp;c=saved;c.f_odd=&c.f0.u32h;
                    c.r4=0xF4;dora3_game_heap_alloc(rdram,&c);const auto patrol_physical=uint32_t(c.r2);
                    c.r4=int32_t(patrol_physical);dora3_s0_80042CAC(rdram,&c);
                    auto create=[&] {c.r4=int32_t(prop);c.r5=0;c.r6=int32_t(asset);dora3_s0_8002A100(rdram,&c);};
                    if(birth_stamp) {
                        const auto units=uint64_t(birth_stamp)*60,den=uint64_t(divider)*second;
                        const SimulationSlice born{1,den,units/den,units%den};SimulationScope birth(born);create();
                    } else create();
                    auto patrol_model=uint32_t(MEM_W(0x74,int32_t(prop)));
                    c.r4=int32_t(patrol_model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                    c.r4=6;dora3_game_heap_alloc(rdram,&c);const auto clip_list=uint32_t(c.r2);
                    for(unsigned n=0;n<3;++n)MEM_H(int32_t(2*n),int32_t(clip_list))=0;
                    MEM_W(0xB4,int32_t(prop))=int32_t(clip_list);MEM_W(0,int32_t(0x801AEC00))=int32_t(clip);
                    MEM_W(0x98,int32_t(prop))=int32_t(patrol_physical);MEM_W(0x94,int32_t(prop))=(scenario>=1&&scenario<=3)||scenario==6?2:1;
                    MEM_H(0x20,int32_t(patrol_physical))=int16_t(kind);MEM_H(0xBC,int32_t(patrol_physical))=-1;
                    MEM_H(0xD8,int32_t(patrol_physical))=9;
                    MEM_H(0xBE,int32_t(patrol_physical))=-500;MEM_H(0xC2,int32_t(patrol_physical))=-300;
                    MEM_B(0x29,int32_t(patrol_physical))=scenario==7?1:0;
                    const bool x_axis=kind==15||kind==16||kind==20||kind==21;
                    const float limit=kind==13?300.f:kind==14?150.f:kind==15||kind==17?100.f:50.f;
                    float x=kind==20?-3500.f:-500.f,z=kind==22?-100.f:-300.f;
                    const bool negative=scenario==3;
                    float yaw=x_axis?(negative?270.f:90.f):(negative?180.f:0.f);
                    if(scenario==2||scenario==3) {
                        if(kind==20)x=negative?-4800.f:-2300.f;
                        else if(kind==22) {z=negative?3500.f:-800.f;yaw=negative?0.f:180.f;}
                        else if(x_axis)x+=negative?-limit:limit;
                        else z+=negative?-limit:limit;
                        MEM_H(0x40,int32_t(patrol_physical))=negative?1:0;
                    }
                    wf(rdram,prop+0x9C,x);wf(rdram,prop+0xA0,80);wf(rdram,prop+0xA4,z);wf(rdram,prop+0x10,yaw);
                    wf(rdram,patrol_physical+4,yaw);wf(rdram,patrol_physical+0x14,0);
                    auto source_pass=[&](bool compare_original) {
                        if(!compare_original) {reference_dora3_s3_801E8BDC(rdram,&c);return;}
                        const auto before=memory;const auto before_context=c;const auto before_commands=city_commands;
                        const auto before_heap=allocations;const auto before_next=next_address;const auto before_refs=resource_references;
                        assert(patrol_original_reference);reference_dora3_s3_801E8BDC(rdram,&c);
                        const auto expected=memory;const auto expected_context=c;const auto expected_commands=city_commands;
                        const auto expected_heap=allocations;const auto expected_next=next_address;const auto expected_refs=resource_references;
                        memory=before;allocations=before_heap;next_address=before_next;resource_references=before_refs;
                        c=before_context;city_commands=before_commands;patrol_original_reference=false;
                        reference_dora3_s3_801E8BDC(rdram,&c);patrol_original_reference=true;
                        assert(memory==expected&&std::memcmp(&c,&expected_context,sizeof(c))==0&&city_commands==expected_commands);
                        assert(allocations==expected_heap&&next_address==expected_next&&resource_references==expected_refs);
                        ++patrol_original_ram_cases;
                    };
                    if(!birth_stamp) {city_source_stamp={0,0};source_pass(!native);}
                    for(unsigned axis=0;axis<3;++axis)result.initial[axis]=rf(rdram,prop+0x9C+4*axis);
                    city_commands.clear();preview_live=native;
                    if(native&&!birth_stamp)native_gameplay_preview_begin(rdram,0,divider);
                    const std::array<Nanoseconds,5> edges{73'000'000,173'000'000,273'000'000,373'000'000,473'000'000};
                    auto to=[&](Nanoseconds stamp) {
                        while(edge_mode&&edge<edges.size()&&edges[edge]<=stamp) {
                            advance(edges[edge]);city_source_stamp={edges[edge],0};
                            if(edge_mode==1&&edge<2)MEM_B(0xC9,int32_t(patrol_physical))=edge==0?1:0;
                            else if(edge_mode==2&&edge==0) {
                                MEM_W(0x94,int32_t(prop))=99;
                                c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);
                            } else if(edge_mode==2&&edge==1) {
                                MEM_W(0x94,int32_t(prop))=scenario==1?2:1;MEM_B(0x28,int32_t(patrol_physical))=0;
                            } else if(edge_mode==3&&edge==1) {
                                c.r4=int32_t(patrol_model);dora3_s0_8003B77C(rdram,&c);
                                const auto units=uint64_t(edges[edge])*60,den=uint64_t(divider)*second;
                                const SimulationSlice born{1,den,units/den,units%den};
                                {SimulationScope birth(born);create();}
                                patrol_model=uint32_t(MEM_W(0x74,int32_t(prop)));
                                c.r4=int32_t(patrol_model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                            } else if(edge==2) {
                                if(edge_mode==1&&scenario==1)wf(rdram,patrol_physical+4,33);
                                else {MEM_W(0x94,int32_t(prop))=1;MEM_B(0x28,int32_t(patrol_physical))=0;MEM_H(0x34,int32_t(patrol_physical))=0;}
                            } else if(edge==3) {
                                wf(rdram,prop+0x9C,kind==20?-3501.f:-501.f);wf(rdram,prop+0xA4,kind==22?-101.f:-301.f);
                                wf(rdram,patrol_physical+4,33);
                            } else if(edge==4) {
                                c.r4=int32_t(patrol_model);dora3_s0_8003B744(rdram,&c);
                                c.r4=int32_t(patrol_model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                            }
                            ++edge;
                        }
                        advance(stamp);
                    };
                    for(unsigned tick=1;tick<=144;++tick) {
                        if(scenario==7&&tick==13)MEM_B(0x29,int32_t(patrol_physical))=0;
                        const auto units=uint64_t(birth_stamp)*60+uint64_t(second)*divider*tick;
                        input_now=Nanoseconds(units/60);city_source_stamp={input_now,unsigned(units%60)};
                        if(native) {
                            const auto owned=std::array<int32_t,5>{MEM_W(0x94,int32_t(prop)),MEM_H(0x34,int32_t(patrol_physical)),MEM_W(0x9C,int32_t(prop)),MEM_W(0x64,int32_t(patrol_model)),MEM_W(0xA4,int32_t(prop))};
                            to(input_now);
                            if(scenario==4) {
                                assert((owned==std::array<int32_t,5>{MEM_W(0x94,int32_t(prop)),MEM_H(0x34,int32_t(patrol_physical)),MEM_W(0x9C,int32_t(prop)),MEM_W(0x64,int32_t(patrol_model)),MEM_W(0xA4,int32_t(prop))}));
                                c=saved;c.f_odd=&c.f0.u32h;
                                const auto start=uint64_t(birth_stamp)*60+uint64_t(second)*divider*(tick-1);
                                city_source_initial_stamp=SourceTimestamp{Nanoseconds(start/60),unsigned(start%60)};
                                reference_dora3_s3_801E8BDC(rdram,&c);city_source_initial_stamp.reset();
                            }
                        } else {
                            const auto start=uint64_t(birth_stamp)*60+uint64_t(second)*divider*(tick-1);
                            city_source_initial_stamp=SourceTimestamp{Nanoseconds(start/60),unsigned(start%60)};
                            c=saved;c.f_odd=&c.f0.u32h;source_pass(tick==1||tick==39||tick==40||tick==41||tick==50||tick==51||tick==144);city_source_initial_stamp.reset();
                        }
                        std::array<double,15> v{};v[0]=MEM_W(0x94,int32_t(prop));
                        for(unsigned axis=0;axis<3;++axis)v[axis+1]=rf(rdram,prop+0x9C+4*axis);
                        v[4]=rf(rdram,prop+0x10);v[5]=rf(rdram,patrol_physical+4);
                        v[6]=MEM_H(0x34,int32_t(patrol_physical));v[7]=MEM_BU(0x28,int32_t(patrol_physical));
                        v[8]=MEM_H(0x40,int32_t(patrol_physical));v[9]=rf(rdram,patrol_model+0x64);
                        v[10]=MEM_H(0xD8,int32_t(patrol_physical));v[11]=MEM_BU(0xC9,int32_t(patrol_physical));
                        v[12]=allocations.size();v[13]=rf(rdram,patrol_physical);v[14]=game_model_animation().model_count();
                        if(tick==1)for(unsigned axis=0;axis<3;++axis)result.first_end[axis]=v[axis+1];
                        // Exact nanosecond Source checkpoints. Rounding a1/3ns
                        // end upwards would also execute the next period's first
                        // command; flooring it is before its discrete end counters.
                        // All clip commands retain their rational timestamp trace.
                        if(tick%3==0)result.samples.push_back(v);
                    }
                    result.commands=city_commands;patrol_watch=patrol_original_reference=false;native_gameplay_preview_reset(rdram);preview_live=false;
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(prop);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);
                    assert(allocations==patrol_heap&&resource_references==patrol_refs&&game_model_animation().model_count()==6);
                    city_loaded=false;return result;
                };
                const auto expected=run_patrol(false,60),actual=run_patrol(true,fps,0,&expected);
                auto compare=[&](const PatrolResult& a,const PatrolResult& b,unsigned mode) {
                    assert(a.samples.size()==b.samples.size());
                    for(unsigned n=0;n<a.samples.size();++n)for(unsigned k=0;k<a.samples[n].size();++k)
                        if(std::abs(a.samples[n][k]-b.samples[n][k])>((k>=1&&k<=3)?0.001:k==4||k==5||k==9||k==13?0.001:0.)) {
                            std::cerr<<"City patrol mismatch kind="<<kind<<" scenario="<<scenario<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<" tick="<<3*(n+1)<<" field="<<k<<" actual="<<a.samples[n][k]<<" expected="<<b.samples[n][k]<<'\n';return false;
                        }
                    if(a.commands!=b.commands) {
                        std::cerr<<"City patrol command mismatch kind="<<kind<<" scenario="<<scenario<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<'\n';
                        for(const auto& list:{b.commands,a.commands}) {
                            for(const auto& command:list)std::cerr<<std::hex<<command.site<<std::dec<<":"<<command.stamp.nanoseconds<<"+"<<command.stamp.sixtieths<<"/60("<<command.args[0]<<","<<command.args[1]<<","<<command.args[2]<<"); ";
                            std::cerr<<'\n';
                        }
                        return false;
                    }
                    return true;
                };
                if(!compare(actual,expected,0))return 1;
                if(scenario<=1)for(unsigned mode:{1u,2u,3u}) {
                    const auto baseline=run_patrol(true,60,mode),varied=run_patrol(true,fps,mode);
                    if(!compare(varied,baseline,mode))return 1;
                    ++patrol_native_cases;
                }
                if(scenario==4)++patrol_rejections;
                memory=patrol_memory;allocations=patrol_heap;next_address=patrol_next;resource_references=patrol_refs;++patrol_source_cases;
            }
            assert(patrol_source_cases==(cases+1)*64&&patrol_native_cases==(cases+1)*48&&patrol_rejections==(cases+1)*8);
            city_identity_verified=true;

            // Eight actual city dialogue callbacks. Constructor-selected
            // clip/sprite profiles are taken from verified ROM instructions;
            // assets are synthetic, and sprite2 requests are observed boundaries.
            for(unsigned kind:{13u,14u,15u,16u,17u,20u,21u,22u})for(unsigned profile:{0u,2u})for(unsigned scenario=0;scenario<8;++scenario) {
                const auto dialogue_memory=memory;const auto dialogue_heap=allocations;
                const auto dialogue_next=next_address;const auto dialogue_refs=resource_references;
                struct CityDialogueResult {bool first_continuous=false;std::array<double,2> initial{},first_end{};std::vector<std::array<double,18>> samples;std::vector<CityCommand> commands;};
                auto run_city_dialogue=[&](bool native,unsigned render_fps,unsigned edge_mode=0,const CityDialogueResult* oracle=nullptr) {
                    native_gameplay_preview_reset(rdram);preview_live=false;memory=dialogue_memory;
                    allocations=dialogue_heap;next_address=dialogue_next;resource_references=dialogue_refs;input_now=0;
                    room(rdram);city_loaded=city_dialogue_watch=patrol_watch=true;city_watch=false;patrol_original_reference=!native;
                    city_identity_verified=scenario!=4;city_commands.clear();city_source_initial_stamp.reset();
                    for(unsigned slot=30;slot<230;++slot)MEM_W(0x94,int32_t(0x80171CD0u+slot*0xCC))=0;
                    std::memset(rdram+(prop&0xFFFFFF),0,0xCC);c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(prop);dora3_scene_object_init(rdram,&c);
                    MEM_H(0xA8,int32_t(prop))=-1;MEM_B(0x6D,int32_t(prop))=0;MEM_W(0x7C,int32_t(prop))=-1;
                    MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                    wf(rdram,physics,0);wf(rdram,physics+0x14,0);MEM_W(0x94,int32_t(player))=1;
                    wf(rdram,player+0x9C,-700);wf(rdram,player+0xA0,80);wf(rdram,player+0xA4,-50);
                    const Nanoseconds birth_stamp=scenario==5?73'000'000:scenario==6?173'000'000:0;
                    if(native&&birth_stamp) {preview_live=true;native_gameplay_preview_begin(rdram,0,divider);
                        dora3_native_preview_hook(rdram,&c,0x801BB080);dora3_native_preview_hook(rdram,&c,0x801E8BDC);}
                    unsigned frame=0,edge=0;CityDialogueResult result;uint32_t physical=0,model=0;
                    auto observe=[&](Nanoseconds stamp) {
                        if(!oracle||!oracle->first_continuous||scenario>3||stamp<=birth_stamp||uint64_t(stamp-birth_stamp)*60>=uint64_t(second)*divider)return;
                        const double fraction=double(stamp-birth_stamp)*60./(double(second)*divider);
                        for(unsigned i=0;i<2;++i) {
                            const double expected=std::fmod(oracle->initial[i]+std::remainder(oracle->first_end[i]-oracle->initial[i],360.)*fraction+360.,360.);
                            const double actual=rf(rdram,i?physical+4:prop+0x10);
                            if(std::abs(std::remainder(actual-expected,360.))>=0.001) {std::cerr<<"City dialogue intermediate yaw kind="<<kind<<" profile="<<profile<<" fps="<<render_fps<<" divider="<<divider<<" stamp="<<stamp<<" field="<<i<<" actual="<<actual<<" expected="<<expected<<'\n';std::abort();}
                            ++city_dialogue_yaw_samples;
                        }
                    };
                    auto advance=[&](Nanoseconds stamp) {
                        while(second*(frame+1)/render_fps<stamp) {input_now=second*++frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;
                            native_gameplay_preview_update(rdram,&c,input_now);assert(std::memcmp(&c,&saved,sizeof(c))==0);if(physical)observe(input_now);}
                        input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                        assert(std::memcmp(&c,&saved,sizeof(c))==0);if(physical)observe(stamp);
                    };
                    if(native&&birth_stamp)advance(birth_stamp);
                    input_now=birth_stamp;c=saved;c.f_odd=&c.f0.u32h;c.r4=0xF4;dora3_game_heap_alloc(rdram,&c);physical=uint32_t(c.r2);
                    c.r4=int32_t(physical);dora3_s0_80042CAC(rdram,&c);
                    auto create=[&] {c.r4=int32_t(prop);c.r5=0;c.r6=int32_t(asset);dora3_s0_8002A100(rdram,&c);};
                    if(birth_stamp) {const auto units=uint64_t(birth_stamp)*60,den=uint64_t(divider)*second;const SimulationSlice born{1,den,units/den,units%den};SimulationScope birth(born);create();}
                    else create();
                    model=uint32_t(MEM_W(0x74,int32_t(prop)));c.r4=int32_t(model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                    c.r4=18;dora3_game_heap_alloc(rdram,&c);const auto clips=uint32_t(c.r2);for(unsigned n=0;n<9;++n)MEM_H(int32_t(2*n),int32_t(clips))=0;
                    MEM_W(0xB4,int32_t(prop))=int32_t(clips);MEM_W(0,int32_t(0x801AEC00))=int32_t(clip);
                    MEM_W(0x98,int32_t(prop))=int32_t(physical);MEM_W(0x94,int32_t(prop))=3;MEM_H(0x20,int32_t(physical))=int16_t(kind);
                    MEM_H(0xBC,int32_t(physical))=-1;MEM_H(0xD8,int32_t(physical))=9;MEM_H(0xBE,int32_t(physical))=-500;MEM_H(0xC2,int32_t(physical))=-300;
                    const int offset=kind==13||kind==14||kind==15?(profile==0?6:3):kind==17||kind==22?(profile==0?3:6):0;
                    const int sprite=kind==13?(profile==0?2:4):kind==16?(profile==0?1:3):(profile==0?0:2);
                    MEM_H(0x44,int32_t(physical))=int16_t(offset);MEM_H(0x42,int32_t(physical))=int16_t(sprite);
                    wf(rdram,prop+0x9C,kind==20?-3500.f:-500.f);wf(rdram,prop+0xA0,80);wf(rdram,prop+0xA4,kind==22?-100.f:-300.f);
                    wf(rdram,prop+0x10,359);wf(rdram,physical+0x14,0);
                    wf(rdram,physical,0);wf(rdram,physical+4,359);
                    auto contact=[&](bool native) {
                        switch(kind) {
                        case 13:if(native)dora3_s8_level1_8022DD94(rdram,&c);else reference_dora3_s8_level1_8022DD94(rdram,&c);break;
                        case 14:if(native)dora3_s8_level1_8022E6F4(rdram,&c);else reference_dora3_s8_level1_8022E6F4(rdram,&c);break;
                        case 15:if(native)dora3_s8_level1_8022EFC4(rdram,&c);else reference_dora3_s8_level1_8022EFC4(rdram,&c);break;
                        case 16:if(native)dora3_s8_level1_8022F8AC(rdram,&c);else reference_dora3_s8_level1_8022F8AC(rdram,&c);break;
                        case 17:if(native)dora3_s8_level1_80230190(rdram,&c);else reference_dora3_s8_level1_80230190(rdram,&c);break;
                        case 20:if(native)dora3_s8_level1_80231194(rdram,&c);else reference_dora3_s8_level1_80231194(rdram,&c);break;
                        case 21:if(native)dora3_s8_level1_80231A18(rdram,&c);else reference_dora3_s8_level1_80231A18(rdram,&c);break;
                        case 22:if(native)dora3_s8_level1_80232578(rdram,&c);else reference_dora3_s8_level1_80232578(rdram,&c);break;
                        default:std::abort();
                        }
                    };
                    const auto contact_memory=memory;const auto contact_heap=allocations;
                    const auto contact_next=next_address;const auto contact_refs=resource_references;
                    auto run_contact=[&](unsigned mode,int state,int count,bool exact) {
                        MEM_W(0,int32_t(0x8021BDEC))=int32_t(prop);MEM_W(0,int32_t(0x8021BCDC))=int32_t(physical);
                        MEM_W(0,int32_t(0x8021BCE4))=int32_t(mode);MEM_W(0x94,int32_t(prop))=state;
                        MEM_H(0x34,int32_t(physical))=int16_t(count);MEM_B(0x28,int32_t(physical))=1;
                        MEM_W(0,int32_t(0x801B3628))=0;MEM_W(12,int32_t(0x801B3628))=profile==2?int32_t(1u<<30):0;
                        MEM_B(0,int32_t(0x8021710D))=0;city_watch=true;city_source_stamp={birth_stamp,0};c=saved;c.f_odd=&c.f0.u32h;
                        if(exact) {
                            const auto before=memory;const auto before_commands=city_commands;const auto before_context=c;
                            const auto before_heap=allocations;const auto before_next=next_address;const auto before_refs=resource_references;
                            contact(false);const auto expected=memory;const auto expected_commands=city_commands;const auto expected_context=c;
                            const auto expected_heap=allocations;const auto expected_next=next_address;const auto expected_refs=resource_references;
                            memory=before;city_commands=before_commands;c=before_context;allocations=before_heap;next_address=before_next;resource_references=before_refs;
                            contact(true);assert(memory==expected&&city_commands==expected_commands&&std::memcmp(&c,&expected_context,sizeof(c))==0);
                            assert(allocations==expected_heap&&next_address==expected_next&&resource_references==expected_refs);++city_dialogue_contact_cases;
                        } else contact(true);
                        const bool accepted=mode==4&&state!=3&&state!=100;
                        assert(MEM_W(0x94,int32_t(prop))==(accepted?3:state));
                        assert(MEM_B(0,int32_t(0x8021710D))==(accepted?1:0));
                        if(accepted)assert(MEM_BU(0x28,int32_t(physical))==0);
                        city_watch=false;
                    };
                    if(!native&&scenario==0)for(const auto test:std::array<std::array<int,3>,8>{{{0,2,0},{1,2,0},{2,2,0},{3,2,0},{4,3,0},{4,100,0},{4,1,39},{4,1,40}}}) {
                        run_contact(unsigned(test[0]),test[1],test[2],true);
                        memory=contact_memory;allocations=contact_heap;next_address=contact_next;resource_references=contact_refs;city_commands.clear();
                    }
                    if(scenario==0||scenario==1||scenario==5||scenario==6)run_contact(4,2,0,!native);
                    // A completed external dialogue may be released immediately
                    // after the real producer; UI/input progression is a boundary.
                    MEM_B(0,int32_t(0x8021710D))=scenario==1||scenario==6?0:1;
                    auto source_pass=[&](bool compare_original) {
                        if(!compare_original) {reference_dora3_s3_801E8BDC(rdram,&c);return;}
                        const auto before=memory;const auto before_context=c;const auto before_commands=city_commands;
                        const auto before_heap=allocations;const auto before_next=next_address;const auto before_refs=resource_references;
                        assert(patrol_original_reference);reference_dora3_s3_801E8BDC(rdram,&c);
                        const auto expected=memory;const auto expected_context=c;const auto expected_commands=city_commands;
                        const auto expected_heap=allocations;const auto expected_next=next_address;const auto expected_refs=resource_references;
                        memory=before;c=before_context;city_commands=before_commands;allocations=before_heap;next_address=before_next;resource_references=before_refs;
                        patrol_original_reference=false;reference_dora3_s3_801E8BDC(rdram,&c);patrol_original_reference=true;
                        assert(memory==expected&&city_commands==expected_commands&&std::memcmp(&c,&expected_context,sizeof(c))==0);
                        assert(allocations==expected_heap&&next_address==expected_next&&resource_references==expected_refs);++city_dialogue_original_ram_cases;
                    };
                    if(!birth_stamp) {city_source_stamp={0,0};source_pass(!native);}
                    if(scenario==2||scenario==3||scenario==7) {
                        MEM_W(0x94,int32_t(prop))=3;MEM_B(0x28,int32_t(physical))=1;MEM_H(0x34,int32_t(physical))=1;MEM_H(0x36,int32_t(physical))=1;
                        wf(rdram,prop+0x10,scenario==2?350.f:scenario==3?10.f:17.f);wf(rdram,physical+0x58,scenario==2?120.f:scenario==3?240.f:17.f);
                        wf(rdram,physical+4,scenario==7?17.f:rf(rdram,physical+4));wf(rdram,physical+0x5C,scenario==2?5.f:scenario==3?-5.f:0.f);
                    }
                    result.initial={rf(rdram,prop+0x10),rf(rdram,physical+4)};city_commands.clear();preview_live=native;
                    if(native&&!birth_stamp) {native_gameplay_preview_begin(rdram,0,divider);dora3_native_preview_hook(rdram,&c,0x801BB080);dora3_native_preview_hook(rdram,&c,0x801E8BDC);}
                    const std::array<Nanoseconds,5> edges{73'000'000,173'000'000,273'000'000,373'000'000,473'000'000};
                    auto to=[&](Nanoseconds stamp) {
                        while(edge_mode&&edge<edges.size()&&edges[edge]<=stamp) {
                            advance(edges[edge]);city_source_stamp={edges[edge],0};
                            if(edge_mode==1&&edge<2)MEM_B(0xC9,int32_t(physical))=edge==0?1:0;
                            else if(edge_mode==2&&edge==0) {MEM_W(0x94,int32_t(prop))=99;c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);}
                            else if(edge_mode==2&&edge==1) {MEM_W(0x94,int32_t(prop))=3;MEM_B(0x28,int32_t(physical))=0;}
                            else if(edge_mode==3&&edge==1) {c.r4=int32_t(model);dora3_s0_8003B77C(rdram,&c);
                                const auto units=uint64_t(edges[edge])*60,den=uint64_t(divider)*second;const SimulationSlice born{1,den,units/den,units%den};{SimulationScope birth(born);create();}
                                model=uint32_t(MEM_W(0x74,int32_t(prop)));c.r4=int32_t(model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);}
                            else if(edge==2) {MEM_W(0x94,int32_t(prop))=3;MEM_B(0x28,int32_t(physical))=0;}
                            else if(edge==3) {wf(rdram,prop+0x10,33);wf(rdram,physical+4,33);wf(rdram,player+0x9C,-900);}
                            else if(edge==4) {c.r4=int32_t(model);dora3_s0_8003B744(rdram,&c);c.r4=int32_t(model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);}
                            ++edge;
                        }
                        advance(stamp);
                    };
                    for(unsigned tick=1;tick<=96;++tick) {
                        const auto units=uint64_t(birth_stamp)*60+uint64_t(second)*divider*tick;
                        if(tick==13&&!edge_mode&&(scenario==0||scenario==5)) {
                            if(native)to(birth_stamp+second*divider*(tick-1)/60);
                            MEM_B(0,int32_t(0x8021710D))=0;
                        }
                        input_now=Nanoseconds(units/60);city_source_stamp={input_now,unsigned(units%60)};
                        if(native) {
                            const auto state=MEM_W(0x94,int32_t(prop));const auto phase=MEM_H(0x34,int32_t(physical));const auto count=city_commands.size();to(input_now);
                            if(scenario==4) {assert(MEM_W(0x94,int32_t(prop))==state&&MEM_H(0x34,int32_t(physical))==phase&&city_commands.size()==count);
                                c=saved;c.f_odd=&c.f0.u32h;reference_dora3_s3_801E8BDC(rdram,&c);}
                        } else {
                            const auto start=uint64_t(birth_stamp)*60+uint64_t(second)*divider*(tick-1);
                            if(scenario!=4)city_source_initial_stamp=SourceTimestamp{Nanoseconds(start/60),unsigned(start%60)};
                            c=saved;c.f_odd=&c.f0.u32h;source_pass(tick==1||tick==13||tick==14||tick==30||tick==31||tick==96);city_source_initial_stamp.reset();
                        }
                        if(tick==1) {result.first_end={rf(rdram,prop+0x10),rf(rdram,physical+4)};result.first_continuous=MEM_W(0x94,int32_t(prop))==3;}
                        if(tick%3==0) {
                            std::array<double,18> v{};v[0]=MEM_W(0x94,int32_t(prop));v[1]=rf(rdram,prop+0x10);v[2]=rf(rdram,physical+4);
                            v[3]=MEM_BU(0x28,int32_t(physical));v[4]=MEM_H(0x34,int32_t(physical));v[5]=MEM_H(0x36,int32_t(physical));
                            v[6]=rf(rdram,physical+0x58);v[7]=rf(rdram,physical+0x5C);v[8]=rf(rdram,model+0x64);v[9]=MEM_H(0xD8,int32_t(physical));
                            for(unsigned axis=0;axis<3;++axis)v[10+axis]=rf(rdram,prop+0x9C+4*axis);
                            v[13]=MEM_BU(0xC9,int32_t(physical));v[14]=allocations.size();v[15]=resource_references[2];v[16]=rf(rdram,physical);v[17]=game_model_animation().model_count();result.samples.push_back(v);
                        }
                    }
                    result.commands=city_commands;city_dialogue_watch=patrol_watch=patrol_original_reference=false;native_gameplay_preview_reset(rdram);preview_live=false;
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(prop);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);
                    assert(allocations==dialogue_heap&&resource_references==dialogue_refs&&game_model_animation().model_count()==6);city_loaded=false;return result;
                };
                const auto expected=run_city_dialogue(false,60),actual=run_city_dialogue(true,fps,0,&expected);
                auto compare=[&](const CityDialogueResult& a,const CityDialogueResult& b,unsigned mode) {
                    assert(a.samples.size()==b.samples.size());
                    for(unsigned n=0;n<a.samples.size();++n)for(unsigned k=0;k<a.samples[n].size();++k) {
                        const double tolerance=k==1||k==2||k==6||k==7||k==8||k==16?0.001:k>=10&&k<=12?0.03:0.;
                        if(std::abs(a.samples[n][k]-b.samples[n][k])>tolerance) {std::cerr<<"City dialogue mismatch kind="<<kind<<" profile="<<profile<<" scenario="<<scenario<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<" tick="<<3*(n+1)<<" field="<<k<<" actual="<<a.samples[n][k]<<" expected="<<b.samples[n][k]<<'\n';return false;}
                    }
                    if(a.commands!=b.commands) {std::cerr<<"City dialogue command mismatch kind="<<kind<<" profile="<<profile<<" scenario="<<scenario<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<'\n';
                        for(const auto& list:{b.commands,a.commands}) {for(const auto& command:list)std::cerr<<std::hex<<command.site<<std::dec<<":"<<command.stamp.nanoseconds<<"+"<<command.stamp.sixtieths<<"/60("<<command.args[0]<<","<<command.args[1]<<","<<command.args[2]<<"); ";std::cerr<<'\n';}return false;}
                    return true;
                };
                if(!compare(actual,expected,0))return 1;
                if(scenario<=1)for(unsigned mode:{1u,2u,3u}) {const auto baseline=run_city_dialogue(true,60,mode),varied=run_city_dialogue(true,fps,mode);if(!compare(varied,baseline,mode))return 1;++city_dialogue_native_cases;}
                if(scenario==4)++city_dialogue_rejections;
                memory=dialogue_memory;allocations=dialogue_heap;next_address=dialogue_next;resource_references=dialogue_refs;++city_dialogue_source_cases;
            }

            for(auto model:object_models) {c.r4=int32_t(model);dora3_s0_8003B77C(rdram,&c);}
            // Existing terminal effects acquire native ownership after a
            // completed Original pass. Their clip pose is evaluated before
            // Original increments its time, so acquisition must preserve the
            // remaining source-time lifetime rather than consume its pending
            // end flag immediately in the first variable frame.
            std::memset(rdram+(prop&0xFFFFFF),0,200*0xCC);
            constexpr uint32_t effect=0x801724C8,effect_clip=0x80103000;
            std::memcpy(rdram+(effect_clip&0xFFFFFF),rdram+(clip&0xFFFFFF),0x1000);
            wf(rdram,effect_clip+0x204,3);wf(rdram,effect_clip+0x220,3);
            MEM_W(0,int32_t(0x801AEC00+20))=int32_t(asset);
            MEM_W(0,int32_t(0x801AEC00+40))=int32_t(effect_clip);
            MEM_W(0,int32_t(0x801AEC00+80))=int32_t(descriptor);
            const std::array<unsigned,7> shot_kinds{91,93,95,97,99,107,107};
            for(unsigned scenario=0;scenario<shot_kinds.size();++scenario) {
                terminal_end_commit=false;
                const auto kind=shot_kinds[scenario];const bool upgrade=scenario==6;
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;
                MEM_H(0x14,int32_t(control))=0;MEM_W(0x94,int32_t(player))=1;
                wf(rdram,physics,0);wf(rdram,physics+0x14,0);wf(rdram,player+0xA0,0);
                std::memset(rdram+(effect&0xFFFFFF),0,20*0xCC);
                for(unsigned i=0;i<20;++i) {
                    c.r4=int32_t(effect+i*0xCC);dora3_scene_object_init(rdram,&c);
                    MEM_H(0xA8,int32_t(effect+i*0xCC))=-1;
                }
                c.r4=0xF4;dora3_game_heap_alloc(rdram,&c);const auto shot_physics=uint32_t(c.r2);
                c.r4=int32_t(shot_physics);dora3_s0_80042CAC(rdram,&c);
                c.r4=int32_t(effect);c.r5=0;c.r6=int32_t(asset);dora3_s0_8002A100(rdram,&c);
                const auto shot_model=uint32_t(MEM_W(0x74,int32_t(effect)));
                c.r4=int32_t(shot_model);c.r5=int32_t(upgrade?effect_clip:clip);dora3_s0_8003B6BC(rdram,&c);
                if(upgrade) {
                    MEM_B(0x7C,int32_t(shot_model))=1;
                    c.r4=4;dora3_game_heap_alloc(rdram,&c);const auto clip_table=uint32_t(c.r2);
                    MEM_W(0xB4,int32_t(effect))=int32_t(clip_table);
                    MEM_H(2,int32_t(clip_table))=0; // Clip1 resolves to the real fixture clip30.
                }
                MEM_W(0x94,int32_t(effect))=1;MEM_W(0x98,int32_t(effect))=int32_t(shot_physics);
                MEM_H(0xA8,int32_t(effect))=-1;MEM_H(0x20,int32_t(shot_physics))=int16_t(kind);
                MEM_H(0x34,int32_t(shot_physics))=15;
                wf(rdram,shot_physics,upgrade?25:4);wf(rdram,shot_physics+4,90);
                wf(rdram,effect+0x9C,500);wf(rdram,effect+0xA0,100);
                game_model_animation().hook(rdram,&c,0x80030850,0);
                auto source_shot=memory;auto source_context=c;source_context.f_odd=&source_context.f0.u32h;
                reference_dora3_s3_801D10D8(reinterpret_cast<uint8_t*>(source_shot.data()),&source_context);
                dora3_s3_801D10D8(rdram,&c);
                assert(memory==source_shot);
                assert(MEM_W(0x94,int32_t(effect))==1 && std::abs(rf(rdram,effect+0x9C)-(upgrade?525:504))<0.001f);
                impact_spawns=shot_clip_changes=0;terminal_model=shot_model;terminal_deletes=0;
                preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned shot_vi=0;
                auto world_to=[&](Nanoseconds end) {
                    while(second*(shot_vi+1)/60<=end) {
                        ++shot_vi;input_now=second*shot_vi/60;dora3_controller_accumulate(rdram,&c);
                        if(shot_vi%divider==0) {
                            dora3_controller_latch(rdram,&c);
                            dora3_native_preview_hook(rdram,&c,0x801BB080);
                            dora3_s3_801E8BDC(rdram,&c);c.r4=1;dora3_s3_801C8B28(rdram,&c);
                            const auto x=MEM_W(0x9C,int32_t(effect));
                            dora3_s3_801D10D8(rdram,&c);
                            assert(MEM_W(0x9C,int32_t(effect))==x); // Source owner suppresses the shot.
                            dora3_s3_801DBD58(rdram,&c);
                        }
                    }
                    input_now=end;c=saved;c.f_odd=&c.f0.u32h;
                    native_gameplay_preview_update(rdram,&c,end);
                };
                for(unsigned frame=1;frame<=fps/2;++frame)world_to(second*frame/fps);
                world_to(second/2);
                const auto expected=upgrade?525+25+30.*(30./divider-1):504+4.*30/divider;
                if(std::abs(rf(rdram,effect+0x9C)-expected)>0.03 || impact_spawns || terminal_deletes) {
                    std::cerr<<"Flight partition mismatch kind="<<kind<<" divider="<<divider<<" fps="<<fps
                             <<" x="<<rf(rdram,effect+0x9C)<<" expected="<<expected<<" hits="<<impact_spawns<<'\n';return 1;
                }
                assert(shot_clip_changes==unsigned(upgrade));
                if(upgrade)assert(rf(rdram,shot_physics)==30);
                // Pause is a source-time command. Flush first, then ensure both
                // movement and model time remain frozen through variable frames.
                MEM_B(0xC9,int32_t(shot_physics))=1;
                const auto frozen_x=MEM_W(0x9C,int32_t(effect)),frozen_time=MEM_W(0x64,int32_t(shot_model));
                for(unsigned frame=1;frame<=fps/10;++frame)world_to(second/2+second*frame/fps);
                world_to(6*second/10);
                assert(MEM_W(0x9C,int32_t(effect))==frozen_x && MEM_W(0x64,int32_t(shot_model))==frozen_time);
                MEM_B(0xC9,int32_t(shot_physics))=0;MEM_H(0xD4,int32_t(shot_physics))=1;
                const auto hit_frame=second/fps;
                world_to(6*second/10+hit_frame);
                constexpr auto child=effect+0xCC;
                const auto child_physics=uint32_t(MEM_W(0x98,int32_t(child)));
                const auto child_model=uint32_t(MEM_W(0x74,int32_t(child)));
                if(impact_spawns!=1 || terminal_deletes!=1 || MEM_W(0x94,int32_t(effect))!=0 ||
                   MEM_W(0x94,int32_t(child))!=1 || MEM_H(0x20,int32_t(child_physics))!=int(kind+1) ||
                   std::abs(rf(rdram,child_model+0x64)-3*(double(input_now)/second-impact_born_at)*60/divider)>1e-5) {
                    std::cerr<<"Hit/newborn mismatch kind="<<kind<<" fps="<<fps<<" hits="<<impact_spawns
                             <<" deletes="<<terminal_deletes<<" child_time="<<rf(rdram,child_model+0x64)<<'\n';return 1;
                }
                assert(!allocations.count(shot_physics)&&!allocations.count(shot_model));
                // This terminal observation now occurs after the elapsed
                // pose advance. Measure its committed end, preserving the
                // same strict birth-relative lifetime assertion below.
                terminal_model=child_model;terminal_deletes=0;terminal_end_commit=true;
                for(unsigned frame=2;frame<=fps/4+2;++frame)world_to(6*second/10+second*frame/fps);
                if(terminal_deletes!=1 || std::abs(terminal_deleted_at-impact_born_at-double(divider)/60)>1e-8 || impact_spawns!=1) {
                    std::cerr<<"Hit-effect lifetime mismatch kind="<<kind<<" fps="<<fps<<" deleted="<<terminal_deleted_at
                             <<" birth="<<impact_born_at<<'\n';return 1;
                }
                assert(!MEM_W(0x94,int32_t(child))&&!MEM_W(0x98,int32_t(child)));
                assert(!allocations.count(child_physics)&&!allocations.count(child_model));
                assert((resource_references==std::array<unsigned,5>{}));
                terminal_model=0;native_gameplay_preview_reset(rdram);preview_live=false;++flight_cases;
            }
            // Stationary growth/spin and moving age/fade are compared at every
            // source-tick endpoint against independent unpatched dispatches.
            // Continuous frames also run between those samples; their scalar
            // values and retirement must compose across arbitrary partitions.
            for(unsigned kind:{85u,87u,90u}) {
                terminal_end_commit=true;
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;
                MEM_H(0x14,int32_t(control))=0;MEM_W(0x94,int32_t(player))=1;
                std::memset(rdram+(effect&0xFFFFFF),0,20*0xCC);
                auto create=[&](uint8_t* buffer) {
                    auto* rdram=buffer;std::memset(rdram+(effect&0xFFFFFF),0,0xCC);
                    c.r4=0xF4;dora3_game_heap_alloc(rdram,&c);const auto physical=uint32_t(c.r2);
                    c.r4=int32_t(physical);dora3_s0_80042CAC(rdram,&c);
                    c.r4=int32_t(effect);c.r5=0;c.r6=int32_t(asset);dora3_s0_8002A100(rdram,&c);
                    const auto model=uint32_t(MEM_W(0x74,int32_t(effect)));
                    c.r4=int32_t(model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                    MEM_W(0x94,int32_t(effect))=1;MEM_W(0x98,int32_t(effect))=int32_t(physical);
                    MEM_H(0xA8,int32_t(effect))=-1;MEM_H(0x20,int32_t(physical))=int16_t(kind);
                    wf(rdram,physical+0x58,kind==85?0.5f:1.f);wf(rdram,physical,kind==90?2:0);
                    wf(rdram,physical+4,90);wf(rdram,effect+0x9C,500);wf(rdram,effect+0xA0,100);
                    for(unsigned off:{0x18u,0x1Cu,0x20u})wf(rdram,effect+off,0.05f);
                    return std::array<uint32_t,2>{physical,model};
                };
                struct GrowthState {bool active;float x,scale,angle,alpha;int age;};
                auto read=[&](uint8_t* buffer,uint32_t physical) {
                    auto* rdram=buffer;
                    return GrowthState{bool(MEM_W(0x94,int32_t(effect))),rf(rdram,effect+0x9C),rf(rdram,effect+0x18),
                                       rf(rdram,effect+0x14),rf(rdram,physical+0x58),MEM_H(0x36,int32_t(physical))};
                };
                auto legacy=memory;auto* old_ram=reinterpret_cast<uint8_t*>(legacy.data());
                const auto source_owner=create(old_ram);terminal_model=source_owner[1];terminal_deletes=0;
                std::vector<GrowthState> expected;
                for(unsigned tick=0;tick<150;++tick) {
                    input_now=(second*divider*tick+59)/60;
                    reference_dora3_s3_801D10D8(old_ram,&c);
                    expected.push_back(read(old_ram,source_owner[0]));
                    if(!expected.back().active)break;
                }
                assert(!expected.back().active && terminal_deletes==1);
                const auto source_deleted_at=terminal_deleted_at;
                assert(!allocations.count(source_owner[0])&&!allocations.count(source_owner[1]));
                input_now=0;const auto native_owner=create(rdram);
                terminal_model=native_owner[1];terminal_deletes=0;
                game_model_animation().hook(rdram,&c,0x80030850,0);dora3_s3_801D10D8(rdram,&c);
                preview_live=true;native_gameplay_preview_begin(rdram,0,divider);
                unsigned frame=0;
                for(unsigned tick=1;tick<expected.size();++tick) {
                    const auto stamp=(second*divider*tick+59)/60;
                    while(second*(frame+1)/fps<stamp) {
                        input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;
                        native_gameplay_preview_update(rdram,&c,input_now);
                    }
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;
                    native_gameplay_preview_update(rdram,&c,stamp);
                    const auto actual=read(rdram,native_owner[0]);const auto& value=expected[tick];
                    if(actual.active!=value.active || (actual.active &&
                       (std::abs(actual.x-value.x)>0.03 || std::abs(actual.scale-value.scale)>0.0001 ||
                        std::abs(std::remainder(actual.angle-value.angle,360.f))>0.01 ||
                        std::abs(actual.alpha-value.alpha)>0.0001 || actual.age!=value.age))) {
                        std::cerr<<"Growth/spin/fade partition mismatch kind="<<kind<<" divider="<<divider<<" fps="<<fps<<" tick="<<tick
                                 <<" live="<<actual.active<<'/'<<value.active<<" scale="<<actual.scale<<'/'<<value.scale
                                 <<" alpha="<<actual.alpha<<'/'<<value.alpha<<" age="<<actual.age<<'/'<<value.age<<'\n';return 1;
                    }
                }
                if(terminal_deletes!=1 || std::abs(terminal_deleted_at-source_deleted_at)>=2e-9) {
                    std::cerr<<"Growth retirement mismatch kind="<<kind<<" divider="<<divider<<" fps="<<fps
                             <<" count="<<terminal_deletes<<" actual="<<terminal_deleted_at<<" expected="<<source_deleted_at<<'\n';return 1;
                }
                assert(!allocations.count(native_owner[0])&&!allocations.count(native_owner[1]));
                terminal_model=0;native_gameplay_preview_reset(rdram);preview_live=false;++growth_cases;
            }
            terminal_end_commit=false;
            // Real shared gravity, fixed spatial probes, source countdown and
            // impact constructors. Keep NPC contact controlled as in flight
            // tests; the real geometry probe still executes in the flat room.
            for(unsigned kind:{84u,88u,89u}) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;
                MEM_H(0x14,int32_t(control))=0;MEM_W(0x94,int32_t(player))=1;
                std::memset(rdram+(effect&0xFFFFFF),0,20*0xCC);
                for(unsigned i=0;i<20;++i) {c.r4=int32_t(effect+i*0xCC);dora3_scene_object_init(rdram,&c);MEM_H(0xA8,int32_t(effect+i*0xCC))=-1;}
                MEM_W(0,int32_t(0x801AEC00+40))=int32_t(clip);
                c.r4=0xF4;dora3_game_heap_alloc(rdram,&c);const auto physical=uint32_t(c.r2);
                c.r4=int32_t(physical);dora3_s0_80042CAC(rdram,&c);
                c.r4=int32_t(effect);c.r5=0;c.r6=int32_t(asset);dora3_s0_8002A100(rdram,&c);
                const auto model=uint32_t(MEM_W(0x74,int32_t(effect)));
                c.r4=int32_t(model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                MEM_W(0x94,int32_t(effect))=1;MEM_W(0x98,int32_t(effect))=int32_t(physical);
                MEM_H(0x20,int32_t(physical))=int16_t(kind);MEM_H(0x34,int32_t(physical))=int16_t(kind==89?4:15);
                MEM_H(0x36,int32_t(physical))=1;MEM_H(0x38,int32_t(physical))=4;
                wf(rdram,physical,kind==89?0:4);wf(rdram,physical+4,90);
                wf(rdram,physical+0x14,4);wf(rdram,physical+0x1C,0.25f);
                wf(rdram,effect+0x9C,500);wf(rdram,effect+0xA0,100);
                terminal_model=0;impact_spawns=impact_commands=0;
                game_model_animation().hook(rdram,&c,0x80030850,0);
                auto legacy=memory;auto* old_ram=reinterpret_cast<uint8_t*>(legacy.data());
                reference_dora3_s3_801D10D8(old_ram,&c);const auto first_reference=legacy;
                auto read=[&](uint8_t* buffer) {
                    auto* rdram=buffer;
                    return std::array<double,5>{rf(rdram,effect+0x9C),rf(rdram,effect+0xA0),rf(rdram,physical+0x14),
                        double(MEM_H(0x34,int32_t(physical))),double(MEM_H(0x36,int32_t(physical)))};
                };
                std::vector<std::array<double,5>> expected;
                for(unsigned tick=1;tick<=8;++tick) {reference_dora3_s3_801D10D8(old_ram,&c);expected.push_back(read(old_ram));}
                // Re-establish the current RAM's Original observation after
                // evaluating the independent source reference.
                game_model_animation().hook(rdram,&c,0x80030850,0);
                dora3_s3_801D10D8(rdram,&c);assert(memory==first_reference);
                preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds end) {
                    while(second*(frame+1)/fps<end) {
                        input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;
                        native_gameplay_preview_update(rdram,&c,input_now);
                    }
                    input_now=end;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,end);
                };
                for(unsigned tick=1;tick<=8;++tick) {
                    to((second*divider*tick+59)/60);const auto actual=read(rdram);const auto& value=expected[tick-1];
                    for(unsigned k=0;k<5;++k)if(std::abs(actual[k]-value[k])>(k<3?0.03:0)) {
                        std::cerr<<"Ballistic/countdown mismatch kind="<<kind<<" fps="<<fps<<" divider="<<divider
                                 <<" tick="<<tick<<" field="<<k<<" actual="<<actual[k]<<" expected="<<value[k]<<'\n';return 1;
                    }
                }
                assert(impact_spawns==0);
                terminal_model=model;terminal_deletes=0;
                if(kind==89) {
                    to(second);assert(terminal_deletes==1 && !MEM_W(0x94,int32_t(effect)));
                } else {
                    const auto hit=second*divider*8/60;
                    MEM_H(0xD4,int32_t(physical))=1;to(hit+second/fps);
                    assert(terminal_deletes==1 && !MEM_W(0x94,int32_t(effect)));
                    // 88 starts the chain with index0, then four successors;
                    // 84 creates one growing impact85. No child receives time
                    // from before its constructor, and every resource retires.
                    const auto child=effect+0xCC;
                    assert(MEM_W(0x94,int32_t(child)));
                    const auto child_physics=uint32_t(MEM_W(0x98,int32_t(child)));
                    assert(MEM_H(0x20,int32_t(child_physics))==int(kind+1));
                    assert(std::abs(rf(rdram,uint32_t(MEM_W(0x74,int32_t(child)))+0x64)-3*(double(input_now)/second-impact_born_at)*60/divider)<1e-5);
                    for(unsigned f=1;f<=fps*4;++f)to(hit+second/fps+second*f/fps);
                    assert(impact_spawns==unsigned(kind==84?1:5));
                    assert(impact_commands==1);
                    for(unsigned i=0;i<20;++i)assert(!MEM_W(0x94,int32_t(effect+i*0xCC))&&!MEM_W(0x98,int32_t(effect+i*0xCC)));
                }
                assert(!allocations.count(physical)&&!allocations.count(model));
                assert((resource_references==std::array<unsigned,5>{}));
                terminal_model=0;native_gameplay_preview_reset(rdram);preview_live=false;++ballistic_cases;
                MEM_W(0,int32_t(0x801AEC00+40))=int32_t(effect_clip);
            }
            // Four real model lifetimes share one actor/physics lifetime. A
            // clip completion must bind the next phase without delaying that
            // phase's opacity or motion by an arbitrary variable frame.
            {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;
                MEM_H(0x14,int32_t(control))=0;MEM_W(0x94,int32_t(player))=1;
                std::memset(rdram+(effect&0xFFFFFF),0,20*0xCC);
                for(unsigned i=0;i<20;++i) {c.r4=int32_t(effect+i*0xCC);dora3_scene_object_init(rdram,&c);MEM_H(0xA8,int32_t(effect+i*0xCC))=-1;}
                MEM_W(0,int32_t(0x801AEC00+40))=int32_t(clip);
                auto create=[&](uint8_t* buffer) {
                    c.r29=int32_t(0x803F0000);c.f12.fl=500;c.f14.fl=100;c.r6=std::bit_cast<int32_t>(0.f);
                    dora3_s3_core_801CBA4C(buffer,&c);auto* rdram=buffer;
                    assert(c.r2==1 && MEM_H(0x20,MEM_W(0x98,int32_t(effect)))==83);
                    return uint32_t(MEM_W(0x98,int32_t(effect)));
                };
                auto read=[&](uint8_t* buffer,uint32_t physical) {
                    auto* rdram=buffer;
                    return std::array<double,6>{double(bool(MEM_W(0x94,int32_t(effect)))),rf(rdram,effect+0xA0),
                        rf(rdram,physical+0x14),double(MEM_H(0x34,int32_t(physical))),
                        double(MEM_H(0x22,int32_t(physical))),rf(rdram,physical+0x58)};
                };
                auto legacy=memory;auto* old_ram=reinterpret_cast<uint8_t*>(legacy.data());
                const auto old_physical=create(old_ram);std::vector<std::array<double,6>> expected;
                for(unsigned tick=0;tick<60;++tick) {
                    input_now=(second*divider*tick+59)/60;reference_dora3_s3_801D10D8(old_ram,&c);
                    expected.push_back(read(old_ram,old_physical));if(!expected.back()[0])break;
                }
                assert(!expected.back()[0]&&!allocations.count(old_physical)&&(resource_references==std::array<unsigned,5>{}));
                input_now=0;const auto physical=create(rdram);
                game_model_animation().hook(rdram,&c,0x80030850,0);
                auto first_reference=memory;auto reference_context=c;reference_context.f_odd=&reference_context.f0.u32h;
                reference_dora3_s3_801D10D8(reinterpret_cast<uint8_t*>(first_reference.data()),&reference_context);
                dora3_s3_801D10D8(rdram,&c);assert(memory==first_reference);
                preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                for(unsigned tick=1;tick<expected.size();++tick) {
                    const auto stamp=(second*divider*tick+59)/60;
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                    const auto actual=read(rdram,physical);const auto& value=expected[tick];
                    for(unsigned k=0;k<6;++k)if((k==0 || actual[0]) && std::abs(actual[k]-value[k])>((k==1||k==2||k==5)?0.03:0)) {
                        std::cerr<<"Multi-phase mismatch divider="<<divider<<" fps="<<fps<<" tick="<<tick
                                 <<" field="<<k<<" actual="<<actual[k]<<" expected="<<value[k]<<'\n';return 1;
                    }
                }
                assert(!allocations.count(physical)&&(resource_references==std::array<unsigned,5>{}));
                native_gameplay_preview_reset(rdram);preview_live=false;++phase_cases;
                MEM_W(0,int32_t(0x801AEC00+40))=int32_t(effect_clip);
            }
            {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;
                MEM_H(0x14,int32_t(control))=0;MEM_W(0x94,int32_t(player))=1;
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,100);wf(rdram,player+0x10,90);
                wf(rdram,physics,0);wf(rdram,physics+0x14,0);
                std::memset(rdram+(effect&0xFFFFFF),0,20*0xCC);
                for(unsigned i=0;i<20;++i) {c.r4=int32_t(effect+i*0xCC);dora3_scene_object_init(rdram,&c);MEM_H(0xA8,int32_t(effect+i*0xCC))=-1;}
                // A taller real probe room isolates the bounce's temporal
                // stage from incidental ceiling/floor impacts during replay.
                for(unsigned i=0;i<4;++i)wf(rdram,0x80280000+0x20+i*44+36,i<2?-1000:-2000);
                MEM_W(0,int32_t(0x801AEC00+40))=int32_t(clip);
                c.r29=int32_t(0x803F0000);dora3_s3_core_801CBE40(rdram,&c);assert(c.r2==1);
                const auto physical=uint32_t(MEM_W(0x98,int32_t(effect)));
                const auto model=uint32_t(MEM_W(0x74,int32_t(effect)));
                auto read=[&](uint8_t* buffer) {return std::array<double,3>{rf(buffer,effect+0x9C),rf(buffer,effect+0xA0),rf(buffer,physical+0x14)};};
                auto legacy=memory;auto* old_ram=reinterpret_cast<uint8_t*>(legacy.data());
                reference_dora3_s3_801D10D8(old_ram,&c);const auto first_reference=legacy;
                std::array<std::array<double,3>,2> expected{};
                for(auto& value:expected) {reference_dora3_s3_801D10D8(old_ram,&c);value=read(old_ram);assert(allocations.count(physical));}
                game_model_animation().hook(rdram,&c,0x80030850,0);dora3_s3_801D10D8(rdram,&c);assert(memory==first_reference);
                bounce_physical=physical;bounced_at=-1;impact_spawns=impact_commands=0;
                preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                for(unsigned tick=1;tick<=2;++tick) {
                    to((second*divider*tick+59)/60);const auto actual=read(rdram);
                    for(unsigned k=0;k<3;++k)assert(std::abs(actual[k]-expected[tick-1][k])<0.03);
                }
                assert(!impact_spawns && MEM_H(0x34,int32_t(physical))==0);
                // Controlled collision state exercises the actual instantaneous
                // bounce. Its next continuous callback observes the exact
                // committed50/4 velocity/gravity and interval endpoint.
                MEM_H(0x34,int32_t(physical))=1;
                auto source_bounce=memory;auto* bounce_ram=reinterpret_cast<uint8_t*>(source_bounce.data());
                reference_dora3_s3_801D10D8(bounce_ram,&c);
                assert(rf(bounce_ram,physical)==0 && rf(bounce_ram,physical+0x14)==50 && rf(bounce_ram,physical+0x1C)==4);
                const auto start=input_now;terminal_model=model;terminal_deletes=0;
                for(unsigned f=1;f<=fps && !impact_spawns;++f) {
                    to(start+second*f/fps);
                    if(bounced_at>=0 && !impact_spawns) {
                        const auto age=(double(input_now)/second-bounced_at)*60/divider;
                        // Sum Original's post-gravity velocities over the whole
                        // flight, including the current fractional interval.
                        const double velocity=50-4*age,displacement=50*age-2*age*(age+1);
                        if(std::abs(rf(rdram,physical+0x14)-velocity)>0.03 || std::abs(rf(rdram,effect+0xA0)-bounce_y-displacement)>0.05) {
                            std::cerr<<"Bounce trajectory mismatch fps="<<fps<<" divider="<<divider<<" age="<<age<<'\n';return 1;
                        }
                    }
                }
                assert(bounced_at>=0 && impact_spawns==1 && impact_commands==1 && terminal_deletes==1);
                const auto duration=impact_born_at-bounced_at;
                if(!(duration>=10.*divider/60-2e-7 && duration<=10.*divider/60+1./fps+2e-7)) {
                    std::cerr<<"Bounce phase duration mismatch fps="<<fps<<" divider="<<divider<<" bounced="<<bounced_at<<" converted="<<impact_born_at<<" duration="<<duration<<" expected="<<10.*divider/60<<'\n';return 1;
                }
                const auto child=effect+0xCC;const auto child_physics=uint32_t(MEM_W(0x98,int32_t(child)));
                assert(MEM_H(0x20,int32_t(child_physics))==83);
                assert(std::abs(rf(rdram,uint32_t(MEM_W(0x74,int32_t(child)))+0x64)-3*(double(input_now)/second-impact_born_at)*60/divider)<1e-5);
                for(unsigned f=1;f<=fps*3;++f)to(input_now+second/fps);
                for(unsigned i=0;i<20;++i)assert(!MEM_W(0x94,int32_t(effect+i*0xCC))&&!MEM_W(0x98,int32_t(effect+i*0xCC)));
                assert(!allocations.count(physical)&&!allocations.count(model)&&(resource_references==std::array<unsigned,5>{}));
                terminal_model=bounce_physical=0;native_gameplay_preview_reset(rdram);preview_live=false;++bounce_cases;
                room(rdram);MEM_W(0,int32_t(0x801AEC00+40))=int32_t(effect_clip);
            }


            {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=0;
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                MEM_W(0x8,int32_t(control))=-1;MEM_B(0x28,int32_t(control))=1;
                MEM_H(0x2A,int32_t(control))=1000;MEM_H(0xA,int32_t(params))=3;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,1000);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);
                // Each FPS partition starts with the same secondary clip.
                // Its previous projectile scenario may leave a fractional
                // phase; the flight initializer only rebinds the main model.
                c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(owners[1]);dora3_s0_8003B744(rdram,&c);
                c.r4=int32_t(owners[1]);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                wf(rdram,physics,2.75f);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);
                for(unsigned k=0;k<3;++k)wf(rdram,physics+0x10+4*k,0);
                for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                c=saved;c.f_odd=&c.f0.u32h;dora3_s3_801C768C(rdram,&c);
                flight_random_calls=0;c.r4=1;dora3_s3_801C8B28(rdram,&c);
                preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                auto input=[&](uint16_t held,int8_t x,int8_t y) {
                    MEM_H(0,int32_t(0x80171560))=int16_t(held);
                    MEM_B(2,int32_t(0x80171560))=x;MEM_B(3,int32_t(0x80171560))=y;
                    dora3_native_input_sample(rdram,&c);
                };
                to(73'000'000);input(0x8000,30,-30);
                to(173'000'000);input(0,-30,30);
                to(267'000'000);input(0,0,0);to(second);
                assert(native_gameplay_preview_eligible(rdram)&&MEM_W(0x94,int32_t(player))==57);
                std::array<double,17> actual{};
                for(unsigned k=0;k<3;++k)actual[k]=rf(rdram,player+0x9C+4*k);
                actual[3]=rf(rdram,physics);actual[4]=rf(rdram,physics+4);actual[5]=rf(rdram,physics+0xC);
                for(unsigned k=0;k<3;++k) {actual[6+k]=rf(rdram,player+0xC+4*k);actual[9+k]=rf(rdram,physics+0x10+4*k);}
                actual[12]=MEM_W(0x94,int32_t(player));actual[13]=MEM_H(0x2A,int32_t(control));
                actual[14]=rf(rdram,owners[0]+0x64);actual[15]=rf(rdram,owners[1]+0x64);actual[16]=flight_random_calls;
                if(fps==60)flight_edge_baseline=actual;
                else for(unsigned k=0;k<actual.size();++k)if(std::abs(actual[k]-flight_edge_baseline[k])>(k<3?0.035:(k==12||k==13||k==16)?0.:0.001)) {
                    std::cerr<<"Native flight A/stick partition mismatch fps="<<fps<<" divider="<<divider<<" field="<<k<<" actual="<<actual[k]<<" expected="<<flight_edge_baseline[k]<<'\n';return 1;
                }
                native_gameplay_preview_reset(rdram);preview_live=false;
                MEM_B(0x28,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;MEM_W(0x94,int32_t(player))=1;++flight_edge_cases;
            }
            // Lethal35/48 no longer relinquish before their second callback;
            // the connected death matrix below covers the actual hit producer,
            // complete reaction and scene/resource handoff.
            // Healthy flight damage48 shares the reviewed speed/projection,
            // keeps impact orientation and returns to57 after its real clip.
            for(unsigned duration:{3u,7u}) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=0;
                const auto saved_status=MEM_W(0,int32_t(0x8021D4A8));
                const auto saved_health_max=MEM_B(0x18,int32_t(params));
                constexpr uint32_t status=0x80259000;
                MEM_W(0,int32_t(0x8021D4A8))=int32_t(status);MEM_B(1,int32_t(status))=10;
                MEM_B(0x18,int32_t(params))=10;
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                MEM_W(0x8,int32_t(control))=-1;MEM_B(0x28,int32_t(control))=1;
                MEM_H(0x2A,int32_t(control))=1000;MEM_H(0xA,int32_t(params))=3;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=30;MEM_B(3,int32_t(0x80171560))=-30;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,1000);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,30);wf(rdram,player+0x10,90);wf(rdram,player+0x14,20);
                wf(rdram,physics,2.75f);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,30);
                for(unsigned k=0;k<3;++k)wf(rdram,physics+0x10+4*k,0);
                for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                wf(rdram,clip+0x200+4,3.f*duration);
                c=saved;c.f_odd=&c.f0.u32h;dora3_s3_801C768C(rdram,&c);
                MEM_H(0xD8,int32_t(physics))=10000;c.r4=1;dora3_s3_801C8B28(rdram,&c);
                assert(MEM_W(0x94,int32_t(player))==48&&native_gameplay_preview_eligible(rdram));
                auto legacy=memory;auto* reference=reinterpret_cast<uint8_t*>(legacy.data());
                const auto heap_before=allocations;const auto address_before=next_address;
                auto read=[&](uint8_t* buffer) {
                    auto* rdram=buffer;std::array<double,17> v{};
                    for(unsigned k=0;k<3;++k) {v[k]=rf(buffer,player+0x9C+4*k);v[6+k]=rf(buffer,player+0xC+4*k);v[9+k]=rf(buffer,physics+0x10+4*k);}
                    v[3]=rf(buffer,physics);v[4]=rf(buffer,physics+4);v[5]=rf(buffer,physics+0xC);
                    v[12]=MEM_W(0x94,int32_t(player));v[13]=MEM_H(0xD8,int32_t(physics));
                    v[14]=rf(buffer,owners[0]+0x64);v[15]=MEM_H(0x2A,int32_t(control));v[16]=MEM_BU(1,int32_t(status));return v;
                };
                std::vector<std::array<double,17>> expected;
                for(unsigned tick=1;tick<=12;++tick) {
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=1;dora3_s3_801C8B28(reference,&c);
                    // A ceil nanosecond timestamp may lie just after the
                    // exact Source endpoint. At a completed damage clip,
                    // Original's next command already belongs to that time;
                    // compare its real first callback without its future
                    // movement/animation or another timer decrement.
                    auto* rdram=reference;
                    if((uint64_t(tick)*divider*second)%60 && MEM_W(0x94,int32_t(player))==48 && (MEM_BU(0x7C,int32_t(owners[0]))&2)) {
                        auto endpoint=legacy;auto* command_ram=reinterpret_cast<uint8_t*>(endpoint.data());
                        const auto command_heap=allocations;const auto command_address=next_address;
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=0;dora3_s3_core_801C6D8C(command_ram,&c);
                        expected.push_back(read(command_ram));allocations=command_heap;next_address=command_address;
                    } else expected.push_back(read(reference));
                }
                allocations=heap_before;next_address=address_before;preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                for(unsigned tick=1;tick<=expected.size();++tick) {
                    to((second*divider*tick+59)/60);const auto actual=read(rdram);
                    assert(native_gameplay_preview_eligible(rdram));
                    for(unsigned k=0;k<actual.size();++k)if(std::abs(actual[k]-expected[tick-1][k])>(k<3?0.035:(k==12||k==13||k==15||k==16)?0.:0.001)) {
                        std::cerr<<"Healthy flight damage Source mismatch fps="<<fps<<" divider="<<divider<<" duration="<<duration<<" tick="<<tick<<" field="<<k<<" actual="<<actual[k]<<" expected="<<expected[tick-1][k]<<'\n';return 1;
                    }
                }
                assert(MEM_W(0x94,int32_t(player))==57);
                native_gameplay_preview_reset(rdram);preview_live=false;wf(rdram,clip+0x200+4,30);
                MEM_B(0x28,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;MEM_W(0x94,int32_t(player))=1;
                MEM_H(0xD8,int32_t(physics))=0;MEM_W(0,int32_t(0x8021D4A8))=saved_status;
                MEM_B(0x18,int32_t(params))=saved_health_max;++flight_damage_cases;
            }
            // Ground35/36: real one-shot kick, collision/landing contact,
            // model transition, recovery and new invulnerability timer.
            for(unsigned duration:{3u,7u}) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=0;
                const auto saved_status=MEM_W(0,int32_t(0x8021D4A8));
                const auto saved_health_max=MEM_B(0x18,int32_t(params));
                constexpr uint32_t status=0x80259000;
                MEM_W(0,int32_t(0x8021D4A8))=int32_t(status);MEM_B(1,int32_t(status))=10;
                MEM_B(0x18,int32_t(params))=10;
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                MEM_W(0x8,int32_t(control))=-1;MEM_B(0x28,int32_t(control))=0;
                MEM_H(0x2A,int32_t(control))=1000;MEM_H(0xA,int32_t(params))=3;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=30;MEM_B(3,int32_t(0x80171560))=-30;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,100);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);
                wf(rdram,physics,2.75f);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);
                for(unsigned k=0;k<3;++k)wf(rdram,physics+0x10+4*k,0);
                for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                wf(rdram,clip+0x200+4,3.f*duration);
                c=saved;c.f_odd=&c.f0.u32h;dora3_s3_core_801C5D30(rdram,&c);
                MEM_H(0xD8,int32_t(physics))=10000;MEM_H(0x34,int32_t(physics))=0;c.r4=1;dora3_s3_801C8B28(rdram,&c);
                assert(MEM_W(0x94,int32_t(player))==35&&native_gameplay_preview_eligible(rdram));
                auto legacy=memory;auto* reference=reinterpret_cast<uint8_t*>(legacy.data());
                const auto heap_before=allocations;const auto address_before=next_address;
                auto read=[&](uint8_t* buffer) {
                    auto* rdram=buffer;std::array<double,17> v{};
                    for(unsigned k=0;k<3;++k) {v[k]=rf(buffer,player+0x9C+4*k);v[6+k]=rf(buffer,player+0xC+4*k);v[9+k]=rf(buffer,physics+0x10+4*k);}
                    v[3]=rf(buffer,physics);v[4]=rf(buffer,physics+4);v[5]=rf(buffer,physics+0xC);
                    v[12]=MEM_W(0x94,int32_t(player));v[13]=MEM_H(0xD8,int32_t(physics));
                    v[14]=rf(buffer,owners[0]+0x64);v[15]=MEM_H(0x2A,int32_t(control));v[16]=MEM_BU(1,int32_t(status));return v;
                };
                std::vector<std::array<double,17>> expected;
                for(unsigned tick=1;tick<=40;++tick) {
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=1;dora3_s3_801C8B28(reference,&c);
                    // A ceil nanosecond timestamp may lie just after the
                    // exact Source endpoint. At a completed damage clip,
                    // Original's next command already belongs to that time;
                    // compare its real first callback without its future
                    // movement/animation or another timer decrement.
                    auto* rdram=reference;
                    if((uint64_t(tick)*divider*second)%60 && MEM_W(0x94,int32_t(player))==36 && (MEM_BU(0x7C,int32_t(owners[0]))&2)) {
                        auto endpoint=legacy;auto* command_ram=reinterpret_cast<uint8_t*>(endpoint.data());
                        const auto command_heap=allocations;const auto command_address=next_address;
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=0;dora3_s3_core_801C5EF0(command_ram,&c);
                        // The first recovered ground callback contains an
                        // instantaneous stick heading. Execute its real body
                        // only for the sub-nanosecond remainder; do not borrow
                        // a future full Source movement/model/timer update.
                        const uint64_t extra=60-(uint64_t(tick)*divider*second)%60;
                        {SimulationScope remainder({extra,uint64_t(divider)*second,tick,0});
                         c=saved;c.f_odd=&c.f0.u32h;dora3_s3_core_801C3464(command_ram,&c);
                         dora3_s3_801C0E3C(command_ram,&c);}

                        expected.push_back(read(command_ram));allocations=command_heap;next_address=command_address;
                    } else expected.push_back(read(reference));
                }
                allocations=heap_before;next_address=address_before;preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                for(unsigned tick=1;tick<=expected.size();++tick) {
                    to((second*divider*tick+59)/60);const auto actual=read(rdram);
                    assert(native_gameplay_preview_eligible(rdram));
                    for(unsigned k=0;k<actual.size();++k)if(std::abs(actual[k]-expected[tick-1][k])>(k<3?0.035:(k==12||k==13||k==15||k==16)?0.:0.001)) {
                        std::cerr<<"Ground damage Source mismatch fps="<<fps<<" divider="<<divider<<" duration="<<duration<<" tick="<<tick<<" field="<<k<<" actual="<<actual[k]<<" expected="<<expected[tick-1][k]<<'\n';return 1;
                    }
                }
                assert(MEM_W(0x94,int32_t(player))>=1&&MEM_W(0x94,int32_t(player))<=5);
                native_gameplay_preview_reset(rdram);preview_live=false;wf(rdram,clip+0x200+4,30);
                MEM_B(0x28,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;MEM_W(0x94,int32_t(player))=1;
                MEM_H(0xD8,int32_t(physics))=0;MEM_W(0,int32_t(0x8021D4A8))=saved_status;
                MEM_B(0x18,int32_t(params))=saved_health_max;++ground_damage_cases;
            }
            // Heavy37/38/39: one-shot70 kick, count44 Source cadence,
            // actual fall/landing, collapse/rise clips and recovery timer.
            for(unsigned duration:{3u,7u}) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=0;
                const auto saved_status=MEM_W(0,int32_t(0x8021D4A8));
                const auto saved_health_max=MEM_B(0x18,int32_t(params));
                constexpr uint32_t status=0x80259000;
                MEM_W(0,int32_t(0x8021D4A8))=int32_t(status);MEM_B(1,int32_t(status))=10;
                MEM_B(0x18,int32_t(params))=10;
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                MEM_W(0x8,int32_t(control))=-1;MEM_B(0x28,int32_t(control))=0;
                MEM_H(0x2A,int32_t(control))=1000;MEM_H(0xA,int32_t(params))=3;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=30;MEM_B(3,int32_t(0x80171560))=-30;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,100);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);
                wf(rdram,physics,2.75f);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);
                for(unsigned k=0;k<3;++k)wf(rdram,physics+0x10+4*k,0);
                for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                wf(rdram,clip+0x200+4,3.f*duration);
                c=saved;c.f_odd=&c.f0.u32h;dora3_s3_core_801C5FB4(rdram,&c);
                MEM_H(0xD8,int32_t(physics))=10000;MEM_H(0x34,int32_t(physics))=0;c.r4=1;dora3_s3_801C8B28(rdram,&c);
                assert(MEM_W(0x94,int32_t(player))==37&&native_gameplay_preview_eligible(rdram));
                auto legacy=memory;auto* reference=reinterpret_cast<uint8_t*>(legacy.data());
                const auto heap_before=allocations;const auto address_before=next_address;
                auto read=[&](uint8_t* buffer) {
                    auto* rdram=buffer;std::array<double,18> v{};
                    for(unsigned k=0;k<3;++k) {v[k]=rf(buffer,player+0x9C+4*k);v[6+k]=rf(buffer,player+0xC+4*k);v[9+k]=rf(buffer,physics+0x10+4*k);}
                    v[3]=rf(buffer,physics);v[4]=rf(buffer,physics+4);v[5]=rf(buffer,physics+0xC);
                    v[12]=MEM_W(0x94,int32_t(player));v[13]=MEM_H(0xD8,int32_t(physics));
                    v[14]=rf(buffer,owners[0]+0x64);v[15]=MEM_H(0x2A,int32_t(control));v[16]=MEM_BU(1,int32_t(status));v[17]=MEM_H(0x44,int32_t(physics));return v;
                };
                std::vector<std::array<double,18>> expected;
                for(unsigned tick=1;tick<=130;++tick) {
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=1;dora3_s3_801C8B28(reference,&c);
                    // A ceil nanosecond timestamp may lie just after the
                    // exact Source endpoint. At a completed damage clip,
                    // Original's next command already belongs to that time;
                    // compare its real first callback without its future
                    // movement/animation or another timer decrement.
                    auto* rdram=reference;
                    if((uint64_t(tick)*divider*second)%60) {
                        auto endpoint=legacy;auto* command_ram=reinterpret_cast<uint8_t*>(endpoint.data());
                        const auto command_heap=allocations;const auto command_address=next_address;
                        const auto state=MEM_W(0x94,int32_t(player));
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=0;
                        dora3_s3_801C0C7C(command_ram,&c); // Original probes before its first callback.
                        if(state==37)dora3_s3_core_801C6020(command_ram,&c);
                        else if(state==38)dora3_s3_core_801C61B4(command_ram,&c);
                        else if(state==39)dora3_s3_core_801C6270(command_ram,&c);
                        const uint64_t extra=60-(uint64_t(tick)*divider*second)%60;
                        {SimulationScope remainder({extra,uint64_t(divider)*second,tick,0});
                         c=saved;c.f_odd=&c.f0.u32h;
                         auto* rdram=command_ram;const auto next=MEM_W(0x94,int32_t(player));
                         if(next==37)dora3_s3_core_801C6084(command_ram,&c);
                         else if(next==38)dora3_s3_core_801C61E8(command_ram,&c);
                         else if(next==39)dora3_s3_core_801C62D4(command_ram,&c);
                         else if(next>=1 && next<=5)dora3_s3_core_801C3464(command_ram,&c);
                         dora3_s3_801C0E3C(command_ram,&c);dora3_s3_801C0F1C(command_ram,&c);}
                        expected.push_back(read(command_ram));allocations=command_heap;next_address=command_address;
                    } else expected.push_back(read(reference));
                }
                allocations=heap_before;next_address=address_before;preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                for(unsigned tick=1;tick<=expected.size();++tick) {
                    to((second*divider*tick+59)/60);const auto actual=read(rdram);
                    assert(native_gameplay_preview_eligible(rdram));
                    for(unsigned k=0;k<actual.size();++k)if(std::abs(actual[k]-expected[tick-1][k])>(k<3?0.035:(k==12||k==13||k==15||k==16||k==17)?0.:0.001)) {
                        std::cerr<<"Heavy actual values=";for(auto value:actual)std::cerr<<value<<',';
                        std::cerr<<" expected=";for(auto value:expected[tick-1])std::cerr<<value<<',';std::cerr<<'\n';
                        std::cerr<<"Heavy damage Source mismatch fps="<<fps<<" divider="<<divider<<" duration="<<duration<<" tick="<<tick<<" field="<<k<<" actual="<<actual[k]<<" expected="<<expected[tick-1][k]<<'\n';return 1;
                    }
                }
                assert(MEM_W(0x94,int32_t(player))>=1&&MEM_W(0x94,int32_t(player))<=5);
                native_gameplay_preview_reset(rdram);preview_live=false;wf(rdram,clip+0x200+4,30);
                MEM_B(0x28,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;MEM_W(0x94,int32_t(player))=1;
                MEM_H(0xD8,int32_t(physics))=0;MEM_W(0,int32_t(0x8021D4A8))=saved_status;
                MEM_B(0x18,int32_t(params))=saved_health_max;++heavy_damage_cases;
            }
            // Hazard40..45: delayed/ordinary70 kick and count44 Source cadence,
            // actual fall/landing, collapse/rise clips and recovery timer.
            for(unsigned reaction:{40u,43u})for(unsigned duration:{3u,7u}) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=0;
                const auto saved_status=MEM_W(0,int32_t(0x8021D4A8));
                const auto saved_health_max=MEM_B(0x18,int32_t(params));
                constexpr uint32_t status=0x80259000;
                MEM_W(0,int32_t(0x8021D4A8))=int32_t(status);MEM_B(1,int32_t(status))=10;
                MEM_B(0x18,int32_t(params))=10;
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                MEM_W(0x8,int32_t(control))=-1;MEM_B(0x28,int32_t(control))=0;
                MEM_H(0x2A,int32_t(control))=1000;MEM_H(0xA,int32_t(params))=3;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=30;MEM_B(3,int32_t(0x80171560))=-30;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,100);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);
                wf(rdram,physics,2.75f);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);
                for(unsigned k=0;k<3;++k)wf(rdram,physics+0x10+4*k,0);
                for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                wf(rdram,clip+0x200+4,3.f*duration);
                c=saved;c.f_odd=&c.f0.u32h;(reaction==40?dora3_s3_core_801C62F4:dora3_s3_core_801C66DC)(rdram,&c);
                MEM_H(0xD8,int32_t(physics))=10000;MEM_H(0x34,int32_t(physics))=0;c.r4=1;dora3_s3_801C8B28(rdram,&c);
                assert(MEM_W(0x94,int32_t(player))==int(reaction)&&native_gameplay_preview_eligible(rdram));
                auto legacy=memory;auto* reference=reinterpret_cast<uint8_t*>(legacy.data());
                const auto heap_before=allocations;const auto address_before=next_address;
                auto read=[&](uint8_t* buffer) {
                    auto* rdram=buffer;std::array<double,18> v{};
                    for(unsigned k=0;k<3;++k) {v[k]=rf(buffer,player+0x9C+4*k);v[6+k]=rf(buffer,player+0xC+4*k);v[9+k]=rf(buffer,physics+0x10+4*k);}
                    v[3]=rf(buffer,physics);v[4]=rf(buffer,physics+4);v[5]=rf(buffer,physics+0xC);
                    v[12]=MEM_W(0x94,int32_t(player));v[13]=MEM_H(0xD8,int32_t(physics));
                    v[14]=rf(buffer,owners[0]+0x64);v[15]=MEM_H(0x2A,int32_t(control));v[16]=MEM_BU(1,int32_t(status));v[17]=MEM_H(0x44,int32_t(physics));return v;
                };
                std::vector<std::array<double,18>> expected;
                for(unsigned tick=1;tick<=160;++tick) {
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=1;dora3_s3_801C8B28(reference,&c);
                    // A ceil nanosecond timestamp may lie just after the
                    // exact Source endpoint. At a completed damage clip,
                    // Original's next command already belongs to that time;
                    // compare its real first callback without its future
                    // movement/animation or another timer decrement.
                    auto* rdram=reference;
                    if((uint64_t(tick)*divider*second)%60) {
                        auto endpoint=legacy;auto* command_ram=reinterpret_cast<uint8_t*>(endpoint.data());
                        const auto command_heap=allocations;const auto command_address=next_address;
                        const auto state=MEM_W(0x94,int32_t(player));
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=0;
                        dora3_s3_801C0C7C(command_ram,&c); // Original probes before its first callback.
                        if(state==40)dora3_s3_core_801C637C(command_ram,&c);
                        else if(state==41)dora3_s3_core_801C659C(command_ram,&c);
                        else if(state==42)dora3_s3_core_801C6658(command_ram,&c);
                        else if(state==43)dora3_s3_core_801C6764(command_ram,&c);
                        else if(state==44)dora3_s3_core_801C68F8(command_ram,&c);
                        else if(state==45)dora3_s3_core_801C69B4(command_ram,&c);
                        const uint64_t extra=60-(uint64_t(tick)*divider*second)%60;
                        {SimulationScope remainder({extra,uint64_t(divider)*second,tick,0});
                         c=saved;c.f_odd=&c.f0.u32h;
                         auto* rdram=command_ram;const auto next=MEM_W(0x94,int32_t(player));
                         if(next==40)dora3_s3_core_801C63F8(command_ram,&c);
                         else if(next==41)dora3_s3_core_801C65D0(command_ram,&c);
                         else if(next==42)dora3_s3_core_801C66BC(command_ram,&c);
                         else if(next==43)dora3_s3_core_801C67C8(command_ram,&c);
                         else if(next==44)dora3_s3_core_801C692C(command_ram,&c);
                         else if(next==45)dora3_s3_core_801C6A18(command_ram,&c);
                         else if(next>=1 && next<=5)dora3_s3_core_801C3464(command_ram,&c);
                         dora3_s3_801C0E3C(command_ram,&c);dora3_s3_801C0F1C(command_ram,&c);}
                        expected.push_back(read(command_ram));allocations=command_heap;next_address=command_address;
                    } else expected.push_back(read(reference));
                }
                allocations=heap_before;next_address=address_before;preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                for(unsigned tick=1;tick<=expected.size();++tick) {
                    to((second*divider*tick+59)/60);const auto actual=read(rdram);
                    assert(native_gameplay_preview_eligible(rdram));
                    for(unsigned k=0;k<actual.size();++k)if(std::abs(actual[k]-expected[tick-1][k])>(k<3?0.035:(k==12||k==13||k==15||k==16||k==17)?0.:0.001)) {
                        std::cerr<<"Heavy actual values=";for(auto value:actual)std::cerr<<value<<',';
                        std::cerr<<" expected=";for(auto value:expected[tick-1])std::cerr<<value<<',';std::cerr<<'\n';
                        std::cerr<<"Hazard damage Source mismatch fps="<<fps<<" divider="<<divider<<" reaction="<<reaction<<" duration="<<duration<<" tick="<<tick<<" field="<<k<<" actual="<<actual[k]<<" expected="<<expected[tick-1][k]<<'\n';return 1;
                    }
                }
                assert(MEM_W(0x94,int32_t(player))>=1&&MEM_W(0x94,int32_t(player))<=5);
                native_gameplay_preview_reset(rdram);preview_live=false;wf(rdram,clip+0x200+4,30);
                MEM_B(0x28,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;MEM_W(0x94,int32_t(player))=1;
                MEM_H(0xD8,int32_t(physics))=0;MEM_W(0,int32_t(0x8021D4A8))=saved_status;
                MEM_B(0x18,int32_t(params))=saved_health_max;++hazard_damage_cases;
            }
            // Waiting46/47:20-count delay, one-shot15 kick and yaw reversal,
            // actual fall/landing, clip30 and recovery timer.
            for(unsigned duration:{3u,7u}) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=0;
                const auto saved_status=MEM_W(0,int32_t(0x8021D4A8));
                const auto saved_health_max=MEM_B(0x18,int32_t(params));
                constexpr uint32_t status=0x80259000;
                MEM_W(0,int32_t(0x8021D4A8))=int32_t(status);MEM_B(1,int32_t(status))=10;
                MEM_B(0x18,int32_t(params))=10;
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                MEM_W(0x8,int32_t(control))=-1;MEM_B(0x28,int32_t(control))=0;
                MEM_H(0x2A,int32_t(control))=1000;MEM_H(0xA,int32_t(params))=3;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=30;MEM_B(3,int32_t(0x80171560))=-30;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,100);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);
                wf(rdram,physics,2.75f);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);
                for(unsigned k=0;k<3;++k)wf(rdram,physics+0x10+4*k,0);
                for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                wf(rdram,clip+0x200+4,3.f*duration);
                c=saved;c.f_odd=&c.f0.u32h;dora3_s3_core_801C6A38(rdram,&c);
                MEM_H(0xD8,int32_t(physics))=10000;MEM_H(0x34,int32_t(physics))=0;c.r4=1;dora3_s3_801C8B28(rdram,&c);
                assert(MEM_W(0x94,int32_t(player))==46&&native_gameplay_preview_eligible(rdram));
                // Acquisition follows a complete Original world update.
                c.f12.fl=rf(rdram,player+0x9C);c.f14.fl=rf(rdram,player+0xA0);c.r6=MEM_W(0xA4,int32_t(player));
                dora3_s3_801BBDDC(rdram,&c);dora3_s3_801DBD58(rdram,&c);
                auto legacy=memory;auto* reference=reinterpret_cast<uint8_t*>(legacy.data());
                const auto heap_before=allocations;const auto address_before=next_address;
                auto read=[&](uint8_t* buffer) {
                    auto* rdram=buffer;std::array<double,18> v{};
                    for(unsigned k=0;k<3;++k) {v[k]=rf(buffer,player+0x9C+4*k);v[6+k]=rf(buffer,player+0xC+4*k);v[9+k]=rf(buffer,physics+0x10+4*k);}
                    v[3]=rf(buffer,physics);v[4]=rf(buffer,physics+4);v[5]=rf(buffer,physics+0xC);
                    v[12]=MEM_W(0x94,int32_t(player));v[13]=MEM_H(0xD8,int32_t(physics));
                    v[14]=rf(buffer,owners[0]+0x64);v[15]=MEM_H(0x2A,int32_t(control));v[16]=MEM_BU(1,int32_t(status));v[17]=MEM_H(0x44,int32_t(physics));return v;
                };
                std::vector<std::array<double,18>> expected;
                for(unsigned tick=1;tick<=90;++tick) {
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=1;dora3_s3_801C8B28(reference,&c);
                    // Independent Original world order: player, render
                    // origin, camera. Ground steering reads camera yaw.
                    c.f12.fl=rf(reference,player+0x9C);c.f14.fl=rf(reference,player+0xA0);
                    {auto* rdram=reference;c.r6=MEM_W(0xA4,int32_t(player));}
                    dora3_s3_801BBDDC(reference,&c);dora3_s3_801DBD58(reference,&c);

                    // A ceil nanosecond timestamp may lie just after the
                    // exact Source endpoint. At a completed damage clip,
                    // Original's next command already belongs to that time;
                    // compare its real first callback without its future
                    // movement/animation or another timer decrement.
                    auto* rdram=reference;
                    if((uint64_t(tick)*divider*second)%60) {
                        auto endpoint=legacy;auto* command_ram=reinterpret_cast<uint8_t*>(endpoint.data());
                        const auto command_heap=allocations;const auto command_address=next_address;
                        const auto state=MEM_W(0x94,int32_t(player));
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=0;
                        dora3_s3_801C0C7C(command_ram,&c); // Original probes before its first callback.
                        if(state==46)dora3_s3_core_801C6AA8(command_ram,&c);
                        else if(state==47)dora3_s3_core_801C6C68(command_ram,&c);
                        const uint64_t extra=60-(uint64_t(tick)*divider*second)%60;
                        {SimulationScope remainder({extra,uint64_t(divider)*second,tick,0});
                         c=saved;c.f_odd=&c.f0.u32h;
                         auto* rdram=command_ram;const auto next=MEM_W(0x94,int32_t(player));
                         if(next==46)dora3_s3_core_801C6B08(command_ram,&c);
                         else if(next==47)dora3_s3_core_801C6D00(command_ram,&c);
                         else if(next>=1 && next<=5)dora3_s3_core_801C3464(command_ram,&c);
                         dora3_s3_801C0E3C(command_ram,&c);dora3_s3_801C0F1C(command_ram,&c);}
                        expected.push_back(read(command_ram));allocations=command_heap;next_address=command_address;
                    } else expected.push_back(read(reference));
                }
                allocations=heap_before;next_address=address_before;preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                for(unsigned tick=1;tick<=expected.size();++tick) {
                    to((second*divider*tick+59)/60);const auto actual=read(rdram);
                    assert(native_gameplay_preview_eligible(rdram));
                    for(unsigned k=0;k<actual.size();++k)if(std::abs(actual[k]-expected[tick-1][k])>(k<3?0.035:(k==12||k==13||k==15||k==16||k==17)?0.:0.001)) {
                        std::cerr<<"Heavy actual values=";for(auto value:actual)std::cerr<<value<<',';
                        std::cerr<<" expected=";for(auto value:expected[tick-1])std::cerr<<value<<',';std::cerr<<'\n';
                        std::cerr<<"Waiting damage Source mismatch fps="<<fps<<" divider="<<divider<<" duration="<<duration<<" tick="<<tick<<" field="<<k<<" actual="<<actual[k]<<" expected="<<expected[tick-1][k]<<'\n';return 1;
                    }
                }
                assert(MEM_W(0x94,int32_t(player))>=1&&MEM_W(0x94,int32_t(player))<=5);
                native_gameplay_preview_reset(rdram);preview_live=false;wf(rdram,clip+0x200+4,30);
                MEM_B(0x28,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;MEM_W(0x94,int32_t(player))=1;
                MEM_H(0xD8,int32_t(physics))=0;MEM_W(0,int32_t(0x8021D4A8))=saved_status;
                MEM_B(0x18,int32_t(params))=saved_health_max;++waiting_damage_cases;
            }
            // Recovery31: grounded clip10, scalar stop and return to movement
            // through the real dispatcher with the Original camera order.
            for(unsigned duration:{3u,7u}) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=0;
                const auto saved_status=MEM_W(0,int32_t(0x8021D4A8));
                const auto saved_health_max=MEM_B(0x18,int32_t(params));
                constexpr uint32_t status=0x80259000;
                MEM_W(0,int32_t(0x8021D4A8))=int32_t(status);MEM_B(1,int32_t(status))=10;
                MEM_B(0x18,int32_t(params))=10;
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                MEM_W(0x8,int32_t(control))=-1;MEM_B(0x28,int32_t(control))=0;
                MEM_H(0x2A,int32_t(control))=1000;MEM_H(0xA,int32_t(params))=3;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=30;MEM_B(3,int32_t(0x80171560))=-30;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,0);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);
                wf(rdram,physics,2.75f);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);
                for(unsigned k=0;k<3;++k)wf(rdram,physics+0x10+4*k,0);
                for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                wf(rdram,clip+0x200+4,3.f*duration);
                c=saved;c.f_odd=&c.f0.u32h;dora3_s3_core_801C58C8(rdram,&c);
                MEM_H(0xD8,int32_t(physics))=0;MEM_H(0x34,int32_t(physics))=0;c.r4=1;dora3_s3_801C8B28(rdram,&c);
                assert(MEM_W(0x94,int32_t(player))==31&&native_gameplay_preview_eligible(rdram));
                // Acquisition follows a complete Original world update.
                c.f12.fl=rf(rdram,player+0x9C);c.f14.fl=rf(rdram,player+0xA0);c.r6=MEM_W(0xA4,int32_t(player));
                dora3_s3_801BBDDC(rdram,&c);dora3_s3_801DBD58(rdram,&c);
                auto legacy=memory;auto* reference=reinterpret_cast<uint8_t*>(legacy.data());
                const auto heap_before=allocations;const auto address_before=next_address;
                auto read=[&](uint8_t* buffer) {
                    auto* rdram=buffer;std::array<double,18> v{};
                    for(unsigned k=0;k<3;++k) {v[k]=rf(buffer,player+0x9C+4*k);v[6+k]=rf(buffer,player+0xC+4*k);v[9+k]=rf(buffer,physics+0x10+4*k);}
                    v[3]=rf(buffer,physics);v[4]=rf(buffer,physics+4);v[5]=rf(buffer,physics+0xC);
                    v[12]=MEM_W(0x94,int32_t(player));v[13]=MEM_H(0xD8,int32_t(physics));
                    v[14]=rf(buffer,owners[0]+0x64);v[15]=MEM_H(0x2A,int32_t(control));v[16]=MEM_BU(1,int32_t(status));v[17]=MEM_H(0x44,int32_t(physics));return v;
                };
                std::vector<std::array<double,18>> expected;
                for(unsigned tick=1;tick<=30;++tick) {
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=1;dora3_s3_801C8B28(reference,&c);
                    // Independent Original world order: player, render
                    // origin, camera. Ground steering reads camera yaw.
                    c.f12.fl=rf(reference,player+0x9C);c.f14.fl=rf(reference,player+0xA0);
                    {auto* rdram=reference;c.r6=MEM_W(0xA4,int32_t(player));}
                    dora3_s3_801BBDDC(reference,&c);dora3_s3_801DBD58(reference,&c);

                    // A ceil nanosecond timestamp may lie just after the
                    // exact Source endpoint. At a completed damage clip,
                    // Original's next command already belongs to that time;
                    // compare its real first callback without its future
                    // movement/animation or another timer decrement.
                    auto* rdram=reference;
                    if((uint64_t(tick)*divider*second)%60) {
                        auto endpoint=legacy;auto* command_ram=reinterpret_cast<uint8_t*>(endpoint.data());
                        const auto command_heap=allocations;const auto command_address=next_address;
                        const auto state=MEM_W(0x94,int32_t(player));
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=0;
                        dora3_s3_801C0C7C(command_ram,&c); // Original probes before its first callback.
                        if(state==31)dora3_s3_core_801C5910(command_ram,&c);
                        const uint64_t extra=60-(uint64_t(tick)*divider*second)%60;
                        {SimulationScope remainder({extra,uint64_t(divider)*second,tick,0});
                         c=saved;c.f_odd=&c.f0.u32h;
                         auto* rdram=command_ram;const auto next=MEM_W(0x94,int32_t(player));
                         if(next==31)dora3_s3_core_801C59AC(command_ram,&c);
                         else if(next>=1 && next<=5)dora3_s3_core_801C3464(command_ram,&c);
                         dora3_s3_801C0E3C(command_ram,&c);dora3_s3_801C0F1C(command_ram,&c);}
                        expected.push_back(read(command_ram));allocations=command_heap;next_address=command_address;
                    } else expected.push_back(read(reference));
                }
                allocations=heap_before;next_address=address_before;preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                for(unsigned tick=1;tick<=expected.size();++tick) {
                    to((second*divider*tick+59)/60);const auto actual=read(rdram);
                    assert(native_gameplay_preview_eligible(rdram));
                    for(unsigned k=0;k<actual.size();++k)if(std::abs(actual[k]-expected[tick-1][k])>(k<3?0.035:(k==12||k==13||k==15||k==16||k==17)?0.:0.001)) {
                        std::cerr<<"Heavy actual values=";for(auto value:actual)std::cerr<<value<<',';
                        std::cerr<<" expected=";for(auto value:expected[tick-1])std::cerr<<value<<',';std::cerr<<'\n';
                        std::cerr<<"Recovery31 Source mismatch fps="<<fps<<" divider="<<divider<<" duration="<<duration<<" tick="<<tick<<" field="<<k<<" actual="<<actual[k]<<" expected="<<expected[tick-1][k]<<'\n';return 1;
                    }
                }
                assert(MEM_W(0x94,int32_t(player))>=1&&MEM_W(0x94,int32_t(player))<=5);
                native_gameplay_preview_reset(rdram);preview_live=false;wf(rdram,clip+0x200+4,30);
                MEM_B(0x28,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;MEM_W(0x94,int32_t(player))=1;
                MEM_H(0xD8,int32_t(physics))=0;MEM_W(0,int32_t(0x8021D4A8))=saved_status;
                MEM_B(0x18,int32_t(params))=saved_health_max;++recovery31_cases;
            }
            // Hover49/63: actual constructor, Source21-count recovery,
            // instantaneous lift/descent, scalar/yaw control, B exit and landing.
            for(unsigned scenario=0;scenario<8;++scenario) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=0;
                const auto saved_status=MEM_W(0,int32_t(0x8021D4A8));
                const auto saved_health_max=MEM_B(0x18,int32_t(params));
                const auto saved_hover_max=MEM_H(0xC,int32_t(params));constexpr uint32_t status=0x80259000;
                MEM_W(0,int32_t(0x8021D4A8))=int32_t(status);MEM_B(1,int32_t(status))=10;
                MEM_B(0x18,int32_t(params))=10;MEM_H(0xC,int32_t(params))=12;
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;MEM_W(8,int32_t(control))=-1;
                MEM_B(0x2C,int32_t(control))=1;MEM_H(0x2A,int32_t(control))=1000;MEM_H(0xA,int32_t(params))=3;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=scenario&1?30:0;MEM_B(3,int32_t(0x80171560))=scenario&1?30:0;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,1000);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,scenario==3?359:90);wf(rdram,player+0x14,0);
                wf(rdram,physics,2.75f);wf(rdram,physics+4,scenario==3?359:90);wf(rdram,physics+8,10);wf(rdram,physics+0xC,90);
                for(unsigned k=0;k<3;++k)wf(rdram,physics+0x10+4*k,0);
                for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                wf(rdram,clip+0x200+4,60.f);c=saved;c.f_odd=&c.f0.u32h;
                (scenario<2||scenario>=4?dora3_s3_core_801C6E28:dora3_s3_801C8028)(rdram,&c);
                MEM_H(0xD8,int32_t(physics))=scenario==5?10001:scenario==2||scenario==3?0:10000;MEM_H(0x34,int32_t(physics))=0;
                c.r4=1;dora3_s3_801C8B28(rdram,&c);assert(native_gameplay_preview_eligible(rdram));
                c.f12.fl=rf(rdram,player+0x9C);c.f14.fl=rf(rdram,player+0xA0);c.r6=MEM_W(0xA4,int32_t(player));
                dora3_s3_801BBDDC(rdram,&c);dora3_s3_801DBD58(rdram,&c);
                auto legacy=memory;auto* reference=reinterpret_cast<uint8_t*>(legacy.data());
                const auto heap_before=allocations;const auto address_before=next_address;
                auto read=[&](uint8_t* buffer) {
                    auto* rdram=buffer;std::array<double,21> v{};
                    for(unsigned k=0;k<3;++k) {v[k]=rf(buffer,player+0x9C+4*k);v[6+k]=rf(buffer,player+0xC+4*k);v[9+k]=rf(buffer,physics+0x10+4*k);}
                    v[3]=rf(buffer,physics);v[4]=rf(buffer,physics+4);v[5]=rf(buffer,physics+0xC);
                    v[12]=MEM_W(0x94,int32_t(player));v[13]=MEM_H(0xD8,int32_t(physics));v[14]=rf(buffer,owners[0]+0x64);
                    v[15]=MEM_H(0x2A,int32_t(control));v[16]=MEM_BU(1,int32_t(status));v[17]=MEM_H(0x44,int32_t(physics));
                    v[18]=MEM_BU(0x2C,int32_t(control));v[19]=rf(buffer,physics+8);v[20]=rf(buffer,owners[1]+0x64);return v;
                };
                auto apply=[&](uint8_t* buffer,unsigned tick,bool native) {
                    auto* rdram=buffer;
                    const uint16_t buttons=tick>=31&&tick<37?0x8000:tick==76?0x4000:0;
                    MEM_H(0,int32_t(0x80171560))=int16_t(buttons);
                    c=saved;c.f_odd=&c.f0.u32h;
                    if(scenario==4&&tick==31) {dora3_s3_core_801C6E28(buffer,&c);MEM_H(0xD8,int32_t(physics))=10000;}
                    if(native)dora3_native_input_sample(buffer,&c);
                    else {dora3_controller_reset_accumulator(buffer,&c);dora3_controller_accumulate(buffer,&c);dora3_controller_latch(buffer,&c);}
                };
                std::vector<std::array<double,21>> expected;
                for(unsigned tick=1;tick<=120;++tick) {
                    apply(reference,tick,false);c=saved;c.f_odd=&c.f0.u32h;c.r4=1;dora3_s3_801C8B28(reference,&c);
                    c.f12.fl=rf(reference,player+0x9C);c.f14.fl=rf(reference,player+0xA0);{auto* rdram=reference;c.r6=MEM_W(0xA4,int32_t(player));}
                    dora3_s3_801BBDDC(reference,&c);dora3_s3_801DBD58(reference,&c);
                    // Every third Source endpoint has an exact integer-ns
                    // timestamp for20/30Hz. No future command belongs to it.
                    if(tick%3==0)expected.push_back(read(reference));
                }
                allocations=heap_before;next_address=address_before;preview_live=true;input_now=0;
                native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                for(unsigned tick=1;tick<=120;++tick) {
                    apply(rdram,tick,true);to((second*divider*tick+59)/60);assert(native_gameplay_preview_eligible(rdram));
                    if(tick%3)continue;
                    const auto actual=read(rdram);
                    for(unsigned k=0;k<actual.size();++k)if(std::abs(actual[k]-expected[tick/3-1][k])>(k<3?0.035:(k==12||k==13||k==15||k==16||k==17||k==18)?0.:0.001)) {
                        std::cerr<<"Hover Source mismatch camera="<<camera_mode<<" fps="<<fps<<" divider="<<divider<<" scenario="<<scenario<<" tick="<<tick<<" field="<<k<<" actual="<<actual[k]<<" expected="<<expected[tick/3-1][k]<<'\n';return 1;
                    }
                }
                assert(MEM_W(0x94,int32_t(player))>=1&&MEM_W(0x94,int32_t(player))<=5&&MEM_BU(0x2C,int32_t(control))==0);
                native_gameplay_preview_reset(rdram);preview_live=false;wf(rdram,clip+0x200+4,30);
                MEM_B(0x2C,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;MEM_H(0xC,int32_t(params))=saved_hover_max;
                MEM_W(0x94,int32_t(player))=1;MEM_H(0xD8,int32_t(physics))=0;
                MEM_W(0,int32_t(0x8021D4A8))=saved_status;MEM_B(0x18,int32_t(params))=saved_health_max;++hover_source_cases;
            }

            // Off-grid hover: actual health-loss and49 constructor at73ms,
            // re-hit/rebind/writes, A lift/descent, B exit and complete landing.
            // The external producer selecting49 is not yet reviewed.
            for(unsigned mode=0;mode<6;++mode) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=0;
                const auto saved_status=MEM_W(0,int32_t(0x8021D4A8));
                const auto saved_health_max=MEM_B(0x18,int32_t(params));
                const auto saved_hover_max=MEM_H(0xC,int32_t(params));constexpr uint32_t status=0x80259000;
                MEM_W(0,int32_t(0x8021D4A8))=int32_t(status);MEM_B(1,int32_t(status))=10;
                MEM_B(0x18,int32_t(params))=10;MEM_H(0xC,int32_t(params))=12;
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;MEM_W(8,int32_t(control))=-1;
                MEM_B(0x2C,int32_t(control))=1;MEM_H(0x2A,int32_t(control))=1000;MEM_H(0xA,int32_t(params))=3;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=mode&1?30:0;MEM_B(3,int32_t(0x80171560))=mode&1?30:0;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,1000);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,mode==3?359:90);wf(rdram,player+0x14,0);
                wf(rdram,physics,2.75f);wf(rdram,physics+4,mode==3?359:90);wf(rdram,physics+8,10);wf(rdram,physics+0xC,90);
                for(unsigned k=0;k<3;++k)wf(rdram,physics+0x10+4*k,0);
                for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                wf(rdram,clip+0x200+4,60.f);c=saved;c.f_odd=&c.f0.u32h;
                dora3_s3_801C8028(rdram,&c);
                MEM_H(0xD8,int32_t(physics))=0;MEM_H(0x34,int32_t(physics))=0;
                c.r4=1;dora3_s3_801C8B28(rdram,&c);assert(native_gameplay_preview_eligible(rdram));
                c.f12.fl=rf(rdram,player+0x9C);c.f14.fl=rf(rdram,player+0xA0);c.r6=MEM_W(0xA4,int32_t(player));
                dora3_s3_801BBDDC(rdram,&c);dora3_s3_801DBD58(rdram,&c);
                const auto initial=memory;const auto heap_initial=allocations;const auto address_initial=next_address;
                auto read=[&] {
                    std::array<double,23> v{};
                    for(unsigned k=0;k<3;++k) {v[k]=rf(rdram,player+0x9C+4*k);v[6+k]=rf(rdram,player+0xC+4*k);v[9+k]=rf(rdram,physics+0x10+4*k);}
                    v[3]=rf(rdram,physics);v[4]=rf(rdram,physics+4);v[5]=rf(rdram,physics+0xC);
                    v[12]=MEM_W(0x94,int32_t(player));v[13]=MEM_H(0xD8,int32_t(physics));v[14]=rf(rdram,owners[0]+0x64);
                    v[15]=MEM_H(0x2A,int32_t(control));v[16]=MEM_BU(1,int32_t(status));v[17]=MEM_H(0x44,int32_t(physics));
                    v[18]=MEM_BU(0x2C,int32_t(control));v[19]=rf(rdram,physics+8);v[20]=rf(rdram,owners[1]+0x64);
                    v[21]=hover_commands[0];v[22]=hover_commands[1];return v;
                };
                auto run=[&](unsigned render_fps) {
                    native_gameplay_preview_reset(rdram);preview_live=false;memory=initial;allocations=heap_initial;next_address=address_initial;
                    for(auto model:owners) {
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(model);dora3_s0_8003B744(rdram,&c);
                        c.r4=int32_t(model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                    }
                    hover_command_watch=true;hover_commands={};preview_live=true;input_now=0;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                    auto to=[&](Nanoseconds stamp) {
                        while(second*(frame+1)/render_fps<stamp) {input_now=second*++frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                        input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);assert(native_gameplay_preview_eligible(rdram));
                    };
                    auto buttons=[&](uint16_t value) {
                        MEM_H(0,int32_t(0x80171560))=int16_t(value);c=saved;c.f_odd=&c.f0.u32h;dora3_native_input_sample(rdram,&c);
                    };
                    auto damage=[&] {
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=-2;dora3_s3_801C0738(rdram,&c);
                        dora3_s3_core_801C6E28(rdram,&c);MEM_H(0xD8,int32_t(physics))=10000;
                    };
                    std::vector<std::array<double,23>> values;
                    to(73'000'000);damage();to(83'000'000);values.push_back(read());assert(MEM_W(0x94,int32_t(player))==49);
                    to(173'000'000);
                    if(mode==2)damage();
                    if(mode==3) {
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(owners[0]);dora3_s0_8003B744(rdram,&c);
                        c.r4=int32_t(owners[0]);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                    }
                    to(183'000'000);values.push_back(read());to(273'000'000);
                    if(mode==4) {wf(rdram,player+0xA0,700);wf(rdram,player+0x9C,520);wf(rdram,physics+4,123);wf(rdram,physics,7.5f);}
                    if(mode==5)wf(rdram,physics,7.5f);
                    to(283'000'000);values.push_back(read());
                    to(1'173'000'000);to(1'193'000'000);values.push_back(read());
                    to(1'273'000'000);buttons(0x8000);to(1'293'000'000);values.push_back(read());
                    to(1'573'000'000);buttons(0);to(1'593'000'000);values.push_back(read());
                    to(2'073'000'000);buttons(0x4000);to(2'093'000'000);values.push_back(read());assert(MEM_BU(0x2C,int32_t(control))==0);
                    to(2'173'000'000);buttons(0);to(2'193'000'000);values.push_back(read());
                    for(unsigned tick=67;tick<=150;++tick) {to((uint64_t(second)*divider*tick+59)/60);values.push_back(read());}
                    assert(MEM_W(0x94,int32_t(player))>=1&&MEM_W(0x94,int32_t(player))<=5);
                    assert(hover_commands[0]==(mode==2?2u:1u)&&hover_commands[1]==1);
                    hover_command_watch=false;return values;
                };
                const auto baseline=run(60),actual=run(fps);assert(baseline.size()==actual.size());
                for(unsigned n=0;n<actual.size();++n)for(unsigned k=0;k<actual[n].size();++k)
                    if(std::abs(actual[n][k]-baseline[n][k])>(k<3?0.04:(k==12||k==13||k==15||k==16||k==17||k==18||k>=21)?0.:0.002)) {
                        std::cerr<<"Hover Native partition mismatch camera="<<camera_mode<<" fps="<<fps<<" divider="<<divider<<" mode="<<mode<<" sample="<<n<<" field="<<k<<" actual="<<actual[n][k]<<" baseline="<<baseline[n][k]<<'\n';return 1;
                    }
                native_gameplay_preview_reset(rdram);preview_live=false;memory=initial;allocations=heap_initial;next_address=address_initial;
                wf(rdram,clip+0x200+4,30);MEM_B(0x2C,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;
                MEM_H(0xC,int32_t(params))=saved_hover_max;MEM_W(0x94,int32_t(player))=1;MEM_H(0xD8,int32_t(physics))=0;
                MEM_W(0,int32_t(0x8021D4A8))=saved_status;MEM_B(0x18,int32_t(params))=saved_health_max;++hover_native_cases;
            }

            // Special64/67: all three actual character event branches, complete
            // clip/cancel recovery and special surface exit through ten Source counts.
            for(unsigned scenario=0;scenario<9;++scenario) {
                const bool flying=scenario==6;const unsigned action=scenario<4||scenario>=7?64:67;
                const unsigned character=scenario==1?5:scenario==2||scenario==8?1:0;
                const unsigned ticks=action==64?130:40;
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=0;
                const auto saved_status=MEM_W(0,int32_t(0x8021D4A8));
                const auto saved_health_max=MEM_B(0x18,int32_t(params));
                constexpr uint32_t status=0x80259000;
                MEM_W(0,int32_t(0x8021D4A8))=int32_t(status);MEM_B(1,int32_t(status))=10;
                MEM_B(0x18,int32_t(params))=10;
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                MEM_W(0x8,int32_t(control))=-1;MEM_B(0x28,int32_t(control))=flying;
                MEM_H(0x2A,int32_t(control))=1000;MEM_H(0xA,int32_t(params))=3;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,flying?1000.f:0.f);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,0);wf(rdram,player+0x14,0);
                wf(rdram,physics,2.75f);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);
                for(unsigned k=0;k<3;++k)wf(rdram,physics+0x10+4*k,0);
                for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                wf(rdram,clip+0x200+4,360.f);
                // C02A8 loads model3 from the actor's mesh table B0,
                // distinct from animation table B4. Its actual ctor/bone
                // attachment/destructor execute; sprite5 stays instrumented.
                const auto saved_mesh_table=MEM_W(0xB0,int32_t(player));
                MEM_W(0xB0,int32_t(player))=int32_t(0x80257100);
                MEM_H(6,int32_t(0x80257100))=1;MEM_W(0,int32_t(0x801AEC14))=int32_t(asset);
                const auto saved_character=MEM_BU(0,int32_t(0x801B374A));MEM_B(0,int32_t(0x801B374A))=int8_t(character);
                if(scenario==5||scenario==6)for(unsigned n=0;n<2;++n)MEM_B(40,int32_t(0x80280000+0x20+n*44))=int8_t(0xF1);
                c=saved;c.f_odd=&c.f0.u32h;c.r4=0;
                if(scenario>=7) {
                    dora3_s3_801C3124(rdram,&c);MEM_H(0x14,int32_t(control))=298;
                } else (action==64?dora3_s3_core_801C8144:dora3_s3_801C858C)(rdram,&c);
                MEM_H(0xD8,int32_t(physics))=0;MEM_H(0x34,int32_t(physics))=0;c.r4=1;dora3_s3_801C8B28(rdram,&c);
                assert((MEM_W(0x94,int32_t(player))==int(action)||scenario==4||scenario>=7)&&native_gameplay_preview_eligible(rdram));
                special_command_watch=true;special_commands={};
                // Acquisition follows a complete Original world update.
                c.f12.fl=rf(rdram,player+0x9C);c.f14.fl=rf(rdram,player+0xA0);c.r6=MEM_W(0xA4,int32_t(player));
                dora3_s3_801BBDDC(rdram,&c);dora3_s3_801DBD58(rdram,&c);
                auto legacy=memory;auto* reference=reinterpret_cast<uint8_t*>(legacy.data());
                const auto heap_before=allocations;const auto address_before=next_address;const auto refs_before=resource_references;
                auto read=[&](uint8_t* buffer) {
                    auto* rdram=buffer;std::array<double,23> v{};
                    for(unsigned k=0;k<3;++k) {v[k]=rf(buffer,player+0x9C+4*k);v[6+k]=rf(buffer,player+0xC+4*k);v[9+k]=rf(buffer,physics+0x10+4*k);}
                    v[3]=rf(buffer,physics);v[4]=rf(buffer,physics+4);v[5]=rf(buffer,physics+0xC);
                    v[12]=MEM_W(0x94,int32_t(player));v[13]=MEM_H(0xD8,int32_t(physics));
                    v[14]=rf(buffer,owners[0]+0x64);v[15]=MEM_H(0x2A,int32_t(control));v[16]=MEM_BU(1,int32_t(status));v[17]=MEM_H(0x44,int32_t(physics));v[18]=MEM_H(0x42,int32_t(physics));
                    for(unsigned k=0;k<3;++k)v[19+k]=special_commands[k];
                    v[22]=scenario>=7?MEM_H(0x14,int32_t(control)):0;
                    return v;
                };
                // Press at the start of period12:400/600ms are exact for
                // both source dividers. A rounded period11 timestamp is an
                // off-grid input and must not be compared as Source-aligned.
                auto apply=[&](uint8_t* buffer,unsigned tick) {
                    auto* rdram=buffer;
                    const uint16_t buttons=scenario==3&&tick==13?0x8000:0;
                    MEM_H(0,int32_t(0x80171560))=int16_t(buttons);
                    if((scenario==5||scenario==6) && tick==7)for(unsigned n=0;n<2;++n)MEM_B(40,int32_t(0x80280000+0x20+n*44))=0;
                    c=saved;c.f_odd=&c.f0.u32h;
                    dora3_controller_accumulate(buffer,&c);dora3_controller_latch(buffer,&c);dora3_controller_reset_accumulator(buffer,&c);
                };
                std::vector<std::array<double,23>> expected;
                for(unsigned tick=1;tick<=ticks;++tick) {
                    apply(reference,tick);c=saved;c.f_odd=&c.f0.u32h;c.r4=1;dora3_s3_801C8B28(reference,&c);
                    // Independent Original world order: player, render
                    // origin, camera. Ground steering reads camera yaw.
                    c.f12.fl=rf(reference,player+0x9C);c.f14.fl=rf(reference,player+0xA0);
                    {auto* rdram=reference;c.r6=MEM_W(0xA4,int32_t(player));}
                    dora3_s3_801BBDDC(reference,&c);dora3_s3_801DBD58(reference,&c);

                    // A ceil nanosecond timestamp may lie just after the
                    // exact Source endpoint. At a completed damage clip,
                    // Original's next command already belongs to that time;
                    // compare its real first callback without its future
                    // movement/animation or another timer decrement.
                    auto* rdram=reference;
                    if((uint64_t(tick)*divider*second)%60) {
                        auto endpoint=legacy;auto* command_ram=reinterpret_cast<uint8_t*>(endpoint.data());
                        const auto command_heap=allocations;const auto command_address=next_address;
                        const auto command_counts=special_commands;const auto command_refs=resource_references;
                        const auto state=MEM_W(0x94,int32_t(player));
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=0;
                        if(scenario>=7)reference_dora3_player_idle_prelude(command_ram,&c);
                        dora3_s3_801C0C7C(command_ram,&c); // Original probes before its first callback.
                        if(state==64)dora3_s3_core_801C81B0(command_ram,&c);
                        else if(state==67)dora3_s3_core_801C85C8(command_ram,&c);
                        else if(scenario>=4 && state>=1 && state<=5)
                            dora3_s3_core_801C3300(command_ram,&c);
                        const uint64_t extra=60-(uint64_t(tick)*divider*second)%60;
                        {SimulationScope remainder({extra,uint64_t(divider)*second,tick,0});
                         c=saved;c.f_odd=&c.f0.u32h;
                         auto* rdram=command_ram;const auto next=MEM_W(0x94,int32_t(player));
                         // Equality in the independent Original command reads the
                         // previous full-step frame, without the Native event ledger.
                         if(next==64) {SimulationSuspension original_command;dora3_s3_core_801C8258(command_ram,&c);}
                         else if(next==67)dora3_s3_core_801C875C(command_ram,&c);
                         else if(next>=1 && next<=5) {
                             if(state==67) {SimulationSuspension original_command;dora3_s3_core_801C3464(command_ram,&c);}
                             else dora3_s3_core_801C3464(command_ram,&c);
                         } else if(next==57)dora3_s3_core_801C7810(command_ram,&c);
                         if(flying)dora3_s3_801C0D88(command_ram,&c);
                         else {dora3_s3_801C0E3C(command_ram,&c);dora3_s3_801C0F1C(command_ram,&c);}}
                        expected.push_back(read(command_ram));allocations=command_heap;next_address=command_address;
                        special_commands=command_counts;resource_references=command_refs;
                    } else expected.push_back(read(reference));
                }
                allocations=heap_before;next_address=address_before;resource_references=refs_before;special_command_watch=false;
                // Independent Original release must not unregister a model
                // that the restored Native start still contains. Recreate
                // that real auxiliary lifetime before acquiring simulation.
                if(scenario==2) {
                    c=saved;c.f_odd=&c.f0.u32h;dora3_s3_801C0398(rdram,&c);
                    dora3_s3_core_801C02A8(rdram,&c);
                }
                special_commands={};special_command_watch=true;preview_live=true;
                native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                for(unsigned tick=1;tick<=expected.size();++tick) {
                    apply(rdram,tick);to((second*divider*tick+59)/60);const auto actual=read(rdram);
                    assert(native_gameplay_preview_eligible(rdram));
                    for(unsigned k=0;k<actual.size();++k)if(std::abs(actual[k]-expected[tick-1][k])>(k<3?0.035:(k==12||k==13||k==15||k==16||k>=17)?0.:0.001)) {
                        std::cerr<<"Heavy actual values=";for(auto value:actual)std::cerr<<value<<',';
                        std::cerr<<" expected=";for(auto value:expected[tick-1])std::cerr<<value<<',';std::cerr<<'\n';
                        std::cerr<<"Special64/67 Source mismatch fps="<<fps<<" divider="<<divider<<" scenario="<<scenario<<" tick="<<tick<<" field="<<k<<" actual="<<actual[k]<<" expected="<<expected[tick-1][k]<<'\n';return 1;
                    }
                }
                assert((!flying&&MEM_W(0x94,int32_t(player))>=1&&MEM_W(0x94,int32_t(player))<=18) ||
                       (flying&&MEM_W(0x94,int32_t(player))==57));
                special_command_watch=false;MEM_B(0,int32_t(0x801B374A))=int8_t(saved_character);
                MEM_W(0xB0,int32_t(player))=saved_mesh_table;
                native_gameplay_preview_reset(rdram);preview_live=false;wf(rdram,clip+0x200+4,30);
                MEM_B(0x28,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;MEM_W(0x94,int32_t(player))=1;
                MEM_H(0xD8,int32_t(physics))=0;MEM_W(0,int32_t(0x8021D4A8))=saved_status;
                MEM_B(0x18,int32_t(params))=saved_health_max;++special_recovery_cases;
            }
            // Special65/66: actual stop/cancellation and Source-latched scripted turn,
            // including short-direction dead zone, wrap, disabled turn and new target.
            for(unsigned scenario=0;scenario<11;++scenario) {
                const bool flying=scenario>=9;const unsigned action=scenario<3?65:66;
                const unsigned turn=scenario<3?0:scenario>=9?scenario-9:scenario-3;
                const float initial_yaw=turn==1?359.f:turn==2?5.f:turn==3?123.f:turn==4?0.f:0.f;
                const float turn_target=turn==1?31.f:turn==2?335.f:turn==3?250.f:turn==4?30.f:180.f;
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=0;
                const auto saved_status=MEM_W(0,int32_t(0x8021D4A8));
                const auto saved_health_max=MEM_B(0x18,int32_t(params));
                constexpr uint32_t status=0x80259000;
                MEM_W(0,int32_t(0x8021D4A8))=int32_t(status);MEM_B(1,int32_t(status))=10;
                MEM_B(0x18,int32_t(params))=10;
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                MEM_W(0x8,int32_t(control))=-1;MEM_B(0x28,int32_t(control))=flying;
                MEM_H(0x2A,int32_t(control))=1000;MEM_H(0xA,int32_t(params))=3;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,flying?1000.f:0.f);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,initial_yaw);wf(rdram,player+0x14,0);
                wf(rdram,physics,2.75f);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);
                for(unsigned k=0;k<3;++k)wf(rdram,physics+0x10+4*k,0);
                for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                wf(rdram,clip+0x200+4,30.f);
                MEM_H(0xD4,int32_t(physics))=turn==3?-1:0;wf(rdram,physics+0xEC,turn_target);
                c=saved;c.f_odd=&c.f0.u32h;(action==65?dora3_s3_core_801C8390:dora3_s3_801C8478)(rdram,&c);
                MEM_H(0xD8,int32_t(physics))=0;MEM_H(0x34,int32_t(physics))=0;c.r4=1;dora3_s3_801C8B28(rdram,&c);
                assert(MEM_W(0x94,int32_t(player))==int(action)&&native_gameplay_preview_eligible(rdram));
                // Acquisition follows a complete Original world update.
                c.f12.fl=rf(rdram,player+0x9C);c.f14.fl=rf(rdram,player+0xA0);c.r6=MEM_W(0xA4,int32_t(player));
                dora3_s3_801BBDDC(rdram,&c);dora3_s3_801DBD58(rdram,&c);
                auto legacy=memory;auto* reference=reinterpret_cast<uint8_t*>(legacy.data());
                const auto heap_before=allocations;const auto address_before=next_address;
                auto read=[&](uint8_t* buffer) {
                    auto* rdram=buffer;std::array<double,18> v{};
                    for(unsigned k=0;k<3;++k) {v[k]=rf(buffer,player+0x9C+4*k);v[6+k]=rf(buffer,player+0xC+4*k);v[9+k]=rf(buffer,physics+0x10+4*k);}
                    v[3]=rf(buffer,physics);v[4]=rf(buffer,physics+4);v[5]=rf(buffer,physics+0xC);
                    v[12]=MEM_W(0x94,int32_t(player));v[13]=MEM_H(0xD8,int32_t(physics));
                    v[14]=rf(buffer,owners[0]+0x64);v[15]=MEM_H(0x2A,int32_t(control));v[16]=MEM_BU(1,int32_t(status));v[17]=MEM_H(0x44,int32_t(physics));return v;
                };
                // Press at the start of period12:400/600ms are exact for
                // both source dividers. A rounded period11 timestamp is an
                // off-grid input and must not be compared as Source-aligned.
                auto apply=[&](uint8_t* buffer,unsigned tick) {
                    auto* rdram=buffer;
                    const uint16_t buttons=scenario==1&&tick==13?0x8000:scenario==2&&tick==13?0x4000:0;
                    MEM_H(0,int32_t(0x80171560))=int16_t(buttons);
                    if(action==66 && turn==5 && tick==7)wf(buffer,physics+0xEC,20.f);
                    c=saved;c.f_odd=&c.f0.u32h;
                    dora3_controller_accumulate(buffer,&c);dora3_controller_latch(buffer,&c);dora3_controller_reset_accumulator(buffer,&c);
                };
                std::vector<std::array<double,18>> expected;
                for(unsigned tick=1;tick<=24;++tick) {
                    apply(reference,tick);c=saved;c.f_odd=&c.f0.u32h;c.r4=1;dora3_s3_801C8B28(reference,&c);
                    // Independent Original world order: player, render
                    // origin, camera. Ground steering reads camera yaw.
                    c.f12.fl=rf(reference,player+0x9C);c.f14.fl=rf(reference,player+0xA0);
                    {auto* rdram=reference;c.r6=MEM_W(0xA4,int32_t(player));}
                    dora3_s3_801BBDDC(reference,&c);dora3_s3_801DBD58(reference,&c);

                    // A ceil nanosecond timestamp may lie just after the
                    // exact Source endpoint. At a completed damage clip,
                    // Original's next command already belongs to that time;
                    // compare its real first callback without its future
                    // movement/animation or another timer decrement.
                    auto* rdram=reference;
                    if((uint64_t(tick)*divider*second)%60) {
                        auto endpoint=legacy;auto* command_ram=reinterpret_cast<uint8_t*>(endpoint.data());
                        const auto command_heap=allocations;const auto command_address=next_address;
                        const auto state=MEM_W(0x94,int32_t(player));
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=0;
                        dora3_s3_801C0C7C(command_ram,&c); // Original probes before its first callback.
                        if(state==65)dora3_s3_core_801C83D8(command_ram,&c);
                        const uint64_t extra=60-(uint64_t(tick)*divider*second)%60;
                        {SimulationScope remainder({extra,uint64_t(divider)*second,tick,0});
                         c=saved;c.f_odd=&c.f0.u32h;
                         auto* rdram=command_ram;const auto next=MEM_W(0x94,int32_t(player));
                         if(next==65)dora3_s3_core_801C844C(command_ram,&c);
                         else if(next>=1 && next<=5)dora3_s3_core_801C3464(command_ram,&c);
                         if(!flying) {dora3_s3_801C0E3C(command_ram,&c);dora3_s3_801C0F1C(command_ram,&c);}}
                        expected.push_back(read(command_ram));allocations=command_heap;next_address=command_address;
                    } else expected.push_back(read(reference));
                }
                allocations=heap_before;next_address=address_before;preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                for(unsigned tick=1;tick<=expected.size();++tick) {
                    apply(rdram,tick);to((second*divider*tick+59)/60);const auto actual=read(rdram);
                    assert(native_gameplay_preview_eligible(rdram));
                    for(unsigned k=0;k<actual.size();++k)if(std::abs(actual[k]-expected[tick-1][k])>(k<3?0.035:(k==12||k==13||k==15||k==16||k==17)?0.:0.001)) {
                        std::cerr<<"Heavy actual values=";for(auto value:actual)std::cerr<<value<<',';
                        std::cerr<<" expected=";for(auto value:expected[tick-1])std::cerr<<value<<',';std::cerr<<'\n';
                        std::cerr<<"Special65/66 Source mismatch fps="<<fps<<" divider="<<divider<<" scenario="<<scenario<<" tick="<<tick<<" field="<<k<<" actual="<<actual[k]<<" expected="<<expected[tick-1][k]<<'\n';return 1;
                    }
                }
                assert(MEM_W(0x94,int32_t(player))==int(action) ||
                       (action==65&&scenario>0&&MEM_W(0x94,int32_t(player))>=1&&MEM_W(0x94,int32_t(player))<=18));
                native_gameplay_preview_reset(rdram);preview_live=false;wf(rdram,clip+0x200+4,30);
                MEM_B(0x28,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;MEM_W(0x94,int32_t(player))=1;
                MEM_H(0xD8,int32_t(physics))=0;MEM_W(0,int32_t(0x8021D4A8))=saved_status;
                MEM_B(0x18,int32_t(params))=saved_health_max;++special_player_cases;
            }
            // Off-grid special65/66 commands: real A/B at73ms, target/D4,
            // angle/speed writes and a real model rebind. Compare the complete
            // command recovery with60FPS, without backdating to a Source tick.
            for(unsigned mode=0;mode<6;++mode) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=0;
                const auto saved_status=MEM_W(0,int32_t(0x8021D4A8));
                const auto saved_health_max=MEM_B(0x18,int32_t(params));constexpr uint32_t status=0x80259000;
                MEM_W(0,int32_t(0x8021D4A8))=int32_t(status);MEM_B(1,int32_t(status))=10;MEM_B(0x18,int32_t(params))=10;
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;MEM_W(8,int32_t(control))=-1;
                const bool flying=mode==4;MEM_B(0x28,int32_t(control))=flying;MEM_H(0x2A,int32_t(control))=1000;
                MEM_H(0xA,int32_t(params))=3;wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,flying?1000.f:0.f);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,0);wf(rdram,player+0x14,0);
                wf(rdram,physics,2.75f);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);
                for(unsigned k=0;k<3;++k)wf(rdram,physics+0x10+4*k,0);
                MEM_H(0xD8,int32_t(physics))=0;MEM_H(0xD4,int32_t(physics))=0;wf(rdram,physics+0xEC,180);
                for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                wf(rdram,clip+0x200+4,30);
                c=saved;c.f_odd=&c.f0.u32h;(mode<2?dora3_s3_core_801C8390:dora3_s3_801C8478)(rdram,&c);
                c.r4=1;dora3_s3_801C8B28(rdram,&c);
                c.f12.fl=rf(rdram,player+0x9C);c.f14.fl=rf(rdram,player+0xA0);c.r6=MEM_W(0xA4,int32_t(player));
                dora3_s3_801BBDDC(rdram,&c);dora3_s3_801DBD58(rdram,&c);
                const auto initial=memory;const auto heap_initial=allocations;const auto address_initial=next_address;
                auto read=[&] {
                    std::array<double,18> v{};
                    for(unsigned k=0;k<3;++k) {v[k]=rf(rdram,player+0x9C+4*k);v[6+k]=rf(rdram,player+0xC+4*k);v[9+k]=rf(rdram,physics+0x10+4*k);}
                    v[3]=rf(rdram,physics);v[4]=rf(rdram,physics+4);v[5]=rf(rdram,physics+0xC);
                    v[12]=MEM_W(0x94,int32_t(player));v[13]=MEM_H(0xD8,int32_t(physics));v[14]=rf(rdram,owners[0]+0x64);
                    v[15]=MEM_H(0x2A,int32_t(control));v[16]=MEM_BU(1,int32_t(status));v[17]=MEM_H(0x44,int32_t(physics));return v;
                };
                auto run=[&](unsigned render_fps) {
                    native_gameplay_preview_reset(rdram);preview_live=false;memory=initial;allocations=heap_initial;next_address=address_initial;
                    // Real rebind resets both owned animation runs, including
                    // a same-address model restored for the second schedule.
                    for(auto model:owners) {
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(model);dora3_s0_8003B744(rdram,&c);
                        c.r4=int32_t(model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                    }
                    preview_live=true;input_now=0;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                    auto to=[&](Nanoseconds stamp) {
                        while(second*(frame+1)/render_fps<stamp) {input_now=second*++frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                        input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                        assert(native_gameplay_preview_eligible(rdram));
                    };
                    auto buttons=[&](uint16_t value) {
                        MEM_H(0,int32_t(0x80171560))=int16_t(value);c=saved;c.f_odd=&c.f0.u32h;
                        dora3_controller_accumulate(rdram,&c);dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                    };
                    std::vector<std::array<double,18>> values;
                    to(73'000'000);
                    if(mode<2)buttons(mode==0?0x8000:0x4000);
                    else if(mode==3)MEM_H(0xD4,int32_t(physics))=-1;
                    else if(mode==5) {c.r4=int32_t(owners[0]);dora3_s0_8003B744(rdram,&c);c.r4=int32_t(owners[0]);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);}
                    else wf(rdram,physics+0xEC,20);
                    to(83'000'000);values.push_back(read());
                    if(mode<2)assert(MEM_W(0x94,int32_t(player))>=1&&MEM_W(0x94,int32_t(player))<=18);
                    else assert(MEM_W(0x94,int32_t(player))==66);
                    to(173'000'000);
                    if(mode<2)buttons(0);
                    else if(mode==3) {MEM_H(0xD4,int32_t(physics))=0;wf(rdram,physics+0xEC,20);}
                    else wf(rdram,player+0x10,123);
                    to(183'000'000);values.push_back(read());
                    to(273'000'000);if(mode>=2)wf(rdram,physics,7.5f);
                    to(283'000'000);values.push_back(read());
                    for(unsigned tick=10;tick<=40;++tick) {to((uint64_t(second)*divider*tick+59)/60);values.push_back(read());}
                    return values;
                };
                const auto baseline=run(60),actual=run(fps);assert(baseline.size()==actual.size());
                for(unsigned n=0;n<actual.size();++n)for(unsigned k=0;k<actual[n].size();++k)
                    if(std::abs(actual[n][k]-baseline[n][k])>(k<3?0.04:(k==12||k==13||k==15||k==16||k==17)?0.:0.002)) {
                        std::cerr<<"Native special partition mismatch camera="<<camera_mode<<" fps="<<fps<<" divider="<<divider<<" mode="<<mode<<" sample="<<n<<" field="<<k<<" actual="<<actual[n][k]<<" baseline="<<baseline[n][k]<<'\n';return 1;
                    }
                native_gameplay_preview_reset(rdram);preview_live=false;memory=initial;allocations=heap_initial;next_address=address_initial;
                MEM_B(0x28,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;MEM_W(0x94,int32_t(player))=1;
                MEM_H(0xD8,int32_t(physics))=0;MEM_W(0,int32_t(0x8021D4A8))=saved_status;MEM_B(0x18,int32_t(params))=saved_health_max;
                ++native_special_cases;
            }
            // Actual constructor commands at73ms, real idle entry from298,
            // character-specific complete model3 lifetimes and surface exit.
            for(unsigned mode=0;mode<8;++mode) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=0;
                const auto saved_status=MEM_W(0,int32_t(0x8021D4A8));
                const auto saved_health_max=MEM_B(0x18,int32_t(params));constexpr uint32_t status=0x80259000;
                MEM_W(0,int32_t(0x8021D4A8))=int32_t(status);MEM_B(1,int32_t(status))=10;MEM_B(0x18,int32_t(params))=10;
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;MEM_W(8,int32_t(control))=-1;
                const bool flying=mode==6;MEM_B(0x28,int32_t(control))=flying;MEM_H(0x2A,int32_t(control))=1000;
                MEM_H(0xA,int32_t(params))=3;wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,flying?1000.f:0.f);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,0);wf(rdram,player+0x14,0);
                wf(rdram,physics,2.75f);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);
                for(unsigned k=0;k<3;++k)wf(rdram,physics+0x10+4*k,0);
                MEM_H(0xD8,int32_t(physics))=0;MEM_H(0xD4,int32_t(physics))=0;wf(rdram,physics+0xEC,180);
                for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                wf(rdram,clip+0x200+4,360);
                const auto saved_character=MEM_BU(0,int32_t(0x801B374A));
                const unsigned character=mode==2?5:mode==3?1:0;
                MEM_B(0,int32_t(0x801B374A))=int8_t(character);
                const auto saved_mesh_table=MEM_W(0xB0,int32_t(player));
                MEM_W(0xB0,int32_t(player))=int32_t(0x80257100);
                MEM_H(6,int32_t(0x80257100))=1;MEM_W(0,int32_t(0x801AEC14))=int32_t(asset);
                c=saved;c.f_odd=&c.f0.u32h;(flying?dora3_s3_801C768C:dora3_s3_801C3124)(rdram,&c);
                if(mode==0)MEM_H(0x14,int32_t(control))=298;
                c.r4=1;dora3_s3_801C8B28(rdram,&c);
                c.f12.fl=rf(rdram,player+0x9C);c.f14.fl=rf(rdram,player+0xA0);c.r6=MEM_W(0xA4,int32_t(player));
                dora3_s3_801BBDDC(rdram,&c);dora3_s3_801DBD58(rdram,&c);
                const auto initial=memory;const auto heap_initial=allocations;const auto address_initial=next_address;
                auto read=[&] {
                    std::array<double,23> v{};
                    for(unsigned k=0;k<3;++k) {v[k]=rf(rdram,player+0x9C+4*k);v[6+k]=rf(rdram,player+0xC+4*k);v[9+k]=rf(rdram,physics+0x10+4*k);}
                    v[3]=rf(rdram,physics);v[4]=rf(rdram,physics+4);v[5]=rf(rdram,physics+0xC);
                    v[12]=MEM_W(0x94,int32_t(player));v[13]=MEM_H(0xD8,int32_t(physics));v[14]=rf(rdram,owners[0]+0x64);
                    v[15]=MEM_H(0x2A,int32_t(control));v[16]=MEM_BU(1,int32_t(status));v[17]=MEM_H(0x44,int32_t(physics));v[18]=MEM_H(0x42,int32_t(physics));
                    for(unsigned k=0;k<3;++k)v[19+k]=special_commands[k];
                    v[22]=MEM_H(0x14,int32_t(control));return v;
                };
                auto run=[&](unsigned render_fps) {
                    native_gameplay_preview_reset(rdram);preview_live=false;memory=initial;allocations=heap_initial;next_address=address_initial;
                    // Real rebind resets both owned animation runs, including
                    // a same-address model restored for the second schedule.
                    for(auto model:owners) {
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(model);dora3_s0_8003B744(rdram,&c);
                        c.r4=int32_t(model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                    }
                    special_commands={};special_command_watch=true;preview_live=true;input_now=0;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                    auto to=[&](Nanoseconds stamp) {
                        while(second*(frame+1)/render_fps<stamp) {input_now=second*++frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                        input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                        assert(native_gameplay_preview_eligible(rdram));
                    };
                    auto buttons=[&](uint16_t value) {
                        MEM_H(0,int32_t(0x80171560))=int16_t(value);c=saved;c.f_odd=&c.f0.u32h;
                        dora3_controller_accumulate(rdram,&c);dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                    };
                    std::vector<std::array<double,23>> values;
                    to(73'000'000);
                    if(mode>0) {
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=mode==7?1:0;
                        if(mode<=4)dora3_s3_core_801C8144(rdram,&c);
                        else {
                            if(mode==5||mode==6)for(unsigned n=0;n<2;++n)MEM_B(40,int32_t(0x80280000+0x20+n*44))=int8_t(0xF1);
                            dora3_s3_801C858C(rdram,&c);
                        }
                    }
                    to(83'000'000);values.push_back(read());
                    if(mode>=5&&mode<=6)assert(MEM_W(0x94,int32_t(player))==67);
                    else if(mode>0&&mode<=4)assert(MEM_W(0x94,int32_t(player))==64);
                    to(173'000'000);
                    if(mode==4)buttons(0x8000);
                    if(mode>=5) {
                        for(unsigned n=0;n<2;++n)MEM_B(40,int32_t(0x80280000+0x20+n*44))=0;
                        MEM_W(0,int32_t(0x8021D4B8))=0;
                    }
                    to(183'000'000);values.push_back(read());
                    to(273'000'000);if(mode==4)buttons(0);
                    to(283'000'000);values.push_back(read());
                    for(unsigned tick=10;tick<=130;++tick) {to((uint64_t(second)*divider*tick+59)/60);values.push_back(read());}
                    if(mode<=3)assert(special_commands[0]==1);
                    if(mode==4)assert(special_commands[0]==0&&special_commands[1]==1);
                    if(mode>=5)assert(special_commands[2]==1);
                    assert((!flying&&MEM_W(0x94,int32_t(player))>=1&&MEM_W(0x94,int32_t(player))<=18) ||
                           (flying&&MEM_W(0x94,int32_t(player))==57));
                    special_command_watch=false;
                    return values;
                };
                const auto baseline=run(60),actual=run(fps);assert(baseline.size()==actual.size());
                for(unsigned n=0;n<actual.size();++n)for(unsigned k=0;k<actual[n].size();++k)
                    if(std::abs(actual[n][k]-baseline[n][k])>(k<3?0.04:(k==12||k==13||k==15||k==16||k>=17)?0.:0.002)) {
                        std::cerr<<"Native special recovery partition mismatch camera="<<camera_mode<<" fps="<<fps<<" divider="<<divider<<" mode="<<mode<<" sample="<<n<<" field="<<k<<" actual="<<actual[n][k]<<" baseline="<<baseline[n][k]<<'\n';return 1;
                    }
                native_gameplay_preview_reset(rdram);preview_live=false;memory=initial;allocations=heap_initial;next_address=address_initial;
                MEM_B(0x28,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;MEM_W(0x94,int32_t(player))=1;
                MEM_H(0xD8,int32_t(physics))=0;MEM_W(0,int32_t(0x8021D4A8))=saved_status;MEM_B(0x18,int32_t(params))=saved_health_max;
                MEM_B(0,int32_t(0x801B374A))=int8_t(saved_character);
                MEM_W(0xB0,int32_t(player))=saved_mesh_table;wf(rdram,clip+0x200+4,30);
                ++native_special_recovery_cases;
            }
            // Air32/33/34: one-shot30 kick, continuous drive, landing,
            // collapse/rise clips and recovery cooldown.
            for(unsigned duration:{3u,7u}) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=0;
                const auto saved_status=MEM_W(0,int32_t(0x8021D4A8));
                const auto saved_health_max=MEM_B(0x18,int32_t(params));
                constexpr uint32_t status=0x80259000;
                MEM_W(0,int32_t(0x8021D4A8))=int32_t(status);MEM_B(1,int32_t(status))=10;
                MEM_B(0x18,int32_t(params))=10;
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                MEM_W(0x8,int32_t(control))=-1;MEM_B(0x28,int32_t(control))=0;
                MEM_H(0x2A,int32_t(control))=1000;MEM_H(0xA,int32_t(params))=3;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=30;MEM_B(3,int32_t(0x80171560))=-30;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,100);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);
                wf(rdram,physics,2.75f);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);
                for(unsigned k=0;k<3;++k)wf(rdram,physics+0x10+4*k,0);
                for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                wf(rdram,clip+0x200+4,3.f*duration);
                c=saved;c.f_odd=&c.f0.u32h;dora3_s3_core_801C59D8(rdram,&c);
                MEM_H(0xD8,int32_t(physics))=10000;MEM_H(0x34,int32_t(physics))=0;c.r4=1;dora3_s3_801C8B28(rdram,&c);
                assert(MEM_W(0x94,int32_t(player))==32&&native_gameplay_preview_eligible(rdram));
                auto legacy=memory;auto* reference=reinterpret_cast<uint8_t*>(legacy.data());
                const auto heap_before=allocations;const auto address_before=next_address;
                auto read=[&](uint8_t* buffer) {
                    auto* rdram=buffer;std::array<double,18> v{};
                    for(unsigned k=0;k<3;++k) {v[k]=rf(buffer,player+0x9C+4*k);v[6+k]=rf(buffer,player+0xC+4*k);v[9+k]=rf(buffer,physics+0x10+4*k);}
                    v[3]=rf(buffer,physics);v[4]=rf(buffer,physics+4);v[5]=rf(buffer,physics+0xC);
                    v[12]=MEM_W(0x94,int32_t(player));v[13]=MEM_H(0xD8,int32_t(physics));
                    v[14]=rf(buffer,owners[0]+0x64);v[15]=MEM_H(0x2A,int32_t(control));v[16]=MEM_BU(1,int32_t(status));v[17]=MEM_H(0x44,int32_t(physics));return v;
                };
                std::vector<std::array<double,18>> expected;
                for(unsigned tick=1;tick<=80;++tick) {
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=1;dora3_s3_801C8B28(reference,&c);
                    // A ceil nanosecond timestamp may lie just after the
                    // exact Source endpoint. At a completed damage clip,
                    // Original's next command already belongs to that time;
                    // compare its real first callback without its future
                    // movement/animation or another timer decrement.
                    auto* rdram=reference;
                    if((uint64_t(tick)*divider*second)%60) {
                        auto endpoint=legacy;auto* command_ram=reinterpret_cast<uint8_t*>(endpoint.data());
                        const auto command_heap=allocations;const auto command_address=next_address;
                        const auto state=MEM_W(0x94,int32_t(player));
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=0;
                        dora3_s3_801C0C7C(command_ram,&c); // Original probes before its first callback.
                        if(state==32)dora3_s3_core_801C5A38(command_ram,&c);
                        else if(state==33)dora3_s3_core_801C5BAC(command_ram,&c);
                        else if(state==34)dora3_s3_core_801C5C78(command_ram,&c);
                        const uint64_t extra=60-(uint64_t(tick)*divider*second)%60;
                        {SimulationScope remainder({extra,uint64_t(divider)*second,tick,0});
                         c=saved;c.f_odd=&c.f0.u32h;
                         auto* rdram=command_ram;const auto next=MEM_W(0x94,int32_t(player));
                         if(next==32)dora3_s3_core_801C5A78(command_ram,&c);
                         else if(next==33)dora3_s3_core_801C5BF0(command_ram,&c);
                         else if(next==34)dora3_s3_core_801C5D10(command_ram,&c);
                         else if(next>=1 && next<=5)dora3_s3_core_801C3464(command_ram,&c);
                         dora3_s3_801C0E3C(command_ram,&c);dora3_s3_801C0F1C(command_ram,&c);}
                        expected.push_back(read(command_ram));allocations=command_heap;next_address=command_address;
                    } else expected.push_back(read(reference));
                }
                allocations=heap_before;next_address=address_before;preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                for(unsigned tick=1;tick<=expected.size();++tick) {
                    to((second*divider*tick+59)/60);const auto actual=read(rdram);
                    assert(native_gameplay_preview_eligible(rdram));
                    for(unsigned k=0;k<actual.size();++k)if(std::abs(actual[k]-expected[tick-1][k])>(k<3?0.035:(k==12||k==13||k==15||k==16||k==17)?0.:0.001)) {
                        std::cerr<<"Air damage Source mismatch fps="<<fps<<" divider="<<divider<<" duration="<<duration<<" tick="<<tick<<" field="<<k<<" actual="<<actual[k]<<" expected="<<expected[tick-1][k]<<'\n';return 1;
                    }
                }
                assert(MEM_W(0x94,int32_t(player))>=1&&MEM_W(0x94,int32_t(player))<=5);
                native_gameplay_preview_reset(rdram);preview_live=false;wf(rdram,clip+0x200+4,30);
                MEM_B(0x28,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;MEM_W(0x94,int32_t(player))=1;
                MEM_H(0xD8,int32_t(physics))=0;MEM_W(0,int32_t(0x8021D4A8))=saved_status;
                MEM_B(0x18,int32_t(params))=saved_health_max;++air_damage_cases;
            }
            // Real C2978 damage at73ms: ordinary ground35, strong ground37
            // flight48 and airborne32, full recovery. Each nonuniform FPS partition is
            // compared with the60FPS native run, including command/health,
            // exact initial impulse, timers, model/lifetime and Source endpoints.
            for(unsigned mode=0;mode<7;++mode)for(unsigned duration:{3u,7u}) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=0;MEM_W(0,int32_t(0x8021D4B8))=0;
                const auto saved_status=MEM_W(0,int32_t(0x8021D4A8));
                const auto saved_health_max=MEM_B(0x18,int32_t(params));constexpr uint32_t status=0x80259000;
                MEM_W(0,int32_t(0x8021D4A8))=int32_t(status);MEM_B(1,int32_t(status))=10;MEM_B(0x18,int32_t(params))=10;
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                MEM_W(8,int32_t(control))=-1;MEM_B(0x28,int32_t(control))=mode==2;
                MEM_H(0x2A,int32_t(control))=1000;MEM_H(0xA,int32_t(params))=3;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=30;MEM_B(3,int32_t(0x80171560))=-30;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,mode==2?1000:0);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);
                wf(rdram,physics,2.75f);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);
                for(unsigned k=0;k<3;++k)wf(rdram,physics+0x10+4*k,0);
                MEM_H(0xD8,int32_t(physics))=0;
                for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                wf(rdram,clip+0x200+4,3.f*duration);
                c=saved;c.f_odd=&c.f0.u32h;
                if(mode==2)dora3_s3_801C768C(rdram,&c);else if(mode==3)dora3_s3_core_801C3638(rdram,&c);else dora3_s3_801C3124(rdram,&c);
                c.r4=1;dora3_s3_801C8B28(rdram,&c);
                const auto initial=memory;const auto heap_initial=allocations;const auto address_initial=next_address;
                auto read=[&] {
                    std::array<double,18> v{};
                    for(unsigned k=0;k<3;++k) {v[k]=rf(rdram,player+0x9C+4*k);v[6+k]=rf(rdram,player+0xC+4*k);v[9+k]=rf(rdram,physics+0x10+4*k);}
                    v[3]=rf(rdram,physics);v[4]=rf(rdram,physics+4);v[5]=rf(rdram,physics+0xC);
                    v[12]=MEM_W(0x94,int32_t(player));v[13]=MEM_H(0xD8,int32_t(physics));v[14]=rf(rdram,owners[0]+0x64);
                    v[15]=MEM_H(0x2A,int32_t(control));v[16]=MEM_BU(1,int32_t(status));v[17]=MEM_H(0x44,int32_t(physics));return v;
                };
                auto run=[&](unsigned render_fps) {
                    native_gameplay_preview_reset(rdram);preview_live=false;memory=initial;allocations=heap_initial;next_address=address_initial;
                    // Restore the secondary pose through its real rebind;
                    // leftover acquisition from the first run must not leak.
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=int32_t(owners[1]);dora3_s0_8003B744(rdram,&c);
                    c.r4=int32_t(owners[1]);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
                    preview_live=true;input_now=0;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                    auto to=[&](Nanoseconds stamp) {
                        while(second*(frame+1)/render_fps<stamp) {input_now=second*++frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                        input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                    };
                    to(73'000'000);assert(native_gameplay_preview_eligible(rdram));
                    MEM_H(0xD8,int32_t(physics))=10000;
                    if(mode==1||mode==4||mode==5) {
                        // Real floor metadata: C08E8 must acquire the hazard
                        // from the collision map, as it does in the game.
                        for(unsigned n=0;n<2;++n)MEM_B(40,int32_t(0x80280000+0x20+n*44))=int8_t(mode==1?0xFA:mode==4?0xF3:0xF8);
                    } else if(mode==3||mode==6) {
                        // An actor hit publishes health loss and D8 before
                        // C2978 dispatches the airborne reaction. Expiry of
                        // the special2A timer instead always selects35.
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=-2;dora3_s3_801C0738(rdram,&c);
                        if(mode==6)MEM_B(0x50,int32_t(control))=1;
                    } else MEM_H(0x2A,int32_t(control))=1;
                    auto source=memory;auto* reference=reinterpret_cast<uint8_t*>(source.data());
                    const auto command_heap=allocations;const auto command_address=next_address;
                    c=saved;c.f_odd=&c.f0.u32h;
                    c.f12.fl=0;c.f14.fl=0;dora3_s3_801C08E8(reference,&c);dora3_s3_801C0C7C(reference,&c);
                    c.r4=0;dora3_s3_core_801C2978(reference,&c);assert(c.r2==1);
                    {auto* rdram=reference;const auto expected=mode==0?35:mode==1?37:mode==2?48:mode==3?32:mode==4?40:mode==5?43:46;
                     if(MEM_W(0x94,int32_t(player))!=expected) {
                         std::cerr<<"Native damage reference mode="<<mode<<" fps="<<render_fps<<" state="<<MEM_W(0x94,int32_t(player))<<" expected="<<expected<<" surface="<<MEM_W(0,int32_t(0x8021D4B8))<<" contacts="<<MEM_W(0,int32_t(0x8021D68C))<<" health="<<unsigned(MEM_BU(1,int32_t(status)))<<" air="<<unsigned(MEM_BU(3,int32_t(control)))<<'\n';std::abort();
                     }}
                    allocations=command_heap;next_address=command_address;
                    native_damage_rumble=(mode==0||mode==3||mode==6)?std::array<unsigned,3>{10,2,2}:std::array<unsigned,3>{8,3,1};
                    native_damage_rumble_watch=true;native_damage_rumble_commands=0;native_damage_effect_commands=0;to(83'000'000);native_damage_rumble_watch=false;
                    assert(native_damage_rumble_commands==1 && native_gameplay_preview_eligible(rdram));
                    assert(native_damage_effect_commands==((mode==1||mode==4||mode==6)?1u:0u));
                    const auto state=MEM_W(0x94,int32_t(player));
                    if(state!=(mode==0?35:mode==1?37:mode==2?48:mode==3?32:mode==4?40:mode==5?43:46)) {
                        std::cerr<<"Native damage actual mode="<<mode<<" fps="<<render_fps<<" state="<<state<<" timer="<<MEM_H(0x2A,int32_t(control))<<" air="<<unsigned(MEM_BU(3,int32_t(control)))<<" health="<<unsigned(MEM_BU(1,int32_t(status)))<<" y="<<rf(rdram,player+0xA0)<<'\n';std::abort();
                    }
                    {auto* rdram=reference;const auto health=MEM_BU(1,int32_t(status));assert(health==((mode==1||mode==4||mode==5)?9:8));}
                    assert(MEM_BU(1,int32_t(status))==((mode==1||mode==4||mode==5)?9:8));
                    for(unsigned n=0;n<2;++n)MEM_B(40,int32_t(0x80280000+0x20+n*44))=0;
                    MEM_W(0,int32_t(0x8021D4B8))=0;
                    std::vector<std::array<double,18>> values{read()};
                    for(unsigned tick=3;tick<=160;++tick) {to((uint64_t(second)*divider*tick+59)/60);assert(native_gameplay_preview_eligible(rdram));values.push_back(read());}
                    assert(MEM_W(0x94,int32_t(player))==(mode==2?57:4));
                    return values;
                };
                const auto baseline=run(60),actual=run(fps);assert(baseline.size()==actual.size());
                for(unsigned n=0;n<actual.size();++n)for(unsigned k=0;k<actual[n].size();++k)
                    if(std::abs(actual[n][k]-baseline[n][k])>(k<3?0.04:(k==12||k==13||k==15||k==16||k==17)?0.:0.002)) {
                        for(unsigned j=n>3?n-3:0;j<std::min<unsigned>(actual.size(),n+2);++j) {
                            std::cerr<<"Damage trace sample="<<j<<" baseline=";
                            for(auto v:baseline[j])std::cerr<<v<<',';
                            std::cerr<<" actual=";for(auto v:actual[j])std::cerr<<v<<',';std::cerr<<'\n';
                        }
                        std::cerr<<"Native damage partition mismatch fps="<<fps<<" divider="<<divider<<" mode="<<mode<<" duration="<<duration<<" sample="<<n<<" field="<<k<<" actual="<<actual[n][k]<<" baseline="<<baseline[n][k]<<'\n';return 1;
                    }
                native_gameplay_preview_reset(rdram);preview_live=false;memory=initial;allocations=heap_initial;next_address=address_initial;
                wf(rdram,clip+0x200+4,30);MEM_B(0x28,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;
                MEM_W(0x94,int32_t(player))=1;MEM_H(0xD8,int32_t(physics))=0;
                MEM_W(0,int32_t(0x8021D4A8))=saved_status;MEM_B(0x18,int32_t(params))=saved_health_max;++native_damage_cases;
            }
            // Whole player dispatch, real Source flight speed/orientation,
            // input A transition57/58, collision solver, clip clock and RNG.
            for(unsigned pattern=0;pattern<4;++pattern) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;
                MEM_W(0,int32_t(0x801B36E0))=0;room(rdram);
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                MEM_B(0x28,int32_t(control))=1;MEM_W(0x8,int32_t(control))=-1;
                MEM_H(0x2A,int32_t(control))=1000;wf(rdram,control+0x1C,1);
                MEM_H(0x16,int32_t(control))=40;MEM_H(0xA,int32_t(params))=3;
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,150);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,pattern==2?359:0);wf(rdram,player+0x10,pattern==2?359:90);wf(rdram,player+0x14,pattern==2?359:0);
                wf(rdram,physics,pattern==3?2.875f:1.5f);wf(rdram,physics+4,pattern==2?359:90);
                wf(rdram,physics+8,0);wf(rdram,physics+0xC,pattern==2?359:0);
                for(unsigned k=0;k<3;++k)wf(rdram,physics+0x10+4*k,0);
                for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                c=saved;c.f_odd=&c.f0.u32h;dora3_s3_801C768C(rdram,&c);
                assert(MEM_W(0x94,int32_t(player))==57 && native_gameplay_preview_eligible(rdram));
                auto apply=[&](uint8_t* buffer,unsigned tick,bool native) {
                    auto* rdram=buffer;
                    // Source comparison edges must lie on exact clock
                    // boundaries for both dividers. First A is at0; release
                    // is at9 periods (300/450ms). Rounded1/30s is off-grid.
                    const uint16_t held=(pattern==3 && tick<10)?0x8000:0;
                    const int8_t x=pattern==1?30:pattern==2?-30:0;
                    const int8_t y=pattern==1?30:pattern==2?-30:0;
                    MEM_H(0,int32_t(0x80171560))=int16_t(held);
                    MEM_B(2,int32_t(0x80171560))=x;MEM_B(3,int32_t(0x80171560))=y;
                    c=saved;c.f_odd=&c.f0.u32h;
                    if(native)dora3_native_input_sample(buffer,&c);
                    else {
                        // Compare commands with the same accepted input.
                        // Start a fresh poll window while retaining the
                        // preceding held history for real press/release edges.
                        dora3_controller_reset_accumulator(buffer,&c);
                        dora3_controller_accumulate(buffer,&c);dora3_controller_latch(buffer,&c);
                    }
                };
                auto read=[&](uint8_t* buffer) {
                    auto* rdram=buffer;std::array<double,15> v{};
                    for(unsigned k=0;k<3;++k)v[k]=rf(buffer,player+0x9C+4*k);
                    v[3]=rf(buffer,physics);v[4]=rf(buffer,physics+4);v[5]=rf(buffer,physics+0xC);
                    for(unsigned k=0;k<3;++k) {v[6+k]=rf(buffer,player+0xC+4*k);v[9+k]=rf(buffer,physics+0x10+4*k);}
                    v[12]=MEM_W(0x94,int32_t(player));v[13]=MEM_H(0x2A,int32_t(control));v[14]=rf(buffer,owners[0]+0x64);
                    return v;
                };
                dora3_controller_reset_accumulator(rdram,&c);apply(rdram,0,false);
                auto legacy=memory;auto* old_ram=reinterpret_cast<uint8_t*>(legacy.data());
                const auto heap_before=allocations;
                const auto address_before=next_address;
                flight_random_calls=0;apply(old_ram,0,false);c.r4=1;dora3_s3_801C8B28(old_ram,&c);
                const auto first_reference=legacy;
                std::vector<std::array<double,15>> expected;
                for(unsigned tick=1;tick<=24;++tick) {
                    apply(old_ram,tick,false);c.r4=1;dora3_s3_801C8B28(old_ram,&c);expected.push_back(read(old_ram));
                }
                const auto source_random_calls=flight_random_calls;
                allocations=heap_before;next_address=address_before;flight_random_calls=0;
                c=saved;c.f_odd=&c.f0.u32h;apply(rdram,0,false);c.r4=1;dora3_s3_801C8B28(rdram,&c);
                assert(memory==first_reference);preview_live=true;input_now=0;
                native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                for(unsigned tick=1;tick<=expected.size();++tick) {
                    apply(rdram,tick,true);to((second*divider*tick+59)/60);
                    assert(native_gameplay_preview_eligible(rdram));const auto actual=read(rdram);
                    for(unsigned k=0;k<actual.size();++k)if(std::abs(actual[k]-expected[tick-1][k])>((k<3)?0.035:(k==12||k==13)?0.:0.001)) {
                        std::cerr<<"Player flight Source mismatch fps="<<fps<<" divider="<<divider<<" pattern="<<pattern<<" tick="<<tick<<" field="<<k<<" actual="<<actual[k]<<" expected="<<expected[tick-1][k]<<'\n';return 1;
                    }
                }
                assert(flight_random_calls==source_random_calls);
                native_gameplay_preview_reset(rdram);preview_live=false;MEM_B(0x28,int32_t(control))=0;
                MEM_H(0xA,int32_t(params))=0;MEM_W(0x94,int32_t(player))=1;++player_flight_cases;
            }
            for(unsigned character:{1u,2u}) for(bool moving:{false,true}) for(unsigned pattern=0;pattern<4;++pattern) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;
                const auto old_character=MEM_BU(0,int32_t(0x801B374A));MEM_B(0,int32_t(0x801B374A))=int8_t(character);
                MEM_H(0x14,int32_t(control))=0;MEM_W(0x94,int32_t(player))=1;
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,100);wf(rdram,player+0x10,pattern==1?359:90);
                wf(rdram,physics,0);wf(rdram,physics+0x14,0);
                std::memset(rdram+(effect&0xFFFFFF),0,20*0xCC);
                for(unsigned i=0;i<20;++i) {c.r4=int32_t(effect+i*0xCC);dora3_scene_object_init(rdram,&c);MEM_H(0xA8,int32_t(effect+i*0xCC))=-1;}
                for(unsigned i=30;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                MEM_W(0x94,int32_t(prop))=pattern==2?0:1;MEM_W(0x98,int32_t(prop))=int32_t(prop_physics);
                MEM_B(0xC9,int32_t(prop_physics))=1;
                wf(rdram,prop+0x9C,pattern==1?200:pattern==3?950:1000);
                wf(rdram,prop+0xA0,pattern==1?190:pattern==3?149:150);wf(rdram,prop+0xA4,pattern==3?1:500);
                c.r29=int32_t(0x803F0000);dora3_s3_core_801CC8F4(rdram,&c);assert(c.r2==1);
                const auto physical=uint32_t(MEM_W(0x98,int32_t(effect)));
                const auto model=uint32_t(MEM_W(0x74,int32_t(effect)));
                wf(rdram,physical,moving?25:0);wf(rdram,physical+8,moving?7:0);
                auto legacy=memory;auto* old_ram=reinterpret_cast<uint8_t*>(legacy.data());
                reference_dora3_s3_801D10D8(old_ram,&c);const auto first_reference=legacy;
                auto read=[&](uint8_t* buffer) {auto* rdram=buffer;return std::array<double,8>{double(MEM_H(0x34,int32_t(physical))),double(MEM_H(0x36,int32_t(physical))),rf(rdram,physical+4),rf(rdram,physical+0x58),rf(rdram,effect+0x9C),rf(rdram,effect+0xA0),rf(rdram,effect+0xA4),rf(rdram,physical+0xC)};};
                std::vector<std::array<double,8>> expected(moving?6:102);
                std::vector<std::array<double,9>> curves(expected.size());
                for(unsigned tick=0;tick<expected.size();++tick) {
                    auto& curve=curves[tick];const auto yaw=rf(old_ram,physical+4);
                    for(unsigned k=0;k<3;++k)curve[k]=rf(old_ram,effect+0x9C+4*k);
                    c.r4=int32_t(physical);dora3_s3_core_801CB2F8(old_ram,&c);
                    c.r4=int32_t(physical);dora3_s3_core_801CB3A8(old_ram,&c);
                    for(unsigned k=0;k<3;++k)curve[3+k]=rf(old_ram,physical+0x10+4*k);
                    reference_dora3_s3_801D10D8(old_ram,&c);expected[tick]=read(old_ram);assert(allocations.count(physical));
                    // Build an independent curved sample from Original's
                    // incoming projection and actual next wave pose. Steering
                    // is a following command, so this bend retains incoming yaw.
                    const auto next_yaw=rf(old_ram,physical+4);wf(old_ram,physical+4,yaw);
                    c.r4=int32_t(physical);dora3_s3_core_801CB2F8(old_ram,&c);
                    c.r4=int32_t(physical);dora3_s3_core_801CB3A8(old_ram,&c);
                    for(unsigned k=0;k<3;++k)curve[6+k]=rf(old_ram,physical+0x10+4*k)-curve[3+k];
                    wf(old_ram,physical+4,next_yaw);
                }
                game_model_animation().hook(rdram,&c,0x80030850,0);dora3_s3_801D10D8(rdram,&c);assert(memory==first_reference);
                terminal_model=model;terminal_deletes=0;impact_spawns=0;
                preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto update=[&](Nanoseconds stamp) {
                    const auto old_time=input_now;const auto old_pitch=rf(rdram,physical+0xC);
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                    if(MEM_W(0x94,int32_t(effect))) {
                        const auto delta=std::abs(std::remainder(rf(rdram,physical+0xC)-old_pitch,360.f));
                        assert(delta<=25.*double(stamp-old_time)*60./(double(second)*divider)+0.0001);
                    }
                };
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp)update(second*++frame/fps);
                    update(stamp);
                };
                for(unsigned tick=1;tick<=expected.size();++tick) {
                    for(unsigned part=1;part<4;++part) {
                        to((second*divider*(4*(tick-1)+part)+239)/240);
                        const auto& curve=curves[tick-1];const double age=double(part)/4;
                        for(unsigned k=0;k<3;++k) {
                            const auto expected_position=curve[k]+curve[3+k]*age+curve[6+k]*age*(age-1)*0.5;
                            if(std::abs(rf(rdram,effect+0x9C+4*k)-expected_position)>0.03) {
                                std::cerr<<"Homing curved interval mismatch fps="<<fps<<" divider="<<divider<<" moving="<<moving<<" pattern="<<pattern<<" tick="<<tick<<" part="<<part<<" axis="<<k<<'\n';return 1;
                            }
                        }
                    }
                    to((second*divider*tick+59)/60);const auto actual=read(rdram);
                    for(unsigned k=0;k<8;++k)if(std::abs((k==2||k==3||k==7)?std::remainder(actual[k]-expected[tick-1][k],360.):actual[k]-expected[tick-1][k])>(k<2?0:0.03)) {
                        std::cerr<<"Homing flight mismatch fps="<<fps<<" divider="<<divider<<" character="<<character<<" moving="<<moving<<" pattern="<<pattern<<" tick="<<tick<<" field="<<k<<" actual="<<actual[k]<<" expected="<<expected[tick-1][k]<<'\n';return 1;
                    }
                }
                MEM_H(0xD4,int32_t(physical))=0;
                to(input_now+second/fps);assert(terminal_deletes==1 && impact_spawns==1);
                const auto child=effect+0xCC;const auto child_physical=uint32_t(MEM_W(0x98,int32_t(child)));
                assert(MEM_H(0x20,int32_t(child_physical))==87);
                for(unsigned f=1;f<=fps*3;++f)to(input_now+second/fps);
                for(unsigned i=0;i<20;++i)assert(!MEM_W(0x94,int32_t(effect+i*0xCC))&&!MEM_W(0x98,int32_t(effect+i*0xCC)));
                assert(!allocations.count(physical)&&!allocations.count(model)&&(resource_references==std::array<unsigned,5>{}));
                terminal_model=0;native_gameplay_preview_reset(rdram);preview_live=false;++homing_cases;
                MEM_W(0x94,int32_t(prop))=0;MEM_B(0xC9,int32_t(prop_physics))=0;MEM_B(0,int32_t(0x801B374A))=int8_t(old_character);
            }
            for(unsigned kind:{101u,103u}) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;
                MEM_H(0x14,int32_t(control))=0;MEM_W(0x94,int32_t(player))=1;
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,100);wf(rdram,player+0xA4,0);
                wf(rdram,player+0x10,90);wf(rdram,player+0xC,0);
                wf(rdram,physics,0);wf(rdram,physics+0x14,0);
                std::memset(rdram+(effect&0xFFFFFF),0,20*0xCC);
                for(unsigned i=0;i<20;++i) {c.r4=int32_t(effect+i*0xCC);dora3_scene_object_init(rdram,&c);MEM_H(0xA8,int32_t(effect+i*0xCC))=-1;}
                auto create=[&]() {
                    c.r29=int32_t(0x803F0000);
                    (kind==101?dora3_s3_core_801CF7E4:dora3_s3_core_801CFDEC)(rdram,&c);assert(c.r2==1);
                    return std::array<uint32_t,2>{uint32_t(MEM_W(0x98,int32_t(effect))),uint32_t(MEM_W(0x74,int32_t(effect)))};
                };
                const auto old=create();terminal_model=old[1];terminal_deletes=0;terminal_end_commit=true;
                auto read=[&](uint8_t* buffer,const std::array<uint32_t,2>& owner) {
                    auto* rdram=buffer;
                    return std::array<double,8>{double(MEM_W(0x94,int32_t(effect))),rf(rdram,effect+0x9C),rf(rdram,effect+0xA0),rf(rdram,effect+0xA4),rf(rdram,owner[0]),double(MEM_H(0x34,int32_t(owner[0]))),rf(rdram,owner[0]+0x58),rf(rdram,owner[1]+0x78)};
                };
                reference_dora3_s3_801D10D8(rdram,&c);std::array<std::array<double,8>,7> expected{};
                for(unsigned tick=1;tick<=7;++tick) {
                    input_now=(second*divider*tick+59)/60;c=saved;c.f_odd=&c.f0.u32h;c.r4=1;dora3_s3_801C8B28(rdram,&c);
                    reference_dora3_s3_801D10D8(rdram,&c);
                    expected[tick-1]=read(rdram,old);assert(expected[tick-1][0]==1);
                }
                input_now=(second*divider*8+59)/60;c=saved;c.f_odd=&c.f0.u32h;c.r4=1;dora3_s3_801C8B28(rdram,&c);reference_dora3_s3_801D10D8(rdram,&c);
                assert(terminal_deletes==1);const auto source_deleted_at=terminal_deleted_at;
                input_now=(second*divider*9+59)/60;reference_dora3_s3_801D10D8(rdram,&c);
                assert(!allocations.count(old[0])&&!allocations.count(old[1])&&(resource_references==std::array<unsigned,5>{}));
                input_now=0;MEM_W(0x94,int32_t(player))=1;
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,100);wf(rdram,player+0xA4,0);wf(rdram,player+0x10,90);wf(rdram,player+0xC,0);
                wf(rdram,physics,0);wf(rdram,physics+0x14,0);
                const auto owner=create();terminal_model=owner[1];terminal_deletes=0;impact_spawns=impact_commands=0;
                game_model_animation().hook(rdram,&c,0x80030850,0);auto legacy=memory;auto legacy_context=c;legacy_context.f_odd=&legacy_context.f0.u32h;
                reference_dora3_s3_801D10D8(reinterpret_cast<uint8_t*>(legacy.data()),&legacy_context);
                game_model_animation().hook(rdram,&c,0x80030850,0);dora3_s3_801D10D8(rdram,&c);assert(memory==legacy);
                preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                for(unsigned tick=1;tick<=7;++tick) {
                    to((second*divider*tick+59)/60);const auto actual=read(rdram,owner);
                    for(unsigned k=0;k<8;++k)if(std::abs(actual[k]-expected[tick-1][k])>((k==0||k==5)?0:0.03)) {
                        std::cerr<<"Charge mismatch kind="<<kind<<" fps="<<fps<<" divider="<<divider<<" tick="<<tick<<" field="<<k<<" actual="<<actual[k]<<" expected="<<expected[tick-1][k]<<'\n';return 1;
                    }
                }
                to((second*divider*8+59)/60);
                assert(terminal_deletes==1 && std::abs(terminal_deleted_at-source_deleted_at)<2e-7 && impact_spawns==1 && impact_commands==1);
                const auto child=effect+0xCC;const auto child_physical=uint32_t(MEM_W(0x98,int32_t(child)));
                assert(MEM_H(0x20,int32_t(child_physical))==int(kind+1));
                to((second*divider*10+59)/60);
                for(unsigned i=0;i<20;++i)assert(!MEM_W(0x94,int32_t(effect+i*0xCC))&&!MEM_W(0x98,int32_t(effect+i*0xCC)));
                assert(!allocations.count(owner[0])&&!allocations.count(owner[1])&&(resource_references==std::array<unsigned,5>{}));
                terminal_model=0;terminal_end_commit=false;native_gameplay_preview_reset(rdram);preview_live=false;++charge_cases;
                // A real constructor runs during the player's live interval.
                // Its zero-time initial commands must not acquire elapsed time
                // belonging to the interval before this birth.
                input_now=0;frame=0;MEM_W(0x94,int32_t(player))=1;
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,100);wf(rdram,player+0xA4,0);wf(rdram,player+0x10,90);wf(rdram,player+0xC,0);
                wf(rdram,physics,0);wf(rdram,physics+0x14,0);
                terminal_deletes=0;impact_spawns=impact_commands=0;spawn_charge=kind;
                preview_live=true;native_gameplay_preview_begin(rdram,0,divider);
                to((second*divider+59)/60);assert(!spawn_charge);
                const auto born_physical=uint32_t(MEM_W(0x98,int32_t(effect))),born_model=uint32_t(MEM_W(0x74,int32_t(effect)));
                assert(MEM_H(0x34,int32_t(born_physical))==21 && std::abs(rf(rdram,born_physical)-10)<1e-5);
                assert(std::abs(rf(rdram,born_model+0x64))<1e-5 && rf(rdram,born_model+0x78)<=1);
                terminal_model=born_model;terminal_end_commit=true;
                to((second*divider*9+59)/60);
                assert(terminal_deletes==1 && std::abs(terminal_deleted_at-charge_born_at-8.*divider/60)<2e-7 && impact_spawns==1 && impact_commands==1);
                to((second*divider*11+59)/60);
                for(unsigned i=0;i<20;++i)assert(!MEM_W(0x94,int32_t(effect+i*0xCC))&&!MEM_W(0x98,int32_t(effect+i*0xCC)));
                assert(!allocations.count(born_physical)&&!allocations.count(born_model)&&(resource_references==std::array<unsigned,5>{}));
                terminal_model=0;terminal_end_commit=false;native_gameplay_preview_reset(rdram);preview_live=false;++charge_birth_cases;
            }
            for(unsigned scenario=0;scenario<5;++scenario) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;
                MEM_H(0x14,int32_t(control))=0;MEM_W(0x94,int32_t(player))=1;
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,0);wf(rdram,player+0xA4,0);
                wf(rdram,player+0x10,90);wf(rdram,player+0xC,0);wf(rdram,player+0x14,0);wf(rdram,physics,0);wf(rdram,physics+0x14,0);
                std::memset(rdram+(effect&0xFFFFFF),0,20*0xCC);
                for(unsigned i=0;i<20;++i) {c.r4=int32_t(effect+i*0xCC);dora3_scene_object_init(rdram,&c);MEM_H(0xA8,int32_t(effect+i*0xCC))=-1;}
                for(unsigned i=30;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                // Finish the idle player's pending state/orientation command
                // before sampling attachment geometry from a stationary pose.
                c=saved;c.f_odd=&c.f0.u32h;c.r4=1;dora3_s3_801C8B28(rdram,&c);
                MEM_W(0x94,int32_t(prop))=1;MEM_W(0x98,int32_t(prop))=int32_t(prop_physics);MEM_B(0xC9,int32_t(prop_physics))=1;
                c.r4=int32_t(asset);dora3_s0_8003B560(rdram,&c);const auto npc_model=uint32_t(c.r2);MEM_W(0x74,int32_t(prop))=int32_t(npc_model);
                wf(rdram,prop+0x9C,700);wf(rdram,prop+0xA0,55);wf(rdram,prop+0xA4,0);
                wf(rdram,effect_clip+0x204,54);wf(rdram,effect_clip+0x220,54);MEM_W(0,int32_t(0x801AEC00+40))=int32_t(effect_clip);
                auto create=[&](uint8_t* buffer) {
                    c.r29=int32_t(0x803F0000);dora3_s3_core_801D03D0(buffer,&c);assert(c.r2==1);auto* rdram=buffer;
                    const auto physical=uint32_t(MEM_W(0x98,int32_t(effect)));
                    if(scenario && scenario!=4)MEM_H(0x34,int32_t(physical))=scenario==1?2:1;
                    if(scenario==1||scenario==3)MEM_W(0x48,int32_t(physical))=int32_t(prop);
                    MEM_H(0xD6,int32_t(physical))=scenario>=3?16:0;
                    if(scenario==4)MEM_H(0xD4,int32_t(physical))=30;
                    return std::array<uint32_t,2>{physical,uint32_t(MEM_W(0x74,int32_t(effect)))};
                };
                auto read=[&](uint8_t* buffer,uint32_t physical) {auto* rdram=buffer;return std::array<double,7>{double(bool(MEM_W(0x94,int32_t(effect)))),rf(rdram,effect+0x9C),rf(rdram,effect+0xA0),double(MEM_H(0x34,int32_t(physical))),rf(rdram,prop+0x9C),rf(rdram,prop+0xA4),double(bool(MEM_W(0,int32_t(0x8021D4C0))))};};
                const auto legacy_heap=allocations;const auto legacy_resources=resource_references;const auto legacy_address=next_address;
                auto legacy=memory;auto* old_ram=reinterpret_cast<uint8_t*>(legacy.data());const auto reference_owner=create(old_ram);
                reference_dora3_s3_801D10D8(old_ram,&c);std::vector<std::array<double,7>> expected;
                for(unsigned tick=1;tick<=45;++tick) {
                    input_now=(second*divider*tick+59)/60;c=saved;c.f_odd=&c.f0.u32h;c.r4=1;
                    dora3_s3_801C8B28(old_ram,&c);reference_dora3_s3_801D10D8(old_ram,&c);
                    expected.push_back(read(old_ram,reference_owner[0]));
                }
                input_now=0;
                assert(!allocations.count(reference_owner[0])&&!allocations.count(reference_owner[1])&&(resource_references==std::array<unsigned,5>{}));
                // This fork includes the player's real clip commands. Restore
                // the external heap bookkeeping with its copied guest memory.
                assert(resource_references==legacy_resources);allocations=legacy_heap;next_address=legacy_address;
                const auto owner=create(rdram);
                std::array<std::array<double,2>,19> capture_keys{};
                if(scenario==1)for(unsigned key=0;key<capture_keys.size();++key) {
                    auto sample=memory;auto* sample_ram=reinterpret_cast<uint8_t*>(sample.data());
                    wf(sample_ram,owner[1]+0x64,float(3*key));MEM_B(0x7C,int32_t(owner[1]))=0;
                    auto probe=c;probe.f_odd=&probe.f0.u32h;probe.r4=int32_t(effect);probe.r5=int32_t(owner[0]);
                    dora3_s3_core_801D05A8(sample_ram,&probe);
                    capture_keys[key]={rf(sample_ram,prop+0x9C),rf(sample_ram,prop+0xA4)};
                }
                const auto heap_before=allocations;const auto resources_before=resource_references;const auto address_before=next_address;
                auto first_reference=memory;auto first_context=c;first_context.f_odd=&first_context.f0.u32h;
                reference_dora3_s3_801D10D8(reinterpret_cast<uint8_t*>(first_reference.data()),&first_context);
                // The guest allocates its real child in this first pass. The
                // instrumented heap is external to RDRAM, so replay its state
                // too; otherwise equal calls necessarily get different IDs.
                auto cleanup=first_reference;auto* cleanup_ram=reinterpret_cast<uint8_t*>(cleanup.data());
                for(unsigned i=1;i<20;++i) {
                    const auto child=effect+i*0xCC;
                    if(MEM_W(0x94,int32_t(child)))continue;
                    auto* rdram=cleanup_ram;
                    if(MEM_W(0x94,int32_t(child))) {first_context.r4=int32_t(child);first_context.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&first_context);}
                }
                assert(allocations==heap_before && resource_references==resources_before);next_address=address_before;
                game_model_animation().hook(rdram,&c,0x80030850,0);dora3_s3_801D10D8(rdram,&c);
                if(memory!=first_reference) {
                    unsigned shown=0;for(unsigned index=0;index<memory.size()&&shown<8;++index)if(memory[index]!=first_reference[index]) {
                        std::cerr<<"Interaction Original RAM mismatch scenario="<<scenario<<" address="<<std::hex<<(0x80000000u+4*index)<<" actual="<<memory[index]<<" expected="<<first_reference[index]<<std::dec<<'\n';++shown;
                    }
                    return 1;
                }
                terminal_model=owner[1];terminal_deletes=0;terminal_end_commit=true;
                const float acquired_pose=scenario>=2?57.f:0.f;
                assert(rf(rdram,owner[1]+0x64)==(scenario>=2?6.f:3.f));
                preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                assert(rf(rdram,owner[1]+0x64)==acquired_pose);
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                for(unsigned tick=1;tick<=expected.size();++tick) {
                    if(scenario==1 && tick<capture_keys.size())for(unsigned part=1;part<4;++part) {
                        to((second*divider*(4*(tick-1)+part)+239)/240);
                        for(unsigned axis=0;axis<2;++axis) {
                            const auto position=capture_keys[tick-1][axis]+(capture_keys[tick][axis]-capture_keys[tick-1][axis])*double(part)/4;
                            if(std::abs(rf(rdram,prop+(axis?0xA4:0x9C))-position)>0.03) {
                                std::cerr<<"Capture interval mismatch fps="<<fps<<" divider="<<divider<<" tick="<<tick<<" part="<<part<<" axis="<<axis<<" actual="<<rf(rdram,prop+(axis?0xA4:0x9C))<<" expected="<<position<<" clip="<<rf(rdram,owner[1]+0x64)<<" player="<<rf(rdram,player+0x9C)<<" pitch="<<rf(rdram,player+0xC)<<" yaw="<<rf(rdram,player+0x10)<<" roll="<<rf(rdram,player+0x14)<<" binding="<<std::hex<<MEM_W(0,int32_t(0x8021D49C))<<std::dec<<'\n';return 1;
                            }
                        }
                    }
                    to((second*divider*tick+59)/60);const auto actual=read(rdram,owner[0]);
                    for(unsigned k=0;k<actual.size();++k)if(std::abs(actual[k]-expected[tick-1][k])>((k==0||k==3||k==6)?0:0.03)) {
                        std::cerr<<"Interaction mismatch fps="<<fps<<" divider="<<divider<<" scenario="<<scenario<<" tick="<<tick<<" field="<<k<<" actual="<<actual[k]<<" expected="<<expected[tick-1][k]<<" clip="<<rf(rdram,owner[1]+0x64)<<" flags="<<int(MEM_BU(0x7C,int32_t(owner[1])))<<" phase="<<MEM_H(0x34,int32_t(owner[0]))<<" player="<<MEM_W(0x94,int32_t(player))<<" eligible="<<native_gameplay_preview_eligible(rdram)<<'\n';return 1;
                    }
                    if(scenario==1 && MEM_W(0x94,int32_t(effect))) {
                        // A source-owned actor command cannot dislodge a live
                        // capture at this timestamp or advance its clock twice.
                        const auto x=rf(rdram,prop+0x9C),z=rf(rdram,prop+0xA4),time=rf(rdram,owner[1]+0x64);
                        wf(rdram,prop+0x9C,-100);wf(rdram,prop+0xA4,-100);
                        dora3_s3_801D10D8(rdram,&c);
                        assert(std::abs(rf(rdram,prop+0x9C)-x)<0.03 && std::abs(rf(rdram,prop+0xA4)-z)<0.03 && rf(rdram,owner[1]+0x64)==time);
                    }
                }
                assert(terminal_deletes==1&&!allocations.count(owner[0])&&!allocations.count(owner[1])&&(resource_references==std::array<unsigned,5>{}));
                for(unsigned i=0;i<20;++i)assert(!MEM_W(0x94,int32_t(effect+i*0xCC))&&!MEM_W(0x98,int32_t(effect+i*0xCC)));
                terminal_model=0;terminal_end_commit=false;native_gameplay_preview_reset(rdram);preview_live=false;
                c.r4=int32_t(npc_model);dora3_s0_8003B77C(rdram,&c);MEM_W(0x94,int32_t(prop))=0;MEM_B(0xC9,int32_t(prop_physics))=0;++interaction_cases;
                wf(rdram,effect_clip+0x204,3);wf(rdram,effect_clip+0x220,3);
            }
            for(bool reuse_target:{false,true}) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;
                MEM_H(0x14,int32_t(control))=0;MEM_W(0x94,int32_t(player))=1;
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,0);wf(rdram,player+0xA4,0);
                wf(rdram,player+0x10,90);wf(rdram,player+0xC,0);wf(rdram,player+0x14,0);wf(rdram,physics,0);wf(rdram,physics+0x14,0);
                wf(rdram,effect_clip+0x204,54);wf(rdram,effect_clip+0x220,54);
                uint32_t npc_model=0;
                if(reuse_target) {
                    c.r4=int32_t(asset);dora3_s0_8003B560(rdram,&c);npc_model=uint32_t(c.r2);
                    MEM_W(0x74,int32_t(prop))=int32_t(npc_model);MEM_W(0x94,int32_t(prop))=1;
                    MEM_W(0x98,int32_t(prop))=int32_t(prop_physics);MEM_B(0xC9,int32_t(prop_physics))=1;
                }
                spawn_charge=105;capture_birth=reuse_target;terminal_deletes=0;
                preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                to((second*divider+59)/60);assert(!spawn_charge && !capture_birth);
                const auto physical=uint32_t(MEM_W(0x98,int32_t(effect))),model=uint32_t(MEM_W(0x74,int32_t(effect)));
                assert(MEM_H(0x20,int32_t(physical))==105 && MEM_W(0,int32_t(0x8021D4C0))==1);
                assert(interaction_initial_pose==(reuse_target?57.f:0.f) && !(interaction_initial_flags&2));
                assert(std::abs(std::remainder(rf(rdram,model+0x64)-interaction_initial_pose,54.f))<1e-5);
                terminal_model=model;terminal_end_commit=true;
                if(reuse_target) {
                    assert(MEM_H(0x34,int32_t(physical))==2 && uint32_t(MEM_W(0x48,int32_t(physical)))==prop);
                    assert(game_model_animation().bound_actor_valid(rdram,model,physical+0x48,prop));
                    const auto address_after_birth=next_address;
                    c.r4=int32_t(npc_model);dora3_s0_8003B77C(rdram,&c);next_address=npc_model;
                    c.r4=int32_t(asset);dora3_s0_8003B560(rdram,&c);assert(uint32_t(c.r2)==npc_model);next_address=address_after_birth;
                    wf(rdram,prop+0x9C,-1000);wf(rdram,prop+0xA4,-1000);
                    to((second*divider*3+119)/120);
                    assert(MEM_W(0x48,int32_t(physical))==0 && rf(rdram,prop+0x9C)==-1000 && rf(rdram,prop+0xA4)==-1000);
                }
                const unsigned lifetime=reuse_target?1:18;
                to((second*divider*(lifetime+1)+59)/60);
                if(terminal_deletes!=1 || std::abs(terminal_deleted_at-charge_born_at-double(lifetime)*divider/60)>=2e-7) {
                    std::cerr<<"Interaction birth lifetime mismatch fps="<<fps<<" divider="<<divider<<" reused="<<reuse_target<<" born="<<charge_born_at<<" deleted="<<terminal_deleted_at<<" expected age="<<double(lifetime)*divider/60<<" deletes="<<terminal_deletes<<'\n';return 1;
                }
                assert(!MEM_W(0x94,int32_t(effect)) && !MEM_W(0,int32_t(0x8021D4C0)));
                assert(!allocations.count(physical)&&!allocations.count(model)&&(resource_references==std::array<unsigned,5>{}));
                terminal_model=0;terminal_end_commit=false;native_gameplay_preview_reset(rdram);preview_live=false;
                if(npc_model) {c.r4=int32_t(npc_model);dora3_s0_8003B77C(rdram,&c);MEM_W(0x94,int32_t(prop))=0;MEM_B(0xC9,int32_t(prop_physics))=0;}
                wf(rdram,effect_clip+0x204,3);wf(rdram,effect_clip+0x220,3);++interaction_birth_cases;
            }
            // Real action states and their frame18 cue create the charge. No
            // synthetic player-producer callback is involved in this path.
            for(unsigned weapon:{11u,12u})for(bool airborne:{false,true})for(bool button_entry:{false,true}) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;
                MEM_W(0,int32_t(0x801B36E0))=int(weapon);
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                // Complete a released-button input cycle. Original latch does
                // not itself commit held history; the world loop resets its
                // accumulator after the update, otherwise B replays forever.
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                MEM_W(0x94,int32_t(player))=1;
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,airborne?150:0);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);
                wf(rdram,physics,0);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);wf(rdram,physics+0x14,airborne?10:0);
                for(unsigned i=30;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                c=saved;c.f_odd=&c.f0.u32h;
                if(button_entry) {
                    (airborne?dora3_s3_core_801C383C:dora3_s3_801C3124)(rdram,&c);
                    MEM_B(0x3,int32_t(control))=airborne?1:0;
                    MEM_H(0,int32_t(0x80171560))=int16_t(0x4000);
                    dora3_controller_accumulate(rdram,&c);dora3_controller_latch(rdram,&c);
                } else {
                    (airborne?dora3_s3_core_801C46B0:dora3_s3_core_801C44C0)(rdram,&c);
                    assert(MEM_W(0x94,int32_t(player))==(airborne?18:17));
                }
                const auto heap_before=allocations;const auto address_before=next_address;
                auto legacy=memory;auto* old_ram=reinterpret_cast<uint8_t*>(legacy.data());
                auto source=[&](unsigned tick) {
                    input_now=(second*divider*tick+59)/60;c=saved;c.f_odd=&c.f0.u32h;c.r4=1;
                    if(button_entry && tick) {dora3_controller_accumulate(old_ram,&c);dora3_controller_latch(old_ram,&c);c.r4=1;}
                    dora3_s3_801C8B28(old_ram,&c);reference_dora3_s3_801D10D8(old_ram,&c);
                    if(button_entry)dora3_controller_reset_accumulator(old_ram,&c);
                };
                weapon_watch=true;weapon_fires=0;source(0);const auto first_reference=legacy;
                for(unsigned tick=1;tick<=40;++tick)source(tick);
                if(weapon_fires!=1) {std::cerr<<"Original weapon trigger count="<<weapon_fires<<" weapon="<<weapon<<" airborne="<<airborne<<'\n';return 1;}
                const auto expected_shot=weapon_fired_at;
                assert((resource_references==std::array<unsigned,5>{}));
                allocations=heap_before;next_address=address_before;input_now=0;weapon_fires=0;
                c=saved;c.f_odd=&c.f0.u32h;c.r4=1;
                game_model_animation().hook(rdram,&c,0x80030850,0);
                dora3_s3_801C8B28(rdram,&c);dora3_s3_801D10D8(rdram,&c);
                if(button_entry)dora3_controller_reset_accumulator(rdram,&c);
                if(memory!=first_reference) {std::cerr<<"Weapon Original first-pass RAM mismatch weapon="<<weapon<<" airborne="<<airborne<<'\n';return 1;}
                preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                assert(rf(rdram,owners[0]+0x64)==0 && MEM_W(0x94,int32_t(player))==(airborne?18:17));
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                to((second*divider*6+59)/60);
                if(weapon_fires!=1 || std::abs(weapon_fired_at-expected_shot)>2e-7) {
                    std::cerr<<"Weapon trigger mismatch fps="<<fps<<" divider="<<divider<<" weapon="<<weapon<<" airborne="<<airborne<<" fires="<<weapon_fires<<" stamp="<<weapon_fired_at<<" expected="<<expected_shot<<" state="<<MEM_W(0x94,int32_t(player))<<" clip="<<rf(rdram,owners[0]+0x64)<<'\n';return 1;
                }
                const auto physical=uint32_t(MEM_W(0x98,int32_t(effect))),model=uint32_t(MEM_W(0x74,int32_t(effect)));
                assert(MEM_H(0x20,int32_t(physical))==(weapon==11?101:103) && MEM_H(0x34,int32_t(physical))==21);
                assert(std::abs(rf(rdram,model+0x64))<1e-5);
                terminal_model=model;terminal_deletes=0;terminal_end_commit=true;
                to((second*divider*40+59)/60);
                assert(weapon_fires==1 && terminal_deletes==1 && std::abs(terminal_deleted_at-weapon_fired_at-8.*divider/60)<2e-7);
                for(unsigned i=0;i<20;++i)assert(!MEM_W(0x94,int32_t(effect+i*0xCC))&&!MEM_W(0x98,int32_t(effect+i*0xCC)));
                assert(!allocations.count(physical)&&!allocations.count(model)&&(resource_references==std::array<unsigned,5>{}));
                terminal_model=0;terminal_end_commit=false;weapon_watch=false;native_gameplay_preview_reset(rdram);preview_live=false;
                MEM_W(0,int32_t(0x801B36E0))=0;++weapon_cases;
            }
            for(unsigned weapon:{11u,12u})for(bool airborne:{false,true}) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                MEM_W(0,int32_t(0x801B36E0))=int(weapon);
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                MEM_W(0x94,int32_t(player))=1;
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,airborne?150:0);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);
                wf(rdram,physics,0);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);wf(rdram,physics+0x14,airborne?10:0);
                for(unsigned i=30;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                c=saved;c.f_odd=&c.f0.u32h;
                (airborne?dora3_s3_core_801C383C:dora3_s3_801C3124)(rdram,&c);
                MEM_B(3,int32_t(control))=airborne?1:0;
                c.r4=1;game_model_animation().hook(rdram,&c,0x80030850,0);
                dora3_s3_801C8B28(rdram,&c);dora3_s3_801D10D8(rdram,&c);
                weapon_watch=true;weapon_fires=0;preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                constexpr Nanoseconds press=73'000'000;
                to(press);MEM_H(0,int32_t(0x80171560))=int16_t(0x4000);
                dora3_controller_accumulate(rdram,&c);
                const Nanoseconds shot=press+(second*divider*6+59)/60;
                to(shot);
                assert(weapon_fires==1 && std::abs(weapon_fired_at-double(shot)/second)<2e-7);
                const auto physical=uint32_t(MEM_W(0x98,int32_t(effect))),model=uint32_t(MEM_W(0x74,int32_t(effect)));
                assert(MEM_H(0x20,int32_t(physical))==(weapon==11?101:103) && MEM_H(0x34,int32_t(physical))==21);
                terminal_model=model;terminal_deletes=0;terminal_end_commit=true;
                const Nanoseconds expires=shot+(second*divider*8+59)/60;
                to(expires-1);
                if(!MEM_W(0x94,int32_t(effect)) || terminal_deletes) {
                    std::cerr<<"Native B charge duration mismatch fps="<<fps<<" divider="<<divider<<" weapon="<<weapon<<" airborne="<<airborne<<" born="<<weapon_fired_at<<" deleted="<<terminal_deleted_at<<" expected="<<double(expires)/second<<'\n';return 1;
                }
                to(expires+2);
                assert(terminal_deletes==1 && std::abs(terminal_deleted_at-weapon_fired_at-8.*divider/60)<2e-7);
                to(expires+(second*divider*30+59)/60);
                assert(weapon_fires==1 && !allocations.count(physical)&&!allocations.count(model)&&(resource_references==std::array<unsigned,5>{}));
                for(unsigned i=0;i<20;++i)assert(!MEM_W(0x94,int32_t(effect+i*0xCC))&&!MEM_W(0x98,int32_t(effect+i*0xCC)));
                terminal_model=0;terminal_end_commit=false;weapon_watch=false;native_gameplay_preview_reset(rdram);preview_live=false;
                MEM_W(0,int32_t(0x801B36E0))=0;++native_weapon_cases;
            }
            for(unsigned weapon:{1u,2u,3u,4u,5u,6u,7u,8u,13u})for(bool airborne:{false,true})for(float effect_duration:{3.f,7.f}) {
                wf(rdram,effect_clip+0x204,effect_duration);wf(rdram,effect_clip+0x220,effect_duration);
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                MEM_W(0,int32_t(0x801B36E0))=int(weapon);
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                MEM_W(0x94,int32_t(player))=1;
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,airborne?150:0);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);
                wf(rdram,physics,0);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);wf(rdram,physics+0x14,airborne?10:0);
                for(unsigned i=30;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                c=saved;c.f_odd=&c.f0.u32h;
                const bool held_cue=weapon==1 || weapon==13;
                const bool zero_cue=weapon==3 || (weapon>=6 && weapon<=8);
                if(held_cue)(airborne?dora3_s3_core_801C5680:dora3_s3_core_801C5390)(rdram,&c);
                else if(zero_cue)(airborne?dora3_s3_core_801C5024:dora3_s3_core_801C4DA8)(rdram,&c);
                else (airborne?dora3_s3_core_801C4BBC:dora3_s3_core_801C49A0)(rdram,&c);
                assert(MEM_W(0x94,int32_t(player))==(held_cue?(airborne?29:28):zero_cue?(airborne?25:24):(airborne?22:21)));
                using PoolSample=std::array<std::array<double,9>,20>;
                auto read_pool=[&](uint8_t* buffer) {
                    auto* rdram=buffer;PoolSample result{};
                    for(unsigned i=0;i<result.size();++i) {
                        const auto actor=effect+i*0xCC;
                        if(!MEM_W(0x94,int32_t(actor)))continue;
                        const auto physical=uint32_t(MEM_W(0x98,int32_t(actor)));
                        result[i]={1.,double(MEM_H(0x20,int32_t(physical))),rf(rdram,actor+0x9C),rf(rdram,actor+0xA0),rf(rdram,actor+0xA4),rf(rdram,actor+0x18),rf(rdram,physical+0x14),double(MEM_H(0x34,int32_t(physical))),rf(rdram,physical+0x58)};
                    }
                    return result;
                };
                const auto heap_before=allocations;const auto address_before=next_address;
                auto legacy=memory;auto* old_ram=reinterpret_cast<uint8_t*>(legacy.data());
                auto source=[&](unsigned tick) {
                    input_now=(second*divider*tick+59)/60;c=saved;c.f_odd=&c.f0.u32h;c.r4=1;
                    dora3_s3_801C8B28(old_ram,&c);reference_dora3_s3_801D10D8(old_ram,&c);
                };
                weapon_watch=true;weapon_fires=0;source(0);const auto first_reference=legacy;
                auto read_wait=[&](uint8_t* buffer) {
                    auto* rdram=buffer;const auto bone=uint32_t(MEM_W(0x5C,int32_t(owners[0])));
                    return std::array<int32_t,3>{int32_t(bool(MEM_BU(0x7C,int32_t(owners[0]))&8)),MEM_W(0,int32_t(0x8021D4C0)),MEM_W(0x78,int32_t(bone))};
                };
                std::vector<PoolSample> expected;
                std::vector<std::array<int32_t,3>> expected_wait;
                for(unsigned tick=1;tick<=120;++tick) {
                    source(tick);expected.push_back(read_pool(old_ram));
                    if(weapon==13)expected_wait.push_back(read_wait(old_ram));
                }
                assert(weapon_fires==1 && (resource_references==std::array<unsigned,5>{}));
                const auto remaining_source_allocations=allocations.size();
                const auto expected_shot=weapon_fired_at;
                allocations=heap_before;next_address=address_before;input_now=0;weapon_fires=0;
                c=saved;c.f_odd=&c.f0.u32h;c.r4=1;game_model_animation().hook(rdram,&c,0x80030850,0);
                dora3_s3_801C8B28(rdram,&c);dora3_s3_801D10D8(rdram,&c);
                assert(memory==first_reference);
                preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                assert(rf(rdram,owners[0]+0x64)==0);
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                for(unsigned tick=1;tick<=expected.size();++tick) {
                    to((second*divider*tick+59)/60);const auto actual=read_pool(rdram);
                    if(weapon==13 && read_wait(rdram)!=expected_wait[tick-1]) {
                        const auto wait=read_wait(rdram);
                        std::cerr<<"Paused weapon command mismatch fps="<<fps<<" divider="<<divider<<" airborne="<<airborne<<" duration="<<effect_duration<<" tick="<<tick<<" actual:";
                        for(auto v:wait)std::cerr<<' '<<v;
                        std::cerr<<" expected:";for(auto v:expected_wait[tick-1])std::cerr<<' '<<v;std::cerr<<'\n';return 1;
                    }
                    for(unsigned slot=0;slot<actual.size();++slot)for(unsigned field=0;field<actual[slot].size();++field) {
                        // Kind89 stores its launch X at physical58; other
                        // families store alpha there. Use the matching units.
                        const double tolerance=(field==0||field==1||field==7)?0.:field==2||field==3||field==4||(field==8&&actual[slot][1]==89)?0.15:0.002;
                        if(std::abs(actual[slot][field]-expected[tick-1][slot][field])>tolerance) {
                            std::cerr<<"Moving weapon mismatch fps="<<fps<<" divider="<<divider<<" weapon="<<weapon<<" airborne="<<airborne<<" duration="<<effect_duration<<" tick="<<tick<<" slot="<<slot<<" field="<<field<<" actual="<<actual[slot][field]<<" expected="<<expected[tick-1][slot][field]<<'\n';
                            for(unsigned j=0;j<20;++j)if(actual[j][0]||expected[tick-1][j][0]) {
                                std::cerr<<"pool "<<j<<" actual:";for(auto v:actual[j])std::cerr<<' '<<v;
                                std::cerr<<" expected:";for(auto v:expected[tick-1][j])std::cerr<<' '<<v;std::cerr<<'\n';
                            }
                            return 1;
                        }
                    }
                }
                assert(weapon_fires==1 && std::abs(weapon_fired_at-expected_shot)<2e-7 && (resource_references==std::array<unsigned,5>{}));
                for(unsigned i=0;i<20;++i)assert(!MEM_W(0x94,int32_t(effect+i*0xCC))&&!MEM_W(0x98,int32_t(effect+i*0xCC)));
                // Returning to idle/landing binds a real new player clip;
                // its channel addresses can change. The complete owner
                // destruction below must still leave an empty guest heap.
                assert(allocations.size()==remaining_source_allocations && game_model_animation().model_count()==2);
                for(auto model:owners)assert(allocations.count(model));
                weapon_watch=false;native_gameplay_preview_reset(rdram);preview_live=false;
                MEM_W(0,int32_t(0x801B36E0))=0;++moving_weapon_cases;
            }
            wf(rdram,effect_clip+0x204,3);wf(rdram,effect_clip+0x220,3);
            // The same full Source pool comparison joins actual flight
            // weapon9/10 action27 and flight charge11/12 action20.
            for(unsigned weapon:{9u,10u,11u,12u})for(bool airborne:{true})for(float effect_duration:{3.f,7.f}) {
                wf(rdram,effect_clip+0x204,effect_duration);wf(rdram,effect_clip+0x220,effect_duration);
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                MEM_W(0,int32_t(0x801B36E0))=int(weapon);
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_B(0x28,int32_t(control))=1;MEM_H(0x2A,int32_t(control))=1000;MEM_H(0xA,int32_t(params))=3;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                MEM_W(0x94,int32_t(player))=1;
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,airborne?150:0);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);
                wf(rdram,physics,1.5f);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);wf(rdram,physics+0x14,airborne?10:0);
                for(unsigned i=30;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                c=saved;c.f_odd=&c.f0.u32h;
                const bool zero_cue=weapon==9 || weapon==10;
                (zero_cue?dora3_s3_core_801C5254:dora3_s3_core_801C489C)(rdram,&c);
                assert(MEM_W(0x94,int32_t(player))==(zero_cue?27:20));
                using PoolSample=std::array<std::array<double,9>,20>;
                auto read_pool=[&](uint8_t* buffer) {
                    auto* rdram=buffer;PoolSample result{};
                    for(unsigned i=0;i<result.size();++i) {
                        const auto actor=effect+i*0xCC;
                        if(!MEM_W(0x94,int32_t(actor)))continue;
                        const auto physical=uint32_t(MEM_W(0x98,int32_t(actor)));
                        result[i]={1.,double(MEM_H(0x20,int32_t(physical))),rf(rdram,actor+0x9C),rf(rdram,actor+0xA0),rf(rdram,actor+0xA4),rf(rdram,actor+0x18),rf(rdram,physical+0x14),double(MEM_H(0x34,int32_t(physical))),rf(rdram,physical+0x58)};
                    }
                    return result;
                };
                const auto heap_before=allocations;const auto address_before=next_address;
                auto legacy=memory;auto* old_ram=reinterpret_cast<uint8_t*>(legacy.data());
                auto source=[&](unsigned tick) {
                    input_now=(second*divider*tick+59)/60;c=saved;c.f_odd=&c.f0.u32h;c.r4=1;
                    dora3_s3_801C8B28(old_ram,&c);reference_dora3_s3_801D10D8(old_ram,&c);
                };
                weapon_watch=true;weapon_fires=0;source(0);const auto first_reference=legacy;
                auto read_wait=[&](uint8_t* buffer) {
                    auto* rdram=buffer;const auto bone=uint32_t(MEM_W(0x5C,int32_t(owners[0])));
                    return std::array<int32_t,3>{int32_t(bool(MEM_BU(0x7C,int32_t(owners[0]))&8)),MEM_W(0,int32_t(0x8021D4C0)),MEM_W(0x78,int32_t(bone))};
                };
                std::vector<PoolSample> expected;
                std::vector<std::array<int32_t,3>> expected_wait;
                for(unsigned tick=1;tick<=120;++tick) {
                    source(tick);expected.push_back(read_pool(old_ram));
                    if(weapon==13)expected_wait.push_back(read_wait(old_ram));
                }
                assert(weapon_fires==1 && (resource_references==std::array<unsigned,5>{}));
                const auto remaining_source_allocations=allocations.size();
                const auto expected_shot=weapon_fired_at;
                allocations=heap_before;next_address=address_before;input_now=0;weapon_fires=0;
                c=saved;c.f_odd=&c.f0.u32h;c.r4=1;game_model_animation().hook(rdram,&c,0x80030850,0);
                dora3_s3_801C8B28(rdram,&c);dora3_s3_801D10D8(rdram,&c);
                assert(memory==first_reference);
                preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                assert(rf(rdram,owners[0]+0x64)==0);
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                for(unsigned tick=1;tick<=expected.size();++tick) {
                    to((second*divider*tick+59)/60);const auto actual=read_pool(rdram);
                    if(weapon==13 && read_wait(rdram)!=expected_wait[tick-1]) {
                        const auto wait=read_wait(rdram);
                        std::cerr<<"Paused weapon command mismatch fps="<<fps<<" divider="<<divider<<" airborne="<<airborne<<" duration="<<effect_duration<<" tick="<<tick<<" actual:";
                        for(auto v:wait)std::cerr<<' '<<v;
                        std::cerr<<" expected:";for(auto v:expected_wait[tick-1])std::cerr<<' '<<v;std::cerr<<'\n';return 1;
                    }
                    for(unsigned slot=0;slot<actual.size();++slot)for(unsigned field=0;field<actual[slot].size();++field) {
                        // Kind89 stores its launch X at physical58; other
                        // families store alpha there. Use the matching units.
                        const double tolerance=(field==0||field==1||field==7)?0.:field==2||field==3||field==4||(field==8&&actual[slot][1]==89)?0.15:0.002;
                        if(std::abs(actual[slot][field]-expected[tick-1][slot][field])>tolerance) {
                            std::cerr<<"Moving weapon mismatch fps="<<fps<<" divider="<<divider<<" weapon="<<weapon<<" airborne="<<airborne<<" duration="<<effect_duration<<" tick="<<tick<<" slot="<<slot<<" field="<<field<<" actual="<<actual[slot][field]<<" expected="<<expected[tick-1][slot][field]<<'\n';
                            for(unsigned j=0;j<20;++j)if(actual[j][0]||expected[tick-1][j][0]) {
                                std::cerr<<"pool "<<j<<" actual:";for(auto v:actual[j])std::cerr<<' '<<v;
                                std::cerr<<" expected:";for(auto v:expected[tick-1][j])std::cerr<<' '<<v;std::cerr<<'\n';
                            }
                            return 1;
                        }
                    }
                }
                assert(weapon_fires==1 && std::abs(weapon_fired_at-expected_shot)<2e-7 && (resource_references==std::array<unsigned,5>{}));
                for(unsigned i=0;i<20;++i)assert(!MEM_W(0x94,int32_t(effect+i*0xCC))&&!MEM_W(0x98,int32_t(effect+i*0xCC)));
                // Returning to idle/landing binds a real new player clip;
                // its channel addresses can change. The complete owner
                // destruction below must still leave an empty guest heap.
                assert(allocations.size()==remaining_source_allocations && game_model_animation().model_count()==2);
                for(auto model:owners)assert(allocations.count(model));
                weapon_watch=false;native_gameplay_preview_reset(rdram);preview_live=false;
                MEM_W(0,int32_t(0x801B36E0))=0;++flight_weapon_cases;
                MEM_B(0x28,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;
            }
            wf(rdram,effect_clip+0x204,3);wf(rdram,effect_clip+0x220,3);
            for(unsigned weapon:{1u,2u,3u,4u,5u,6u,7u,8u,13u})for(bool airborne:{false,true}) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                // Keep initial-age observations in free space; the separate
                // full-pool Source comparison exercises the low ceiling hits.
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=int(weapon);
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                MEM_W(0x94,int32_t(player))=1;
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,airborne?150:0);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);
                wf(rdram,physics,0);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);wf(rdram,physics+0x14,airborne?10:0);
                for(unsigned i=30;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                c=saved;c.f_odd=&c.f0.u32h;
                (airborne?dora3_s3_core_801C383C:dora3_s3_801C3124)(rdram,&c);
                MEM_B(3,int32_t(control))=airborne?1:0;
                c.r4=1;game_model_animation().hook(rdram,&c,0x80030850,0);
                dora3_s3_801C8B28(rdram,&c);dora3_s3_801D10D8(rdram,&c);
                initial_geometry_watch=true;initial_shot_position.reset();
                weapon_watch=true;weapon_fires=0;preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                constexpr Nanoseconds press=73'000'000,release=173'000'000;
                const bool release_weapon=weapon==4||weapon==7||weapon==8;
                const bool zero_cue=weapon==3||(weapon>=6&&weapon<=8);
                to(press);MEM_H(0,int32_t(0x80171560))=int16_t(0x4000);dora3_controller_accumulate(rdram,&c);
                Nanoseconds trigger=press;
                if(release_weapon) {
                    to(release);assert(weapon_fires==0);
                    MEM_H(0,int32_t(0x80171560))=0;dora3_controller_accumulate(rdram,&c);trigger=release;
                }
                const unsigned cue_periods=weapon==1||weapon==13?6:5;
                const Nanoseconds shot=trigger+(zero_cue?0:(second*divider*cue_periods+59)/60);
                std::optional<std::array<float,3>> attachment_reference;
                if(weapon==1||weapon==2||weapon==4||weapon==5) {
                    // Independent Original ctor/pool pass at the position of
                    // the preceding action interval, including Native entry
                    // between global Source ticks. No rendering is involved.
                    to(trigger+(second*divider*(cue_periods-1)+59)/60);
                    auto scratch=memory;auto* source_ram=reinterpret_cast<uint8_t*>(scratch.data());
                    const auto heap_before=allocations;const auto address_before=next_address;
                    const auto references_before=resource_references;
                    preview_live=weapon_watch=false;
                    auto reference_context=saved;reference_context.f_odd=&reference_context.f0.u32h;
                    dora3_s3_core_801C120C(source_ram,&reference_context);
                    reference_dora3_s3_801D10D8(source_ram,&reference_context);
                    attachment_reference=std::array<float,3>{rf(source_ram,effect+0x9C),rf(source_ram,effect+0xA0),rf(source_ram,effect+0xA4)};
                    for(unsigned i=0;i<20;++i) {
                        auto* rdram=source_ram;const auto actor=effect+i*0xCC;
                        if(MEM_W(0x94,int32_t(actor))) {
                            reference_context.r4=int32_t(actor);reference_context.r5=0xFFFF;
                            dora3_s0_8002EA4C(rdram,&reference_context);
                        }
                    }
                    assert(allocations==heap_before && resource_references==references_before && game_model_animation().model_count()==2);
                    next_address=address_before;preview_live=weapon_watch=true;
                }
                // Observe zero-cue entry on the normal next variable frame,
                // without inserting a one-nanosecond update at the press.
                const Nanoseconds observed=zero_cue?second*(uint64_t(trigger)*fps/second+1)/fps:shot;
                to(observed);
                if(weapon_fires!=1 || std::abs(weapon_fired_at-double(shot)/second)>2e-7) {
                    std::cerr<<"Native moving B trigger mismatch fps="<<fps<<" divider="<<divider<<" weapon="<<weapon<<" airborne="<<airborne<<" fires="<<weapon_fires<<" at="<<weapon_fired_at<<" expected="<<double(shot)/second<<'\n';return 1;
                }
                const auto physical=uint32_t(MEM_W(0x98,int32_t(effect))),model=uint32_t(MEM_W(0x74,int32_t(effect)));
                const int kind=weapon==1?82:weapon==2?84:weapon==3?86:weapon==4?88:weapon==5?90:weapon==6?91:weapon==7?93:weapon==8?95:105;
                const double expected_age=3.*double(observed-shot)*60/(second*divider);
                if(!MEM_W(0x94,int32_t(effect)) || !physical || !model) {
                    std::cerr<<"Native moving B missing initial owner fps="<<fps<<" divider="<<divider<<" weapon="<<weapon<<" airborne="<<airborne<<'\n';return 1;
                }
                const bool animated=MEM_W(0x60,int32_t(model))!=0;
                if(MEM_H(0x20,int32_t(physical))!=kind || std::abs(rf(rdram,model+0x64)-(animated?expected_age:0.))>2e-5) {
                    std::cerr<<"Native moving B initial-age mismatch fps="<<fps<<" divider="<<divider<<" weapon="<<weapon<<" airborne="<<airborne<<" age="<<rf(rdram,model+0x64)<<" expected="<<expected_age<<'\n';return 1;
                }
                assert(initial_shot_position);
                if(attachment_reference)for(unsigned axis=0;axis<3;++axis) {
                    if(std::abs((*initial_shot_position)[axis]-(*attachment_reference)[axis])>0.015) {
                        std::cerr<<"Native action attachment mismatch fps="<<fps<<" divider="<<divider<<" weapon="<<weapon<<" airborne="<<airborne<<" axis="<<axis<<" actual="<<(*initial_shot_position)[axis]<<" expected="<<(*attachment_reference)[axis]<<'\n';return 1;
                    }
                }
                if(weapon==13)assert((MEM_BU(0x7C,int32_t(owners[0]))&8) && rf(rdram,owners[0]+0x64)==18.f && MEM_W(0,int32_t(0x8021D4C0))==1);
                if(zero_cue) {
                    // Independent Source geometry commands on a scratch copy
                    // provide projected v0/v1; elapsed time comes only from
                    // the input timestamp. Static models have no clip clock.
                    auto scratch=memory;auto* scratch_ram=reinterpret_cast<uint8_t*>(scratch.data());
                    auto* live_ram=rdram;rdram=scratch_ram;
                    for(unsigned i=0;i<initial_shot_physics.size();++i)MEM_W(int32_t(4*i),int32_t(physical))=initial_shot_physics[i];
                    auto projection=c;projection.f_odd=&projection.f0.u32h;projection.r4=int32_t(physical);
                    dora3_s3_core_801CB2F8(rdram,&projection);projection.r4=int32_t(physical);dora3_s3_core_801CB3A8(rdram,&projection);
                    std::array<float,3> v0{},v1{};
                    for(unsigned axis=0;axis<3;++axis)v0[axis]=v1[axis]=rf(rdram,physical+0x10+4*axis);
                    if(kind==86) {
                        union {uint64_t bits;double value;} radians;radians.bits=LD(0,int32_t(0x80210858));
                        projection.f12.fl=float(std::fmod(MEM_H(0x36,int32_t(physical))+40.,360.)*radians.value);
                        dora3_lib_80057A80(rdram,&projection);
                        float pitch=std::fmod(rf(rdram,physical+0x58)+projection.f0.fl*30.f,360.f);if(pitch<0)pitch+=360;
                        wf(rdram,physical+0xC,pitch);projection.r4=int32_t(physical);dora3_s3_core_801CB2F8(rdram,&projection);
                        projection.r4=int32_t(physical);dora3_s3_core_801CB3A8(rdram,&projection);
                        for(unsigned axis=0;axis<3;++axis)v1[axis]=rf(rdram,physical+0x10+4*axis);
                    }
                    rdram=live_ram;const double age=expected_age/3;
                    for(unsigned axis=0;axis<3;++axis) {
                        const double expected_position=(*initial_shot_position)[axis]+v0[axis]*age+(v1[axis]-v0[axis])*age*(age-1)*0.5;
                        if(std::abs(rf(rdram,effect+0x9C+4*axis)-expected_position)>0.015) {
                            std::cerr<<"Native moving B elapsed trajectory mismatch fps="<<fps<<" divider="<<divider<<" weapon="<<weapon<<" airborne="<<airborne<<" axis="<<axis<<" actual="<<rf(rdram,effect+0x9C+4*axis)<<" expected="<<expected_position<<'\n';return 1;
                        }
                    }
                }
                initial_geometry_watch=false;
                MEM_H(0,int32_t(0x80171560))=0;dora3_controller_accumulate(rdram,&c);
                to(shot+(second*divider*150+59)/60);
                assert(weapon_fires==1&&(resource_references==std::array<unsigned,5>{}));
                for(unsigned i=0;i<20;++i)assert(!MEM_W(0x94,int32_t(effect+i*0xCC))&&!MEM_W(0x98,int32_t(effect+i*0xCC)));
                assert(!allocations.count(physical)&&!allocations.count(model)&&game_model_animation().model_count()==2);
                if(weapon==13)assert(!(MEM_BU(0x7C,int32_t(owners[0]))&8) && !MEM_W(0,int32_t(0x8021D4C0)));
                weapon_watch=false;native_gameplay_preview_reset(rdram);preview_live=false;
                MEM_W(0,int32_t(0x801B36E0))=0;++native_moving_edge_cases;
            }
            // Actual off-grid B press/release invokes flight action27/20,
            // with birth time, initial elapsed trajectory and complete cleanup.
            for(unsigned weapon:{9u,10u,11u,12u})for(bool airborne:{true}) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                // Keep initial-age observations in free space; the separate
                // full-pool Source comparison exercises the low ceiling hits.
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=int(weapon);
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_B(0x28,int32_t(control))=1;MEM_H(0x2A,int32_t(control))=1000;MEM_H(0xA,int32_t(params))=3;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                MEM_W(0x94,int32_t(player))=1;
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,airborne?150:0);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);
                wf(rdram,physics,1.5f);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);wf(rdram,physics+0x14,airborne?10:0);
                for(unsigned i=30;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                c=saved;c.f_odd=&c.f0.u32h;
                MEM_W(0x8,int32_t(control))=-1;dora3_s3_801C768C(rdram,&c);
                MEM_B(3,int32_t(control))=airborne?1:0;
                c.r4=1;game_model_animation().hook(rdram,&c,0x80030850,0);
                dora3_s3_801C8B28(rdram,&c);dora3_s3_801D10D8(rdram,&c);
                initial_geometry_watch=true;initial_shot_position.reset();
                weapon_watch=true;weapon_fires=0;preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                constexpr Nanoseconds press=73'000'000,release=173'000'000;
                const bool release_weapon=weapon==10;
                const bool zero_cue=weapon==9||weapon==10;
                to(press);MEM_H(0,int32_t(0x80171560))=int16_t(0x4000);dora3_controller_accumulate(rdram,&c);
                Nanoseconds trigger=press;
                if(release_weapon) {
                    to(release);assert(weapon_fires==0);
                    MEM_H(0,int32_t(0x80171560))=0;dora3_controller_accumulate(rdram,&c);trigger=release;
                }
                const unsigned cue_periods=6;
                const Nanoseconds shot=trigger+(zero_cue?0:(second*divider*cue_periods+59)/60);
                std::optional<std::array<float,3>> attachment_reference;
                if(weapon==1||weapon==2||weapon==4||weapon==5) {
                    // Independent Original ctor/pool pass at the position of
                    // the preceding action interval, including Native entry
                    // between global Source ticks. No rendering is involved.
                    to(trigger+(second*divider*(cue_periods-1)+59)/60);
                    auto scratch=memory;auto* source_ram=reinterpret_cast<uint8_t*>(scratch.data());
                    const auto heap_before=allocations;const auto address_before=next_address;
                    const auto references_before=resource_references;
                    preview_live=weapon_watch=false;
                    auto reference_context=saved;reference_context.f_odd=&reference_context.f0.u32h;
                    dora3_s3_core_801C120C(source_ram,&reference_context);
                    reference_dora3_s3_801D10D8(source_ram,&reference_context);
                    attachment_reference=std::array<float,3>{rf(source_ram,effect+0x9C),rf(source_ram,effect+0xA0),rf(source_ram,effect+0xA4)};
                    for(unsigned i=0;i<20;++i) {
                        auto* rdram=source_ram;const auto actor=effect+i*0xCC;
                        if(MEM_W(0x94,int32_t(actor))) {
                            reference_context.r4=int32_t(actor);reference_context.r5=0xFFFF;
                            dora3_s0_8002EA4C(rdram,&reference_context);
                        }
                    }
                    assert(allocations==heap_before && resource_references==references_before && game_model_animation().model_count()==2);
                    next_address=address_before;preview_live=weapon_watch=true;
                }
                // Observe zero-cue entry on the normal next variable frame,
                // without inserting a one-nanosecond update at the press.
                const Nanoseconds observed=zero_cue?second*(uint64_t(trigger)*fps/second+1)/fps:shot;
                to(observed);
                if(weapon_fires!=1 || std::abs(weapon_fired_at-double(shot)/second)>2e-7) {
                    std::cerr<<"Native moving B trigger mismatch fps="<<fps<<" divider="<<divider<<" weapon="<<weapon<<" airborne="<<airborne<<" fires="<<weapon_fires<<" at="<<weapon_fired_at<<" expected="<<double(shot)/second<<'\n';return 1;
                }
                const auto physical=uint32_t(MEM_W(0x98,int32_t(effect))),model=uint32_t(MEM_W(0x74,int32_t(effect)));
                const int kind=weapon==9?97:weapon==10?99:weapon==11?101:103;
                const double expected_age=3.*double(observed-shot)*60/(second*divider);
                if(!MEM_W(0x94,int32_t(effect)) || !physical || !model) {
                    std::cerr<<"Native moving B missing initial owner fps="<<fps<<" divider="<<divider<<" weapon="<<weapon<<" airborne="<<airborne<<'\n';return 1;
                }
                const bool animated=MEM_W(0x60,int32_t(model))!=0;
                if(MEM_H(0x20,int32_t(physical))!=kind || std::abs(rf(rdram,model+0x64)-(animated?expected_age:0.))>2e-5) {
                    std::cerr<<"Native moving B initial-age mismatch fps="<<fps<<" divider="<<divider<<" weapon="<<weapon<<" airborne="<<airborne<<" age="<<rf(rdram,model+0x64)<<" expected="<<expected_age<<'\n';return 1;
                }
                assert(initial_shot_position);
                if(attachment_reference)for(unsigned axis=0;axis<3;++axis) {
                    if(std::abs((*initial_shot_position)[axis]-(*attachment_reference)[axis])>0.015) {
                        std::cerr<<"Native action attachment mismatch fps="<<fps<<" divider="<<divider<<" weapon="<<weapon<<" airborne="<<airborne<<" axis="<<axis<<" actual="<<(*initial_shot_position)[axis]<<" expected="<<(*attachment_reference)[axis]<<'\n';return 1;
                    }
                }
                if(weapon==13)assert((MEM_BU(0x7C,int32_t(owners[0]))&8) && rf(rdram,owners[0]+0x64)==18.f && MEM_W(0,int32_t(0x8021D4C0))==1);
                if(zero_cue) {
                    // Independent Source geometry commands on a scratch copy
                    // provide projected v0/v1; elapsed time comes only from
                    // the input timestamp. Static models have no clip clock.
                    auto scratch=memory;auto* scratch_ram=reinterpret_cast<uint8_t*>(scratch.data());
                    auto* live_ram=rdram;rdram=scratch_ram;
                    for(unsigned i=0;i<initial_shot_physics.size();++i)MEM_W(int32_t(4*i),int32_t(physical))=initial_shot_physics[i];
                    auto projection=c;projection.f_odd=&projection.f0.u32h;projection.r4=int32_t(physical);
                    dora3_s3_core_801CB2F8(rdram,&projection);projection.r4=int32_t(physical);dora3_s3_core_801CB3A8(rdram,&projection);
                    std::array<float,3> v0{},v1{};
                    for(unsigned axis=0;axis<3;++axis)v0[axis]=v1[axis]=rf(rdram,physical+0x10+4*axis);
                    if(kind==86) {
                        union {uint64_t bits;double value;} radians;radians.bits=LD(0,int32_t(0x80210858));
                        projection.f12.fl=float(std::fmod(MEM_H(0x36,int32_t(physical))+40.,360.)*radians.value);
                        dora3_lib_80057A80(rdram,&projection);
                        float pitch=std::fmod(rf(rdram,physical+0x58)+projection.f0.fl*30.f,360.f);if(pitch<0)pitch+=360;
                        wf(rdram,physical+0xC,pitch);projection.r4=int32_t(physical);dora3_s3_core_801CB2F8(rdram,&projection);
                        projection.r4=int32_t(physical);dora3_s3_core_801CB3A8(rdram,&projection);
                        for(unsigned axis=0;axis<3;++axis)v1[axis]=rf(rdram,physical+0x10+4*axis);
                    }
                    rdram=live_ram;const double age=expected_age/3;
                    for(unsigned axis=0;axis<3;++axis) {
                        const double expected_position=(*initial_shot_position)[axis]+v0[axis]*age+(v1[axis]-v0[axis])*age*(age-1)*0.5;
                        if(std::abs(rf(rdram,effect+0x9C+4*axis)-expected_position)>0.015) {
                            std::cerr<<"Native moving B elapsed trajectory mismatch fps="<<fps<<" divider="<<divider<<" weapon="<<weapon<<" airborne="<<airborne<<" axis="<<axis<<" actual="<<rf(rdram,effect+0x9C+4*axis)<<" expected="<<expected_position<<'\n';return 1;
                        }
                    }
                }
                initial_geometry_watch=false;
                MEM_H(0,int32_t(0x80171560))=0;dora3_controller_accumulate(rdram,&c);
                to(shot+(second*divider*150+59)/60);
                assert(weapon_fires==1&&(resource_references==std::array<unsigned,5>{}));
                for(unsigned i=0;i<20;++i)assert(!MEM_W(0x94,int32_t(effect+i*0xCC))&&!MEM_W(0x98,int32_t(effect+i*0xCC)));
                assert(!allocations.count(physical)&&!allocations.count(model)&&game_model_animation().model_count()==2);
                if(weapon==13)assert(!(MEM_BU(0x7C,int32_t(owners[0]))&8) && !MEM_W(0,int32_t(0x8021D4C0)));
                weapon_watch=false;native_gameplay_preview_reset(rdram);preview_live=false;
                MEM_W(0,int32_t(0x801B36E0))=0;++flight_native_input_cases;
                MEM_B(0x28,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;
            }
            for(unsigned weapon:{6u})for(bool airborne:{false,true}) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                // Keep initial-age observations in free space; the separate
                // full-pool Source comparison exercises the low ceiling hits.
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=int(weapon);
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                MEM_W(0x94,int32_t(player))=1;
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,airborne?150:0);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);
                wf(rdram,physics,0);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);wf(rdram,physics+0x14,airborne?10:0);
                for(unsigned i=30;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                c=saved;c.f_odd=&c.f0.u32h;
                (airborne?dora3_s3_core_801C383C:dora3_s3_801C3124)(rdram,&c);
                MEM_B(3,int32_t(control))=airborne?1:0;
                c.r4=1;game_model_animation().hook(rdram,&c,0x80030850,0);
                dora3_s3_801C8B28(rdram,&c);dora3_s3_801D10D8(rdram,&c);
                repeat_watch=weapon_watch=true;weapon_fires=0;repeat_shots.clear();
                preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                constexpr Nanoseconds press=73'000'000;
                to(press);MEM_H(0,int32_t(0x80171560))=int16_t(0x4000);dora3_controller_accumulate(rdram,&c);
                for(unsigned shot=0;shot<4;++shot) {
                    const Nanoseconds stamp=press+second*divider*6*shot/60;
                    to(shot?stamp:second*(uint64_t(press)*fps/second+1)/fps);
                    if(weapon_fires!=shot+1 || repeat_shots.size()!=shot+1 || std::abs(repeat_shots.back()-double(stamp)/second)>2e-7) {
                        std::cerr<<"Native held B repeat mismatch fps="<<fps<<" divider="<<divider<<" airborne="<<airborne<<" shot="<<shot<<" count="<<weapon_fires<<" last="<<(repeat_shots.empty()?-1:repeat_shots.back())<<" expected="<<double(stamp)/second<<'\n';return 1;
                    }
                    if(shot)assert(std::abs(rf(rdram,owners[0]+0x64))<1e-5);
                }
                MEM_H(0,int32_t(0x80171560))=0;dora3_controller_accumulate(rdram,&c);
                to(press+second*divider*160/60);
                assert(weapon_fires==4&&(resource_references==std::array<unsigned,5>{})&&game_model_animation().model_count()==2);
                for(unsigned i=0;i<20;++i)assert(!MEM_W(0x94,int32_t(effect+i*0xCC))&&!MEM_W(0x98,int32_t(effect+i*0xCC)));
                repeat_watch=weapon_watch=false;native_gameplay_preview_reset(rdram);preview_live=false;
                MEM_W(0,int32_t(0x801B36E0))=0;++repeat_weapon_cases;
            }
            // Actual held-B flight action27 repeats at18 with the shared
            // generated zero-cue cursor, including an exact-boundary update.
            for(unsigned weapon:{9u})for(bool airborne:{true}) {
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;room(rdram);
                // Keep initial-age observations in free space; the separate
                // full-pool Source comparison exercises the low ceiling hits.
                for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                MEM_W(0,int32_t(0x801B36E0))=int(weapon);
                std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(0x30,int32_t(control))=-1;
                wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
                MEM_B(0x28,int32_t(control))=1;MEM_H(0x2A,int32_t(control))=1000;MEM_H(0xA,int32_t(params))=3;
                MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);
                dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                MEM_W(0x94,int32_t(player))=1;
                wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,airborne?150:0);wf(rdram,player+0xA4,0);
                wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);
                wf(rdram,physics,1.5f);wf(rdram,physics+4,90);wf(rdram,physics+8,0);wf(rdram,physics+0xC,0);wf(rdram,physics+0x14,airborne?10:0);
                for(unsigned i=30;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                c=saved;c.f_odd=&c.f0.u32h;
                MEM_W(0x8,int32_t(control))=-1;dora3_s3_801C768C(rdram,&c);
                MEM_B(3,int32_t(control))=airborne?1:0;
                c.r4=1;game_model_animation().hook(rdram,&c,0x80030850,0);
                dora3_s3_801C8B28(rdram,&c);dora3_s3_801D10D8(rdram,&c);
                repeat_watch=weapon_watch=true;weapon_fires=0;repeat_shots.clear();
                preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned frame=0;
                auto to=[&](Nanoseconds stamp) {
                    while(second*(frame+1)/fps<stamp) {input_now=second*++frame/fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                    input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                };
                constexpr Nanoseconds press=73'000'000;
                to(press);MEM_H(0,int32_t(0x80171560))=int16_t(0x4000);dora3_controller_accumulate(rdram,&c);
                for(unsigned shot=0;shot<4;++shot) {
                    const Nanoseconds stamp=press+second*divider*6*shot/60;
                    to(shot?stamp:second*(uint64_t(press)*fps/second+1)/fps);
                    if(weapon_fires!=shot+1 || repeat_shots.size()!=shot+1 || std::abs(repeat_shots.back()-double(stamp)/second)>2e-7) {
                        std::cerr<<"Native held B repeat mismatch fps="<<fps<<" divider="<<divider<<" airborne="<<airborne<<" shot="<<shot<<" count="<<weapon_fires<<" last="<<(repeat_shots.empty()?-1:repeat_shots.back())<<" expected="<<double(stamp)/second<<'\n';return 1;
                    }
                    if(shot)assert(std::abs(rf(rdram,owners[0]+0x64))<1e-5);
                }
                MEM_H(0,int32_t(0x80171560))=0;dora3_controller_accumulate(rdram,&c);
                to(press+second*divider*160/60);
                assert(weapon_fires==4&&(resource_references==std::array<unsigned,5>{})&&game_model_animation().model_count()==2);
                for(unsigned i=0;i<20;++i)assert(!MEM_W(0x94,int32_t(effect+i*0xCC))&&!MEM_W(0x98,int32_t(effect+i*0xCC)));
                repeat_watch=weapon_watch=false;native_gameplay_preview_reset(rdram);preview_live=false;
                MEM_W(0,int32_t(0x801B36E0))=0;++flight_repeat_cases;
                MEM_B(0x28,int32_t(control))=0;MEM_H(0xA,int32_t(params))=0;
            }
            for(unsigned kind:{92u,94u,96u,98u,100u,102u,104u,106u,108u}) {
                terminal_end_commit=true;
                native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;
                MEM_H(0x14,int32_t(control))=0;MEM_W(0x94,int32_t(player))=1;
                std::memset(rdram+(effect&0xFFFFFF),0,20*0xCC);
                c.r4=0xF4;dora3_game_heap_alloc(rdram,&c);const auto effect_physics=uint32_t(c.r2);
                std::memset(rdram+(effect_physics&0xFFFFFF),0,0xF4);
                c.r4=int32_t(effect);c.r5=0;c.r6=int32_t(asset);dora3_s0_8002A100(rdram,&c);
                const auto effect_model=uint32_t(MEM_W(0x74,int32_t(effect)));
                c.r4=int32_t(effect_model);c.r5=int32_t(effect_clip);dora3_s0_8003B6BC(rdram,&c);
                MEM_B(0x7C,int32_t(effect_model))=1;
                MEM_W(0x94,int32_t(effect))=1;MEM_W(0x98,int32_t(effect))=int32_t(effect_physics);
                MEM_H(0xA8,int32_t(effect))=-1;MEM_H(0x20,int32_t(effect_physics))=int16_t(kind);
                wf(rdram,effect+0x9C,600);wf(rdram,effect+0xA0,100);
                game_model_animation().hook(rdram,&c,0x80030850,0);
                dora3_s3_801D10D8(rdram,&c);
                assert(MEM_W(0x94,int32_t(effect))==1 && rf(rdram,effect_model+0x64)==3);
                terminal_model=effect_model;terminal_deletes=0;terminal_deleted_at=-1;
                preview_live=true;native_gameplay_preview_begin(rdram,0,divider);unsigned effect_vi=0;
                for(unsigned frame=1;frame<=fps/2+1;++frame) {
                    while(uint64_t(effect_vi+1)*fps<=uint64_t(frame)*60) {
                        ++effect_vi;input_now=second*effect_vi/60;dora3_controller_accumulate(rdram,&c);
                        if(effect_vi%divider==0) {
                            dora3_controller_latch(rdram,&c);
                            dora3_native_preview_hook(rdram,&c,0x801BB080);
                            dora3_s3_801E8BDC(rdram,&c);c.r4=1;dora3_s3_801C8B28(rdram,&c);
                            dora3_s3_801D10D8(rdram,&c);dora3_s3_801DBD58(rdram,&c);
                        }
                    }
                    input_now=second*frame/fps;c=saved;c.f_odd=&c.f0.u32h;
                    native_gameplay_preview_update(rdram,&c,input_now);
                }
                if(terminal_deletes!=1 || std::abs(terminal_deleted_at-double(divider)/60)>1e-9) {
                    std::cerr<<"Terminal ownership lifetime mismatch divider="<<divider<<" fps="<<fps
                             <<" kind="<<kind<<" deletes="<<terminal_deletes<<" stamp="<<terminal_deleted_at
                             <<" expected="<<double(divider)/60<<'\n';return 1;
                }
                assert(MEM_W(0x94,int32_t(effect))==0 && MEM_W(0x98,int32_t(effect))==0);
                assert(!allocations.count(effect_physics)&&!allocations.count(effect_model));
                terminal_model=0;native_gameplay_preview_reset(rdram);preview_live=false;
                // Independently execute the frozen unpatched pool dispatcher
                // from spawn through its next source tick, with real model
                // and actor/physics retirement on the same synthetic assets.
                std::memset(rdram+(effect&0xFFFFFF),0,0xCC);
                c.r4=0xF4;dora3_game_heap_alloc(rdram,&c);const auto original_physics=uint32_t(c.r2);
                std::memset(rdram+(original_physics&0xFFFFFF),0,0xF4);
                c.r4=int32_t(effect);c.r5=0;c.r6=int32_t(asset);dora3_s0_8002A100(rdram,&c);
                terminal_model=uint32_t(MEM_W(0x74,int32_t(effect)));
                c.r4=int32_t(terminal_model);c.r5=int32_t(effect_clip);dora3_s0_8003B6BC(rdram,&c);
                MEM_B(0x7C,int32_t(terminal_model))=1;
                MEM_W(0x94,int32_t(effect))=1;MEM_W(0x98,int32_t(effect))=int32_t(original_physics);
                MEM_H(0xA8,int32_t(effect))=-1;MEM_H(0x20,int32_t(original_physics))=int16_t(kind);
                terminal_deletes=0;input_now=0;
                reference_dora3_s3_801D10D8(rdram,&c);assert(terminal_deletes==0);
                input_now=second*divider/60;reference_dora3_s3_801D10D8(rdram,&c);
                assert(terminal_deletes==1 && std::abs(terminal_deleted_at-double(divider)/60)<1e-9);
                assert(!allocations.count(original_physics)&&!allocations.count(terminal_model));
                assert(MEM_W(0x94,int32_t(effect))==0 && MEM_W(0x98,int32_t(effect))==0);
                terminal_model=0;++terminal_cases;
            }
            // Surfaces60..62: enter through the actual Ground/C2E94 producer,
            // execute full first/second/collision/model/origin/camera closures.
            // Source samples use exact rational endpoints every three periods.
            for(unsigned surface_state:{60u,61u,62u})for(unsigned scenario=0;scenario<3;++scenario) {
                native_gameplay_preview_reset(rdram);preview_live=false;
                const auto before_surface=memory;
                const auto surface_heap=allocations;const auto surface_next=next_address;
                const auto surface_refs=resource_references;
                auto run_surface=[&](bool native,unsigned render_fps,unsigned edge_mode=0) {
                    native_gameplay_preview_reset(rdram);preview_live=false;memory=before_surface;
                    allocations=surface_heap;next_address=surface_next;resource_references=surface_refs;input_now=0;
                    room(rdram);for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                    const unsigned tile=surface_state==60?0xF5:surface_state==61?0xF4:0xF6;
                    for(unsigned n=0;n<2;++n) {
                        const auto triangle=int32_t(0x80280000+0x20+n*44);
                        MEM_B(40,triangle)=int8_t(tile);MEM_B(41,triangle)=2;
                    }
                    std::memset(rdram+(physics&0xFFFFFF),0,0xF4);
                    std::memset(rdram+(control&0xFFFFFF),0,0x58);
                    MEM_W(8,int32_t(control))=-1;MEM_W(0x30,int32_t(control))=-1;
                    MEM_H(0x16,int32_t(control))=40;MEM_H(0x2A,int32_t(control))=1000;
                    wf(rdram,control+0x1C,1);MEM_H(0xA,int32_t(params))=3;
                    MEM_B(0,int32_t(0x801B36BA))=0;MEM_W(0,int32_t(0x801B36E0))=0;
                    MEM_W(0,int32_t(0x802170EC))=0;MEM_W(0,int32_t(0x802170F0))=0;
                    MEM_B(0,int32_t(0x80217118))=0;
                    MEM_W(0,int32_t(0x8021D4A8))=int32_t(0x80259000);
                    MEM_B(1,int32_t(0x80259000))=10;MEM_B(0x18,int32_t(params))=10;
                    for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                    wf(rdram,player+0x9C,0);wf(rdram,player+0xA0,0);wf(rdram,player+0xA4,0);
                    wf(rdram,player+0xC,0);wf(rdram,player+0x10,359);wf(rdram,player+0x14,0);
                    wf(rdram,physics+4,90);wf(rdram,clip+0x200+4,360);
                    MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                    c=saved;c.f_odd=&c.f0.u32h;dora3_controller_reset_accumulator(rdram,&c);
                    dora3_controller_accumulate(rdram,&c);dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                    dora3_s3_801C3124(rdram,&c);c.r4=1;dora3_s3_801C8B28(rdram,&c);
                    assert(MEM_W(0x94,int32_t(player))==int(surface_state));
                    assert(MEM_W(0,int32_t(0x8021D480))==2);
                    c.f12.fl=rf(rdram,player+0x9C);c.f14.fl=rf(rdram,player+0xA0);c.r6=MEM_W(0xA4,int32_t(player));
                    dora3_s3_801BBDDC(rdram,&c);dora3_s3_801DBD58(rdram,&c);
                    preview_live=native;if(native)native_gameplay_preview_begin(rdram,0,divider);
                    unsigned frame=0;std::vector<std::array<double,18>> values;
                    auto advance_to=[&](Nanoseconds stamp) {
                        while(second*(frame+1)/render_fps<stamp) {
                            input_now=second*++frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;
                            native_gameplay_preview_update(rdram,&c,input_now);
                        }
                        input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                    };
                    unsigned edge=0;
                    const std::array<Nanoseconds,3> edges{73'000'000,173'000'000,273'000'000};
                    auto to=[&](Nanoseconds stamp) {
                        while(edge_mode&&edge<edges.size()&&edges[edge]<=stamp) {
                            advance_to(edges[edge]);c=saved;c.f_odd=&c.f0.u32h;c.r4=0;
                            if(edge_mode==1&&edge<2)
                                (surface_state==60?dora3_s3_core_801C7C20:surface_state==61?dora3_s3_core_801C7D44:dora3_s3_core_801C7EE0)(rdram,&c);
                            else if(edge_mode==2&&edge==0) {
                                for(auto model:owners) {c.r4=int32_t(model);dora3_s0_8003B744(rdram,&c);c.r4=int32_t(model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);}
                            } else if(edge_mode==2&&edge==2) {
                                for(unsigned n=0;n<2;++n)MEM_B(40,int32_t(0x80280000+0x20+n*44))=0;
                            } else {
                                wf(rdram,player+0x10,5);
                                for(unsigned n=0;n<2;++n)MEM_B(41,int32_t(0x80280000+0x20+n*44))=7;
                            }
                            ++edge;
                        }
                        advance_to(stamp);
                    };
                    for(unsigned tick=1;tick<=36;++tick) {
                        if(tick==13&&scenario) {
                            for(unsigned n=0;n<2;++n) {
                                const auto triangle=int32_t(0x80280000+0x20+n*44);
                                if(scenario==1)MEM_B(40,triangle)=0;
                                else MEM_B(41,triangle)=7;
                            }
                            if(scenario==2)wf(rdram,player+0x10,5);
                        }
                        if(native)to((second*divider*tick+59)/60);
                        else {
                            input_now=second*divider*tick/60;c=saved;c.f_odd=&c.f0.u32h;c.r4=1;dora3_s3_801C8B28(rdram,&c);
                            c.f12.fl=rf(rdram,player+0x9C);c.f14.fl=rf(rdram,player+0xA0);c.r6=MEM_W(0xA4,int32_t(player));
                            dora3_s3_801BBDDC(rdram,&c);dora3_s3_801DBD58(rdram,&c);
                        }
                        if(tick%3==0) {
                            std::array<double,18> v{};
                            for(unsigned k=0;k<3;++k) {v[k]=rf(rdram,player+0x9C+4*k);v[6+k]=rf(rdram,physics+0x10+4*k);}
                            v[3]=rf(rdram,physics);v[4]=rf(rdram,physics+4);v[5]=rf(rdram,player+0x10);
                            v[9]=MEM_W(0x94,int32_t(player));v[10]=MEM_H(0x44,int32_t(physics));
                            v[11]=MEM_BU(3,int32_t(control));v[12]=MEM_H(0xD8,int32_t(physics));
                            v[13]=rf(rdram,owners[0]+0x64);v[14]=MEM_H(0x2A,int32_t(control));
                            v[15]=MEM_W(0,int32_t(0x8021D480));v[16]=MEM_W(0,int32_t(0x8021D4B8));v[17]=MEM_BU(1,int32_t(0x80259000));
                            values.push_back(v);
                        }
                    }
                    if(scenario==1||edge_mode==2)assert(MEM_W(0x94,int32_t(player))!=int(surface_state));
                    else assert(MEM_W(0x94,int32_t(player))==int(surface_state));
                    native_gameplay_preview_reset(rdram);preview_live=false;return values;
                };
                const auto expected=run_surface(false,60),actual=run_surface(true,fps);
                assert(actual.size()==expected.size());
                for(unsigned n=0;n<actual.size();++n)for(unsigned k=0;k<actual[n].size();++k) {
                    if(std::abs(actual[n][k]-expected[n][k])>(k<3?0.035:k<9||k==13?0.001:0.)) {
                        std::cerr<<"Surface Source mismatch state="<<surface_state<<" scenario="<<scenario<<" fps="<<fps<<" divider="<<divider<<" sample="<<n<<" field="<<k<<" actual="<<actual[n][k]<<" expected="<<expected[n][k]<<'\n';return 1;
                    }
                }
                if(scenario==0)for(unsigned edge_mode:{1u,2u}) {
                    const auto baseline=run_surface(true,60,edge_mode),varied=run_surface(true,fps,edge_mode);
                    assert(baseline.size()==varied.size());
                    for(unsigned n=0;n<varied.size();++n)for(unsigned k=0;k<varied[n].size();++k)
                        if(std::abs(varied[n][k]-baseline[n][k])>(k<3?0.035:k<9||k==13?0.001:0.)) {
                            std::cerr<<"Surface Native partition mismatch state="<<surface_state<<" edge_mode="<<edge_mode<<" fps="<<fps<<" divider="<<divider<<" sample="<<n<<" field="<<k<<" actual="<<varied[n][k]<<" baseline="<<baseline[n][k]<<'\n';return 1;
                        }
                    ++surface_native_cases;
                }
                memory=before_surface;allocations=surface_heap;next_address=surface_next;resource_references=surface_refs;
                ++surface_source_cases;
            }

            // Door59: real surface producer, actor pause, camera command writes,
            // game-over/special-room fades and room request. Room DMA/creation
            // is still the separately documented external boundary.
            for(unsigned door_scenario=0;door_scenario<4;++door_scenario) {
                native_gameplay_preview_reset(rdram);preview_live=false;
                const auto before_door=memory;const auto door_refs=resource_references;
                const auto door_heap=allocations;const auto door_next=next_address;
                auto run_door=[&](bool native,unsigned render_fps) {
                    native_gameplay_preview_reset(rdram);preview_live=false;memory=before_door;
                    allocations=door_heap;next_address=door_next;resource_references=door_refs;input_now=0;
                    door_watch=false;door_commands.clear();door_source_stamp={0,0};death_fade.reset();
                    room(rdram);for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                    for(unsigned n=0;n<2;++n) {const auto triangle=int32_t(0x80280000+0x20+n*44);MEM_B(40,triangle)=int8_t(0xF1);MEM_B(41,triangle)=door_scenario==2?0:1;}
                    constexpr uint32_t header=0x80259100,entries=0x80259200;
                    MEM_W(0,int32_t(0x8021D6B4))=int32_t(header);MEM_W(8,int32_t(header))=int32_t(entries);
                    MEM_W(0,int32_t(entries))=123;MEM_W(4,int32_t(entries))=456;MEM_W(8,int32_t(entries))=door_scenario==1||door_scenario==3?-1:123;MEM_W(12,int32_t(entries))=456;
                    MEM_H(0,int32_t(0x801B36B8))=door_scenario==2?169:1;
                    for(unsigned n=0;n<4;++n)MEM_W(int32_t(4*n),int32_t(0x801B3628))=0;
                    MEM_B(0,int32_t(0x801B36BA))=0;MEM_W(0,int32_t(0x801B36E0))=0;
                    MEM_W(0,int32_t(0x802170EC))=0;MEM_W(0,int32_t(0x802170F0))=0;MEM_B(0,int32_t(0x80217118))=0;
                    MEM_H(0,int32_t(0x8021811A))=0;MEM_H(0,int32_t(0x80217186))=0;
                    MEM_H(0,int32_t(0x8021719E))=0;MEM_H(0,int32_t(0x802171A4))=0;
                    MEM_H(0,int32_t(0x801AA660))=0;MEM_B(0,int32_t(0x801A99A0))=0;MEM_B(0,int32_t(0x80171580))=16;
                    MEM_W(0,int32_t(0x801A950C))=1;MEM_B(1,int32_t(0x80171C50))=door_scenario==3?1:2;
                    for(unsigned n=0;n<6;++n) {MEM_B(int32_t(2*n),int32_t(0x801B361B))=1;MEM_B(int32_t(2*n+1),int32_t(0x801B361B))=door_scenario==3&&n==1?10:0;}
                    MEM_W(0,int32_t(0x8021D4A8))=int32_t(0x80259000);MEM_B(1,int32_t(0x80259000))=10;MEM_B(0x18,int32_t(params))=10;
                    std::memset(rdram+(physics&0xFFFFFF),0,0xF4);std::memset(rdram+(control&0xFFFFFF),0,0x58);
                    MEM_W(8,int32_t(control))=-1;MEM_W(0x30,int32_t(control))=-1;MEM_H(0x16,int32_t(control))=40;MEM_H(0x2A,int32_t(control))=1000;
                    MEM_H(0xA,int32_t(params))=3;wf(rdram,control+0x1C,1);
                    for(unsigned i=10;i<230;++i)MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                    // Pause writes must really affect one active projectile and
                    // one active NPC physics record, with no model ownership.
                    for(unsigned i:{10u,30u}) {
                        const auto actor=int32_t(0x80171CD0+i*0xCC),physical=int32_t(0x80259400+(i==30?0x100:0));
                        MEM_W(0x94,actor)=1;MEM_W(0x98,actor)=physical;MEM_B(0xC9,physical)=0;
                    }
                    wf(rdram,player+0x9C,0);wf(rdram,player+0xA0,0);wf(rdram,player+0xA4,0);
                    wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);wf(rdram,physics+4,90);wf(rdram,clip+0x200+4,360);
                    MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                    c=saved;c.f_odd=&c.f0.u32h;dora3_controller_reset_accumulator(rdram,&c);
                    dora3_controller_accumulate(rdram,&c);dora3_controller_latch(rdram,&c);dora3_controller_reset_accumulator(rdram,&c);
                    dora3_s3_801C3124(rdram,&c);door_watch=true;c.r4=1;dora3_s3_801C8B28(rdram,&c);
                    if(MEM_W(0x94,int32_t(player))!=59||MEM_H(0x44,int32_t(physics))!=0) {std::cerr<<"Door entry scenario="<<door_scenario<<" native="<<native<<" state="<<MEM_W(0x94,int32_t(player))<<" count="<<MEM_H(0x44,int32_t(physics))<<" tile="<<MEM_W(0,int32_t(0x8021D4B8))<<" flags="<<MEM_W(0,int32_t(0x8021D68C))<<" room="<<MEM_H(0,int32_t(0x801B36B8))<<" active="<<unsigned(MEM_BU(0,int32_t(control)))<<" producer="<<MEM_W(0,int32_t(0x8021D6B4))<<"\n";std::abort();}
                    assert(MEM_BU(0xC9,int32_t(0x80259400))==1&&MEM_BU(0xC9,int32_t(0x80259500))==1);
                    // Remove instrumented actors after checking their real pause.
                    for(unsigned i:{10u,30u})MEM_W(0x94,int32_t(0x80171CD0+i*0xCC))=0;
                    c.f12.fl=rf(rdram,player+0x9C);c.f14.fl=rf(rdram,player+0xA0);c.r6=MEM_W(0xA4,int32_t(player));
                    dora3_s3_801BBDDC(rdram,&c);dora3_s3_801DBD58(rdram,&c);
                    preview_live=native;
                    if(native) {death_fade.emplace(death_read_fade(rdram),divider,0);native_gameplay_preview_begin(rdram,0,divider);}
                    unsigned frame=0;std::vector<std::array<double,14>> values;
                    auto to=[&](Nanoseconds stamp) {
                        while(second*(frame+1)/render_fps<stamp&&native_gameplay_preview_eligible(rdram)) {
                            input_now=second*++frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);
                        }
                        if(native_gameplay_preview_eligible(rdram)) {input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);}
                    };
                    for(unsigned tick=1;tick<=20;++tick) {
                        door_source_stamp={second*divider*(tick-1)/60,unsigned(second*divider*(tick-1)%60)};
                        if(native)to((second*divider*tick+59)/60);
                        else {
                            input_now=second*divider*tick/60;c=saved;c.f_odd=&c.f0.u32h;c.r4=1;
                            try {dora3_s3_801C8B28(rdram,&c);}catch(const DeathHandoff&) {}
                            if(native_gameplay_preview_eligible(rdram)) {
                                c.f12.fl=rf(rdram,player+0x9C);c.f14.fl=rf(rdram,player+0xA0);c.r6=MEM_W(0xA4,int32_t(player));
                                dora3_s3_801BBDDC(rdram,&c);dora3_s3_801DBD58(rdram,&c);
                            }
                        }
                        const bool stopped=!native_gameplay_preview_eligible(rdram);
                        if(tick%3==0||stopped) {
                            std::array<double,14> v{rf(rdram,player+0x9C),rf(rdram,player+0xA0),rf(rdram,player+0xA4),
                                double(MEM_H(0x44,int32_t(physics))),double(MEM_W(0,int32_t(0x802170EC))),double(MEM_W(0,int32_t(0x802170FC))),
                                double(MEM_W(0,int32_t(0x80217100))),double(MEM_W(0,int32_t(0x80217104))),double(MEM_W(0,int32_t(0x8021D474))),
                                double(MEM_W(0,int32_t(0x801A950C))),double(MEM_H(0,int32_t(0x80217186))),double(MEM_H(0,int32_t(0x8021811A))),
                                double(MEM_H(0,int32_t(0x8021719E))),double(MEM_H(0,int32_t(0x802171A4)))};
                            values.push_back(v);
                        }
                        if(stopped)break;
                    }
                    assert(!native_gameplay_preview_eligible(rdram));
                    if(door_scenario==0||door_scenario==2)assert(MEM_W(0,int32_t(0x802170EC))==1);
                    else assert(MEM_W(0,int32_t(0x801A950C))==(door_scenario==3?0x5A:0x1E));
                    native_gameplay_preview_reset(rdram);preview_live=false;death_fade.reset();door_watch=false;return values;
                };
                const auto expected=run_door(false,60);const auto expected_commands=door_commands;
                const auto actual=run_door(true,fps);assert(actual.size()==expected.size());
                if(door_commands!=expected_commands) {
                    std::cerr<<"Door command mismatch scenario="<<door_scenario<<" fps="<<fps<<" divider="<<divider<<'\n';return 1;
                }
                for(unsigned n=0;n<actual.size();++n)for(unsigned k=0;k<actual[n].size();++k)if(std::abs(actual[n][k]-expected[n][k])>(k<3?0.035:0.)) {
                    std::cerr<<"Door Source mismatch scenario="<<door_scenario<<" fps="<<fps<<" divider="<<divider<<" sample="<<n<<" field="<<k<<" actual="<<actual[n][k]<<" expected="<<expected[n][k]<<'\n';return 1;
                }
                memory=before_door;allocations=door_heap;next_address=door_next;resource_references=door_refs;++door_source_cases;
            }

            // Death50..56: actual full player first/second/collision/model bodies,
            // resident fade commands, game-over selection and EF5B0 scene cleanup.
            // Resource lookup/audio/stream free are explicit CPU boundaries.
            for(unsigned state:{50u,51u,52u,53u,54u,55u,56u,32u,35u,37u,40u,43u,46u,47u,48u,49u})for(unsigned mode=0;mode<(state==51?8u:state==55||state<50?4u:2u);++mode) {
                const bool damage=state<50,flying=state==48||(state==55&&mode>=2),hover=state==49||state==56;
                const bool remaining=damage?mode%2==1:flying?mode==3:mode!=0,respawn=state==51&&mode>=2;
                const unsigned character=respawn?mode-2:0;
                native_gameplay_preview_reset(rdram);preview_live=false;death_watch=false;death_fade.reset();
                respawn_watch=false;
                const auto before_death=memory;const auto allocations_before=allocations;const auto address_before_death=next_address;
                std::vector<std::array<double,18>> respawn_recovery_reference;
                auto run_death=[&](bool native,unsigned render_fps) {
                    native_gameplay_preview_reset(rdram);preview_live=false;memory=before_death;next_address=address_before_death;input_now=0;death_commands.clear();death_source_stamp={0,0};
                    assert(allocations==allocations_before);
                    respawn_watch=false;death_transition_site=0;
                    MEM_B(0,int32_t(0x801B374A))=int8_t(character);
                    room(rdram);for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                    const auto death_models=models(rdram,c);death_owned_models=death_models;
                    for(unsigned i=0;i<6;++i) {
                        MEM_B(int32_t(2*i),int32_t(0x801B361B))=1;
                        MEM_B(int32_t(2*i+1),int32_t(0x801B361B))=remaining&&i==1?10:0;
                    }
                    const auto character_status=int32_t(0x801B361B+2*character);
                    MEM_W(0,int32_t(0x8021D4A8))=character_status;MEM_B(0x18,int32_t(params))=10;
                    MEM_B(1,character_status)=10;
                    MEM_W(0,int32_t(0x801B36E0))=0;
                    std::memset(rdram+(control&0xFFFFFF),0,0x58);MEM_W(8,int32_t(control))=-1;MEM_W(0x30,int32_t(control))=-1;
                    MEM_H(0x2A,int32_t(control))=1000;MEM_H(0x16,int32_t(control))=40;wf(rdram,control+0x1C,1);
                    MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
                    dora3_controller_reset_accumulator(rdram,&c);dora3_controller_accumulate(rdram,&c);dora3_controller_latch(rdram,&c);
                    dora3_controller_reset_accumulator(rdram,&c);
                    wf(rdram,player+0x9C,500);wf(rdram,player+0xA0,hover||flying?1000:damage&&state!=32?0:150);wf(rdram,player+0xA4,0);
                    wf(rdram,player+0xC,0);wf(rdram,player+0x10,90);wf(rdram,player+0x14,0);
                    wf(rdram,physics,7.5f);wf(rdram,physics+4,90);wf(rdram,physics+8,state==56?10:0);wf(rdram,physics+0xC,state==56?90:0);
                    for(unsigned k=0;k<3;++k)wf(rdram,physics+0x10+4*k,0);
                    MEM_H(0xD8,int32_t(physics))=0;MEM_H(0x34,int32_t(physics))=0;MEM_H(0x42,int32_t(physics))=0;
                    MEM_H(0x44,int32_t(physics))=0;MEM_B(0x2C,int32_t(control))=0;MEM_B(0x28,int32_t(control))=flying;
                    for(auto address:{0x8021D484u,0x8021D494u})MEM_W(0,int32_t(address))=0;
                    MEM_W(0,int32_t(0x801A950C))=1;MEM_B(1,int32_t(0x80171C50))=2;
                    MEM_H(0,int32_t(0x801AA660))=0;MEM_B(0,int32_t(0x801A99A0))=0;MEM_B(0,int32_t(0x80171580))=16;
                    MEM_W(0,int32_t(0x800DAD28))=0;MEM_W(0,int32_t(0x800DAE64))=int32_t(0x80280000);
                    // Actor cleanup must really destroy two live guest models.
                    MEM_W(0,int32_t(0x800881CC))=1;MEM_W(0,int32_t(0x8008C1E0))=3;
                    for(unsigned i=0;i<2;++i) {
                        const auto carrier=int32_t(0x80171CD0+(i+1)*0xCC);
                        std::memset(rdram+(uint32_t(carrier)&0xFFFFFF),0,0xCC);
                        MEM_W(0x94,carrier)=1;MEM_W(0x74,carrier)=int32_t(death_models[i]);MEM_B(0x6B,carrier)=3;MEM_H(0xA8,carrier)=-1;
                    }
                    for(unsigned i=0;i<50;++i)MEM_H(int32_t(2*i),int32_t(0x80217120))=-1;
                    MEM_W(0,int32_t(0x800918B8))=int32_t(0x802F0100);MEM_W(0,int32_t(0x800DAE64))=int32_t(0x80280000);
                    // 3FF98's optional heap stream is absent in this CPU fixture.
                    MEM_W(0,int32_t(0x800FEC6C))=0;
                    const std::array<recomp_func_t*,7> constructors{dora3_s3_core_801C6F58,dora3_s3_core_801C7104,
                        dora3_s3_core_801C72DC,dora3_s3_core_801C7430,dora3_s3_core_801C74D8,dora3_s3_core_801C756C,dora3_s3_core_801C7600};
                    c=saved;c.f_odd=&c.f0.u32h;(flying?dora3_s3_801C768C:dora3_s3_801C3124)(rdram,&c);
                    // Acquire one full healthy Original update. Then issue the
                    // real death constructor at this completed command boundary.
                    c=saved;c.f_odd=&c.f0.u32h;c.r4=1;dora3_s3_801C8B28(rdram,&c);
                    c.f12.fl=rf(rdram,player+0x9C);c.f14.fl=rf(rdram,player+0xA0);c.r6=MEM_W(0xA4,int32_t(player));
                    dora3_s3_801BBDDC(rdram,&c);dora3_s3_801DBD58(rdram,&c);
                    MEM_B(1,character_status)=respawn?10:damage?1:0;MEM_B(0x2C,int32_t(control))=hover;
                    if(respawn) {
                        // Checked lookup ids come from the actual Original ROM
                        // lists and all six character model/clip offset tables.
                        respawn_resource_ids.clear();uint32_t table=0x801FA130;
                        const auto base=MEM_H(int32_t(2*character),int32_t(0x801FA1A4));
                        for(unsigned kind=1;kind<=4;++kind) {
                            while(MEM_H(0,int32_t(table))!=-1) {
                                respawn_resource_ids.emplace(kind,MEM_H(0,int32_t(table))+base);table+=2;
                            }
                            table+=2;
                        }
                        respawn_resource_ids.emplace(1,MEM_H(0,int32_t(0x801FA238+10*character)));
                        for(unsigned i=1;i<=4;++i)respawn_resource_ids.emplace(3,MEM_H(int32_t(2*i),int32_t(0x801FA238+10*character)));
                        respawn_resource_ids.emplace(1,MEM_H(0,int32_t(0x801FA274+4*character)));
                        respawn_resource_ids.emplace(2,MEM_H(2,int32_t(0x801FA274+4*character)));
                        MEM_W(0x98,int32_t(0x80171D9C))=int32_t(physics);
                        // Resource1 mesh and resource2 clips remain distinct.
                        MEM_W(0,int32_t(0x801AEC14))=int32_t(asset);
                        MEM_W(0,int32_t(0x801AEC28))=int32_t(clip);
                        // Actual C97E8/C03FC addresses character weapon nodes
                        //5..12. Supply a real13-node tree for those commands.
                        MEM_W(0x14,int32_t(asset))=0x1000;
                        std::memset(rdram+((asset+0x1000)&0xFFFFFF),0,13*48);
                        for(unsigned bone=0;bone<13;++bone) {
                            const auto node=asset+0x1000+48*bone;
                            for(unsigned offset:{0x1Cu,0x20u,0x24u})wf(rdram,node+offset,1);
                            MEM_W(0x2C,int32_t(node))=bone<12?1:0;
                        }
                        respawn_watch=true;
                    }
                    wf(rdram,physics+8,state==56?10:0);wf(rdram,physics+0xC,state==56?90:0);
                    c=saved;c.f_odd=&c.f0.u32h;
                    if(damage) {
                        native_damage_rumble_watch=true;native_damage_rumble_commands=0;
                        native_damage_rumble=state==32||state==35||state==46||state==47?std::array<unsigned,3>{10,2,2}:std::array<unsigned,3>{8,3,1};
                        c.r4=-2;dora3_s3_801C0738(rdram,&c);assert(MEM_B(1,character_status)==0);
                        wf(rdram,clip+0x200+4,mode<2?9.f:21.f);
                        MEM_H(0xD8,int32_t(physics))=10000;
                        switch(state) {
                        //35/48 are selected by actual C2978 in the next full
                        //player first callback, after this real health command.
                        case 35:case 48:break;
                        case 32:dora3_s3_core_801C59D8(rdram,&c);break;
                        case 37:dora3_s3_core_801C5FB4(rdram,&c);break;
                        case 40:dora3_s3_core_801C62F4(rdram,&c);break;
                        case 43:dora3_s3_core_801C66DC(rdram,&c);break;
                        case 46:dora3_s3_core_801C6A38(rdram,&c);break;
                        case 47:dora3_s3_core_801C6BDC(rdram,&c);break;
                        case 49:dora3_s3_core_801C6E28(rdram,&c);break;
                        default:assert(false);
                        }
                    } else constructors[state-50](rdram,&c);
                    if(!native_gameplay_preview_eligible(rdram)) {
                        std::cerr<<"Death acquisition failed entry="<<state<<" mode="<<mode<<" actual="<<MEM_W(0x94,int32_t(player))<<" Native="<<native<<" ctrl="<<MEM_BU(0x28,int32_t(control))<<","<<MEM_BU(0x2C,int32_t(control))<<" health="<<MEM_B(1,character_status)<<" request="<<MEM_W(0,int32_t(0x802170EC))<<" hold="<<MEM_W(0,int32_t(0x802170F0))<<'\n';std::abort();
                    }
                    death_watch=true;death_commands.clear();death_barriers=death_stream_frees=0;death_handler=nullptr;
                    if(native) {preview_live=true;native_gameplay_preview_begin(rdram,0,divider);death_fade.emplace(death_read_fade(rdram),divider,0);}
                    auto read=[&] {
                        std::array<double,18> v{};
                        for(unsigned k=0;k<3;++k) {v[k]=rf(rdram,player+0x9C+4*k);v[6+k]=rf(rdram,physics+0x10+4*k);}
                        v[3]=rf(rdram,physics);v[4]=rf(rdram,physics+4);v[5]=rf(rdram,physics+0x14);
                        v[9]=MEM_W(0x94,MEM_W(0,int32_t(0x8021D49C)));v[10]=MEM_H(0x42,int32_t(physics));v[11]=MEM_H(0x44,int32_t(physics));
                        v[12]=MEM_W(0,int32_t(0x8021D484));v[13]=MEM_W(0,int32_t(0x8021D494));
                        v[14]=death_read_fade(rdram).alpha;v[15]=death_read_fade(rdram).direction;
                        v[16]=MEM_W(0,int32_t(0x801A950C));v[17]=MEM_BU(0,int32_t(0x8021BE44));return v;
                    };
                    std::vector<std::array<double,18>> values;unsigned frame=0;
                    auto native_to=[&](Nanoseconds stamp) {
                        while(second*(frame+1)/render_fps<stamp&&native_gameplay_preview_eligible(rdram)&&!death_barriers) {
                            input_now=second*++frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);
                            if(death_fade)death_write_fade(rdram,death_fade->sample(death_read_fade(rdram),input_now));
                        }
                        if(native_gameplay_preview_eligible(rdram)&&!death_barriers) {input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);}
                        if(death_fade)death_write_fade(rdram,death_fade->sample(death_read_fade(rdram),stamp));
                    };
                    for(unsigned tick=1;tick<=160;++tick) {
                        const auto stamp=(second*divider*tick+59)/60;
                        if(native)native_to(stamp);
                        else {
                            const auto units=uint64_t(second)*divider*(tick-1);
                            death_source_stamp={Nanoseconds(units/60),unsigned(units%60)};
                            input_now=(second*divider*(tick-1)+59)/60;c=saved;c.f_odd=&c.f0.u32h;c.r4=1;
                            try {dora3_s3_801C8B28(rdram,&c);}catch(const DeathHandoff&) {}
                            input_now=stamp;
                            if(native_gameplay_preview_eligible(rdram))dora3_fade_tick(rdram,&c);
                        }
                        if(!native_gameplay_preview_eligible(rdram)||death_barriers) {values.push_back(read());break;}
                        if(tick%3==0)values.push_back(read());
                    }
                    assert(!native_gameplay_preview_eligible(rdram)||death_barriers);
                    if(respawn) {
                        assert(death_barriers==1&&!death_stream_frees&&death_transition_site==0x801C9BE0);
                        assert(uint32_t(MEM_W(0,int32_t(0x8021D49C)))==0x80171D9Cu&&uint32_t(MEM_W(0,int32_t(0x8021D640)))==0x80171E68u);
                        assert(uint32_t(MEM_W(0,int32_t(0x8021D6C0)))==0x8021D4D0u);
                        assert(uint32_t(MEM_W(0,int32_t(0x8021D648)))==0x801B8370u+28*character);
                        assert(uint32_t(MEM_W(0,int32_t(0x8021D4A8)))==uint32_t(character_status)&&MEM_B(1,character_status)==10);
                        assert(MEM_W(0x94,int32_t(0x80171D9C))==67&&MEM_H(0x40,int32_t(physics))==1&&MEM_H(0xD8,int32_t(physics))==30);
                        assert(MEM_W(0,int32_t(0x802170EC))==1&&MEM_W(0,int32_t(0x802170FC))==4&&uint32_t(MEM_W(0,int32_t(0x8021D474)))==0x80217104u);
                        for(auto model:death_models)assert(!allocations.count(model));
                        assert(game_model_animation().model_count()==4);
                        death_watch=false;death_fade.reset();preview_live=false;
                        // A synthetic room-ready boundary supplies collision
                        // assets; real DMA/room asset construction is not claimed.
                        // Pending requests/holds must not reacquire this live67.
                        assert(!native_gameplay_preview_eligible(rdram));
                        for(unsigned request:{1u,2u}) {
                            MEM_W(0,int32_t(0x802170EC))=int32_t(request);
                            assert(!native_gameplay_preview_eligible(rdram));
                        }
                        MEM_W(0,int32_t(0x802170EC))=0;MEM_W(0,int32_t(0x802170F0))=3;
                        for(unsigned hold=3;hold;--hold) {
                            assert(!native_gameplay_preview_eligible(rdram));
                            c=saved;c.f_odd=&c.f0.u32h;dora3_s3_801B8FB4(rdram,&c);assert(c.r2==hold-1);
                        }
                        MEM_B(0,int32_t(0x80217118))=1;assert(!native_gameplay_preview_eligible(rdram));
                        MEM_B(0,int32_t(0x80217118))=0;
                        constexpr uint32_t reborn=0x80171D9C,room_header=0x802F0200;
                        MEM_W(0,int32_t(0x8021D654))=int32_t(room_header);MEM_B(0x22,int32_t(room_header))=int8_t(camera_mode);
                        room(rdram);for(unsigned n=2;n<4;++n)wf(rdram,0x80280000+0x20+n*44+36,-10000);
                        wf(rdram,reborn+0x9C,500);wf(rdram,reborn+0xA0,0);wf(rdram,reborn+0xA4,0);wf(rdram,reborn+0x10,90);
                        c=saved;c.f_odd=&c.f0.u32h;dora3_s3_801C9C58(rdram,&c);
                        assert(MEM_W(0x94,int32_t(reborn))==67&&MEM_H(0xD8,int32_t(physics))==1);
                        // Actual completed Original player/origin/camera update
                        // returns67 to Ground before acquiring the new models.
                        c=saved;c.f_odd=&c.f0.u32h;c.r4=1;dora3_s3_801C8B28(rdram,&c);
                        c.f12.fl=rf(rdram,reborn+0x9C);c.f14.fl=rf(rdram,reborn+0xA0);c.r6=MEM_W(0xA4,int32_t(reborn));
                        dora3_s3_801BBDDC(rdram,&c);dora3_s3_801DBD58(rdram,&c);
                        assert(MEM_W(0x94,int32_t(reborn))==1&&native_gameplay_preview_eligible(rdram));
                        input_now=0;unsigned recovered_frame=0;
                        if(native){preview_live=true;native_gameplay_preview_begin(rdram,0,divider);}
                        auto recovery_read=[&] {
                            std::array<double,18> v{};
                            for(unsigned k=0;k<3;++k){v[k]=rf(rdram,reborn+0x9C+4*k);v[6+k]=rf(rdram,physics+0x10+4*k);v[9+k]=rf(rdram,reborn+0xC+4*k);}
                            v[3]=rf(rdram,physics);v[4]=rf(rdram,physics+4);v[5]=rf(rdram,physics+0xC);
                            v[12]=MEM_W(0x94,int32_t(reborn));v[13]=MEM_H(0xD8,int32_t(physics));
                            v[14]=rf(rdram,uint32_t(MEM_W(0x74,int32_t(reborn)))+0x64);v[15]=MEM_H(0x2A,MEM_W(0,int32_t(0x8021D6C0)));
                            v[16]=MEM_BU(1,character_status);v[17]=game_model_animation().model_count();return v;
                        };
                        for(unsigned tick=1;tick<=24;++tick) {
                            const Nanoseconds stamp=(second*divider*tick+59)/60;
                            if(native) {
                                while(second*(recovered_frame+1)/render_fps<stamp){input_now=second*++recovered_frame/render_fps;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,input_now);}
                                input_now=stamp;c=saved;c.f_odd=&c.f0.u32h;native_gameplay_preview_update(rdram,&c,stamp);
                            } else {
                                c=saved;c.f_odd=&c.f0.u32h;c.r4=1;dora3_s3_801C8B28(rdram,&c);
                                c.f12.fl=rf(rdram,reborn+0x9C);c.f14.fl=rf(rdram,reborn+0xA0);c.r6=MEM_W(0xA4,int32_t(reborn));
                                dora3_s3_801BBDDC(rdram,&c);dora3_s3_801DBD58(rdram,&c);
                            }
                            assert(native_gameplay_preview_eligible(rdram));
                            if(tick%3==0) {
                                const auto v=recovery_read();
                                if(!native)respawn_recovery_reference.push_back(v);
                                else for(unsigned k=0;k<v.size();++k)if(std::abs(v[k]-respawn_recovery_reference[tick/3-1][k])>(k<3?0.035:k<12||k==14?0.001:0.)) {
                                    std::cerr<<"Respawn reacquisition mismatch character="<<character<<" fps="<<fps<<" divider="<<divider<<" tick="<<tick<<" field="<<k<<" actual="<<v[k]<<" expected="<<respawn_recovery_reference[tick/3-1][k]<<'\n';std::abort();
                                }
                            }
                        }
                        if(native)++respawn_recovery_cases;
                        native_gameplay_preview_reset(rdram);preview_live=false;
                        // All new actor models/resource lists are genuinely
                        // retired again; no hand-written reference decrement.
                        c=saved;c.f_odd=&c.f0.u32h;dora3_s3_801C9794(rdram,&c);
                        assert((resource_references==std::array<unsigned,5>{}));
                    } else if(remaining) {
                        assert(death_barriers==1&&death_stream_frees==1);
                        assert(MEM_W(0,int32_t(0x8021D484))==3&&MEM_BU(0,int32_t(0x8021BE44))==(state==51?2:1));
                        for(auto model:death_models)assert(!allocations.count(model));
                        for(unsigned i=0;i<2;++i) {
                            const auto carrier=int32_t(0x80171CD0+(i+1)*0xCC);
                            assert(MEM_W(0x94,carrier)==0&&MEM_BU(0x6B,carrier)==0);
                        }
                    } else {
                        assert(!death_barriers&&!death_stream_frees&&MEM_W(0,int32_t(0x801A950C))==33);
                        for(auto model:death_models) {c.r4=int32_t(model);dora3_s0_8003B77C(rdram,&c);}
                    }
                    native_gameplay_preview_reset(rdram);preview_live=false;death_watch=false;death_fade.reset();respawn_watch=false;native_damage_rumble_watch=false;
                    assert(allocations==allocations_before&&game_model_animation().model_count()==2);return values;
                };
                const auto expected=run_death(false,60);
                const auto commands=death_commands;
                const auto actual=run_death(true,fps);
                const auto native_commands=death_commands;
                assert(actual.size()==expected.size());
                if(native_commands!=commands) {
                    std::cerr<<"Death command timestamp mismatch state="<<state<<" mode="<<mode<<" fps="<<fps<<" divider="<<divider<<'\n';
                    for(const auto& list:{commands,native_commands}) {
                        for(const auto& event:list)std::cerr<<std::hex<<event.site<<std::dec<<":"<<event.stamp.nanoseconds<<"+"<<event.stamp.sixtieths<<"/60 "<<event.a0<<","<<event.a1<<"; ";
                        std::cerr<<'\n';
                    }
                    return 1;
                }
                for(unsigned n=0;n<actual.size();++n)for(unsigned k=0;k<actual[n].size();++k) {
                    // Final resource handoff cancels motion before the new
                    // scene's body. Compare commands/counters, not a dead actor.
                    if(n+1==actual.size()&&(k<9||k==14||k==15))continue;
                    if(std::abs(actual[n][k]-expected[n][k])>(k<3?0.035:k<9?0.001:0.)) {
                        std::cerr<<"Death Source mismatch state="<<state<<" remaining="<<remaining<<" fps="<<fps<<" divider="<<divider<<" sample="<<n<<" field="<<k<<" actual="<<actual[n][k]<<" expected="<<expected[n][k]<<'\n';return 1;
                    }
                }
                memory=before_death;next_address=address_before_death;death_watch=false;death_fade.reset();++death_source_cases;
            }

            // Source character initialization cancels ownership before a subsequent step.
            MEM_W(0x94,int32_t(player))=0;assert(!native_gameplay_preview_eligible(rdram));
            native_gameplay_preview_reset(rdram);preview_live=false;
            dora3_controller_latch(rdram,&c);assert(MEM_HU(0,int32_t(0x801A48EC))==0);
            for(auto model:owners) {c.r4=int32_t(model);dora3_s0_8003B77C(rdram,&c);}
            assert(game_model_animation().model_count()==0 && allocations.empty());
            assert(interaction_cases==(cases+1)*5 && interaction_birth_cases==(cases+1)*2 && weapon_cases==(cases+1)*8 && moving_weapon_cases==(cases+1)*36 && native_weapon_cases==(cases+1)*4 && native_moving_edge_cases==(cases+1)*18 && repeat_weapon_cases==(cases+1)*2);++cases;
        }
    }
    const unsigned npc_contact_cases=check_npc_action_contacts(image);assert(npc_contact_cases==3192);
    assert(slot09_constructor_cases==cases*42&&slot09_constructor_skips==cases*12);
    assert(slot09_source_cases==cases*212&&slot09_native_cases==cases*30&&slot09_rejections==cases*24&&slot09_original_cases==cases*1436);
    assert(slot09_yaw_samples>=cases*8);
    assert(linked_constructor_cases==cases*2&&linked_source_cases==cases*42&&linked_native_cases==cases*30&&linked_rejections==cases*12&&linked_original_ram_cases==cases*282&&linked_contact_cases==cases*16);
    assert(city_dialogue_source_cases==cases*128&&city_dialogue_native_cases==cases*96&&city_dialogue_rejections==cases*16&&city_dialogue_original_ram_cases==cases*864&&city_dialogue_contact_cases==cases*192);
    assert(city_actual_constructor_cases==cases*48);
    assert(city_story_source_cases==cases*96&&city_story_native_cases==cases*48&&city_story_rejections==cases*32&&city_story_original_cases==cases*640);
    assert(city_special_source_cases==cases*32&&city_special_native_cases==cases*8&&city_special_rejections==cases*4&&city_special_original_cases==cases*220&&city_special_counter_samples==cases*12);
    std::cout<<"PASS: "<<slot09_constructor_cases<<" actual slot09 constructors; "<<slot09_source_cases<<" slot09 Source schedules; "<<slot09_native_cases<<" slot09 Native schedules; "<<slot09_rejections<<" slot09 rejections; "<<slot09_original_cases<<" slot09 frozen189 comparisons; "<<slot09_yaw_samples<<" slot09 intermediate yaw samples; "<<npc_contact_cases<<" common NPC B schedules; "<<city_actual_constructor_cases<<" actual city constructor/profile/Original comparisons; "<<city_story_source_cases<<" story100 Source schedules; "<<city_story_native_cases<<" story100 Native edge schedules; "<<city_story_rejections<<" story100 ownership rejections; "<<city_story_original_cases<<" story100 Original full-RAM/context/heap/resource/command/model-count comparisons; "<<city_special_source_cases<<" special200/201 Source schedules; "<<city_special_native_cases<<" special200/201 Native edge schedules; "<<city_special_rejections<<" special200/201 ownership rejections; "<<city_special_original_cases<<" special200/201 Original full-RAM/context/heap/resource/command/model-count comparisons; "<<city_special_counter_samples<<" dense Original special counter samples; "<<city_dialogue_contact_cases<<" city dialogue actual contact-producer Original comparisons; "<<city_dialogue_source_cases<<" city dialogue Source schedules; "<<city_dialogue_native_cases<<" city dialogue Native edge schedules; "<<city_dialogue_rejections<<" city dialogue overlay rejections; "<<city_dialogue_yaw_samples<<" city dialogue intermediate yaw samples; "<<city_dialogue_original_ram_cases<<" city dialogue Original full-RAM/context/heap/resource/command comparisons; "<<linked_constructor_cases<<" linked actual descriptor construction/attachment/retirement schedules; "<<linked_source_cases<<" linked Source schedules; "<<linked_native_cases<<" linked Native schedules; "<<linked_rejections<<" linked ownership rejections; "<<linked_yaw_samples<<" linked intermediate yaw samples; "<<linked_original_ram_cases<<" linked Original full-RAM/context/heap/resource/command comparisons; "<<linked_contact_cases<<" linked actual contact-producer Original comparisons; "<<patrol_source_cases<<" city patrol Source schedules; "<<patrol_native_cases<<" city patrol Native schedules; "<<patrol_rejections<<" city patrol overlay rejections; "<<patrol_position_samples<<" city intermediate position samples; "<<patrol_original_ram_cases<<" city Original full-RAM/context/command comparisons; "<<dialogue_actor_source_cases<<" dialogue75/76/77 Source outcome/bob/turn/contact/birth schedules; "<<dialogue_actor_native_cases<<" dialogue Native pause/reentry/rebind/write schedules; "<<dialogue_actor_rejections<<" dialogue overlay rejection schedules; "<<dialogue_actor_return_samples<<" dialogue signed return/wrap intermediate yaw samples; "<<city_source_cases<<" city7/9 Source projection/sprite/contact/dialogue/delay/event/retirement schedules; "<<city_native_cases<<" city Native off-grid schedules; "<<city_overlay_rejections<<" city overlay-identity rejection schedules; "<<world_source_cases<<" simple object Source schedules; "<<world_native_cases<<" simple object Native off-grid schedules; "<<orbit_source_cases<<" orbital Source/contact/birth/moving-target/AI-resume schedules; "<<orbit_native_cases<<" orbital Native pause/rebind/recreation/write/state schedules; "<<orbit_return_samples<<" orbital intermediate signed return/wrap yaw samples; "<<surface_native_cases<<" off-grid surface constructor/reentry/model-rebind/yaw/exit Native schedules; "<<surface_source_cases<<" full surface60..62 producer/spin/exit/kick/orientation Source schedules; "<<door_source_cases<<" connected door59 pause/camera/fade/room-request Source schedules; "<<death_source_cases<<" connected lethal32..49/death50..56/respawn Source/body/fade/scene/real-model-retirement schedules; "<<respawn_recovery_cases<<" six-character post-ready67/Ground/new-model reacquisition schedules; "<<hover_source_cases<<" healthy hover49/63 Source count/control/exit/landing schedules; "<<hover_native_cases<<" Native hover off-grid recovery schedules; "<<native_special_recovery_cases<<" Native constructor/idle/complete special64/67 recovery schedules; "<<special_recovery_cases<<" Source special64/67 character frame commands/clip/cancel/surface recovery schedules; "<<native_special_cases<<" Native off-grid special command/stop/turn/write/rebind schedules; "<<special_player_cases<<" Source special65/66 stop/turn/cancellation schedules; "<<recovery31_cases<<" recovery31 scalar stop/clip/return Source schedules; "<<waiting_damage_cases<<" waiting46/47 count/kick/yaw/recovery Source schedules; "<<hazard_damage_cases<<" hazard40..45 delay/kick/fall/collapse/rise/recovery Source schedules; "<<air_damage_cases<<" airborne ground damage kick/fall/collapse/rise/recovery Source schedules; "<<native_damage_cases<<" Native real damage/health/rumble/complete recovery partition schedules; "<<heavy_damage_cases<<" heavy damage kick/counter/fall/collapse/rise/recovery Source schedules; "<<ground_damage_cases<<" ground damage kick/landing/recovery/cooldown Source schedules; "<<flight_damage_cases<<" healthy flight damage speed/orientation/clip/return schedules; "<<flight_edge_cases<<" Native off-grid flight A/stick partition schedules; "<<flight_repeat_cases<<" held-B four-shot flight repeat/release/lifetime schedules; "<<flight_native_input_cases<<" Native flight weapon press/release/birth/trajectory/retirement schedules; "<<flight_weapon_cases<<" flight weapon9/10 and charge11/12 full Source pool schedules; "<<player_flight_cases<<" whole-player flight Source speed/orientation/collision/animation/A-transition/RNG schedules; "<<cases<<" full original ground/jump/fall/landing and "<<cases*2<<" reversal/braking dispatcher/input/drive/contact/model-clip/pose schedules; "<<flight_cases<<" flight/pause/hit/real-newborn/lifetime schedules; "<<growth_cases<<" growth/spin/fade schedules against Original at every source tick; "<<ballistic_cases<<" ballistic/countdown/real five-link-chain schedules; "<<phase_cases<<" real four-phase/rebind/opacity/lifetime schedules; "<<bounce_cases<<" bounce/real conversion/complete child lifetimes; "<<homing_cases<<" homing clocks/real impact/retirement schedules; "<<charge_cases<<" real charge/attachment/radius/opacity/conversion/retirement schedules; "<<charge_birth_cases<<" live player-producer birth/initial commands/zero-time clock/lifetime schedules; "<<interaction_cases<<" interaction free/capture/seek/spawn/drag/retirement schedules and Original quarter-interval capture geometry; "<<interaction_birth_cases<<" interaction live births including same-address captured NPC recreation; "<<weapon_cases<<" real charge weapon action/B-entry/shot/complete lifetime schedules; "<<native_weapon_cases<<" Native B-edge exact shot and birth-relative charge lifetime schedules; "<<repeat_weapon_cases<<" held B four-shot repeat/release/full retirement schedules; "<<native_moving_edge_cases<<" Native press/release moving-weapon trigger/initial-age/complete resource lifetimes; "<<moving_weapon_cases<<" moving weapon action/shot/full pool Source endpoint/complete child lifetime schedules; "<<terminal_cases<<" terminal-effect lifetimes/retirements against unpatched Original and "<<cases*38<<" full-RAM common-world Original comparisons; both source rates at 60/73/120/144/165/360 FPS; single held-button jump and landing, one transition per reversal state, free-bank pose restoration, source-time counters/sounds, legacy player suppression, coupled native props/source-owned AI command timestamps, opacity/blink/real flipbook cadence, contact-list preservation and context preservation; no game/GPU.\n";
}

// Synthetic loaded-module epoch; actual reload behavior is checked separately.
extern "C" uint64_t dora3_overlay_generation(uint32_t rom){return dora3_overlay_loaded(rom)?1:0;}
