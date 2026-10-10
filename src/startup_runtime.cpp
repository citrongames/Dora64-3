#include "graphics_profile.hpp"
#include "diagnostics.hpp"
#include "modern_camera.hpp"
#include "performance_profile.hpp"
#include "resource_retirement.hpp"
#include "guest_storage.hpp"
#include "n64_hardware.hpp"
#include "native_enemy.hpp"
#include "game_reset.hpp"
#include "cutscene_input.hpp"
#include "native_game_loop.hpp"
#include "native_frame_bridge.hpp"
#include "native_simulation_time.hpp"
// Research frontend: runs reviewed original functions through the real runtime.
// Unreviewed functions stop explicitly. The default mode stops at graphics/PCM;
// the optional live RT64 mode continues through SDL input/audio.
// The live mode has SDL input/audio; no validated gameplay loop yet.
#include <atomic>
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <vector>
#include <thread>
#include <array>
#include <cstring>
#include <mutex>
#include <unordered_set>
#include <unordered_map>
#include <sstream>
#include <string_view>
#include "ultramodern/ultramodern.hpp"
#include "librecomp/game.hpp"
#include "librecomp/overlays.hpp"
#include "recomp_overlays.inl"
#include "overlay_regions.hpp"
#ifdef DORA3_LIVE_RT64
#include "live_graphics.hpp"
#include "live_io.hpp"
#include "port_menu.hpp"
#endif
#define XXH_INLINE_ALL
#include "xxHash/xxhash.h"

