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
#include <map>
using namespace dora3::native_time;
#include "player-fixture.hpp"
constexpr uint32_t other=0x80254000,asset=0x80100000,clip=0x80101000;
static bool preview_live=false;
#ifdef DORA3_STORY_EFFECT_POOL_CHECK
static bool flash_pool_reference=false;
#endif
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
static const std::set<std::pair<unsigned,int32_t>> linked_resource_ids{{1,692},{4,693},{1,712},{4,713},{1,710},{4,711},{1,1819},{2,1820},{1,906},{1,917},{1,964},{1,997},{1,1001},{2,354},{2,907},{2,908},{2,909},{2,910},{2,911},{2,912},{2,913},{2,914},{2,915},{2,916},{2,918},{2,919},{2,920},{2,921},{2,922},{2,923},{2,924},{2,925},{2,965},{2,966},{2,967},{2,968},{2,969},{2,970},{2,998},{2,1002},{2,1003},{2,1004},{2,1005},{2,1006},{2,1007},{3,983},{3,984},{3,999},{3,1000},{3,1009},{3,1010},{4,802},{1,777},{1,803},{1,804},{1,810},{1,816},{1,817},{1,830},{1,831},{1,846},{1,880},{1,893},{2,97},{2,99},{2,778},{2,779},{2,780},{2,781},{2,782},{2,783},{2,784},{2,785},{2,786},{2,787},{2,788},{2,789},{2,790},{2,791},{2,792},{2,793},{2,794},{2,795},{2,818},{2,819},{2,820},{2,821},{2,822},{2,823},{2,824},{2,832},{2,833},{2,834},{2,835},{2,836},{2,837},{2,838},{2,839},{2,840},{2,841},{2,847},{2,848},{2,849},{2,850},{2,851},{2,852},{2,853},{2,881},{2,882},{2,883},{2,884},{2,885},{2,886},{2,887},{2,888},{2,894},{2,895},{2,896},{2,897},{2,898},{2,899},{2,900},{2,933},{3,796},{3,797},{3,798},{3,799},{3,800},{3,801},{3,805},{3,806},{3,807},{3,808},{3,809},{3,811},{3,812},{3,813},{3,814},{3,815},{3,825},{3,826},{3,827},{3,828},{3,829},{3,842},{3,843},{3,844},{3,845},{3,854},{3,855},{3,856},{3,857},{3,858},{3,889},{3,890},{3,891},{3,892},{3,901},{3,902},{3,903},{3,904},{3,905},{4,802},{1,859},{1,869},{2,860},{2,861},{2,862},{2,863},{2,864},{2,865},{2,866},{2,867},{2,868},{2,870},{2,871},{2,872},{2,873},{3,874},{3,875},{3,876},{3,877},{3,878},{4,879},{1,0x476},{2,0x477}};
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

#if defined(DORA3_PERIODIC_EFFECT_SOURCE_CHECK)||defined(DORA3_PERIODIC_PRODUCER_CHECK)||defined(DORA3_PERIODIC_EFFECT_GUARD_CHECK)
    if(uint32_t(address)==0x801ED8ECu)return dora3_s3_core_801ED8EC;
    if(uint32_t(address)==0x801ED8F4u)return dora3_s3_core_801ED8F4;
    if(uint32_t(address)==0x801ED8FCu)return flash_pool_reference?reference_dora3_s3_core_801ED8FC:dora3_s3_core_801ED8FC;
