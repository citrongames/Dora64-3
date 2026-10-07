// Platform adapter only: no game addresses, hooks or controller-state writes.
#include "live_io.hpp"
#include "port_menu.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <vector>

namespace {
struct Input { uint16_t buttons=0; float x=0,y=0; };
std::mutex input_mutex, audio_mutex;
Input input;
bool input_enabled=true;
bool keyboard_release=false;
int applied_background=-1;
SDL_GameController* controller=nullptr; // SDL main thread only.
SDL_AudioDeviceID audio_device=0;
uint32_t audio_rate=0;
bool audio_enabled=true;
[[noreturn]] void audio_error(){std::fprintf(stderr,"LIVE AUDIO ERROR %s\n",SDL_GetError());std::_Exit(87);}
float axis(Sint16 value,float deadzone){
    const float magnitude=std::abs(int(value))/32767.0f;
    return magnitude<=deadzone ? 0.0f : std::copysign(std::min(1.0f,(magnitude-deadzone)/(1.0f-deadzone)),float(value));
}
}

void dora3_prepare_sdl_input(){
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,dora3::port::settings().background_gamepad_input?"1":"0");
    #if defined(__linux__)
    // Same pre-initialization WSL workaround as Dora64: js* passthrough may
    // exist without usable event* devices. Regular Linux keeps SDL's default.
    if(std::getenv("WSL_INTEROP") || std::getenv("WSL_DISTRO_NAME"))
        SDL_SetHint(SDL_HINT_LINUX_JOYSTICK_CLASSIC,"1");
    #endif
}

void dora3_configure_live_input(recomp::Configuration& cfg){
    input_enabled=std::getenv("DORA3_NO_INPUT")==nullptr;
    audio_enabled=std::getenv("DORA3_NO_AUDIO")==nullptr;
    cfg.input_callbacks.get_connected_device_info=[](int port){
        using namespace ultramodern::input;
        return connected_device_info_t{input_enabled && port==0 ? Device::Controller : Device::None,Pak::None};
    };
    cfg.input_callbacks.get_input=[](int port,uint16_t* buttons,float* x,float* y){
        if(!input_enabled || port!=0)return false;
        std::lock_guard lock(input_mutex);
        if(dora3::port::menu_open()){*buttons=0;*x=*y=0;}else {*buttons=input.buttons;*x=input.x;*y=input.y;}
        return true;
    };
    std::fprintf(stderr,"LIVE INPUT port0=%s pak=none; keyboard/gamepad; focus clears state\n",input_enabled?"connected":"disconnected");
}

void dora3_poll_live_input(SDL_Window* window){
    if(controller && !SDL_GameControllerGetAttached(controller)){
        SDL_GameControllerClose(controller);controller=nullptr;
        std::fprintf(stderr,"LIVE INPUT gamepad detached\n");
    }
    if(!controller)for(int i=0;i<SDL_NumJoysticks();i++)if(SDL_IsGameController(i)){
        controller=SDL_GameControllerOpen(i);
        if(controller){std::fprintf(stderr,"LIVE INPUT gamepad attached\n");break;}
    }
    using namespace dora3::port;
    const auto config=settings();
    if(applied_background!=int(config.background_gamepad_input)){
        SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,config.background_gamepad_input?"1":"0");
        applied_background=int(config.background_gamepad_input);
    }
    PhysicalInput physical;
    physical.background_gamepad=config.background_gamepad_input;
    physical.focused=(SDL_GetWindowFlags(window)&SDL_WINDOW_INPUT_FOCUS)!=0;
    physical.connected=controller!=nullptr;
    const auto* keys=SDL_GetKeyboardState(nullptr);
    if(!physical.focused)keyboard_release=true;
    if(physical.focused&&std::none_of(keys,keys+SDL_NUM_SCANCODES,[](Uint8 v){return v!=0;}))keyboard_release=false;
    if(physical.focused&&!keyboard_release)std::copy_n(keys,SDL_NUM_SCANCODES,physical.keys.begin());
    if(controller){
        const char* name=SDL_GameControllerName(controller);physical.controller_name=name?name:"Gamepad";
        if(physical.focused||config.background_gamepad_input){
        for(int i=0;i<SDL_CONTROLLER_BUTTON_MAX;i++)physical.pad[i]=SDL_GameControllerGetButton(controller,SDL_GameControllerButton(i))?1.0f:0.0f;
        for(int i=0;i<SDL_CONTROLLER_AXIS_MAX;i++){
            const float v=axis(SDL_GameControllerGetAxis(controller,SDL_GameControllerAxis(i)),config.deadzone);
            physical.pad[SDL_CONTROLLER_BUTTON_MAX+i*2]=std::max(0.0f,-v);
            physical.pad[SDL_CONTROLLER_BUTTON_MAX+i*2+1]=std::max(0.0f,v);
        }
    }
    }
    Input next{};
    const bool blocked=process_port_input(physical);
    if(input_enabled&&!blocked){
        constexpr uint16_t masks[]={0x8000,0x4000,0x2000,0x1000,0x20,0x10,0x800,0x400,0x200,0x100,8,4,2,1};
        const auto amount=[&](int action){
            const int key=config.keys[action],binding=config.pad[action];
            const float keyboard=key>0&&physical.keys[key]?1.0f:0.0f;
            const int index=binding<100?binding:SDL_CONTROLLER_BUTTON_MAX+binding-100;
            const float gamepad=index>=0&&index<int(physical.pad.size())?physical.pad[index]:0.0f;
            return std::max(keyboard,gamepad);
        };
        for(int i=0;i<StickUp;i++)if(amount(i)>(i>=CUp?0.32f:0.01f))next.buttons|=masks[i];
        next.x=amount(StickRight)-amount(StickLeft);next.y=amount(StickUp)-amount(StickDown);
        const float length=std::sqrt(next.x*next.x+next.y*next.y);
        if(length>1){next.x/=length;next.y/=length;}
    }
    std::lock_guard lock(input_mutex);input=next;
}