RspExitReason dora3_aspMain(uint8_t*, uint32_t);
void dora3_configure_hardware(const recomp::Configuration&);
extern "C" void dora3_controller_status(uint8_t*,recomp_context*);
extern "C" void dora3_music_reset(uint8_t*,recomp_context*);
namespace {
std::array<std::atomic<bool>,std::size(dora3_overlay_regions)> overlay_active{};
std::array<std::atomic<uint64_t>,std::size(dora3_overlay_regions)> overlay_generation{};
std::atomic<uint64_t> next_overlay_generation{1};
std::filesystem::path diagnostic_path;
constexpr size_t overlay_count=std::size(dora3_overlay_regions);
// The ROM registry is immutable. Index it once at compile time; permission
// still comes from the actual atomic verified-load state on every call.
constexpr size_t overlay_index_size=128;
static_assert(overlay_count<overlay_index_size);
constexpr size_t overlay_bucket(uint32_t rom) {
    return ((rom>>4)*uint32_t(0x9E3779B9u))>>(32-7);
}
constexpr auto make_overlay_index() {
    std::array<size_t,overlay_index_size> result{};result.fill(overlay_count);
    for(size_t i=0;i<overlay_count;++i) {
        auto bucket=overlay_bucket(dora3_overlay_regions[i].rom);
        while(result[bucket]!=overlay_count)bucket=(bucket+1)&(overlay_index_size-1);
        result[bucket]=i;
    }
    return result;
}
constexpr auto overlay_indices=make_overlay_index();
constexpr size_t overlay_index(uint32_t rom) {
    auto bucket=overlay_bucket(rom);
    while(overlay_indices[bucket]!=overlay_count) {
        const auto index=overlay_indices[bucket];
        if(dora3_overlay_regions[index].rom==rom)return index;
        bucket=(bucket+1)&(overlay_index_size-1);
    }
    return overlay_count;
}
struct DependencyIndex {
    std::array<size_t,overlay_count> begin{},count{};
    std::array<size_t,std::size(dora3_overlay_dependencies)> parents{};
};
constexpr auto make_dependency_index() {
    DependencyIndex result{};size_t next=0;
    for(size_t i=0;i<overlay_count;++i) {
        result.begin[i]=next;
        for(const auto& dependency:dora3_overlay_dependencies)
            if(dependency.rom==dora3_overlay_regions[i].rom) {
                result.parents[next++]=dependency.required_index;++result.count[i];
            }
    }
    return result;
}
constexpr auto overlay_parents=make_dependency_index();
static_assert([] {
    for(size_t i=0;i<overlay_count;++i)
        if(overlay_index(dora3_overlay_regions[i].rom)!=i)return false;
    for(const auto& dependency:dora3_overlay_dependencies)
        if(overlay_index(dependency.rom)==overlay_count||dependency.required_index>=overlay_count)return false;
    return true;
}());
bool overlay_dependencies_active_index(size_t index) {
    for(size_t n=0;n<overlay_parents.count[index];++n)
        if(!overlay_active[overlay_parents.parents[overlay_parents.begin[index]+n]].load())return false;
    return true;
}
bool overlay_dependencies_active(uint32_t rom) {
    const auto index=overlay_index(rom);
    return index==overlay_count||overlay_dependencies_active_index(index);
}
#ifdef DORA3_DIAGNOSTICS
struct TraceRange {uint32_t base=0,size=0;size_t word_offset=0;};
struct TraceLayout {std::array<TraceRange,overlay_count+1> ranges{};size_t words=0;};
// Derive code ranges from the existing generated section metadata. The
// resident section and each overlay get separate bits, even at shared VRAM.
const TraceLayout trace_layout=[] {
    TraceLayout result{};
    for(const auto& section:section_table) {
        const auto overlay=overlay_index(section.rom_addr);
        if(section.rom_addr!=0x1000&&overlay==overlay_count)continue;
        const auto index=section.rom_addr==0x1000?0:overlay+1;
        result.ranges[index]={uint32_t(section.ram_addr),uint32_t(section.size),result.words};
        result.words+=(uint64_t(section.size)+255)/256;
    }
    return result;
}();
bool first_trace(uint32_t address,uint32_t rom,size_t overlay) {
    if(rom==0x1000||overlay!=overlay_count) {
        const auto& range=trace_layout.ranges[rom==0x1000?0:overlay+1];
        if(!(address&3)&&address>=range.base&&uint64_t(address)-range.base<range.size) {
            static thread_local std::vector<uint64_t> entered(trace_layout.words);
            const auto bit=(address-range.base)/4;
            auto& word=entered[range.word_offset+bit/64];
            const auto mask=uint64_t(1)<<(bit&63);
            const bool first=!(word&mask);word|=mask;return first;
        }
    }
    // Preserve the old identity and logging behavior for out-of-range calls.
    static thread_local std::unordered_set<uint64_t> entered;
    return entered.insert((uint64_t(rom)<<32)|address).second;
}
#endif

void unload_scene_code(){
    for(size_t i=0;i<overlay_active.size();i++)if(overlay_active[i]){
        const auto& region=dora3_overlay_regions[i];
        unload_overlays((int32_t)region.ram,region.size);overlay_active[i]=false;
        DORA3_LOG(stderr,"OVERLAY UNLOADED rom=%08X ram=%08X\n",region.rom,region.ram);
    }
}
std::atomic<unsigned> audio_submitted{0},audio_completed{0},pcm_buffers{0};
std::mutex captured_vi_mutex;
ultramodern::renderer::ViRegs captured_vi{};
bool captured_vi_ready=false;
// Freeze the caller at a diagnostic boundary, but let an already submitted
// native RSP task finish. No original call receives a fake successful return.
[[noreturn]] void stop_diagnostic(int code){
    static std::atomic<int> first_code{0};
    int unset=0;first_code.compare_exchange_strong(unset,code);
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::milliseconds(250);
    // At the graphics boundary also allow the already completed audio task
    // to reach AI on its next retrace. The renderer still never returns success.
    while((audio_completed.load()<audio_submitted.load() ||
           (code==78 && audio_submitted.load()>0 && pcm_buffers.load()==0)) &&
          std::chrono::steady_clock::now()<deadline)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    #ifdef DORA3_LIVE_RT64
    dora3_drain_live_graphics();
    #endif
    DORA3_LOG(stderr,"DIAGNOSTIC STOP code=%d audio_submitted=%u audio_completed=%u\n",first_code.load(),audio_submitted.load(),audio_completed.load());
    std::fflush(nullptr);std::_Exit(first_code.load());
}
RspExitReason checked_audio_rsp(uint8_t* rdram, uint32_t ucode_addr) {
#ifdef DORA3_DIAGNOSTICS
    const auto rom=recomp::get_rom();
    // Verify actual loaded code, not just a task address. OSTask declares a
    // padded 0x1000-byte range; only 0xE20 bytes are compiled instructions.
    for(uint32_t i=0;i<0xE20;i++){
        if(MEM_BU(i,(gpr)(int32_t)(ucode_addr|0x80000000U))!=rom[0x3DAF0+i]){
            DORA3_LOG(stderr,"AUDIO RSP loaded text mismatch at %X\n",i);std::_Exit(82);
        }
    }
#endif
    const auto reason=dora3_aspMain(rdram,ucode_addr);
    DORA3_LOG(stderr,"AUDIO RSP COMPLETED reason=%u\n",(unsigned)reason);
    if(reason!=RspExitReason::Broke)std::_Exit(82);
    audio_completed.fetch_add(1);
    return reason;
}
// Private Linux/x86-64 research capture: little-endian words, byteswapped RDRAM.
// Captured before osSpTaskLoad/StartGo and before graphics SP completion.
void capture_first_graphics(uint8_t* rdram,gpr sc_task){
    if constexpr(!dora3::diagnostics::enabled)return;
    static bool captured=false;
    if(captured || !std::getenv("DORA3_CAPTURE_GRAPHICS"))return;
    captured=true;
    std::lock_guard lock(ultramodern::get_graphics_rdram_mutex());
    std::array<uint32_t,35> header{};
    header[0]=0x44335231;header[1]=1;header[2]=0x400000;header[3]=0x01020304;
    for(unsigned i=0;i<16;i++)header[4+i]=MEM_W(0x10+i*4,sc_task);
    static_assert(sizeof(ultramodern::renderer::ViRegs)==14*sizeof(uint32_t));
    {
        std::lock_guard vi_lock(captured_vi_mutex);
        if(!captured_vi_ready){DORA3_LOG(stderr,"GRAPHICS CAPTURE has no published VI snapshot\n");std::_Exit(83);}
        std::memcpy(header.data()+20,&captured_vi,14*sizeof(uint32_t));
    }
    header[34]=MEM_W(0xC,sc_task);
    const auto path=diagnostic_path/"first-graphics.snapshot";
    std::ofstream out(path,std::ios::binary);
    out.write(reinterpret_cast<const char*>(header.data()),sizeof(header));
    out.write(reinterpret_cast<const char*>(rdram),0x400000);
    out.close();
    if(!out){DORA3_LOG(stderr,"GRAPHICS CAPTURE failed\n");std::_Exit(83);}
    DORA3_LOG(stderr,"GRAPHICS CAPTURE saved immutable first-task snapshot, framebuffer=%08X\n",header[34]);
}
struct PendingDma { uint32_t rom, ram, size; bool active; };
thread_local PendingDma pending_dma{};
thread_local uint32_t resource_rom=0;
thread_local bool stream_dma_call=false;
}