#endif
    if(uint32_t(address)==0x801ECC6Cu)return dora3_s3_core_801ECC6C;
    if(uint32_t(address)==0x801ECBE4u)return dora3_s3_core_801ECBE4;
    if(uint32_t(address)==0x801ED020u)return dora3_s3_core_801ED020;
    if(uint32_t(address)==0x801ECFB4u)return dora3_s3_core_801ECFB4;
    if(uint32_t(address)==0x801ED244u)return dora3_s3_core_801ED244;
    if(uint32_t(address)==0x801ED228u)return dora3_s3_core_801ED228;
    if(uint32_t(address)==0x801ED898u) {
#ifdef DORA3_STORY_EFFECT_POOL_CHECK
        if(flash_pool_reference)return reference_dora3_s3_core_801ED898;
#endif
        return dora3_s3_core_801ED898;
    }
    if(uint32_t(address)==0x801ED4A4u) {
#ifdef DORA3_STORY_EFFECT_POOL_CHECK
        if(flash_pool_reference)return reference_dora3_s3_core_801ED4A4;
#endif
        return dora3_s3_core_801ED4A4;
    }
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
#ifdef DORA3_STORY_SOURCE_NATIVE_CHECK
static unsigned story_native_intervals=0,story_native_ctor_models=0;
#ifdef DORA3_STORY_EFFECT_POOL_CHECK
static unsigned reward_item_yaw_samples=0,reward_item_height_samples=0,reward_item_alpha_samples=0;
#ifdef DORA3_STORY_PICKUP_CONTACT_CHECK
static unsigned reward_contact_cases=0;
static unsigned reward_idle_yaw_samples=0;
#endif
#endif
[[maybe_unused]] static std::vector<int32_t> story_actor_state(uint8_t* rdram) {
    std::vector<int32_t> state;
    for(unsigned slot=30;slot<230;++slot) {
        const auto actor=0x80171CD0u+slot*0xCC;
        if(!MEM_W(0x94,int32_t(actor)))continue;
        const auto physical=uint32_t(MEM_W(0x98,int32_t(actor)));
        const auto kind=MEM_H(0x20,int32_t(physical));
        assert(kind==118||kind==123||kind==127||kind==130);
        state.push_back(int32_t(actor));
        for(unsigned offset=0;offset<0xCC;offset+=4)state.push_back(MEM_W(int32_t(offset),int32_t(actor)));
        for(unsigned offset=0;offset<0xF4;offset+=4)state.push_back(MEM_W(int32_t(offset),int32_t(physical)));
        const auto model=uint32_t(MEM_W(0x74,int32_t(actor)));
        assert(model&&game_model_animation().lifetime_token(model));
        for(unsigned offset=0;offset<0x84;offset+=4)state.push_back(MEM_W(int32_t(offset),int32_t(model)));
    }
    return state;
}
#endif
#include "native-flash-delay-check.inc"
#include "native-flash-motion-check.inc"
#include "native-flash-pool-check.inc"
#include "native-periodic-effect-source-check.inc"
#include "native-periodic-effect-guard-check.inc"
#include "native-periodic-producer-check.inc"
#include "native-periodic-producer-guard-check.inc"
#include "native-pickup-uncollected-source-check.inc"
int main(int argc,char** argv) {
    assert(argc==2);std::vector<uint32_t> image(0x400000/4);std::ifstream in(argv[1],std::ios::binary);
    in.read(reinterpret_cast<char*>(image.data()),0x400000);assert(in.gcount()==0x400000);
#ifdef DORA3_PICKUP_UNCOLLECTED_SOURCE_CHECK
    check_pickup_uncollected_source(image);return 0;
#endif
#ifdef DORA3_PERIODIC_EFFECT_GUARD_CHECK
    check_periodic_effect_guards(image);return 0;
#endif
#ifdef DORA3_PERIODIC_EFFECT_SOURCE_CHECK
    check_periodic_effect_source(image);return 0;
#endif
#ifdef DORA3_PERIODIC_PRODUCER_GUARD_CHECK
    check_periodic_producer_guards(image);return 0;
#endif
#ifdef DORA3_PERIODIC_PRODUCER_CHECK
    check_periodic_producer(image);return 0;
#endif
    unsigned native_moving_edge_cases=0,repeat_weapon_cases=0,player_flight_cases=0,flight_weapon_cases=0,flight_native_input_cases=0,flight_repeat_cases=0,flight_edge_cases=0,flight_damage_cases=0,ground_damage_cases=0,heavy_damage_cases=0,native_damage_cases=0,air_damage_cases=0,hazard_damage_cases=0,waiting_damage_cases=0,recovery31_cases=0,special_player_cases=0,native_special_cases=0,special_recovery_cases=0,native_special_recovery_cases=0,hover_source_cases=0,hover_native_cases=0,death_source_cases=0,respawn_recovery_cases=0,surface_source_cases=0,door_source_cases=0,surface_native_cases=0;
    unsigned cases=0,terminal_cases=0,flight_cases=0,growth_cases=0,ballistic_cases=0,phase_cases=0,bounce_cases=0,homing_cases=0,charge_cases=0,charge_birth_cases=0,interaction_cases=0,interaction_birth_cases=0,weapon_cases=0,moving_weapon_cases=0,native_weapon_cases=0;
#ifdef DORA3_STORY_EFFECT_POOL_CHECK
    for(unsigned playback:{0u,1u})
#ifdef DORA3_STORY_PICKUP_IDLE_GUARD_CHECK
    for(unsigned guard_case:{1u,2u,3u,4u,5u,6u,7u,8u})
    for(unsigned contact_tick:{20u})
#elif defined(DORA3_STORY_PICKUP_CONTACT_CHECK)
    for(unsigned guard_case:{0u})
    for(unsigned contact_tick:{2u,4u,11u,20u})
#else
    for(unsigned guard_case:{0u})
    for(unsigned contact_tick:{0u})
#endif
#endif
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
            assert(native_gameplay_preview_eligible(rdram));preview_live=true;
            decisions=actions=footsteps=sprite_commands=jumps=falls=landings=0;animations={};dora3_native_camera_dispatch_reset();
            input_now=0;native_gameplay_preview_begin(rdram,0,divider);
            unsigned vi=0;const auto saved=c;
            native_gameplay_preview_reset(rdram);preview_live=false;
            room(rdram);city_loaded=false;slot09_loaded=linked_construct_watch=linked_watch=true;
            city_watch=patrol_watch=city_dialogue_watch=false;city_commands.clear();city_source_initial_stamp.reset();
            std::copy(slot09_overlay_words.begin(),slot09_overlay_words.end(),memory.begin()+slot09_overlay_vram_offset/4);
            MEM_W(0,int32_t(0x801AEC00+20))=int32_t(asset);MEM_W(0,int32_t(0x801AEC00+40))=int32_t(clip);
            MEM_W(0,int32_t(0x801AEC00+80))=int32_t(0x80102000u);
            const auto baseline_memory=memory;const auto baseline_heap=allocations;
            const auto baseline_next=next_address;const auto baseline_refs=resource_references;
            const auto baseline_models=game_model_animation().model_count();assert(baseline_models==2);
            for(unsigned profile=0;profile<7;++profile) {
#ifdef DORA3_STORY_EFFECT_DELAY_CHECK
                if(profile!=0)continue;
#endif
                memory=baseline_memory;allocations=baseline_heap;next_address=baseline_next;resource_references=baseline_refs;
                c=saved;c.f_odd=&c.f0.u32h;city_commands.clear();input_now=0;city_source_stamp={0,0};
                for(unsigned slot=30;slot<230;++slot) {c.r4=int32_t(0x80171CD0u+slot*0xCC);dora3_scene_object_init(rdram,&c);MEM_H(0xA8,c.r4)=-1;}
                for(unsigned off=0;off<16;off+=4)MEM_W(int32_t(off),int32_t(0x801B3628))=0;
                auto quest=[&](unsigned flag) {MEM_W(0,int32_t(0x801B3628+4*(flag/32)))|=int32_t(1u<<(flag%32));};
                auto has=[&](unsigned flag) {return (uint32_t(MEM_W(0,int32_t(0x801B3628+4*(flag/32))))&(1u<<(flag%32)))!=0;};
                MEM_H(0,int32_t(0x801B36B8))=0;
                MEM_H(0,int32_t(0x8021BE02))=-500;MEM_H(2,int32_t(0x8021BE02))=80;MEM_H(4,int32_t(0x8021BE02))=-300;MEM_H(6,int32_t(0x8021BE02))=359;
                if(profile==3||profile==6)dora3_s44_remaining_8022E0FC(rdram,&c);else dora3_s44_remaining_8022EC90(rdram,&c);
                const auto parent=prop,pp=uint32_t(MEM_W(0x98,int32_t(parent))),pm=uint32_t(MEM_W(0x74,int32_t(parent)));
                assert(MEM_W(0x94,int32_t(parent))!=0&&pp&&pm);
                MEM_W(0,int32_t(0x8021BDEC))=int32_t(parent);MEM_W(0,int32_t(0x8021BCDC))=int32_t(pp);
                MEM_B(0,int32_t(0x8021710D))=0;
                if(profile==3||profile==6) {
                    MEM_H(0,int32_t(0x801B36B8))=0xC0;MEM_W(0x94,int32_t(parent))=101;quest(0x7C);quest(0x32);
                    if(profile==6)quest(0x2D);
                    MEM_W(0,int32_t(0x8021BCE4))=4;
                } else {
                    MEM_H(0,int32_t(0x801B36B8))=profile==2||profile==5?0xC1:0xAC;
                    MEM_B(0,int32_t(0x801B374A))=profile==1?1:0;
                    if(profile==2||profile==5)quest(0x7B);
                    if(profile==4)quest(0x10);
                    if(profile==5)quest(0x2F);
                    MEM_W(0x94,int32_t(parent))=102;MEM_B(0x28,int32_t(pp))=1;
                    MEM_H(0x34,int32_t(pp))=1;MEM_H(0x36,int32_t(pp))=1;MEM_H(0x44,int32_t(pp))=0;
                    c.r4=int32_t(parent);c.r5=0;c.r6=5;dora3_s0_8002A328(rdram,&c);
                    MEM_B(0x7C,int32_t(pm))=2;wf(rdram,pp+0x58,359);wf(rdram,pp+0x5C,0);
                }
                const auto before=memory;const auto before_heap=allocations;const auto before_next=next_address;const auto before_refs=resource_references;
                auto source_producer=[&](bool frozen) {
                    c=saved;c.f_odd=&c.f0.u32h;city_commands.clear();city_source_stamp={Nanoseconds(second*divider/60),unsigned((uint64_t(second)*divider)%60)};
                    if(profile==3||profile==6) {if(frozen)reference_dora3_s44_remaining_8022E200(rdram,&c);else dora3_s44_remaining_8022E200(rdram,&c);}
                    else {if(frozen)reference_dora3_s44_remaining_8022F254(rdram,&c);else dora3_s44_remaining_8022F254(rdram,&c);}
                };
                source_producer(true);const auto expected=memory;auto expected_context=c;expected_context.f_odd=nullptr;
                const auto expected_heap=allocations;const auto expected_next=next_address;const auto expected_refs=resource_references;
                const auto expected_commands=city_commands;const auto expected_models=game_model_animation().model_count();
                // Retire actual allocations before replay, then restore the
                // pre-producer model because only the new item adds a model.
                for(unsigned slot=31;slot<230;++slot) {const auto a=0x80171CD0u+slot*0xCC;if(MEM_W(0x94,int32_t(a))) {c.r4=int32_t(a);dora3_s3_801E656C(rdram,&c);c.r4=int32_t(a);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);c.r4=int32_t(a);dora3_scene_object_init(rdram,&c);}}
                memory=before;allocations=before_heap;next_address=before_next;resource_references=before_refs;
#ifdef DORA3_STORY_SOURCE_NATIVE_CHECK
#ifdef DORA3_STORY_EFFECT_POOL_CHECK
                flash_pool_reference=playback==0;
                if(playback==1) {
#endif
                assert(native_gameplay_preview_eligible(rdram));preview_live=true;
                native_gameplay_preview_begin(rdram,0,divider);input_now=0;
                // The actual BB080 Source entry suspends individual Native
                // roots before its Original callbacks publish commands.
                dora3_native_preview_hook(rdram,&c,0x801BB080);
#ifdef DORA3_STORY_EFFECT_POOL_CHECK
                }
#endif
#endif
                source_producer(false);auto actual_context=c;actual_context.f_odd=nullptr;
                assert(memory==expected&&std::memcmp(&actual_context,&expected_context,sizeof(c))==0&&allocations==expected_heap&&next_address==expected_next&&resource_references==expected_refs&&city_commands==expected_commands&&game_model_animation().model_count()==expected_models);
                std::vector<uint32_t> items;const unsigned item_kind=profile==3?127:profile==2?123:118;
                for(unsigned slot=31;slot<230;++slot) {const auto a=0x80171CD0u+slot*0xCC;if(MEM_W(0x94,int32_t(a))) {const auto ph=uint32_t(MEM_W(0x98,int32_t(a)));assert(MEM_H(0x20,int32_t(ph))==int16_t(item_kind));items.push_back(a);assert(MEM_W(0x94,int32_t(a))==10&&MEM_H(0xD8,int32_t(ph))==0);}}
                assert(items.size()==(profile<4?1u:0u));
#ifdef DORA3_STORY_PICKUP_CONTACT_CHECK
                // Exercise genuine Source contacts after different idle ages.
                // The external state1 command is fixture input;
                // the constructor and mode3 contact remain actual game code.
                for(const auto item:items) {
                    MEM_W(0x94,int32_t(item))=1;
                    MEM_B(0x28,MEM_W(0x98,int32_t(item)))=0;
                }
#endif
#ifdef DORA3_STORY_SOURCE_NATIVE_CHECK
                for(const auto item:items) {
#ifdef DORA3_STORY_EFFECT_POOL_CHECK
                    if(playback==1)
#endif
                    assert(game_model_animation().native_birth(uint32_t(MEM_W(0x74,int32_t(item)))));
                    ++story_native_ctor_models;
                }
                unsigned story_frame=1;
#ifdef DORA3_STORY_EFFECT_POOL_CHECK
                if(playback==0)preview_live=true;
#endif
#endif
                if(profile<=1)assert(has(0x10)&&has(0xB)&&has(profile?0x13:0x12)&&!has(profile?0x12:0x13));
                if(profile==2)assert(has(0x2F)&&has(0x2C));
                if(profile==3)assert(has(0x2D));
                c.r4=int32_t(parent);dora3_s3_801E656C(rdram,&c);c.r4=int32_t(parent);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);c.r4=int32_t(parent);dora3_scene_object_init(rdram,&c);
