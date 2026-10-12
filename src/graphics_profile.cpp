#include "graphics_profile.hpp"
#ifdef DORA3_GRAPHICS_PROFILE
#include <array>
#include <atomic>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <mutex>
#include <vector>
#include <thread>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#else
#include <time.h>
#endif
namespace dora3::graphics_profile {
namespace {
constexpr size_t stage_count=size_t(Stage::Count),counter_count=size_t(Counter::Count);
constexpr std::array<const char*,stage_count> names{
 "native_update","native_frame","producer","screen","hle","rdram_check","full_sync",
 "flush","tile_validation","call_preparation","draw_upload","eager_path","tiles",
 "eager_setup","target_safety","eager_render","producer_upload_wait","producer_gpu_wait",
 "texture_wait","cpu_readback","queue_advance","inspect","materialize","materialize_wait",
 "workload","render","render_upload_wait","render_gpu_wait","present","present_wait",
 "vertex_sample_1_in_64","triangle_sample_1_in_64","matrix_sample_1_in_64",
 "draw_state_sample_1_in_64","flush_sample_1_in_64","load_planning_sample_1_in_64",
 "tmem_load_sample_1_in_64","texture_upload_sample_1_in_64","texture_hash_sample_1_in_64",
 "object_timeline_sample_1_in_64","object_registration_sample_1_in_64",
 "model_rate_plan_sample_1_in_64","enemy_callback_sample_1_in_64",
 "enemy_finalize_sample_1_in_64","model_boundary_sample_1_in_64","model_pose_sample_1_in_64","native_common","native_player","frame_build",
 "vertex_state_part","vertex_append_part","vertex_override_part","vertex_transform_part","vertex_texcoord_part",
 "triangle_state_part","triangle_bookkeeping_part","triangle_bounds_part","audio_task","graphics_ram_wait","texture_evict","upload_commands","texture_bind","texture_lists"};
constexpr std::array<const char*,counter_count> counter_names{
 "output_frames","guest_frames","vertices_sum","faces_sum","matrices_sum","pairs_sum","tiles_sum","render_gpu_timestamp_ns",
 "gate_not_native_build","gate_not_gameplay","gate_dimensions","gate_cpu_framebuffer",
 "request_off","no_pairs","reject_operations","reject_format","reject_height",
 "reject_depth_clear","reject_color_address","reject_depth_address","reject_tile_copy",
 "no_writes","deferred","eager","materializations","hash_memo_hit","hash_memo_miss","hash_memo_bypass"};
struct Total {std::atomic<uint64_t> calls{0},wall{0},self{0},cpu{0},cycles{0},maximum{0};};
std::array<Total,stage_count> totals;
std::array<std::atomic<uint64_t>,stage_count> observed{};
thread_local std::array<uint64_t,stage_count> ordinals{};
std::array<std::atomic<uint64_t>,counter_count> counters{};
std::atomic<bool> enabled{false};
std::atomic<unsigned> scene{0},fps{0},mode{0};
std::atomic<float> distance{1};
std::atomic<uint64_t> next_report{0},errors{0};
std::mutex output_mutex;
std::ofstream output;
uint64_t origin=0,last_report=0;
uint64_t now(){return uint64_t(std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count());}
struct CPU {uint64_t ns=0,cycles=0;};
CPU cpu(){
 CPU result;
#ifdef _WIN32
 FILETIME created,exited,kernel,user;
 if(GetThreadTimes(GetCurrentThread(),&created,&exited,&kernel,&user)){
  const auto value=[](FILETIME f){return (uint64_t(f.dwHighDateTime)<<32)|f.dwLowDateTime;};
  result.ns=(value(kernel)+value(user))*100;
 }
 ULONG64 cycles=0;if(QueryThreadCycleTime(GetCurrentThread(),&cycles))result.cycles=cycles;
#else
 timespec t{};if(clock_gettime(CLOCK_THREAD_CPUTIME_ID,&t)==0)result.ns=uint64_t(t.tv_sec)*1000000000+uint64_t(t.tv_nsec);
#endif
 return result;
}
struct Frame {uint64_t ticket,started,children;CPU cpu;Stage stage;bool cpu_measured;};
struct Stack {std::array<Frame,64> frames{};unsigned depth=0;uint64_t serial=0;};
thread_local Stack stack;
struct Event {uint64_t start,duration,correlation,argument;unsigned kind,thread;};
std::mutex event_mutex;
std::vector<Event> pending_events,reported_events;
uint64_t dropped_events=0;
unsigned thread_id(){
#ifdef _WIN32
 static thread_local const unsigned id=GetCurrentThreadId();
#else
 static thread_local const unsigned id=unsigned(std::hash<std::thread::id>{}(std::this_thread::get_id()));
#endif
 return id;
}
void event(unsigned kind,uint64_t start,uint64_t duration,uint64_t correlation=0,uint64_t argument=0){
 std::scoped_lock lock(event_mutex);
 if(pending_events.size()<65536)pending_events.push_back({start,duration,correlation,argument,kind,thread_id()});
 else ++dropped_events;
}
bool trace_stage(Stage stage){
 switch(stage){
 case Stage::NativeUpdate:case Stage::NativeFrame:case Stage::Producer:
 case Stage::Screen:case Stage::HLE:case Stage::FullSync:case Stage::Workload:
 case Stage::AudioTask:case Stage::GraphicsRAMWait:case Stage::TextureEvict:case Stage::UploadCommands:
 case Stage::ProducerGPUWait:case Stage::RenderGPUWait:case Stage::Present:case Stage::PresentWait:
 case Stage::Render:case Stage::NativeCommon:case Stage::NativePlayer:case Stage::FrameBuild:return true;
 default:return false;
 }
}
}
void mark(Mark kind,uint64_t correlation,uint64_t argument){
 if(enabled.load(std::memory_order_relaxed))event(1000+unsigned(kind),now(),0,correlation,argument);
}
void configure(const std::filesystem::path& path){
 pending_events.reserve(65536);reported_events.reserve(65536);
 std::scoped_lock lock(output_mutex);output.open(path);output<<std::setprecision(12);origin=last_report=now();
 output<<"{\"type\":\"header\",\"schema\":6,\"mode\":\"post086-completion-delivery\",\"period_ms\":1000,\"detail_sample_stride\":0,\"detail_clock\":\"hot-function sampling disabled; coarse wall and CPU plus event delivery\",\"timing\":\"wall inclusive/self; CPU thread time is coarse on Windows; cycles are unconverted; stages on different threads overlap\",\"policy\":\"observation only; generic traces, game profiler and diagnostic logs off\"}\n";
 output<<"{\"type\":\"trace_schema\",\"columns\":[\"kind\",\"thread\",\"start_us\",\"duration_us\",\"correlation\",\"argument\"],\"clock\":\"steady since profile origin\",\"stage_names\":[";
 for(size_t i=0;i<stage_count;++i){if(i)output<<',';output<<'"'<<names[i]<<'"';}
 output<<"],\"marker_names\":{\"1000\":\"task_submit_dl_bytes\",\"1001\":\"parse_start_dl_bytes\",\"1002\":\"wake_post_serial_epoch\",\"1003\":\"wake_receive_serial_epoch\",\"1004\":\"frame_ticket_serial_bank\",\"1005\":\"guest_done_serial_bank\",\"1006\":\"host_done_serial_bank\",\"1007\":\"wake_lateness_serial_ns\",\"1008\":\"external_queued_id_source_message\",\"1009\":\"external_delivered_id_queue\",\"1010\":\"external_blocked_id_queue\",\"1011\":\"scheduler_received_queue_message\",\"1012\":\"audio_submit_task_type\"}}\n";
 output.flush();enabled.store(bool(output),std::memory_order_release);next_report.store(origin+1000000000);
}
uint64_t begin(Stage stage){
 if(!enabled.load(std::memory_order_relaxed))return 0;
 if(stack.depth==stack.frames.size()){errors.fetch_add(1);return 0;}
 const auto ticket=++stack.serial;
 const auto started_cpu=cpu();
 stack.frames[stack.depth++]={ticket,now(),0,started_cpu,stage,true};return ticket;
}
uint64_t begin_sampled(Stage stage){
 if(!enabled.load(std::memory_order_relaxed))return 0;
 const auto index=size_t(stage);observed[index].fetch_add(1,std::memory_order_relaxed);
 const auto mixed=(++ordinals[index])*0x9E3779B97F4A7C15ull;
 if(((mixed^(mixed>>32))&63u)!=0)return 0;
 if(stack.depth==stack.frames.size()){errors.fetch_add(1);return 0;}
 const auto ticket=++stack.serial;
 stack.frames[stack.depth++]={ticket,now(),0,{},stage,false};return ticket;
}
uint64_t begin_part(Stage stage,bool active){
 if(!active)return 0;
 if(stack.depth==stack.frames.size()){errors.fetch_add(1);return 0;}
 const auto ticket=++stack.serial;
 stack.frames[stack.depth++]={ticket,now(),0,{},stage,false};return ticket;
}
void end(uint64_t ticket){
 if(!ticket)return;
 if(!stack.depth||stack.frames[stack.depth-1].ticket!=ticket){errors.fetch_add(1);return;}
 const auto stopped=now();const auto frame=stack.frames[--stack.depth];
 const auto stopped_cpu=frame.cpu_measured?cpu():CPU{};
 const auto elapsed=stopped-frame.started;auto& total=totals[size_t(frame.stage)];
 total.calls.fetch_add(1,std::memory_order_relaxed);total.wall.fetch_add(elapsed,std::memory_order_relaxed);
 total.self.fetch_add(elapsed>frame.children?elapsed-frame.children:0,std::memory_order_relaxed);
 if(frame.cpu_measured&&stopped_cpu.ns>=frame.cpu.ns)total.cpu.fetch_add(stopped_cpu.ns-frame.cpu.ns,std::memory_order_relaxed);
 if(frame.cpu_measured&&stopped_cpu.cycles>=frame.cpu.cycles)total.cycles.fetch_add(stopped_cpu.cycles-frame.cpu.cycles,std::memory_order_relaxed);
 auto maximum=total.maximum.load(std::memory_order_relaxed);
 while(maximum<elapsed&&!total.maximum.compare_exchange_weak(maximum,elapsed,std::memory_order_relaxed)){}
 if(stack.depth)stack.frames[stack.depth-1].children+=elapsed;
 if(trace_stage(frame.stage))event(unsigned(frame.stage),frame.started,elapsed);
}
void add(Counter counter,uint64_t value){if(enabled.load(std::memory_order_relaxed))counters[size_t(counter)].fetch_add(value,std::memory_order_relaxed);}
void context(unsigned s,unsigned f,unsigned m,float d){scene.store(s);fps.store(f);mode.store(m);distance.store(d);}
void report(){
 if(!enabled.load(std::memory_order_relaxed))return;
 const auto stamp=now();auto due=next_report.load(std::memory_order_relaxed);
 if(stamp<due||!next_report.compare_exchange_strong(due,stamp+1000000000,std::memory_order_relaxed))return;
 std::scoped_lock lock(output_mutex);const auto interval=stamp-last_report;last_report=stamp;
 output<<"{\"type\":\"sample\",\"t_ms\":"<<(stamp-origin)/1000000<<",\"interval_ms\":"<<double(interval)/1e6
  <<",\"scene\":"<<scene.load()<<",\"requested_fps\":"<<fps.load()<<",\"frame_rate_mode\":"<<mode.load()
  <<",\"draw_distance\":"<<distance.load()<<",\"scope_errors\":"<<errors.exchange(0)<<",\"counters\":{";
 for(size_t i=0;i<counter_count;++i){if(i)output<<',';output<<'"'<<counter_names[i]<<"\":"<<counters[i].exchange(0);}
 output<<"},\"stages\":{";bool comma=false;
 for(size_t i=0;i<stage_count;++i){auto& t=totals[i];const auto calls=t.calls.exchange(0),wall=t.wall.exchange(0),self=t.self.exchange(0),cpu_ns=t.cpu.exchange(0),cycles=t.cycles.exchange(0),maximum=t.maximum.exchange(0);
  const auto observed_calls=observed[i].exchange(0);
  if(!calls&&!wall&&!observed_calls)continue;if(comma)output<<',';comma=true;
  output<<'"'<<names[i]<<"\":{\"calls\":"<<calls<<",\"thread_cpu_measured\":"<<((i<size_t(Stage::VertexSample)||(i>=size_t(Stage::NativeCommon)&&i<=size_t(Stage::FrameBuild))||i>=size_t(Stage::AudioTask))?"true":"false")<<",\"observed_calls\":"<<observed_calls<<",\"inclusive_ms\":"<<double(wall)/1e6<<",\"self_ms\":"<<double(self)/1e6
   <<",\"cpu_ms\":"<<double(cpu_ns)/1e6<<",\"thread_cycles\":"<<cycles<<",\"max_ms\":"<<double(maximum)/1e6<<'}';
 }
 output<<"}}\n";
 uint64_t dropped=0;
 {std::scoped_lock events_lock(event_mutex);pending_events.swap(reported_events);dropped=dropped_events;dropped_events=0;}
 output<<"{\"type\":\"trace\",\"dropped\":"<<dropped<<",\"events\":[";
 for(size_t i=0;i<reported_events.size();++i){const auto& e=reported_events[i];if(i)output<<',';
  output<<'['<<e.kind<<','<<e.thread<<','<<double(e.start-origin)/1000.0<<','<<double(e.duration)/1000.0<<','<<e.correlation<<','<<e.argument<<']';}
 output<<"]}\n";reported_events.clear();output.flush();
}
}
#endif