extern "C" void dora3_mark_stream_dma(){stream_dma_call=true;}

// Read the same verified-load state used by Original overlay calls. VRAM
// alone cannot identify city code: other levels reuse these addresses.
extern "C" int dora3_overlay_loaded(uint32_t section_rom) {
    dora3::profile::Scope detailProfile(dora3::profile::Tag::OverlayLookupSample,dora3::profile::sample_stage(dora3::profile::Tag::OverlayLookupSample,64));
    const auto index=overlay_index(section_rom);
    return index!=overlay_count&&overlay_active[index].load()&&overlay_dependencies_active_index(index);
}

// Every verified load has a new identity, including byte-identical reloads.
// The parent's newest load changes the child's effective identity as well.
extern "C" uint64_t dora3_overlay_generation(uint32_t section_rom) {
    const auto index=overlay_index(section_rom);
    if(index==overlay_count||!overlay_active[index].load()||!overlay_dependencies_active_index(index))return 0;
    uint64_t value=overlay_generation[index].load();
    for(size_t n=0;n<overlay_parents.count[index];++n) {
        const auto required=overlay_parents.parents[overlay_parents.begin[index]+n];
        const auto parent=dora3_overlay_generation(dora3_overlay_regions[required].rom);
        if(!parent)return 0;
        value=std::max(value,parent);
    }
    return value;
}
extern "C" void dora3_pending(uint32_t address) {
    DORA3_LOG(stderr,"PENDING original function %08X; execution stopped, no success stub\n",address);
    stop_diagnostic(77);
}
extern "C" void dora3_capture_controller_state(uint8_t* rdram,const char* reason) {
    if constexpr(!dora3::diagnostics::enabled)return;
    // The observer follows the real read, Source dispatch, and scene consumers.
    // Nothing is published to the guest; each point has its own change cache.
    // Boot/reset are serialized with guest quiescence by the runtime barrier.
    static unsigned session=0;
    static uint64_t polls=0,source_dispatches=0,source_completions=0,world_updates=0,script_updates=0;
    static std::unordered_map<std::string,std::string> previous;
    static std::unordered_map<std::string,uint64_t> observations;
    const std::string point=reason;
    const bool boot=point=="boot",forced=boot||point=="before-reset";
    if(boot){++session;polls=source_dispatches=source_completions=world_updates=script_updates=0;previous.clear();observations.clear();}
    if(point=="after-read")++polls;
    if(point=="source-dispatch")++source_dispatches;
    if(point=="source-complete")++source_completions;
    if(point=="world-update")++world_updates;
    if(point=="script-input")++script_updates;
    const auto visits=++observations[point];
    auto half=[&](uint32_t address){return MEM_HU(0,int32_t(address));};
    auto word=[&](uint32_t address){return MEM_W(0,int32_t(address));};
    std::ostringstream state;
    state<<"\"presence\":"<<unsigned(MEM_BU(0,int32_t(0x801A48B4)))
         <<",\"pads\":[";
    for(unsigned port=0;port<4;++port){
        if(port)state<<',';
        const auto pad=int32_t(0x80171560+6*port),status=int32_t(0x801B1408+4*port);
        state<<"{\"buttons\":"<<MEM_HU(0,pad)<<",\"x\":"<<int(MEM_B(2,pad))
             <<",\"y\":"<<int(MEM_B(3,pad))<<",\"error\":"<<unsigned(MEM_BU(4,pad))
             <<",\"type\":"<<MEM_HU(0,status)<<",\"status_error\":"<<unsigned(MEM_BU(3,status))<<'}';
    }
    // 305E0 computes99AC and copies it to48EC. 300C4 copies the selected
    // port's48EC toC69C; scene1's script interpreter reads48EC directly.
    state<<"],\"held\":"<<half(0x801A9504)<<",\"pressed\":"<<half(0x801A99AC)
         <<",\"read_enabled\":"<<half(0x801AC6D8)
         <<",\"latched_pressed\":"<<half(0x801A48EC)
         <<",\"selected_port\":"<<half(0x801A39E0)
         <<",\"selected_held\":"<<half(0x801A9AD0)
         <<",\"selected_pressed\":"<<half(0x801AC69C)
         <<",\"native_banks\":"<<dora3_native_loop_owns_frame_banks()
         <<",\"native_gameplay\":"<<dora3_native_loop_gameplay_active()
         <<",\"requested_fps\":"<<dora3_requested_native_fps()
         <<",\"processes\":[";
    for(unsigned slot=0;slot<8;++slot){
        if(slot)state<<',';
        const auto record=0x801A44F0u+slot*20;
        state<<"{\"status\":"<<word(record)<<",\"phase\":"<<word(record+12)
             <<",\"callback\":"<<uint32_t(word(record+16))<<'}';
    }
    state<<"],\"update_enabled\":"<<word(0x801AA648)<<",\"draw_enabled\":"<<word(0x801A9500)
         <<",\"divider\":"<<word(0x801A39DC);
    const bool world_loaded=dora3_overlay_loaded(0x70A50);
    if(world_loaded){
        state<<",\"world\":{\"scripted\":"<<word(0x80218AD8)
             <<",\"pause\":"<<word(0x8021D484)<<",\"transition_wait\":"<<word(0x8021D494)
             <<",\"room_pending\":"<<word(0x802170EC)
             <<",\"demo\":"<<unsigned(MEM_BU(0,int32_t(0x801B36EC)))
             <<",\"start_requested\":"<<half(0x80218B00)<<'}';
    }
#ifdef DORA3_LIVE_RT64
    const auto input=dora3_live_input_snapshot();const auto physical=dora3::port::physical();
    state<<",\"host_buttons\":"<<input.buttons<<",\"host_x\":"<<input.x<<",\"host_y\":"<<input.y
         <<",\"host_enabled\":"<<input.enabled<<",\"focused\":"<<physical.focused
         <<",\"menu\":"<<dora3::port::menu_open()<<",\"ui_blocked\":"<<dora3::port::ui_input_blocked();
#endif
    const auto current=state.str();
    if(!forced&&current==previous[point]&&(visits%300)!=0)return;
    previous[point]=current;
    std::ostringstream detail;
    // Changing clocks/waits do not defeat deduplication. They are included
    // in each written observation, including the independent point heartbeat.
    detail<<",\"steady_ns\":"<<dora3::native_time::monotonic_time()
          <<",\"source_dispatches\":"<<source_dispatches<<",\"source_completions\":"<<source_completions
          <<",\"world_updates\":"<<world_updates<<",\"script_updates\":"<<script_updates
          <<",\"source_tick\":"<<uint32_t(word(0x801AC6B4))
          <<",\"retrace_count\":"<<uint32_t(word(0x801AC694))
          <<",\"accumulator\":"<<half(0x801A44E0)<<",\"history\":"<<half(0x801A9548)
          <<",\"built\":"<<word(0x801A48C0)<<",\"deferred\":"<<word(0x801B1430);
    if(world_loaded&&word(0x80218AD8)==1){
        detail<<",\"script_threads\":[";
        for(unsigned slot=0;slot<8;++slot){
            if(slot)detail<<',';
            const auto record=0x80218350u+slot*16;
            const auto command=uint32_t(word(record));
            detail<<"{\"command\":"<<command<<",\"finished\":"<<MEM_H(4,int32_t(record))
                  <<",\"wait\":"<<MEM_H(6,int32_t(record))<<",\"paused\":"<<MEM_H(10,int32_t(record));
            if(!(command&1)&&command>=0x80000000u&&uint64_t(command)+2<=0x80800000ull)
                detail<<",\"opcode\":"<<half(command);
            detail<<'}';
        }
        detail<<']';
    }
    std::ofstream out(diagnostic_path/"controller-input.jsonl",std::ios::app);
    out<<"{\"schema\":2,\"session\":"<<session<<",\"reason\":\""<<point<<"\",\"polls\":"<<polls
       <<",\"point_visits\":"<<visits<<','<<current<<detail.str()<<"}\n";
    if(!out)DORA3_LOG_TEXT("INPUT SNAPSHOT write failed\n",stderr);
}

