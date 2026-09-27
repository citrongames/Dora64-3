// Research frontend: runs reviewed original functions through the real runtime.
// Unreviewed functions stop explicitly. The default mode stops at graphics/PCM;
// the optional Linux RT64 mode continues with measured, discarded PCM.
// Neither mode has an audio device or a validated gameplay loop.
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
#include "ultramodern/ultramodern.hpp"
#include "librecomp/game.hpp"
#include "librecomp/overlays.hpp"
#include "recomp_overlays.inl"
#ifdef DORA3_LIVE_RT64
#include "live_graphics.hpp"
#endif
#define XXH_INLINE_ALL
#include "xxHash/xxhash.h"

RspExitReason dora3_aspMain(uint8_t*, uint32_t);
namespace {
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
    const auto path=recomp::get_config_path()/"first-graphics.snapshot";
    std::ofstream out(path,std::ios::binary);
    out.write(reinterpret_cast<const char*>(header.data()),sizeof(header));
    out.write(reinterpret_cast<const char*>(rdram),0x400000);
    out.close();
    if(!out){std::fprintf(stderr,"GRAPHICS CAPTURE failed\n");std::_Exit(83);}
    std::fprintf(stderr,"GRAPHICS CAPTURE saved immutable first-task snapshot, framebuffer=%08X\n",header[34]);
}
struct PendingDma { uint32_t rom, ram, size; bool active; };
thread_local PendingDma pending_dma{};
}

extern "C" void dora3_pending(uint32_t address) {
    std::fprintf(stderr,"PENDING original function %08X; execution stopped, no success stub\n",address);
    stop_diagnostic(77);
}
extern "C" void dora3_trace(uint8_t* rdram,recomp_context* ctx,uint32_t address) {
    std::fprintf(stderr,"ENTER %08X sp=%08X a0=%08X\n",address,(uint32_t)ctx->r29,(uint32_t)ctx->r4);
    if(address==0x80026E24 && ctx->r5){
        if(MEM_W(0x10,ctx->r5)==2)audio_submitted.fetch_add(1);
        else if(MEM_W(0x10,ctx->r5)==1)capture_first_graphics(rdram,ctx->r5);
    }
    if(address==0x8002730C){
        if(pending_dma.active){std::fprintf(stderr,"Unexpected nested DMA wrapper\n");std::_Exit(81);}
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
    pending_dma.active=false;
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

int main(int argc,char** argv){
    if(argc!=3){std::fprintf(stderr,"Usage: dora3_startup ROM PRIVATE_RUN_DIRECTORY\n");return 2;}
    #ifdef DORA3_LIVE_RT64
    if(std::getenv("DORA3_CAPTURE_GRAPHICS")){std::fprintf(stderr,"Use the diagnostic target for first-task capture\n");return 2;}
    #endif
    std::setvbuf(stderr,nullptr,_IONBF,0);
    const auto run_dir=std::filesystem::absolute(argv[2]);
    if(std::filesystem::exists(run_dir)){std::fprintf(stderr,"Use a fresh run directory\n");return 2;}
    std::filesystem::create_directories(run_dir);
    std::ifstream input(argv[1],std::ios::binary);
    std::vector<char> rom{std::istreambuf_iterator<char>(input),{}};
    constexpr uint64_t expected_rom_hash=0xF3BFFF0C58827510ULL;
    if(rom.size()!=0x1000000 || XXH3_64bits(rom.data(),rom.size())!=expected_rom_hash){std::fprintf(stderr,"Unsupported ROM; exact SHA-256 also checked by run_startup.py\n");return 2;}
    recomp::overlays::register_overlays({section_table,ARRLEN(section_table),num_sections},{overlay_sections_by_index,ARRLEN(overlay_sections_by_index)});
    recomp::register_config_path(run_dir);
    recomp::GameEntry game{};
    game.rom_hash=expected_rom_hash;
    game.internal_name=std::string(rom.data()+0x20,20);
    while(!game.internal_name.empty() && (game.internal_name.back()==' ' || game.internal_name.back()=='\0'))game.internal_name.pop_back();
    game.display_name="Dora64-3 startup research";game.game_id=u8"dora3.n64.jp";
    game.save_type=recomp::SaveType::Eep16k;game.is_enabled=true;
    game.entrypoint_address=(int32_t)0x80025C00;game.entrypoint=recomp_entrypoint;
    game.rom_path=std::filesystem::absolute(argv[1]);
    game.thread_create_callback=[](uint8_t* rdram,recomp_context* ctx){
        std::fprintf(stderr,"THREAD CONTEXT sp=%08X arg=%08X\n",(uint32_t)ctx->r29,(uint32_t)ctx->r4);
    };
    recomp::register_game(game);
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
    cfg.audio_callbacks.get_frames_remaining=[]()->size_t{return 0;};
    cfg.audio_callbacks.queue_samples=[](int16_t* samples,size_t count){
        pcm_buffers.fetch_add(1);
        size_t nonzero=0;for(size_t i=0;i<count;i++)nonzero+=samples[i]!=0;
        std::fprintf(stderr,"PCM OUTPUT reached samples=%zu frames=%zu nonzero=%zu; device not connected\n",count,count/2,nonzero);
        #ifndef DORA3_LIVE_RT64
        stop_diagnostic(78);
        #endif
    };
    cfg.audio_callbacks.set_frequency=[](uint32_t f){std::fprintf(stderr,"AUDIO FREQUENCY %u\n",f);};
    cfg.events_callbacks.vi_callback=[](){static std::atomic<bool> started=false;if(!started.exchange(true))recomp::start_game(u8"dora3.n64.jp","");};
    cfg.error_handling_callbacks.message_box=[](const char* m){std::fprintf(stderr,"RUNTIME ERROR %s\n",m);std::_Exit(80);};
    #ifdef DORA3_LIVE_RT64
    std::fprintf(stderr,"RESEARCH: live RT64, deferred graphics SP completion; PCM measured/discarded, EEPROM 16k; not a playable port\n");
    #else
    std::fprintf(stderr,"DIAGNOSTIC: no renderer/audio output, stop codes 77 pending / 78 task / 79 timeout\n");
    #endif
    recomp::start(cfg);return 0;
}
