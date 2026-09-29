#include "resource_retirement.hpp"
#include "native_present.hpp"
#include "native_game_loop.hpp"
void dora3_set_native_present_target(unsigned(*query)());
// Windows/Linux research frontend, original VI and F3DEX2, no game-one patches.
#include "live_graphics.hpp"
#include "live_io.hpp"
#include "port_menu.hpp"
#include "port_display.hpp"
#include "rhi/rt64_render_hooks.h"
#include "frame_lights.hpp"
#include "graphics_sync.hpp"
#include "n64_hardware.hpp"
void dora3_raw_dp_event();
#include "ultramodern/ultramodern.hpp"
#include <SDL.h>
#ifdef _WIN32
#include <SDL_syswm.h>
// ultramodern uses WIN32_LEAN_AND_MEAN; DXC still needs COM types.
#include <objbase.h>
#include <oleauto.h>
#include <psapi.h>
#else
#include <unistd.h>
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

RT64::RenderHookDraw* previous_draw=nullptr;
void count_output(plume::RenderCommandList* list,plume::RenderFramebuffer* framebuffer){
    if(previous_draw)previous_draw(list,framebuffer);
    dora3::port::record_output_frame();
}
void log_memory_usage(){
    static auto next=std::chrono::steady_clock::time_point{};
    const auto now=std::chrono::steady_clock::now();
    if(now<next)return;
    next=now+std::chrono::seconds(60);
    uint64_t resident=0,private_bytes=0;
    bool available=false;
    #ifdef _WIN32
    PROCESS_MEMORY_COUNTERS_EX counters{};
    counters.cb=sizeof(counters);
    available=GetProcessMemoryInfo(GetCurrentProcess(),reinterpret_cast<PROCESS_MEMORY_COUNTERS*>(&counters),sizeof(counters))!=0;
    if(available){resident=counters.WorkingSetSize;private_bytes=counters.PrivateUsage;}
    #else
    if(FILE* stats=std::fopen("/proc/self/statm","r")){
        unsigned long pages=0,resident_pages=0;
        const long page_size=sysconf(_SC_PAGESIZE);
        available=std::fscanf(stats,"%lu %lu",&pages,&resident_pages)==2 && page_size>0;
        if(available)resident=uint64_t(resident_pages)*uint64_t(page_size);
        std::fclose(stats);
    }
    #endif
    std::fprintf(stderr,"MEMORY available=%u resident_bytes=%llu private_bytes=%llu periodic_queued=%llu periodic_coalesced=%llu\n",
        unsigned(available),static_cast<unsigned long long>(resident),static_cast<unsigned long long>(private_bytes),
        static_cast<unsigned long long>(dora3::sync::periodic_present.submissions()),
        static_cast<unsigned long long>(dora3::sync::periodic_present.coalesced()));
}
std::atomic<unsigned> submitted{0},completed{0};
class LiveContext final : public ultramodern::renderer::RendererContext {
    std::array<uint8_t,0x1000> dmem{}, imem{};
    std::array<uint8_t,0x40> header{};
    std::array<uint32_t,9> registers{};
    uint8_t* ram;
    std::unique_ptr<RT64::Application> app;
    unsigned tasks=0;
    int applied_resolution=-1,applied_filtering=-1,applied_rate=-1,applied_limit=-1;
    bool applied_native=false;
    bool apply_graphics(){
        const bool native=dora3::native_time::native_presentation_owned();
        const auto s=dora3::port::settings();
        if(native==applied_native&&s.resolution==applied_resolution&&s.filtering==applied_filtering&&s.frame_rate_mode==applied_rate&&s.frame_rate_limit==applied_limit)return false;
        using U=RT64::UserConfiguration;
        app->userConfig.resolution=s.resolution==3?U::Resolution::WindowIntegerScale:U::Resolution::Manual;
        app->userConfig.resolutionMultiplier=s.resolution==1?2.0f:s.resolution==2?4.0f:1.0f;
        app->userConfig.downsampleMultiplier=1;
        app->userConfig.threePointFiltering=s.filtering==0;
        // Display keeps donor interpolation; Manual uses native scene policies only.
        // Original cadence is the fallback until a scene policy has been reviewed.
        app->userConfig.refreshRate=(native || s.frame_rate_mode==2)?U::RefreshRate::Original:s.frame_rate_mode==1?U::RefreshRate::Display:U::RefreshRate::Original;
        applied_native=native;
        app->userConfig.refreshRateTarget=s.frame_rate_limit;
        applied_rate=s.frame_rate_mode;applied_limit=s.frame_rate_limit;
        applied_resolution=s.resolution;applied_filtering=s.filtering;
        return true;
    }
    inline static LiveContext* current=nullptr;
    bool parsing_raw=false;
    unsigned raw_syncs=0;
    dora3::frame::Lights frame_lights{};
    bool frame_lights_active=false;
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
        config.updateOverlayInput=dora3::port::overlay_input;
        config.drawOverlay=dora3::port::draw_overlay;
        app=std::make_unique<RT64::Application>(core,config);
        // Same Automatic policy as Dora64, including RT64 driver fallbacks.
        app->userConfig.graphicsAPI=RT64::UserConfiguration::GraphicsAPI::Automatic;
        app->userConfig.resolution=RT64::UserConfiguration::Resolution::Original;
        app->userConfig.aspectRatio=RT64::UserConfiguration::AspectRatio::Original;
        app->userConfig.refreshRate=RT64::UserConfiguration::RefreshRate::Original;
        app->userConfig.antialiasing=RT64::UserConfiguration::Antialiasing::None;
        app->userConfig.idleWorkActive=false;
        apply_graphics();
        previous_draw=RT64::GetRenderHookDraw();
        RT64::SetRenderHooks(RT64::GetRenderHookInit(),count_output,RT64::GetRenderHookDeinit());
        const auto result=app->setup(0);
        std::fprintf(stderr,"LIVE RT64 SETUP result=%u\n",unsigned(result));
        if(result!=RT64::Application::SetupResult::Success)std::_Exit(84);
        static_assert(sizeof(RT64::RSP::Light)==16);
        app->state->rsp->lightDataCallback=[](uint32_t address)->const void* {
            return current->frame_lights_active ? current->frame_lights.lookup(address) : nullptr;
        };
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
        // Disable RT64 interpolation BEFORE parsing the first native workload;
        // restore the selected legacy policy when native ownership has drained.
        if ((applied_native || dora3::native_time::native_presentation_owned()) && apply_graphics())
            app->updateUserConfig(true);
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
        if(!dora3::frame::take(dl,frame_lights)) {
            std::fprintf(stderr,"LIVE RT64 missing frame light snapshot dl=%08X\n",dl);
            std::abort();
        }
        app->state->rsp->reset();
        app->interpreter->loadUCodeGBI(text,data,true);
        if(!app->interpreter->hleGBI || app->interpreter->hleGBI->ucode!=RT64::GBIUCode::F3DEX2){
            std::fprintf(stderr,"LIVE RT64 unreviewed microcode\n");std::_Exit(84);
        }
        frame_lights_active=true;
        app->processDisplayLists(ram,dl,0,true);
        frame_lights_active=false;
        dora3::retirement::parsed(dl);
        std::fprintf(stderr,"LIVE RT64 PARSED task=%u dl=%08X bytes=%X gbi=7\n",++tasks,dl,task->t.data_size);
        completed.fetch_add(1);
    }
    void send_dummy_workload(uint32_t) override{}
    void update_screen(bool=false) override{
        // RT64 configuration belongs to its producer thread; UI only edits snapshots.
        if(apply_graphics())app->updateUserConfig(true);
        // Same exclusion as PI DMA and the reviewed guest CPU framebuffer copy.
        std::scoped_lock lock(ultramodern::get_graphics_rdram_mutex());
        app->updateScreen();
    }
    void reset_game() override{frame_lights_active=false;dora3::frame::reset();dora3::retirement::reset();}
    void shutdown() override{
        app->end();
        RT64::SetRenderHooks(RT64::GetRenderHookInit(),previous_draw,RT64::GetRenderHookDeinit());
    }
    uint32_t get_display_framerate() const override{
        if(!app||!app->sharedQueueResources)return 0;
        std::scoped_lock lock(app->sharedQueueResources->configurationMutex);
        const auto rate=app->sharedQueueResources->swapChainRate;
        dora3::port::publish_display_rate(rate);return rate;
    }
    float get_resolution_scale() const override{return 1.0f;}
};
}

