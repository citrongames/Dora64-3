#define DORA3_PROFILE_IMPLEMENTATION
#ifndef DORA3_PROFILE
#include "performance_profile.hpp"
extern "C" uint64_t dora3_profile_begin(unsigned){return 0;}
extern "C" void dora3_profile_end(uint64_t){}
extern "C" void dora3_profile_context(uint8_t*){}
#else
#include "performance_profile.hpp"
#include "draw_distance.hpp"
#include "guest_storage.hpp"
#include "port_settings.hpp"
#include "port_menu.hpp"
#include "recomp.h"
#include <algorithm>
#include <array>
#include <atomic>
#include <bit>
#include <chrono>
#include <fstream>
#include <mutex>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <time.h>
#endif

extern "C" int dora3_overlay_loaded(uint32_t);
namespace {
using dora3::profile::Tag;
constexpr size_t count=size_t(Tag::Count);
constexpr std::array<const char*,count> names{
    "native_update","native_frame","world_source_update","world_gameplay",
    "actor_manager","common_actors","contacts","player","projectiles",
    "world_draw","world_geometry","model_advance","model_evaluate",
    "frame_commands","task_submit","native_model_pose","texture_bind",
    "trace_sample_1_in_64","distance_admission","rt64_parse","rt64_screen",
    "rt64_render_cpu_including_wait","rt64_gpu_timestamp","rt64_gpu_wait",
    "rt64_upload_wait","rt64_workload_total","present_swap","present_workload_wait",
    "enemy_callback","object_timeline","model_draw_commands",
    "model_rate_plan","model_clip_boundaries","object_clock_registration",
    "preview_hook_sample_1_in_16","enemy_axis_sample_1_in_16",
    "common_ownership_sample_1_in_16","enemy_identity_sample_1_in_16",
    "overlay_lookup_sample_1_in_64","enemy_pool_finalize",
    "native_model_clock_sample_1_in_16","gameplay_eligibility_sample_1_in_32"
};
constexpr std::array<uint64_t,11> limits{
    100000,500000,1000000,2000000,5000000,10000000,20000000,50000000,
    100000000,250000000,500000000
};
struct Totals {
    std::atomic<uint64_t> calls{0},total{0},self{0},maximum{0};
    std::atomic<uint64_t> cpu_calls{0},cpu_total{0};
    std::array<std::atomic<uint64_t>,limits.size()+1> histogram{};
};
std::array<Totals,count> totals;
std::array<std::atomic<uint64_t>,count> observed_calls{};
std::atomic<bool> enabled{false};
std::atomic<uint64_t> trace_calls{0},outputs{0},guest_frames{0},bad_scopes{0};
std::atomic<uint64_t> vertices{0},faces{0},matrices{0},next_report{0},next_context{0};
std::atomic<unsigned> scene{0},pause{0},actors{0},collidable{0},fps{0},mode{0},menu{0};
std::atomic<float> profile_distance{1.f};
std::mutex report_mutex;
std::ofstream profile_log;
uint64_t origin=0,last_report=0;
std::atomic<uint64_t> gaps{0},gap_total{0},gap_max{0},slices{0},input_segments{0},input_edges{0};
std::array<std::atomic<uint64_t>,5> split_reasons{};
std::array<std::atomic<uint64_t>,8> slice_histogram{};
constexpr std::array<uint64_t,7> slice_upper_ns{1,1000,10000,100000,1000000,5000000,10000000};
uint64_t thread_cpu_ns() {
#ifdef _WIN32
    FILETIME created,exited,kernel,user;
    if(!GetThreadTimes(GetCurrentThread(),&created,&exited,&kernel,&user))return 0;
    const auto value=[](const FILETIME& f){return (uint64_t(f.dwHighDateTime)<<32)|f.dwLowDateTime;};
    return (value(kernel)+value(user))*100;
#else
    timespec value{};
    return clock_gettime(CLOCK_THREAD_CPUTIME_ID,&value)==0?uint64_t(value.tv_sec)*1000000000+uint64_t(value.tv_nsec):0;
#endif
}
bool observe_cpu(Tag tag) {
    return tag==Tag::NativeUpdate||tag==Tag::NativeFrame||tag==Tag::ObjectTimeline||
        tag==Tag::WorldGameplay||tag==Tag::RT64Parse;
}
struct Frame {uint64_t ticket,start,children,cpu_start;Tag tag;};
struct Stack {std::array<Frame,128> frames{};std::array<uint64_t,count> samples{};unsigned depth=0;uint64_t serial=0,trace=0;};
thread_local Stack stack;
uint64_t now() {
    return uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
}
void add(Tag tag,uint64_t elapsed,uint64_t self) {
    const auto i=size_t(tag);if(i>=count)return;
    auto& target=totals[i];target.calls.fetch_add(1,std::memory_order_relaxed);
    target.total.fetch_add(elapsed,std::memory_order_relaxed);
    target.self.fetch_add(self,std::memory_order_relaxed);
    auto maximum=target.maximum.load(std::memory_order_relaxed);
    while(maximum<elapsed&&!target.maximum.compare_exchange_weak(maximum,elapsed,std::memory_order_relaxed)){}
    const auto bin=size_t(std::lower_bound(limits.begin(),limits.end(),elapsed)-limits.begin());
    target.histogram[bin].fetch_add(1,std::memory_order_relaxed);
}
}
namespace dora3::profile {
void configure(const std::filesystem::path& path) {
    std::scoped_lock lock(report_mutex);
    profile_log.open(path,std::ios::out);origin=last_report=now();
    profile_log<<"{\"schema\":1,\"type\":\"header\",\"period_ms\":1000,"
          "\"timing\":\"CPU wall durations; nested self excludes measured children; GPU uses existing RT64 timestamps\","
          "\"trace_sampling\":64,\"histogram_upper_ms\":[0.1,0.5,1,2,5,10,20,50,100,250,500,null],"
          "\"policy\":\"Observation only; game logic, draw distance, limits and scheduling remain068\"}\n";
    profile_log.flush();enabled.store(bool(profile_log),std::memory_order_release);
    next_report.store(origin+1000000000,std::memory_order_release);
}
uint64_t begin(Tag tag) {
    if(!enabled.load(std::memory_order_relaxed))return 0;
    if(stack.depth==stack.frames.size()){bad_scopes.fetch_add(1,std::memory_order_relaxed);return 0;}
    const auto ticket=++stack.serial;
    stack.frames[stack.depth++]={ticket,now(),0,observe_cpu(tag)?thread_cpu_ns():0,tag};return ticket;
}
void end(uint64_t ticket) {
    if(!ticket)return;
    if(!stack.depth){bad_scopes.fetch_add(1,std::memory_order_relaxed);return;}
    const auto stopped=now();
    while(stack.depth&&stack.frames[stack.depth-1].ticket!=ticket) {
        --stack.depth;bad_scopes.fetch_add(1,std::memory_order_relaxed);
    }
    if(!stack.depth){bad_scopes.fetch_add(1,std::memory_order_relaxed);return;}
    const auto frame=stack.frames[--stack.depth];const auto elapsed=stopped-frame.start;
    add(frame.tag,elapsed,elapsed-std::min(elapsed,frame.children));
    if(frame.cpu_start) {
        const auto stopped_cpu=thread_cpu_ns();
        if(stopped_cpu>=frame.cpu_start) {
            totals[size_t(frame.tag)].cpu_calls.fetch_add(1,std::memory_order_relaxed);
            totals[size_t(frame.tag)].cpu_total.fetch_add(stopped_cpu-frame.cpu_start,std::memory_order_relaxed);
        }
    }
    if(stack.depth)stack.frames[stack.depth-1].children+=elapsed;
}
void record(Tag tag,uint64_t nanoseconds) {
    if(enabled.load(std::memory_order_relaxed))add(tag,nanoseconds,nanoseconds);
}
bool sample_stage(Tag tag,unsigned stride) {
    if(!enabled.load(std::memory_order_relaxed))return false;
    const auto index=size_t(tag);if(index>=count||!stride)return false;
    observed_calls[index].fetch_add(1,std::memory_order_relaxed);
    // Hash the per-tag ordinal to avoid aligning the same pool slot/site with
    // every sample. This uses no guest RNG or game state.
    const auto ordinal=++stack.samples[index];
    const auto hash=ordinal*0x9E3779B97F4A7C15ull;
    return ((hash^(hash>>32))&(stride-1))==0;
}
bool sample_trace() {
    if(!enabled.load(std::memory_order_relaxed))return false;
    trace_calls.fetch_add(1,std::memory_order_relaxed);
    const auto ordinal=++stack.trace;
    const auto hash=ordinal*0x9E3779B97F4A7C15ull;
    return ((hash^(hash>>32))&63u)==0;
}
void timeline_gap(uint64_t nanoseconds) {
    if(!enabled.load(std::memory_order_relaxed))return;
    gaps.fetch_add(1,std::memory_order_relaxed);gap_total.fetch_add(nanoseconds,std::memory_order_relaxed);
    auto maximum=gap_max.load(std::memory_order_relaxed);
    while(maximum<nanoseconds&&!gap_max.compare_exchange_weak(maximum,nanoseconds,std::memory_order_relaxed)){}
}
void timeline_input_segment(bool edge) {
    if(!enabled.load(std::memory_order_relaxed))return;
    input_segments.fetch_add(1,std::memory_order_relaxed);
    if(edge)input_edges.fetch_add(1,std::memory_order_relaxed);
}
void timeline_slice(uint64_t units) {
    if(!enabled.load(std::memory_order_relaxed))return;
    slices.fetch_add(1,std::memory_order_relaxed);
    const auto nanoseconds=units/60;
    const auto bin=size_t(std::lower_bound(slice_upper_ns.begin(),slice_upper_ns.end(),nanoseconds)-slice_upper_ns.begin());
    slice_histogram[bin].fetch_add(1,std::memory_order_relaxed);
}
void timeline_limit(unsigned reason,uint64_t) {
    if(enabled.load(std::memory_order_relaxed)&&reason<split_reasons.size())
        split_reasons[reason].fetch_add(1,std::memory_order_relaxed);
}
void output_frame(){if(enabled.load(std::memory_order_relaxed))outputs.fetch_add(1,std::memory_order_relaxed);}
void guest_frame(){if(enabled.load(std::memory_order_relaxed))guest_frames.fetch_add(1,std::memory_order_relaxed);}
void graphics_counts(uint64_t v,uint64_t f,uint64_t m) {
    vertices.store(v,std::memory_order_relaxed);faces.store(f,std::memory_order_relaxed);matrices.store(m,std::memory_order_relaxed);
}
void capture_context(uint8_t* ram) {
    if(!enabled.load(std::memory_order_relaxed))return;
    const auto stamp=now();auto next=next_context.load(std::memory_order_relaxed);
    const auto current_distance=dora3::distance::multiplier();
    if(stamp<next&&current_distance==profile_distance.load(std::memory_order_relaxed))return;
    if(!next_context.compare_exchange_strong(next,stamp+100000000,std::memory_order_relaxed))return;
    profile_distance.store(current_distance,std::memory_order_relaxed);
    const auto settings=dora3::port::settings();
    fps.store(settings.frame_rate_limit,std::memory_order_relaxed);
    mode.store(settings.frame_rate_mode,std::memory_order_relaxed);
    menu.store(dora3::port::menu_open(),std::memory_order_relaxed);
    uint8_t* rdram=ram;
    scene.store(MEM_HU(0,int32_t(0x801B36B8)),std::memory_order_relaxed);
    const bool world=dora3_overlay_loaded(0x70A50);
    pause.store(world?uint32_t(MEM_W(0,int32_t(0x8021D484))):0,std::memory_order_relaxed);
    unsigned active=0,contacts=0;
    if(world)for(unsigned slot=dora3::distance::common_begin;slot<dora3::distance::common_end;++slot) {
        const auto actor=dora3::distance::actor_base+slot*0xCC;
        if(!MEM_W(0x94,int32_t(actor)))continue;
        ++active;const auto physical=uint32_t(MEM_W(0x98,int32_t(actor)));
        if(physical>=0x80000000u&&uint64_t(physical)+0xDA<=dora3::distance::game_ram_end&&MEM_H(0xD8,int32_t(physical))>0)++contacts;
    }
    actors.store(active,std::memory_order_relaxed);collidable.store(contacts,std::memory_order_relaxed);
}
void report() {
    if(!enabled.load(std::memory_order_relaxed))return;
    const auto stamp=now();auto due=next_report.load(std::memory_order_relaxed);
    if(stamp<due||!next_report.compare_exchange_strong(due,stamp+1000000000,std::memory_order_relaxed))return;
    std::scoped_lock lock(report_mutex);
    const auto interval=stamp-last_report;last_report=stamp;
    const auto trace_observed=trace_calls.exchange(0);
    profile_log<<"{\"type\":\"sample\",\"t_ms\":"<<(stamp-origin)/1000000
       <<",\"interval_ms\":"<<double(interval)/1e6<<",\"draw_distance\":"<<profile_distance.load()
       <<",\"scene\":"<<scene.load()<<",\"world_pause\":"<<pause.load()<<",\"port_menu\":"<<menu.load()
       <<",\"requested_fps\":"<<fps.load()<<",\"frame_rate_mode\":"<<mode.load()
       <<",\"output_frames\":"<<outputs.exchange(0)<<",\"guest_frames\":"<<guest_frames.exchange(0)
       <<",\"common_alive\":"<<actors.load()<<",\"common_collidable\":"<<collidable.load()
       <<",\"rt64_vertices\":"<<vertices.load()<<",\"rt64_faces\":"<<faces.load()<<",\"rt64_matrices\":"<<matrices.load()
       <<",\"trace_calls\":"<<trace_observed<<",\"scope_errors\":"<<bad_scopes.exchange(0)<<",\"stages\":{";
    bool comma=false;
    for(size_t i=0;i<count;++i) {
        auto& tag=totals[i];const auto calls=tag.calls.exchange(0);const auto elapsed=tag.total.exchange(0);
        const auto self=tag.self.exchange(0),maximum=tag.maximum.exchange(0);
        const auto observed=i==size_t(Tag::TraceSample)?trace_observed:observed_calls[i].exchange(0);
        const auto cpu_calls=tag.cpu_calls.exchange(0),cpu_total=tag.cpu_total.exchange(0);
        std::array<uint64_t,limits.size()+1> histogram{};
        for(size_t j=0;j<histogram.size();++j)histogram[j]=tag.histogram[j].exchange(0);
        if(!calls&&!elapsed&&!observed)continue;
        if(comma)profile_log<<',';comma=true;
        profile_log<<'"'<<names[i]<<"\":{\"calls\":"<<calls<<",\"inclusive_ms\":"<<double(elapsed)/1e6
           <<",\"observed_calls\":"<<(observed?observed:calls)<<",\"sampled\":"<<(observed?"true":"false")
           <<",\"cpu_calls\":"<<cpu_calls<<",\"cpu_ms\":"<<double(cpu_total)/1e6
           <<",\"self_ms\":"<<double(self)/1e6<<",\"max_ms\":"<<double(maximum)/1e6<<",\"histogram\":[";
        for(size_t j=0;j<histogram.size();++j){if(j)profile_log<<',';profile_log<<histogram[j];}
        profile_log<<"]}";
    }
    profile_log<<"},\"timeline\":{\"updates\":"<<gaps.exchange(0)
        <<",\"elapsed_ms\":"<<double(gap_total.exchange(0))/1e6<<",\"max_gap_ms\":"<<double(gap_max.exchange(0))/1e6
        <<",\"slices\":"<<slices.exchange(0)<<",\"input_segments\":"<<input_segments.exchange(0)
        <<",\"input_edges\":"<<input_edges.exchange(0)<<",\"limit_reasons\":[";
    for(size_t i=0;i<split_reasons.size();++i){if(i)profile_log<<',';profile_log<<split_reasons[i].exchange(0);}
    profile_log<<"],\"slice_histogram\":[";
    for(size_t i=0;i<slice_histogram.size();++i){if(i)profile_log<<',';profile_log<<slice_histogram[i].exchange(0);}
    profile_log<<"]}}\n";profile_log.flush();
}
}
extern "C" uint64_t dora3_profile_begin(unsigned tag){return dora3::profile::begin(static_cast<Tag>(tag));}
extern "C" void dora3_profile_end(uint64_t ticket){dora3::profile::end(ticket);}
extern "C" void dora3_profile_context(uint8_t* ram){dora3::profile::capture_context(ram);}

#endif