void dora3_audio_frequency(uint32_t frequency){
    if(!audio_enabled)return;
    if(frequency<8000 || frequency>96000){std::fprintf(stderr,"LIVE AUDIO unsupported rate %u\n",frequency);std::_Exit(87);}
    std::lock_guard lock(audio_mutex);
    if(audio_device && audio_rate==frequency)return;
    if(audio_device){SDL_CloseAudioDevice(audio_device);audio_device=0;}
    SDL_AudioSpec desired{},obtained{};
    desired.freq=int(frequency);desired.format=AUDIO_S16SYS;desired.channels=2;desired.samples=512;
    audio_device=SDL_OpenAudioDevice(nullptr,0,&desired,&obtained,0);
    if(!audio_device)audio_error();
    if(obtained.freq!=int(frequency) || obtained.format!=AUDIO_S16SYS || obtained.channels!=2){
        SDL_SetError("Unexpected audio format");audio_error();
    }
    audio_rate=frequency;SDL_PauseAudioDevice(audio_device,0);
    std::fprintf(stderr,"LIVE AUDIO OPEN rate=%u channels=2 format=S16 driver=%s\n",frequency,SDL_GetCurrentAudioDriver());
}

void dora3_audio_queue(const int16_t* samples,size_t count){
    if(!audio_enabled)return;
    std::lock_guard lock(audio_mutex);
    if(!audio_device || count%2 || count>0x20000){SDL_SetError("Invalid PCM buffer/device");audio_error();}
    // Word-swapped N64 RAM stores each stereo pair as R,L on this host.
    std::vector<int16_t> ordered(count);
    const int volume=dora3::port::settings().volume;
    for(size_t i=0;i<count;i+=2){ordered[i]=int16_t(int(samples[i+1])*volume/100);ordered[i+1]=int16_t(int(samples[i])*volume/100);}
    if(SDL_QueueAudio(audio_device,ordered.data(),Uint32(count*sizeof(int16_t))))audio_error();
    // Fail explicitly if the device stalls; never silently drop game audio.
    if(SDL_GetQueuedAudioSize(audio_device)>audio_rate*4*2){SDL_SetError("Audio queue exceeded two seconds");audio_error();}
}

size_t dora3_audio_remaining(){
    std::lock_guard lock(audio_mutex);
    // Report all pending stereo frames so synthesis can correct queue growth.
    // The runtime applies its existing half-VI reserve after this callback.
    // audio_queue_patches.py saturates the guest subtraction before alignment;
    // clipping here to one DMA hides accumulated latency from that feedback.
    return audio_device ? size_t(SDL_GetQueuedAudioSize(audio_device)/(2*sizeof(int16_t))) : 0;
}

void dora3_shutdown_live_io(){
    if(controller){SDL_GameControllerClose(controller);controller=nullptr;}
    std::lock_guard lock(audio_mutex);
    if(audio_device){SDL_CloseAudioDevice(audio_device);audio_device=0;}
}