#ifdef DORA3_STORY_EFFECT_DELAY_CHECK
                for(const auto item:items) {c.r4=int32_t(item);dora3_s3_801E656C(rdram,&c);c.r4=int32_t(item);c.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&c);c.r4=int32_t(item);dora3_scene_object_init(rdram,&c);}
#ifdef DORA3_STORY_EFFECT_MOTION_CHECK
                check_flash_motion(rdram,c,fps,divider);
#else
                check_flash_delay(rdram,c,fps,divider);
#endif
#else
                // The real pool dispatcher visits the item and its six actual
                // newly constructed effects. No fake model/effect destructor.
                unsigned seen_effects=0;std::set<uint32_t> effect_addresses;bool item_retired=items.empty();
                for(unsigned tick=1;tick<=96;++tick) {
                    const auto units=uint64_t(second)*divider*tick;
#ifdef DORA3_STORY_PICKUP_IDLE_GUARD_CHECK
                    // Commands occur at the previous whole Source boundary,
                    // before this period's Native intervals. Both playbacks
                    // receive exactly the same external command and timestamp.
                    input_now=Nanoseconds((uint64_t(second)*divider*(tick-1)+59)/60);
                    city_source_stamp={Nanoseconds(uint64_t(second)*divider*(tick-1)/60),
                        unsigned((uint64_t(second)*divider*(tick-1))%60)};
                    for(const auto item:items)if(MEM_W(0x94,int32_t(item))) {
                        const auto ph=MEM_W(0x98,int32_t(item));
                        if(guard_case==1) {
                            if(tick==5)MEM_B(0xC9,ph)=1;
                            if(tick==21)MEM_B(0xC9,ph)=0;
                        } else if(guard_case==2) {
                            if(tick==5)wf(rdram,uint32_t(ph)+0x10,11);
                            if(tick==10)wf(rdram,uint32_t(ph)+0x10,0);
                        } else if(guard_case==3) {
                            if(tick==5)MEM_W(0x94,int32_t(item))=9;
                            if(tick==10)MEM_W(0x94,int32_t(item))=1;
                        } else if(guard_case==4) {
                            if(tick==5)MEM_W(0,int32_t(0x80202064u+item_kind*84u))=int32_t(0x801E5D84u);
                            if(tick==10)MEM_W(0,int32_t(0x80202064u+item_kind*84u))=int32_t(item_kind==118?0x801ECC6Cu:item_kind==123?0x801ED020u:0x801ED244u);
                        } else if(guard_case==5) {
                            if(tick==5)MEM_W(0,int32_t(0x80202068u+item_kind*84u))=int32_t(0x801E5D84u);
                            if(tick==10)MEM_W(0,int32_t(0x80202068u+item_kind*84u))=int32_t(item_kind==118?0x801ECBE4u:item_kind==123?0x801ECFB4u:0x801ED228u);
                        } else if(guard_case==6) {
                            if(tick==5)wf(rdram,item+0x10,175);
                            if(tick==7)wf(rdram,item+0x10,359);
                            if(tick==9)wf(rdram,item+0x10,-2);
                        } else if(guard_case==7) {
                            if(tick==5){wf(rdram,uint32_t(ph)+0xC4,.25f);MEM_B(0xC8,ph)=2;}
                            if(tick==8){wf(rdram,uint32_t(ph)+0xC4,.875f);MEM_B(0xC8,ph)=1;}
                        } else if(guard_case==8&&tick==5) {
                            // These item actors have no B4 clip table. Use the
                            // real model seek command with its actual ABI:
                            // model pointer in a0 and float time bits in a1.
                            c.r4=MEM_W(0x74,int32_t(item));c.r5=0;
                            dora3_s0_80039564(rdram,&c);c=saved;c.f_odd=&c.f0.u32h;
                        }
                    }
#endif
#ifdef DORA3_STORY_SOURCE_NATIVE_CHECK
                    const auto boundary=Nanoseconds((units+59)/60);
                    auto native_to=[&](Nanoseconds stamp) {
#ifdef DORA3_STORY_EFFECT_POOL_CHECK
                        const auto state=flash_pool_source_items(rdram);const auto heights=flash_pool_heights(rdram);
                        std::map<uint32_t,std::array<float,3>> item_poses;
                        for(auto item:items)if(MEM_W(0x94,int32_t(item))) {
                            const auto ph=MEM_W(0x98,int32_t(item));
                            item_poses[item]={rf(rdram,item+0x10),rf(rdram,item+0xA0),rf(rdram,uint32_t(ph)+0x58)};
                        }
#else
                        const auto state=story_actor_state(rdram);
#endif
                        const auto heap=allocations;
                        const auto refs=resource_references;const auto model_count=game_model_animation().model_count();
                        const auto commands=city_commands;const auto caller=c;
                        input_now=stamp;native_gameplay_preview_update(rdram,&c,stamp);
#ifdef DORA3_STORY_EFFECT_POOL_CHECK
                        assert(flash_pool_source_items(rdram)==state);++flash_pool_intervals;
                        for(auto [item,pose]:item_poses)if(MEM_W(0x94,int32_t(item))) {
                            const auto ph=MEM_W(0x98,int32_t(item));
#ifdef DORA3_STORY_PICKUP_CONTACT_CHECK
                            if(MEM_W(0x94,int32_t(item))==1)reward_idle_yaw_samples+=pose[0]!=rf(rdram,item+0x10);
#endif
                            reward_item_yaw_samples+=pose[0]!=rf(rdram,item+0x10);
                            reward_item_height_samples+=pose[1]!=rf(rdram,item+0xA0);
                            reward_item_alpha_samples+=pose[2]!=rf(rdram,uint32_t(ph)+0x58);
                        }
                        const auto next_heights=flash_pool_heights(rdram);
                        if(next_heights.size()==heights.size())for(unsigned i=0;i<heights.size();++i)if(heights[i]!=next_heights[i])++flash_pool_samples;
#else
                        assert(story_actor_state(rdram)==state&&allocations==heap&&resource_references==refs&&game_model_animation().model_count()==model_count&&city_commands==commands);
#endif
                        assert(std::memcmp(&caller,&c,sizeof(c))==0);
                        ++story_native_intervals;
                    };
#ifdef DORA3_STORY_EFFECT_POOL_CHECK
                    if(playback==1) {
#endif
                    while(second*story_frame/fps<boundary) {native_to(second*story_frame/fps);++story_frame;}
                    native_to(boundary);
#ifdef DORA3_STORY_EFFECT_POOL_CHECK
                    }
#endif
#endif
                    input_now=Nanoseconds((units+59)/60);city_source_stamp={Nanoseconds(units/60),unsigned(units%60)};
                    c=saved;c.f_odd=&c.f0.u32h;
#ifdef DORA3_STORY_PICKUP_CONTACT_CHECK
                    if(tick==contact_tick)for(const auto item:items) {
                        assert(MEM_W(0x94,int32_t(item))==1);
                        const auto old_actor=MEM_W(0,int32_t(0x8021BDEC)),old_physical=MEM_W(0,int32_t(0x8021BCDC)),old_mode=MEM_W(0,int32_t(0x8021BCE4));
                        const auto ph=MEM_W(0x98,int32_t(item));
                        MEM_W(0,int32_t(0x8021BDEC))=int32_t(item);
                        MEM_W(0,int32_t(0x8021BCDC))=ph;
                        MEM_W(0,int32_t(0x8021BCE4))=3;
                        if(item_kind==118) {
                            if(playback==0)reference_dora3_s3_core_801ECBE4(rdram,&c);
                            else dora3_s3_core_801ECBE4(rdram,&c);
                        } else if(item_kind==123) {
                            if(playback==0)reference_dora3_s3_core_801ECFB4(rdram,&c);
                            else dora3_s3_core_801ECFB4(rdram,&c);
                        } else {
                            if(playback==0)reference_dora3_s3_core_801ED228(rdram,&c);
                            else dora3_s3_core_801ED228(rdram,&c);
                        }
                        assert(MEM_W(0x94,int32_t(item))==10&&MEM_BU(0x28,ph)==0);
                        MEM_W(0,int32_t(0x8021BDEC))=old_actor;
                        MEM_W(0,int32_t(0x8021BCDC))=old_physical;
                        MEM_W(0,int32_t(0x8021BCE4))=old_mode;
                        c=saved;c.f_odd=&c.f0.u32h;
                        if(playback==1)++reward_contact_cases;
                    }
#endif
#ifdef DORA3_STORY_EFFECT_POOL_CHECK
                    if(playback==0)reference_dora3_s3_801E8BDC(rdram,&c);else
#endif
                    dora3_s3_801E8BDC(rdram,&c);
#ifdef DORA3_STORY_EFFECT_POOL_CHECK
                    if(playback==0)flash_pool_oracles[{fps,divider,camera_mode,profile,contact_tick,guard_case}].push_back(flash_pool_snapshot(rdram));
                    else flash_pool_compare(rdram,fps,divider,camera_mode,profile,tick,contact_tick,guard_case);
#endif
                    for(unsigned slot=30;slot<230;++slot) {const auto a=0x80171CD0u+slot*0xCC;if(!MEM_W(0x94,int32_t(a)))continue;const auto ph=uint32_t(MEM_W(0x98,int32_t(a)));const auto k=MEM_H(0x20,int32_t(ph));assert(k==int(item_kind)||k==130);
                        if(k==130)effect_addresses.insert(a);
                    }
                    if(!items.empty()&&!MEM_W(0x94,int32_t(items[0])))item_retired=true;
                }
                seen_effects=unsigned(effect_addresses.size());assert(seen_effects==(profile<4?6u:0u)&&item_retired);
                for(unsigned slot=30;slot<230;++slot)assert(MEM_W(0x94,int32_t(0x80171CD0u+slot*0xCC))==0);
                assert(allocations==baseline_heap&&resource_references==baseline_refs&&game_model_animation().model_count()==baseline_models);
