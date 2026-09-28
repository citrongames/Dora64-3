// Platform adapter only: no game addresses, hooks or controller-state writes.
#include "live_io.hpp"
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
SDL_GameController* controller=nullptr; // SDL main thread only.
SDL_AudioDeviceID audio_device=0;
uint32_t audio_rate=0;
size_t maximum_buffer_frames=0;
bool audio_enabled=true;
[[noreturn]] void audio_error(){std::fprintf(stderr,"LIVE AUDIO ERROR %s\n",SDL_GetError());std::_Exit(87);}
float axis(Sint16 value){
    const float magnitude=std::abs(int(value));
    return magnitude<=8000 ? 0.0f : std::copysign(std::min(1.0f,(magnitude-8000)/24767.0f),float(value));
}
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
        std::lock_guard lock(input_mutex);*buttons=input.buttons;*x=input.x;*y=input.y;return true;
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
    Input next{};
    if(input_enabled && (SDL_GetWindowFlags(window)&SDL_WINDOW_INPUT_FOCUS)){
        const auto* keys=SDL_GetKeyboardState(nullptr);
        const auto key=[&](SDL_Scancode code,uint16_t mask){if(keys[code])next.buttons|=mask;};
        key(SDL_SCANCODE_Z,0x8000);key(SDL_SCANCODE_X,0x4000);key(SDL_SCANCODE_SPACE,0x2000);
        key(SDL_SCANCODE_RETURN,0x1000);key(SDL_SCANCODE_Q,0x20);key(SDL_SCANCODE_E,0x10);
        key(SDL_SCANCODE_UP,0x800);key(SDL_SCANCODE_DOWN,0x400);key(SDL_SCANCODE_LEFT,0x200);key(SDL_SCANCODE_RIGHT,0x100);
        key(SDL_SCANCODE_I,8);key(SDL_SCANCODE_K,4);key(SDL_SCANCODE_J,2);key(SDL_SCANCODE_L,1);
        next.x=float(keys[SDL_SCANCODE_D])-keys[SDL_SCANCODE_A];
        next.y=float(keys[SDL_SCANCODE_W])-keys[SDL_SCANCODE_S];
        if(controller){
            const auto button=[&](SDL_GameControllerButton b,uint16_t mask){if(SDL_GameControllerGetButton(controller,b))next.buttons|=mask;};
            button(SDL_CONTROLLER_BUTTON_A,0x8000);button(SDL_CONTROLLER_BUTTON_B,0x4000);
            button(SDL_CONTROLLER_BUTTON_START,0x1000);button(SDL_CONTROLLER_BUTTON_LEFTSHOULDER,0x20);button(SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,0x10);
            button(SDL_CONTROLLER_BUTTON_DPAD_UP,0x800);button(SDL_CONTROLLER_BUTTON_DPAD_DOWN,0x400);
            button(SDL_CONTROLLER_BUTTON_DPAD_LEFT,0x200);button(SDL_CONTROLLER_BUTTON_DPAD_RIGHT,0x100);
            if(SDL_GameControllerGetAxis(controller,SDL_CONTROLLER_AXIS_TRIGGERLEFT)>8000)next.buttons|=0x2000;
            const float x=axis(SDL_GameControllerGetAxis(controller,SDL_CONTROLLER_AXIS_LEFTX));
            const float y=-axis(SDL_GameControllerGetAxis(controller,SDL_CONTROLLER_AXIS_LEFTY));
            if(std::abs(x)>std::abs(next.x))next.x=x;
            if(std::abs(y)>std::abs(next.y))next.y=y;
            const int cx=SDL_GameControllerGetAxis(controller,SDL_CONTROLLER_AXIS_RIGHTX),cy=SDL_GameControllerGetAxis(controller,SDL_CONTROLLER_AXIS_RIGHTY);
            if(cx>16000)next.buttons|=1;if(cx< -16000)next.buttons|=2;
            if(cy>16000)next.buttons|=4;if(cy< -16000)next.buttons|=8;
        }
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
    audio_rate=frequency;maximum_buffer_frames=0;SDL_PauseAudioDevice(audio_device,0);
    std::fprintf(stderr,"LIVE AUDIO OPEN rate=%u channels=2 format=S16 driver=%s\n",frequency,SDL_GetCurrentAudioDriver());
}

void dora3_audio_queue(const int16_t* samples,size_t count){
    if(!audio_enabled)return;
    std::lock_guard lock(audio_mutex);
    if(!audio_device || count%2 || count>0x20000){SDL_SetError("Invalid PCM buffer/device");audio_error();}
    maximum_buffer_frames=std::max(maximum_buffer_frames,count/2);
    // Word-swapped N64 RAM stores each stereo pair as R,L on this host.
    std::vector<int16_t> ordered(count);
    for(size_t i=0;i<count;i+=2){ordered[i]=samples[i+1];ordered[i+1]=samples[i];}
    if(SDL_QueueAudio(audio_device,ordered.data(),Uint32(count*sizeof(int16_t))))audio_error();
    // Fail explicitly if the device stalls; never silently drop game audio.
    if(SDL_GetQueuedAudioSize(audio_device)>audio_rate*4*2){SDL_SetError("Audio queue exceeded two seconds");audio_error();}
}

size_t dora3_audio_remaining(){
    std::lock_guard lock(audio_mutex);
    // AI_LEN describes one playing DMA, not every future buffer queued by SDL.
    // Dora3 8004E674 subtracts it with unsigned arithmetic before its lower bound.
    return audio_device ? std::min(maximum_buffer_frames,size_t(SDL_GetQueuedAudioSize(audio_device)/(2*sizeof(int16_t)))) : 0;
}

void dora3_shutdown_live_io(){
    if(controller){SDL_GameControllerClose(controller);controller=nullptr;}
    std::lock_guard lock(audio_mutex);
    if(audio_device){SDL_CloseAudioDevice(audio_device);audio_device=0;}
}