extern "C" void dora3_trace(uint8_t* rdram,recomp_context* ctx,uint32_t address,uint32_t section_rom) {
    dora3::profile::Scope profile(dora3::profile::Tag::TraceSample,dora3::profile::sample_trace());
    if(section_rom==0x1000&&address==0x80030380)
        dora3_capture_controller_state(rdram,"after-read");
    if(!dora3::native_time::active_frame_binding()&&!dora3::native_time::active_simulation_slice()) {
        if(section_rom==0x1000&&address==0x800281B8&&uint32_t(ctx->r4)==0)
            dora3_capture_controller_state(rdram,"source-dispatch");
        if(section_rom==0x70A50&&address==0x801BB730)
            dora3_capture_controller_state(rdram,"world-update");
        if(section_rom==0x70A50&&address==0x801E4484) {
            dora3::cutscene_input_before_script(rdram);
            dora3_capture_controller_state(rdram,"script-input");
        }
        if(section_rom==0x635C0&&(address==0x801BB52C||address==0x801BCEF8||
           address==0x801BFAA4||address==0x801C0088||address==0x801C0BBC||address==0x801C24A4))
            dora3_capture_controller_state(rdram,"scene-update");
    }
    if(address==0x80025C00){
        dora3::distance::initialize_storage(rdram);
        // Runtime initially maps every section in the boot 1MiB at entrypoint+ROM offset.
        // Only the resident section is executable there. Remove the rest before game code.
        unload_overlays((int32_t)0x8007EA80,0xA7180);
        for(auto& active:overlay_active)active=false;
        DORA3_LOG(stderr,"OVERLAY BOOT aliases removed; resident only\n");
    }
    if(section_rom==0x1000&&address==0x80030324 &&
       !(MEM_BU(0,int32_t(0x801A48B4))&1)) {
        // Original 80030324 skips all pad reads when port 0 was absent at
        // startup. Its only later status refresh depends on peripheral code
        // which 8003D110 can disable (800DAD20), leaving input dead forever.
        // Retry the real status query before the read gate, independently of
        // peripherals. Do not invent a connected port or bypass OS pad errors.
        // This is an entry hook before the read function's stack prologue;
        // the query may use the same free stack area, but not its registers.
        auto query_context=*ctx;
        dora3_controller_status(rdram,&query_context);
    }
    dora3::retirement::trace(rdram,ctx,address);
    if(address==0x80043390)unload_scene_code();
#ifdef DORA3_DIAGNOSTICS
    const auto overlay=section_rom==0x1000?overlay_count:overlay_index(section_rom);
    if(overlay!=overlay_count&&(!overlay_active[overlay].load()||!overlay_dependencies_active_index(overlay))) {
        DORA3_LOG(stderr,"OVERLAY CALL BEFORE VERIFIED LOAD OR REQUIRED OVERLAY address=%08X rom=%08X\n",address,section_rom);std::_Exit(85);
    }
#endif
    // Resident SDK3955C returns model64. A discrete Native enemy command
    // samples the complete Source step before that genuine getter executes.
    // Outside the guarded enemy command this adapter does nothing.
    if(section_rom==0x1000&&address==0x8003955C)
        dora3_native_enemy_model_read(rdram,uint32_t(ctx->r4));
    if(address==0x80058248)DORA3_LOG(stderr,"ORIGINAL DEBUG format=%08X\n",(uint32_t)ctx->r4);
    if(address==0x80030AE8)resource_rom=(uint32_t)ctx->r5;
    // Byte-wise decoder helpers are covered by the resource comparison below.
    if(address==0x80034000 || address==0x800340D0)return;
    // Keep per-instruction hooks active, but log each function once per host thread.
    // Section ROM is part of identity: overlaid functions may share a VRAM.
#ifdef DORA3_DIAGNOSTICS
    if(first_trace(address,section_rom,overlay))
        DORA3_LOG(stderr,"ENTER %08X section_rom=%08X sp=%08X a0=%08X\n",address,section_rom,(uint32_t)ctx->r29,(uint32_t)ctx->r4);
    if(address==0x800314B4)
        DORA3_LOG(stderr,"SCENE SELECT id=%u state=%u\n",(uint32_t)ctx->r4,(uint32_t)ctx->r5);
#endif
    if(address==0x80026E24 && ctx->r5){
        if(MEM_W(0x10,ctx->r5)==2)audio_submitted.fetch_add(1);
        else if(MEM_W(0x10,ctx->r5)==1)capture_first_graphics(rdram,ctx->r5);
    }
    if(address==0x8002730C){
        // Only the verified full refill call from this game's reader. The original
        // wrapper still performs real queued PI DMA in four256-byte chunks.
        const bool from_stream=stream_dma_call;stream_dma_call=false;
        if(from_stream && ((uint32_t)ctx->r5!=0x8007EF00 || (uint32_t)ctx->r6>0x3FF)){
            DORA3_LOG(stderr,"Unreviewed stream refill shape\n");std::_Exit(86);
        }
        if(from_stream && (uint32_t)ctx->r6==0x3FF){
            ctx->r6=0x400;
            DORA3_LOG(stderr,"STREAM DMA full refill adapted3FF->400 rom=%08X\n",(uint32_t)ctx->r4);
        }
        if(pending_dma.active){DORA3_LOG(stderr,"Unexpected nested DMA wrapper\n");std::_Exit(81);}
        const uint32_t destination=(uint32_t)ctx->r5,size=(uint32_t)ctx->r6;
        for(size_t i=0;i<overlay_active.size();i++)if(overlay_active[i]){
            const auto& region=dora3_overlay_regions[i];
            if(destination<region.ram+region.size && uint64_t(destination)+size>region.ram){
                unload_overlays((int32_t)region.ram,region.size);overlay_active[i]=false;
                DORA3_LOG(stderr,"OVERLAY INVALIDATED before DMA rom=%08X\n",region.rom);
            }
        }
        pending_dma={(uint32_t)ctx->r4,(uint32_t)ctx->r5,(uint32_t)ctx->r6,true};
    }
}