void dora3_configure_live_graphics(recomp::Configuration& cfg){
    dora3_set_native_present_target(dora3_native_presentation_fps);
    dora3::frame::enable();
    dora3::retirement::enable();
    dora3::port::initialize(recomp::get_config_path());
    dora3_configure_live_input(cfg);
    std::fprintf(stderr,"LIVE SESSION: no time or graphics-task limit\n");
    cfg.events_callbacks.gfx_task_submitted_callback=[](uint8_t*,uint32_t dl,uint32_t bytes){
        std::fprintf(stderr,"LIVE RT64 SUBMITTED task=%u dl=%08X bytes=%X\n",submitted.fetch_add(1)+1,dl,bytes);
    };
    cfg.renderer_callbacks.create_render_context=[](uint8_t* ram,ultramodern::renderer::WindowHandle w,bool)
        ->std::unique_ptr<ultramodern::renderer::RendererContext>{return std::make_unique<LiveContext>(ram,w);};
    cfg.gfx_callbacks.create_window=[](void*)->ultramodern::renderer::WindowHandle{
        SDL_SetMainReady();
        dora3_prepare_sdl_input();
        if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS|SDL_INIT_GAMECONTROLLER|SDL_INIT_AUDIO)!=0){std::fprintf(stderr,"SDL: %s\n",SDL_GetError());std::_Exit(84);}
        uint32_t flags=SDL_WINDOW_RESIZABLE|SDL_WINDOW_ALLOW_HIGHDPI;
        #ifndef _WIN32
        flags|=SDL_WINDOW_VULKAN;
        #endif
        dora3::port::refresh_displays();
        const auto settings=dora3::port::settings();
        const int centered=SDL_WINDOWPOS_CENTERED_DISPLAY(settings.display_index);
        auto* window=SDL_CreateWindow("Dora64-3 test build",centered,centered,settings.window_width,settings.window_height,flags);
        if(!window){std::fprintf(stderr,"SDL window: %s\n",SDL_GetError());std::_Exit(84);}
        live_window=window;
        SDL_SetWindowMinimumSize(window,320,240);
        dora3::port::update_display_window(window);
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
        while(SDL_PollEvent(&event)){
            if(event.type==SDL_QUIT)ultramodern::quit();
            if(event.type==SDL_DISPLAYEVENT)dora3::port::request_display_refresh();
        }
        if(dora3::port::take_exit_request())ultramodern::quit();
        dora3::port::update_display_window(live_window);
        dora3_poll_live_input(live_window);
        log_memory_usage();
    };
}

void dora3_drain_live_graphics(){
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(2);
    while(completed.load()<submitted.load() && std::chrono::steady_clock::now()<deadline)
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    std::fprintf(stderr,"LIVE RT64 DRAIN submitted=%u parsed=%u\n",submitted.load(),completed.load());
}
