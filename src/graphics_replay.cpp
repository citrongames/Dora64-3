// Isolated replay of a private first-task capture. This is not a game frontend.
#include <array>
#include <bit>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <vector>
#include <SDL.h>
#include "hle/rt64_application.h"
#include "hle/rt64_state.h"
#include "hle/rt64_rsp.h"

int main(int argc,char** argv){
    if(argc!=3){std::fprintf(stderr,"Usage: dora3_graphics_replay SNAPSHOT FRESH_OUTPUT_DIRECTORY\n");return 2;}
    static_assert(std::endian::native==std::endian::little);
    std::setvbuf(stderr,nullptr,_IONBF,0);
    std::array<uint32_t,35> h{};
    std::ifstream in(argv[1],std::ios::binary);
    in.read(reinterpret_cast<char*>(h.data()),sizeof(h));
    if(!in || h[0]!=0x44335231 || h[1]!=1 || h[2]!=0x400000 || h[3]!=0x01020304 || h[4]!=1){
        std::fprintf(stderr,"Unsupported capture\n");return 2;
    }
    std::vector<uint8_t> ram(0x800000);
    in.read(reinterpret_cast<char*>(ram.data()),h[2]);
    if(!in || in.peek()!=EOF){std::fprintf(stderr,"Wrong capture length\n");return 2;}
    const uint32_t text=h[8]&0x1FFFFFFF,data=h[10]&0x1FFFFFFF,dl=h[16]&0x1FFFFFFF;
    if(text!=0x63510 || h[9]!=0x1000 || data>=h[2]-0x1000 ||
       dl>=h[2] || h[17]==0 || h[17]>h[2]-dl || (dl&7) || (h[17]&7)){
        std::fprintf(stderr,"Unsupported first-task ranges\n");return 2;
    }
    const auto output=std::filesystem::absolute(argv[2]);
    if(std::filesystem::exists(output)){std::fprintf(stderr,"Use fresh output directory\n");return 2;}
    std::filesystem::create_directories(output);
    if(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_EVENTS)!=0){std::fprintf(stderr,"SDL: %s\n",SDL_GetError());return 3;}
    auto* window=SDL_CreateWindow("Dora64-3 graphics research",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,640,480,SDL_WINDOW_VULKAN);
    if(!window){std::fprintf(stderr,"SDL window: %s\n",SDL_GetError());SDL_Quit();return 3;}
    std::array<uint8_t,0x1000> dmem{},imem{};
    std::array<uint8_t,0x40> rom_header{};
    std::array<uint32_t,9> regs{};
    RT64::Application::Core core{};
    core.window=window;core.HEADER=rom_header.data();core.RDRAM=ram.data();core.DMEM=dmem.data();core.IMEM=imem.data();
    core.checkInterrupts=[](){};
    core.MI_INTR_REG=&regs[0];core.DPC_START_REG=&regs[1];core.DPC_END_REG=&regs[2];
    core.DPC_CURRENT_REG=&regs[3];core.DPC_STATUS_REG=&regs[4];core.DPC_CLOCK_REG=&regs[5];
    core.DPC_BUFBUSY_REG=&regs[6];core.DPC_PIPEBUSY_REG=&regs[7];core.DPC_TMEM_REG=&regs[8];
    core.VI_STATUS_REG=&h[20];core.VI_ORIGIN_REG=&h[21];core.VI_WIDTH_REG=&h[22];core.VI_INTR_REG=&h[23];
    core.VI_V_CURRENT_LINE_REG=&h[24];core.VI_TIMING_REG=&h[25];core.VI_V_SYNC_REG=&h[26];
    core.VI_H_SYNC_REG=&h[27];core.VI_LEAP_REG=&h[28];core.VI_H_START_REG=&h[29];core.VI_V_START_REG=&h[30];
    core.VI_V_BURST_REG=&h[31];core.VI_X_SCALE_REG=&h[32];core.VI_Y_SCALE_REG=&h[33];
    RT64::ApplicationConfiguration config{};
    config.appId="dora64-3-research";config.detectDataPath=false;config.dataPath=output;config.useConfigurationFile=false;
    int result=0;
    {
        RT64::Application app(core,config);
        app.userConfig.graphicsAPI=RT64::UserConfiguration::GraphicsAPI::Vulkan;
        app.userConfig.resolution=RT64::UserConfiguration::Resolution::Original;
        app.userConfig.aspectRatio=RT64::UserConfiguration::AspectRatio::Original;
        app.userConfig.refreshRate=RT64::UserConfiguration::RefreshRate::Original;
        app.userConfig.antialiasing=RT64::UserConfiguration::Antialiasing::None;
        app.userConfig.idleWorkActive=false;
        auto setup=app.setup(0);
        std::fprintf(stderr,"RT64 SETUP result=%u\n",(unsigned)setup);
        if(setup!=RT64::Application::SetupResult::Success)result=4;
        else {
            app.state->rsp->reset();
            app.interpreter->loadUCodeGBI(text,data,true);
            if(!app.interpreter->hleGBI){std::fprintf(stderr,"UNKNOWN GRAPHICS MICROCODE\n");result=5;}
            else{
                std::fprintf(stderr,"RT64 GBI=%u text=%08X data=%08X\n",(unsigned)app.interpreter->hleGBI->ucode,text,data);
                app.processDisplayLists(ram.data(),dl,0,true);
                std::fprintf(stderr,"RT64 DISPLAY LIST PARSED address=%08X bytes=%X; original pre-completion VI retained\n",dl,h[17]);
                app.updateScreen();
                for(int i=0;i<30;i++){SDL_Event event;while(SDL_PollEvent(&event)){}SDL_Delay(10);}
            }
            app.end();
        }
    }
    SDL_DestroyWindow(window);SDL_Quit();
    return result;
}
