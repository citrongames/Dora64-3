#include "resource_retirement.hpp"
// Research frontend: runs reviewed original functions through the real runtime.
// Unreviewed functions stop explicitly. The default mode stops at graphics/PCM;
// the optional live RT64 mode continues through SDL input/audio.
// The live mode has SDL input/audio; no validated gameplay loop yet.
#include <atomic>
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
#include "ultramodern/ultramodern.hpp"
#include "librecomp/game.hpp"
#include "librecomp/overlays.hpp"
#include "recomp_overlays.inl"
#include "overlay_regions.hpp"
#ifdef DORA3_LIVE_RT64
#include "live_graphics.hpp"
#include "live_io.hpp"
#endif
#define XXH_INLINE_ALL
#include "xxHash/xxhash.h"

RspExitReason dora3_aspMain(uint8_t*, uint32_t);
void dora3_configure_hardware(const recomp::Configuration&);
namespace {
std::array<std::atomic<bool>,std::size(dora3_overlay_regions)> overlay_active{};
std::filesystem::path diagnostic_path;
bool overlay_dependencies_active(uint32_t rom){
    for(const auto& dependency:dora3_overlay_dependencies)
        if(dependency.rom==rom && !overlay_active[dependency.required_index].load())return false;
    return true;
}
void unload_scene_code(){
    for(size_t i=0;i<overlay_active.size();i++)if(overlay_active[i]){
        const auto& region=dora3_overlay_regions[i];
        unload_overlays((int32_t)region.ram,region.size);overlay_active[i]=false;
        std::fprintf(stderr,"OVERLAY UNLOADED rom=%08X ram=%08X\n",region.rom,region.ram);
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
    std::fprintf(stderr,"DIAGNOSTIC STOP code=%d audio_submitted=%u audio_completed=%u\n",first_code.load(),audio_submitted.load(),audio_completed.load());
    std::fflush(nullptr);std::_Exit(first_code.load());
}
RspExitReason checked_audio_rsp(uint8_t* rdram, uint32_t ucode_addr) {
    const auto rom=recomp::get_rom();
    // Verify actual loaded code, not just a task address. OSTask declares a
    // padded 0x1000-byte range; only 0xE20 bytes are compiled instructions.
    for(uint32_t i=0;i<0xE20;i++){
        if(MEM_BU(i,(gpr)(int32_t)(ucode_addr|0x80000000U))!=rom[0x3DAF0+i]){
            std::fprintf(stderr,"AUDIO RSP loaded text mismatch at %X\n",i);std::_Exit(82);
        }
    }
    const auto reason=dora3_aspMain(rdram,ucode_addr);
    std::fprintf(stderr,"AUDIO RSP COMPLETED reason=%u\n",(unsigned)reason);
    if(reason!=RspExitReason::Broke)std::_Exit(82);
    audio_completed.fetch_add(1);
    return reason;
}
// Private Linux/x86-64 research capture: little-endian words, byteswapped RDRAM.
// Captured before osSpTaskLoad/StartGo and before graphics SP completion.
void capture_first_graphics(uint8_t* rdram,gpr sc_task){
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
        if(!captured_vi_ready){std::fprintf(stderr,"GRAPHICS CAPTURE has no published VI snapshot\n");std::_Exit(83);}
        std::memcpy(header.data()+20,&captured_vi,14*sizeof(uint32_t));
    }
    header[34]=MEM_W(0xC,sc_task);
    const auto path=diagnostic_path/"first-graphics.snapshot";
    std::ofstream out(path,std::ios::binary);
    out.write(reinterpret_cast<const char*>(header.data()),sizeof(header));
    out.write(reinterpret_cast<const char*>(rdram),0x400000);
    out.close();
    if(!out){std::fprintf(stderr,"GRAPHICS CAPTURE failed\n");std::_Exit(83);}
    std::fprintf(stderr,"GRAPHICS CAPTURE saved immutable first-task snapshot, framebuffer=%08X\n",header[34]);
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
    for(size_t i=0;i<overlay_active.size();++i)
        if(dora3_overlay_regions[i].rom==section_rom)
            return overlay_active[i].load()&&overlay_dependencies_active(section_rom);
    return 0;
}

extern "C" void dora3_pending(uint32_t address) {
    std::fprintf(stderr,"PENDING original function %08X; execution stopped, no success stub\n",address);
    stop_diagnostic(77);
}
extern "C" void dora3_trace(uint8_t* rdram,recomp_context* ctx,uint32_t address,uint32_t section_rom) {
    if(address==0x80025C00){
        // Runtime initially maps every section in the boot 1MiB at entrypoint+ROM offset.
        // Only the resident section is executable there. Remove the rest before game code.
        unload_overlays((int32_t)0x8007EA80,0xA7180);
        for(auto& active:overlay_active)active=false;
        std::fprintf(stderr,"OVERLAY BOOT aliases removed; resident only\n");
    }
    dora3::retirement::trace(rdram,ctx,address);
    if(address==0x80043390)unload_scene_code();
    for(size_t i=0;i<overlay_active.size();i++){
        const auto& region=dora3_overlay_regions[i];
        if(region.rom==section_rom && (!overlay_active[i] || !overlay_dependencies_active(section_rom))){
            std::fprintf(stderr,"OVERLAY CALL BEFORE VERIFIED LOAD OR REQUIRED OVERLAY address=%08X rom=%08X\n",address,section_rom);std::_Exit(85);
        }
    }
    if(address==0x80058248)std::fprintf(stderr,"ORIGINAL DEBUG format=%08X\n",(uint32_t)ctx->r4);
    if(address==0x80030AE8)resource_rom=(uint32_t)ctx->r5;
    // Byte-wise decoder helpers are covered by the resource comparison below.
    if(address==0x80034000 || address==0x800340D0)return;
    // Keep per-instruction hooks active, but log each function once per host thread.
    // Section ROM is part of identity: overlaid functions may share a VRAM.
    static thread_local std::unordered_set<uint64_t> traced;
    if(traced.insert((uint64_t(section_rom)<<32)|address).second)
        std::fprintf(stderr,"ENTER %08X section_rom=%08X sp=%08X a0=%08X\n",address,section_rom,(uint32_t)ctx->r29,(uint32_t)ctx->r4);
    if(address==0x800314B4)
        std::fprintf(stderr,"SCENE SELECT id=%u state=%u\n",(uint32_t)ctx->r4,(uint32_t)ctx->r5);
    if(address==0x80026E24 && ctx->r5){
        if(MEM_W(0x10,ctx->r5)==2)audio_submitted.fetch_add(1);
        else if(MEM_W(0x10,ctx->r5)==1)capture_first_graphics(rdram,ctx->r5);
    }
    if(address==0x8002730C){
        // Only the verified full refill call from this game's reader. The original
        // wrapper still performs real queued PI DMA in four256-byte chunks.
        const bool from_stream=stream_dma_call;stream_dma_call=false;
        if(from_stream && ((uint32_t)ctx->r5!=0x8007EF00 || (uint32_t)ctx->r6>0x3FF)){
            std::fprintf(stderr,"Unreviewed stream refill shape\n");std::_Exit(86);
        }
        if(from_stream && (uint32_t)ctx->r6==0x3FF){
            ctx->r6=0x400;
            std::fprintf(stderr,"STREAM DMA full refill adapted3FF->400 rom=%08X\n",(uint32_t)ctx->r4);
        }
        if(pending_dma.active){std::fprintf(stderr,"Unexpected nested DMA wrapper\n");std::_Exit(81);}
        const uint32_t destination=(uint32_t)ctx->r5,size=(uint32_t)ctx->r6;
        for(size_t i=0;i<overlay_active.size();i++)if(overlay_active[i]){
            const auto& region=dora3_overlay_regions[i];
            if(destination<region.ram+region.size && uint64_t(destination)+size>region.ram){
                unload_overlays((int32_t)region.ram,region.size);overlay_active[i]=false;
                std::fprintf(stderr,"OVERLAY INVALIDATED before DMA rom=%08X\n",region.rom);
            }
        }
        pending_dma={(uint32_t)ctx->r4,(uint32_t)ctx->r5,(uint32_t)ctx->r6,true};
    }
}

extern "C" void dora3_verify_dma(uint8_t* rdram) {
    const auto rom=recomp::get_rom();
    const auto request=pending_dma;
    if(!request.active || request.rom>rom.size() || request.size>rom.size()-request.rom ||
       request.ram<0x80000000 || (uint64_t)request.ram+request.size>0x80400000){
        std::fprintf(stderr,"DMA verification bounds failure\n");std::_Exit(81);
    }
    for(uint32_t i=0;i<request.size;i++){
        const uint8_t actual=MEM_BU(i,(gpr)(int32_t)request.ram);
        if(actual!=rom[request.rom+i]){
            std::fprintf(stderr,"DMA MISMATCH rom=%08X ram=%08X offset=%X actual=%02X expected=%02X\n",request.rom,request.ram,i,actual,rom[request.rom+i]);std::_Exit(81);
        }
    }
    std::fprintf(stderr,"DMA VERIFIED rom=%08X ram=%08X bytes=%X\n",request.rom,request.ram,request.size);
    for(size_t i=0;i<overlay_active.size();i++){
        const auto& region=dora3_overlay_regions[i];
        if(request.rom==region.rom && request.ram==region.ram && request.size==region.size){
            if(!overlay_dependencies_active(region.rom)){
                std::fprintf(stderr,"OVERLAY LOAD WITHOUT REQUIRED OVERLAY rom=%08X\n",region.rom);std::_Exit(85);
            }
            load_overlays(region.rom,(int32_t)region.ram,region.size);overlay_active[i]=true;
            std::fprintf(stderr,"OVERLAY REGISTERED rom=%08X ram=%08X bytes=%X\n",region.rom,region.ram,region.size);
        }
    }
    pending_dma.active=false;
}

// Capture each resource before callers relocate/mutate it. The validating runner
// compares bytes against the independent Python decoder; never publish these files.
extern "C" void dora3_capture_resource(uint8_t* rdram,recomp_context* ctx){
    const uint32_t slot=(uint32_t)ctx->r2;
    if(slot>=512){std::fprintf(stderr,"RESOURCE invalid slot\n");std::_Exit(86);}
    const gpr entry=(gpr)(int32_t)(0x801AEC00+slot*0x14);
    const uint32_t address=MEM_W(0,entry),size=MEM_W(8,entry);
    if(address<0x80000000 || uint64_t(address)+size>0x80400000 || !size){
        std::fprintf(stderr,"RESOURCE invalid buffer\n");std::_Exit(86);
    }
    const auto directory=diagnostic_path/"resources";
    std::filesystem::create_directories(directory);
    char name[32];std::snprintf(name,sizeof(name),"%08X.bin",resource_rom);
    std::vector<uint8_t> data(size);
    for(uint32_t i=0;i<size;i++)data[i]=MEM_BU(i,(gpr)(int32_t)address);
    std::ofstream out(directory/name,std::ios::binary);
    out.write(reinterpret_cast<const char*>(data.data()),data.size());out.close();
    if(!out){std::fprintf(stderr,"RESOURCE capture failed\n");std::_Exit(86);}
    std::fprintf(stderr,"RESOURCE CAPTURE rom=%08X ram=%08X bytes=%X slot=%u\n",resource_rom,address,size,slot);
}

class DiagnosticContext final : public ultramodern::renderer::RendererContext {
public:
    DiagnosticContext(){ setup_result=ultramodern::renderer::SetupResult::Success;chosen_api=ultramodern::renderer::GraphicsApi::Auto; }
    bool valid() override {return true;}
    bool update_config(const ultramodern::renderer::GraphicsConfig&,const ultramodern::renderer::GraphicsConfig&) override{return true;}
    void enable_instant_present() override{}
    void send_dl(const OSTask* task) override {
        std::fprintf(stderr,"GRAPHICS TASK ucode=%08X size=%X data=%08X bytes=%X; renderer not connected\n",(uint32_t)task->t.ucode,task->t.ucode_size,(uint32_t)task->t.data_ptr,task->t.data_size);
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
    if(argc!=1){std::fprintf(stderr,"Start the EXE without arguments; ROM must be beside it.\n");return 2;}
    std::vector<wchar_t> executable(32768);
    const auto length=GetModuleFileNameW(nullptr,executable.data(),DWORD(executable.size()));
    if(length==0 || length>=executable.size())return 2;
    const auto directory=std::filesystem::path(executable.data()).parent_path();
    const auto rom_path=directory/L"Doraemon 3 - Nobita no Machi SOS! (J) [!].z64";
    if(!std::filesystem::is_regular_file(rom_path)){
        MessageBoxW(nullptr,L"Place Doraemon 3 - Nobita no Machi SOS! (J) [!].z64 next to this EXE.",L"Dora64-3: ROM not found",MB_OK|MB_ICONERROR);return 2;
    }
    const auto stamp=std::chrono::system_clock::now().time_since_epoch();
    const auto run_dir=directory/L"logs"/std::to_wstring(std::chrono::duration_cast<std::chrono::milliseconds>(stamp).count());
    if(!std::filesystem::create_directories(run_dir))return 2;
    FILE* stream=nullptr;
    if(_wfreopen_s(&stream,(run_dir/L"runtime.log").c_str(),L"w",stderr)!=0)return 2;
    if(_wfreopen_s(&stream,(run_dir/L"stdout.log").c_str(),L"w",stdout)!=0)return 2;
    std::fprintf(stderr,"BUILD executable=%ls\n",std::filesystem::path(executable.data()).filename().c_str());
    const auto state_dir=run_dir/L"state";
    std::filesystem::create_directories(state_dir);
    std::setvbuf(stderr,nullptr,_IONBF,0);
    std::fprintf(stderr,"Dora64-3 Windows test: RT64 Automatic (Dora64 policy); ROM beside EXE only; pending calls stop explicitly.\n");
#else
int main(int argc,char** argv){
    if(argc!=3){std::fprintf(stderr,"Usage: dora3_startup ROM PRIVATE_RUN_DIRECTORY\n");return 2;}
    const auto rom_path=std::filesystem::absolute(argv[1]);
    const auto state_dir=std::filesystem::absolute(argv[2]);
    if(std::filesystem::exists(state_dir)){std::fprintf(stderr,"Use a fresh run directory\n");return 2;}
    std::filesystem::create_directories(state_dir);
    std::setvbuf(stderr,nullptr,_IONBF,0);
#endif
    #ifdef DORA3_LIVE_RT64
    if(std::getenv("DORA3_CAPTURE_GRAPHICS")){std::fprintf(stderr,"Use the diagnostic target for first-task capture\n");return 2;}
    #endif
    std::ifstream input(rom_path,std::ios::binary);
    std::vector<char> rom{std::istreambuf_iterator<char>(input),{}};
    constexpr uint64_t expected_rom_hash=0xF3BFFF0C58827510ULL;
    if(rom.size()!=0x1000000 || XXH3_64bits(rom.data(),rom.size())!=expected_rom_hash){std::fprintf(stderr,"Unsupported ROM; exact SHA-256 also checked by run_startup.py\n");return 2;}
    recomp::overlays::register_overlays({section_table,ARRLEN(section_table),num_sections},{overlay_sections_by_index,ARRLEN(overlay_sections_by_index)});
    diagnostic_path=state_dir;
    #ifdef _WIN32
    // Test builds beside one another share settings and EEPROM; only diagnostics
    // live under a timestamped run directory. Runtime appends saves/<game_id>.bin.
    recomp::register_config_path(directory);
    std::fprintf(stderr,"SHARED CONFIG root=%ls; saves=%ls\n",directory.c_str(),(directory/L"saves").c_str());
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
    game.thread_create_callback=[](uint8_t* rdram,recomp_context* ctx){
        std::fprintf(stderr,"THREAD CONTEXT sp=%08X arg=%08X\n",(uint32_t)ctx->r29,(uint32_t)ctx->r4);
    };
    recomp::register_game(game);
    // Identification is complete; the runtime loads and owns the game ROM.
    input.close();
    std::vector<char>().swap(rom);
    recomp::Configuration cfg{};cfg.project_version={0,0,1};
    cfg.rsp_callbacks.get_rsp_microcode=[](const OSTask* task)->RspUcodeFunc*{
        std::fprintf(stderr,"RSP TASK type=%u ucode=%08X size=%X data=%08X commands=%08X bytes=%X\n",task->t.type,(uint32_t)task->t.ucode,task->t.ucode_size,(uint32_t)task->t.ucode_data,(uint32_t)task->t.data_ptr,task->t.data_size);
        const uint32_t commands=(uint32_t)task->t.data_ptr&0x1FFFFFFF;
        if(task->t.type==2 && ((uint32_t)task->t.ucode&0x1FFFFFFF)==0x626F0 &&
           task->t.ucode_size==0x1000 && ((uint32_t)task->t.ucode_data&0x1FFFFFFF)==0x78B40 &&
           task->t.ucode_data_size==0x800 && !(commands&7) && task->t.data_size>0 &&
           !(task->t.data_size&7) && commands<0x400000 && task->t.data_size<=0x400000-commands){
            return checked_audio_rsp;
        }
        std::fprintf(stderr,"UNREVIEWED RSP TASK; stopped\n");std::_Exit(78);
    };
    #ifndef DORA3_LIVE_RT64
    cfg.renderer_callbacks.create_render_context=[](uint8_t*,ultramodern::renderer::WindowHandle,bool)->std::unique_ptr<ultramodern::renderer::RendererContext>{return std::make_unique<DiagnosticContext>();};
    cfg.gfx_callbacks.create_window=[](void*)->ultramodern::renderer::WindowHandle{return {};};
    cfg.gfx_callbacks.update_gfx=[](void*){
        static const auto start=std::chrono::steady_clock::now();
        if(std::chrono::steady_clock::now()-start>std::chrono::seconds(15)){
            std::fprintf(stderr,"TIMEOUT: no pending boundary/task reached in 15 seconds\n");std::_Exit(79);
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
        pcm_buffers.fetch_add(1);
        size_t nonzero=0;for(size_t i=0;i<count;i++)nonzero+=samples[i]!=0;
        std::fprintf(stderr,"PCM OUTPUT reached samples=%zu frames=%zu nonzero=%zu\n",count,count/2,nonzero);
        #ifndef DORA3_LIVE_RT64
        stop_diagnostic(78);
        #else
        dora3_audio_queue(samples,count);
        #endif
    };
    cfg.audio_callbacks.set_frequency=[](uint32_t f){
        std::fprintf(stderr,"AUDIO FREQUENCY %u\n",f);
        #ifdef DORA3_LIVE_RT64
        dora3_audio_frequency(f);
        #endif
    };
    cfg.events_callbacks.vi_callback=[](){static std::atomic<bool> started=false;if(!started.exchange(true))recomp::start_game(u8"dora3.n64.jp","");};
    cfg.error_handling_callbacks.message_box=[](const char* m){std::fprintf(stderr,"RUNTIME ERROR %s\n",m);std::_Exit(80);};
    #ifdef DORA3_LIVE_RT64
    std::fprintf(stderr,"RESEARCH: live RT64, deferred graphics SP completion; SDL input/audio, EEPROM 16k; not a playable port\n");
    #else
    std::fprintf(stderr,"DIAGNOSTIC: no renderer/audio output, stop codes 77 pending / 78 task / 79 timeout\n");
    #endif
    dora3_configure_hardware(cfg);
    recomp::start(cfg);
    #ifdef DORA3_LIVE_RT64
    dora3_shutdown_live_io();SDL_Quit();
    #endif
    return 0;
}