#ifdef DORA3_STORY_SOURCE_NATIVE_CHECK
                native_gameplay_preview_reset(rdram);preview_live=false;
#endif
#endif
#ifdef DORA3_STORY_EFFECT_POOL_CHECK
                if(playback==1)++flash_pool_cases;
#endif
                ++cases;
            }
            linked_watch=linked_construct_watch=slot09_loaded=false;native_gameplay_preview_reset(rdram);preview_live=false;
            for(auto m:owners) {c.r4=int32_t(m);dora3_s0_8003B77C(rdram,&c);}
        }
    }
#ifdef DORA3_STORY_SOURCE_NATIVE_CHECK
#ifdef DORA3_STORY_EFFECT_POOL_CHECK
#ifdef DORA3_STORY_PICKUP_IDLE_GUARD_CHECK
    assert(flash_pool_cases==1344&&flash_pool_endpoints==1344*96&&flash_pool_samples>2000);
#elif defined(DORA3_STORY_PICKUP_CONTACT_CHECK)
    assert(flash_pool_cases==672&&flash_pool_endpoints==672*96&&flash_pool_samples>2000);
#else
    assert(flash_pool_cases==168&&flash_pool_endpoints==168*96&&flash_pool_samples>2000);
#endif
    assert(reward_item_yaw_samples>2000&&reward_item_height_samples>1000&&reward_item_alpha_samples>2000);