extern "C" void dora3_verify_dma(uint8_t* rdram) {
    const auto request=pending_dma;
#ifdef DORA3_DIAGNOSTICS
    const auto rom=recomp::get_rom();
    if(!request.active || request.rom>rom.size() || request.size>rom.size()-request.rom ||
       request.ram<0x80000000 || (uint64_t)request.ram+request.size>0x80800000){
        DORA3_LOG(stderr,"DMA verification bounds failure\n");std::_Exit(81);
    }
    for(uint32_t i=0;i<request.size;i++){
        const uint8_t actual=MEM_BU(i,(gpr)(int32_t)request.ram);
        if(actual!=rom[request.rom+i]){
            DORA3_LOG(stderr,"DMA MISMATCH rom=%08X ram=%08X offset=%X actual=%02X expected=%02X\n",request.rom,request.ram,i,actual,rom[request.rom+i]);std::_Exit(81);
        }
    }
    DORA3_LOG(stderr,"DMA VERIFIED rom=%08X ram=%08X bytes=%X\n",request.rom,request.ram,request.size);
#endif
    for(size_t i=0;i<overlay_active.size();i++){
        const auto& region=dora3_overlay_regions[i];
        if(request.rom==region.rom && request.ram==region.ram && request.size==region.size){
            if(!overlay_dependencies_active(region.rom)){
                DORA3_LOG(stderr,"OVERLAY LOAD WITHOUT REQUIRED OVERLAY rom=%08X\n",region.rom);std::_Exit(85);
            }
            const auto generation=next_overlay_generation.fetch_add(1);
            if(!generation){DORA3_LOG(stderr,"OVERLAY generation exhausted\n");std::_Exit(85);}
            load_overlays(region.rom,(int32_t)region.ram,region.size);
            overlay_generation[i]=generation;overlay_active[i]=true;
            DORA3_LOG(stderr,"OVERLAY REGISTERED rom=%08X ram=%08X bytes=%X\n",region.rom,region.ram,region.size);
        }
    }
    pending_dma.active=false;
}

