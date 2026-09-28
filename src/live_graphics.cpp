// Windows/Linux research frontend, original VI and F3DEX2, no game-one patches.
#include "live_graphics.hpp"
#include "live_io.hpp"
#include "n64_hardware.hpp"
void dora3_raw_dp_event();
#include "ultramodern/ultramodern.hpp"
#include <SDL.h>
#ifdef _WIN32
#include <SDL_syswm.h>
// ultramodern uses WIN32_LEAN_AND_MEAN; DXC still needs COM types.
#include <objbase.h>
#include <oleauto.h>
#endif
#include <array>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include "hle/rt64_application.h"
#include "hle/rt64_state.h"
#include "hle/rt64_rsp.h"

namespace {
SDL_Window* live_window=nullptr;
std::atomic<unsigned> submitted{0},completed{0};
class LiveContext final : public ultramodern::renderer::RendererContext {
    std::array<uint8_t,0x1000> dmem{}, imem{};
    std::array<uint8_t,0x40> header{};
    std::array<uint32_t,9> registers{};
    uint8_t* ram;
    std::unique_ptr<RT64::Application> app;
    unsigned tasks=0;
    inline static LiveContext* current=nullptr;
    bool parsing_raw=false;
    unsigned raw_syncs=0;
public:
    LiveContext(uint8_t* rdram, ultramodern::renderer::WindowHandle window):ram(rdram){
        RT64::Application::Core core{};
        #ifdef _WIN32
        core.window=window.window;
        #else
        core.window=window;
        #endif
        core.HEADER=header.data();core.RDRAM=ram;
        core.DMEM=dmem.data();core.IMEM=imem.data();
        current=this;
        core.checkInterrupts=[](){
            if(current->parsing_raw && (current->registers[0]&0x20)) ++current->raw_syncs;
            current->registers[0]=0;
        };
        core.MI_INTR_REG=&registers[0];core.DPC_START_REG=&registers[1];core.DPC_END_REG=&registers[2];
        core.DPC_CURRENT_REG=&registers[3];core.DPC_STATUS_REG=&registers[4];core.DPC_CLOCK_REG=&registers[5];
        core.DPC_BUFBUSY_REG=&registers[6];core.DPC_PIPEBUSY_REG=&registers[7];core.DPC_TMEM_REG=&registers[8];
        auto* vi=ultramodern::renderer::get_vi_regs();
        core.VI_STATUS_REG=&vi->VI_STATUS_REG;core.VI_ORIGIN_REG=&vi->VI_ORIGIN_REG;
        core.VI_WIDTH_REG=&vi->VI_WIDTH_REG;core.VI_INTR_REG=&vi->VI_INTR_REG;
        core.VI_V_CURRENT_LINE_REG=&vi->VI_V_CURRENT_LINE_REG;core.VI_TIMING_REG=&vi->VI_TIMING_REG;
        core.VI_V_SYNC_REG=&vi->VI_V_SYNC_REG;core.VI_H_SYNC_REG=&vi->VI_H_SYNC_REG;
        core.VI_LEAP_REG=&vi->VI_LEAP_REG;core.VI_H_START_REG=&vi->VI_H_START_REG;
        core.VI_V_START_REG=&vi->VI_V_START_REG;core.VI_V_BURST_REG=&vi->VI_V_BURST_REG;
        core.VI_X_SCALE_REG=&vi->VI_X_SCALE_REG;core.VI_Y_SCALE_REG=&vi->VI_Y_SCALE_REG;
        RT64::ApplicationConfiguration config{};
        config.appId="dora64-3-research";config.detectDataPath=false;
        config.dataPath=recomp::get_config_path()/"rt64";config.useConfigurationFile=false;
        app=std::make_unique<RT64::Application>(core,config);
        // Same Automatic policy as Dora64, including RT64 driver fallbacks.
        app->userConfig.graphicsAPI=RT64::UserConfiguration::GraphicsAPI::Automatic;
        app->userConfig.resolution=RT64::UserConfiguration::Resolution::Original;
        app->userConfig.aspectRatio=RT64::UserConfiguration::AspectRatio::Original;
        app->userConfig.refreshRate=RT64::UserConfiguration::RefreshRate::Original;
        app->userConfig.antialiasing=RT64::UserConfiguration::Antialiasing::None;
        app->userConfig.idleWorkActive=false;
        const auto result=app->setup(0);
        std::fprintf(stderr,"LIVE RT64 SETUP result=%u\n",unsigned(result));
        if(result!=RT64::Application::SetupResult::Success)std::_Exit(84);
        setup_result=ultramodern::renderer::SetupResult::Success;
        switch(app->chosenGraphicsAPI){
        case RT64::UserConfiguration::GraphicsAPI::D3D12:
            chosen_api=ultramodern::renderer::GraphicsApi::D3D12;
            std::fprintf(stderr,"LIVE RT64 API D3D12 (Automatic)\n");break;
        case RT64::UserConfiguration::GraphicsAPI::Vulkan:
            chosen_api=ultramodern::renderer::GraphicsApi::Vulkan;
            std::fprintf(stderr,"LIVE RT64 API Vulkan (Automatic)\n");break;
        default:
            std::fprintf(stderr,"LIVE RT64 unexpected API\n");std::_Exit(84);
        }
    }
    bool valid() override{return true;}
    bool update_config(const ultramodern::renderer::GraphicsConfig&,const ultramodern::renderer::GraphicsConfig&) override{return true;}
    void enable_instant_present() override{}
    void send_dl(const OSTask* task) override{
        if(task->t.type==dora3::hw::raw_rdp_task){
            const uint32_t start=uint32_t(task->t.data_ptr),bytes=task->t.data_size;
            if((start&7) || (bytes&7) || start>=dora3::hw::ram_size || bytes>dora3::hw::ram_size-start) std::abort();
            registers[1]=registers[3]=start; registers[2]=start+bytes;
            registers[4]=0x180;
            parsing_raw=true; raw_syncs=0;
            // Raw RDP commands retain DP state; there is no RSP microcode here.
            app->processDisplayLists(ram,start,start+bytes,false);
            parsing_raw=false; registers[3]=start+bytes; registers[4]=0x80;
            dora3::hw::dp_consumed();
            for(unsigned i=0;i<raw_syncs;++i) dora3_raw_dp_event();
            return;
        }
        const auto text=uint32_t(task->t.ucode)&0x1FFFFFFF;
        const auto data=uint32_t(task->t.ucode_data)&0x1FFFFFFF;
        const auto dl=uint32_t(task->t.data_ptr)&0x1FFFFFFF;
        // Restrict this experiment to the resident microcode already identified.
        if(task->t.type!=1 || text!=0x63510 || data!=0x78E00 || task->t.ucode_size!=0x1000 ||
           task->t.ucode_data_size!=0x800 || (dl&7) || !task->t.data_size ||
           (task->t.data_size&7) || dl>=0x400000 || task->t.data_size>0x400000-dl){
            std::fprintf(stderr,"LIVE RT64 unsupported task\n");std::_Exit(84);
        }
        app->state->rsp->reset();
        app->interpreter->loadUCodeGBI(text,data,true);
        if(!app->interpreter->hleGBI || app->interpreter->hleGBI->ucode!=RT64::GBIUCode::F3DEX2){
            std::fprintf(stderr,"LIVE RT64 unreviewed microcode\n");std::_Exit(84);
        }
        app->processDisplayLists(ram,dl,0,true);
        std::fprintf(stderr,"LIVE RT64 PARSED task=%u dl=%08X bytes=%X gbi=7\n",++tasks,dl,task->t.data_size);
        completed.fetch_add(1);
    }
    void send_dummy_workload(uint32_t) override{}
    void update_screen(bool=false) override{app->updateScreen();}
    void reset_game() override{}
    void shutdown() override{app->end();}
    uint32_t get_display_framerate() const override{return 60;}
    float get_resolution_scale() const override{return 1.0f;}
};
}