#ifdef DORA3_STORY_PICKUP_CONTACT_CHECK
#ifdef DORA3_STORY_PICKUP_IDLE_GUARD_CHECK
    assert(reward_contact_cases==768&&reward_idle_yaw_samples>=768);
    std::cout<<"PASS: 768 actual Source mode3 contacts after Native idle pause/resume, moving-state1 rejection, state9 return, both callback replacements, external yaw/opacity commands and actual model seek; paired complete frozen Original lifetimes.\n";
#else
    assert(reward_contact_cases==384);
    assert(reward_idle_yaw_samples>=384);
    std::cout<<"PASS: 384 actual Source mode3 contact callbacks at Original periods2/4/11/20; Native stationary idle yaw advances between Source endpoints; collection initialization stays Source; actual Source state10 acquisition and ordered six-effect lifetime.\n";
#endif
    std::cout<<"Native idle intermediate yaw changes="<<reward_idle_yaw_samples<<"\n";
#endif
    std::cout<<"PASS: "<<flash_pool_cases<<" connected Native flash pool and Source reward producer schedules; "<<flash_pool_intervals<<" real Native intervals; "<<flash_pool_endpoints<<" full live actor/physics/model/heap/resource/command Original endpoints; "<<flash_pool_samples<<" intermediate effect height changes; item yaw/height/alpha samples "<<reward_item_yaw_samples<<"/"<<reward_item_height_samples<<"/"<<reward_item_alpha_samples<<"; actual Source birth pass, all six selectors, full animation tail and genuine retirement; Native collected118/123/127 and flash130 state10; ordered initialization/delay retained at Source boundaries; no game/GPU.\n";