// Capture each resource before callers relocate/mutate it. The validating runner
// compares bytes against the independent Python decoder; never publish these files.
extern "C" void dora3_capture_resource(uint8_t* rdram,recomp_context* ctx){
    if constexpr(!dora3::diagnostics::enabled)return;
    const uint32_t slot=(uint32_t)ctx->r2;
    if(slot>=dora3::distance::resource_capacity){DORA3_LOG(stderr,"RESOURCE invalid slot\n");std::_Exit(86);}
    const gpr entry=(gpr)(int32_t)(dora3::distance::resources+slot*0x14);
    const uint32_t address=MEM_W(0,entry),size=MEM_W(8,entry);
    if(address<0x80000000 || uint64_t(address)+size>0x80800000 || !size){
        DORA3_LOG(stderr,"RESOURCE invalid buffer\n");std::_Exit(86);
    }
    const auto directory=diagnostic_path/"resources";
    std::filesystem::create_directories(directory);
    char name[32];std::snprintf(name,sizeof(name),"%08X.bin",resource_rom);
    std::vector<uint8_t> data(size);
    for(uint32_t i=0;i<size;i++)data[i]=MEM_BU(i,(gpr)(int32_t)address);
    std::ofstream out(directory/name,std::ios::binary);
    out.write(reinterpret_cast<const char*>(data.data()),data.size());out.close();
    if(!out){DORA3_LOG(stderr,"RESOURCE capture failed\n");std::_Exit(86);}
    DORA3_LOG(stderr,"RESOURCE CAPTURE rom=%08X ram=%08X bytes=%X slot=%u\n",resource_rom,address,size,slot);
}

class DiagnosticContext final : public ultramodern::renderer::RendererContext {
public:
    DiagnosticContext(){ setup_result=ultramodern::renderer::SetupResult::Success;chosen_api=ultramodern::renderer::GraphicsApi::Auto; }
    bool valid() override {return true;}
    bool update_config(const ultramodern::renderer::GraphicsConfig&,const ultramodern::renderer::GraphicsConfig&) override{return true;}
    void enable_instant_present() override{}
    void send_dl(const OSTask* task) override {
        DORA3_LOG(stderr,"GRAPHICS TASK ucode=%08X size=%X data=%08X bytes=%X; renderer not connected\n",(uint32_t)task->t.ucode,task->t.ucode_size,(uint32_t)task->t.data_ptr,task->t.data_size);
        stop_diagnostic(78);
    }
    void send_dummy_workload(uint32_t) override{}
    void update_screen(bool=false) override{
        // This callback runs on the owner of get_vi_regs(). Publish a copy;
        // the scheduler must never read that graphics-thread storage directly.
        std::lock_guard lock(captured_vi_mutex);
        captured_vi=*ultramodern::renderer::get_vi_regs();
        captured_vi_ready=true;
    }
    void reset_game() override{}
    void shutdown() override{}
    uint32_t get_display_framerate() const override{return 60;}
    float get_resolution_scale() const override{return 1.0f;}
};