void dora3_configure_live_graphics(recomp::Configuration& cfg){
    dora3_configure_live_input(cfg);
    std::fprintf(stderr,"LIVE SESSION: no time or graphics-task limit\n");
    cfg.events_callbacks.gfx_task_submitted_callback=[](uint8_t*,uint32_t dl,uint32_t bytes){
        std::fprintf(stderr,"LIVE RT64 SUBMITTED task=%u dl=%08X bytes=%X\n",submitted.fetch_add(1)+1,dl,bytes);
    };
    cfg.renderer_callbacks.create_render_context=[](uint8_t* ram,ultramodern::renderer::WindowHandle w,bool)
        ->std::unique_ptr<ultramodern::renderer::RendererContext>{return std::make_unique<LiveContext>(ram,w);};
    cfg.gfx_callbacks.create_window=[](void*)->ultramodern::renderer::WindowHandle{
        SDL_SetMainReady();
        if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_GAMECONTROLLER|SDL_INIT_AUDIO)!=0){std::fprintf(stderr,"SDL: %s\n",SDL_GetError());std::_Exit(84);}
        uint32_t flags=0;
        #ifndef _WIN32
        flags|=SDL_WINDOW_VULKAN;
        #endif
        auto* window=SDL_CreateWindow("Dora64-3 test build",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,640,480,flags);
        if(!window){std::fprintf(stderr,"SDL window: %s\n",SDL_GetError());std::_Exit(84);}
        live_window=window;
        #ifdef _WIN32
        SDL_SysWMinfo info{};SDL_VERSION(&info.version);
        if(!SDL_GetWindowWMInfo(window,&info)){std::fprintf(stderr,"SDL HWND: %s\n",SDL_GetError());std::_Exit(84);}
        return {info.info.win.window,GetCurrentThreadId()};
        #else
        return window;
        #endif
    };
    cfg.gfx_callbacks.update_gfx=[](void*){
        SDL_Event event;
        while(SDL_PollEvent(&event))if(event.type==SDL_QUIT)ultramodern::quit();
        dora3_poll_live_input(live_window);
    };
}

void dora3_drain_live_graphics(){
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(2);
    while(completed.load()<submitted.load() && std::chrono::steady_clock::now()<deadline)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::fprintf(stderr,"LIVE RT64 DRAIN submitted=%u parsed=%u\n",submitted.load(),completed.load());
}