#else
    assert(story_native_ctor_models==96&&story_native_intervals>16000);
    std::cout<<"PASS: "<<cases<<" mixed Native/Source quest item schedules; "<<story_native_intervals<<" real Native intervals preserve all live item/effect actor, physics and model bytes, heap/resources/commands/context; "<<story_native_ctor_models<<" actual Source item constructors under Native banks; no new Native item ownership; no game/GPU.\n";
#endif
#else
#ifdef DORA3_STORY_EFFECT_DELAY_CHECK
#ifdef DORA3_STORY_EFFECT_MOTION_CHECK
    assert(flash_motion_cases==288&&flash_motion_samples>2000);
    std::cout<<"PASS: "<<flash_motion_cases<<" Native flash motion/birth/acquisition/complete retirement schedules; "<<flash_motion_intervals<<" real generated callback and common-tail intervals; "<<flash_motion_endpoints<<" frozen189 live actor/physics/model/heap/resource/command endpoints; "<<flash_motion_samples<<" intermediate height updates; no pool ownership expansion; no game/GPU.\n";
#else
    assert(flash_delay_cases==144&&flash_delay_duplicates>0);
    std::cout<<"PASS: "<<flash_delay_cases<<" Native flash delay schedules; "<<flash_delay_intervals<<" actual patched Original callback intervals; "<<flash_delay_duplicates<<" duplicate-end rejections; six actual descriptor constructors, whole actor/physics/model and commands equal frozen Original at every period end; no ownership expansion; no game/GPU.\n";
#endif
#else
    std::cout<<"PASS: "<<cases<<" Source quest item producer and complete six-effect lifetime schedules; synthetic assets; no new Native ownership; no game/GPU.\n";
#endif
#endif
}