#ifdef _WIN32
int wmain(int argc,wchar_t**){
    if(argc!=1){DORA3_LOG(stderr,"Start the EXE without arguments; ROM must be beside it.\n");return 2;}
    std::vector<wchar_t> executable(32768);
    const auto length=GetModuleFileNameW(nullptr,executable.data(),DWORD(executable.size()));
    if(length==0 || length>=executable.size())return 2;
    const auto directory=std::filesystem::path(executable.data()).parent_path();
    const auto rom_path=directory/L"Doraemon 3 - Nobita no Machi SOS! (J) [!].z64";
    if(!std::filesystem::is_regular_file(rom_path)){
        MessageBoxW(nullptr,L"Place Doraemon 3 - Nobita no Machi SOS! (J) [!].z64 next to this EXE.",L"Dora64-3: ROM not found",MB_OK|MB_ICONERROR);return 2;
    }
#ifdef DORA3_DIAGNOSTICS
    const auto stamp=std::chrono::system_clock::now().time_since_epoch();
    const auto run_dir=directory/L"logs"/std::to_wstring(std::chrono::duration_cast<std::chrono::milliseconds>(stamp).count());
    if(!std::filesystem::create_directories(run_dir))return 2;
    FILE* stream=nullptr;
    if(_wfreopen_s(&stream,(run_dir/L"runtime.log").c_str(),L"w",stderr)!=0)return 2;
    if(_wfreopen_s(&stream,(run_dir/L"stdout.log").c_str(),L"w",stdout)!=0)return 2;
    DORA3_LOG(stderr,"BUILD executable=%ls\n",std::filesystem::path(executable.data()).filename().c_str());
    const auto state_dir=run_dir/L"state";
    std::filesystem::create_directories(state_dir);
    std::setvbuf(stderr,nullptr,_IONBF,0);
    DORA3_LOG(stderr,"Dora64-3 Windows test: RT64 Automatic (Dora64 policy); ROM beside EXE only; pending calls stop explicitly.\n");
#elif defined(DORA3_GRAPHICS_PROFILE)
    // One aggregated profile only; ordinary diagnostics remain disabled.
    const auto stamp=std::chrono::system_clock::now().time_since_epoch();
    const auto state_dir=directory/L"logs"/std::to_wstring(std::chrono::duration_cast<std::chrono::milliseconds>(stamp).count())/L"state";
    if(!std::filesystem::create_directories(state_dir))return 2;
    FILE* stream=nullptr;
    if(_wfreopen_s(&stream,L"NUL",L"w",stderr)!=0)return 2;
    if(_wfreopen_s(&stream,L"NUL",L"w",stdout)!=0)return 2;
#else
    // No session directories, files or unbuffered writes in the speed build.
    const auto state_dir=directory;
    FILE* stream=nullptr;
    if(_wfreopen_s(&stream,L"NUL",L"w",stderr)!=0)return 2;
    if(_wfreopen_s(&stream,L"NUL",L"w",stdout)!=0)return 2;
#endif
#else
int main(int argc,char** argv){
    if(argc!=3){DORA3_LOG(stderr,"Usage: dora3_startup ROM PRIVATE_RUN_DIRECTORY\n");return 2;}
    const auto rom_path=std::filesystem::absolute(argv[1]);
    const auto state_dir=std::filesystem::absolute(argv[2]);
    if(std::filesystem::exists(state_dir)){DORA3_LOG(stderr,"Use a fresh run directory\n");return 2;}
    std::filesystem::create_directories(state_dir);
    std::setvbuf(stderr,nullptr,_IONBF,0);
#endif
    #ifdef DORA3_LIVE_RT64
    if(std::getenv("DORA3_CAPTURE_GRAPHICS")){DORA3_LOG(stderr,"Use the diagnostic target for first-task capture\n");return 2;}
    #endif
    std::ifstream input(rom_path,std::ios::binary);
    std::vector<char> rom{std::istreambuf_iterator<char>(input),{}};
    constexpr uint64_t expected_rom_hash=0xF3BFFF0C58827510ULL;
    if(rom.size()!=0x1000000 || XXH3_64bits(rom.data(),rom.size())!=expected_rom_hash){DORA3_LOG(stderr,"Unsupported ROM; exact SHA-256 also checked by run_startup.py\n");return 2;}
    recomp::overlays::register_overlays({section_table,ARRLEN(section_table),num_sections},{overlay_sections_by_index,ARRLEN(overlay_sections_by_index)});
    diagnostic_path=state_dir;
#ifdef DORA3_GRAPHICS_PROFILE
    dora3::graphics_profile::configure(state_dir/"graphics-profile.jsonl");
#endif
    #ifdef DORA3_PROFILE
    dora3::profile::configure(state_dir/"performance.jsonl");
    DORA3_LOG_TEXT("PROFILER enabled: state/performance.jsonl; gameplay logic remains068\n",stderr);
    #endif
    #ifdef _WIN32
    // Test builds beside one another share settings and EEPROM; only diagnostics
    // live under a timestamped run directory. Runtime appends saves/<game_id>.bin.
    recomp::register_config_path(directory);
    DORA3_LOG(stderr,"SHARED CONFIG root=%ls; saves=%ls\n",directory.c_str(),(directory/L"saves").c_str());
    #elif defined(DORA3_LIVE_RT64)
    // Same desktop policy as Dora64: settings and saves beside the selected ROM.
    recomp::register_config_path(rom_path.parent_path());
    #else
    recomp::register_config_path(state_dir);
    #endif
    recomp::GameEntry game{};
    game.rom_hash=expected_rom_hash;
    game.internal_name=std::string(rom.data()+0x20,20);
    while(!game.internal_name.empty() && (game.internal_name.back()==' ' || game.internal_name.back()=='\0'))game.internal_name.pop_back();
    game.display_name="Dora64-3 startup research";game.game_id=u8"dora3.n64.jp";
    game.save_type=recomp::SaveType::Eep16k;game.is_enabled=true;
    game.entrypoint_address=(int32_t)0x80025C00;game.entrypoint=recomp_entrypoint;
    game.rom_path=rom_path;
    game.on_init_callback=[](uint8_t* rdram,recomp_context* ctx){
        dora3::distance::reset_storage();
        // Initial boot and reset both run here before any new guest thread.
        // Runtime already reset its overlay mappings; retain monotonic load IDs.
        for(auto& active:overlay_active)active=false;
        for(auto& generation:overlay_generation)generation=0;
        pending_dma={};resource_rom=0;stream_dma_call=false;
        dora3::hw::reset_session();dora3_music_reset(rdram,ctx);
        dora3::reset_cutscene_input();
        dora3::camera::reset();
        audio_submitted=0;audio_completed=0;pcm_buffers=0;
        dora3_capture_controller_state(rdram,"boot");
        std::lock_guard lock(captured_vi_mutex);
        captured_vi={};captured_vi_ready=false;
    };
    game.thread_create_callback=[](uint8_t* rdram,recomp_context* ctx){
        DORA3_LOG(stderr,"THREAD CONTEXT sp=%08X arg=%08X\n",(uint32_t)ctx->r29,(uint32_t)ctx->r4);
    };
    recomp::register_game(game);
    // Identification is complete; the runtime loads and owns the game ROM.
    input.close();
    std::vector<char>().swap(rom);
    recomp::Configuration cfg{};cfg.project_version={0,0,1};
    cfg.rsp_callbacks.get_rsp_microcode=[](const OSTask* task)->RspUcodeFunc*{
        DORA3_LOG(stderr,"RSP TASK type=%u ucode=%08X size=%X data=%08X commands=%08X bytes=%X\n",task->t.type,(uint32_t)task->t.ucode,task->t.ucode_size,(uint32_t)task->t.ucode_data,(uint32_t)task->t.data_ptr,task->t.data_size);
        const uint32_t commands=(uint32_t)task->t.data_ptr&0x1FFFFFFF;
        if(task->t.type==2 && ((uint32_t)task->t.ucode&0x1FFFFFFF)==0x626F0 &&
           task->t.ucode_size==0x1000 && ((uint32_t)task->t.ucode_data&0x1FFFFFFF)==0x78B40 &&
           task->t.ucode_data_size==0x800 && !(commands&7) && task->t.data_size>0 &&
           !(task->t.data_size&7) && commands<0x400000 && task->t.data_size<=0x400000-commands){
            return checked_audio_rsp;
        }
        DORA3_LOG(stderr,"UNREVIEWED RSP TASK; stopped\n");std::_Exit(78);
    };
    #ifndef DORA3_LIVE_RT64
    cfg.renderer_callbacks.create_render_context=[](uint8_t*,ultramodern::renderer::WindowHandle,bool)->std::unique_ptr<ultramodern::renderer::RendererContext>{return std::make_unique<DiagnosticContext>();};
    cfg.gfx_callbacks.create_window=[](void*)->ultramodern::renderer::WindowHandle{return {};};
    cfg.gfx_callbacks.update_gfx=[](void*){
        static const auto start=std::chrono::steady_clock::now();
        if(std::chrono::steady_clock::now()-start>std::chrono::seconds(15)){
            DORA3_LOG(stderr,"TIMEOUT: no pending boundary/task reached in 15 seconds\n");std::_Exit(79);
        }
    };
    #else
    dora3_configure_live_graphics(cfg);
    #endif
    cfg.audio_callbacks.get_frames_remaining=[]()->size_t{
        #ifdef DORA3_LIVE_RT64
        return dora3_audio_remaining();
        #else
        return 0;
        #endif
    };
    cfg.audio_callbacks.queue_samples=[](int16_t* samples,size_t count){
#ifdef DORA3_DIAGNOSTICS
        pcm_buffers.fetch_add(1);
        size_t nonzero=0;for(size_t i=0;i<count;i++)nonzero+=samples[i]!=0;
        DORA3_LOG(stderr,"PCM OUTPUT reached samples=%zu frames=%zu nonzero=%zu\n",count,count/2,nonzero);
#endif
        #ifndef DORA3_LIVE_RT64
        stop_diagnostic(78);
        #else
        dora3_audio_queue(samples,count);
        #endif
    };
    cfg.audio_callbacks.set_frequency=[](uint32_t f){
        DORA3_LOG(stderr,"AUDIO FREQUENCY %u\n",f);
        #ifdef DORA3_LIVE_RT64
        dora3_audio_frequency(f);
        #endif
    };
    cfg.events_callbacks.vi_callback=[](){static std::atomic<bool> started=false;if(!started.exchange(true))recomp::start_game(u8"dora3.n64.jp","");};
    cfg.error_handling_callbacks.message_box=[](const char* m){DORA3_LOG(stderr,"RUNTIME ERROR %s\n",m);std::_Exit(80);};
    #ifdef DORA3_LIVE_RT64
    DORA3_LOG(stderr,"RESEARCH: live RT64, deferred graphics SP completion; SDL input/audio, EEPROM 16k; not a playable port\n");
    #else
    DORA3_LOG(stderr,"DIAGNOSTIC: no renderer/audio output, stop codes 77 pending / 78 task / 79 timeout\n");
    #endif
    dora3_configure_hardware(cfg);
    recomp::start(cfg);
    #ifdef DORA3_LIVE_RT64
    dora3_shutdown_live_io();SDL_Quit();
    #endif
    return 0;
}
