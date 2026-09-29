// Focused Linux adapter check. No ROM; synthetic keyboard, SDL virtual pad and PCM.
#include "live_io.hpp"
#include "port_menu.hpp"
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
static Uint8 keys[SDL_NUM_SCANCODES]{};
static Uint32 flags=SDL_WINDOW_INPUT_FOCUS,queued=0;
static std::vector<int16_t> last_pcm;
extern "C" const Uint8* __wrap_SDL_GetKeyboardState(int* n){if(n)*n=SDL_NUM_SCANCODES;return keys;}
extern "C" Uint32 __wrap_SDL_GetWindowFlags(SDL_Window*){return flags;}
extern "C" int __wrap_SDL_QueueAudio(SDL_AudioDeviceID,const void* data,Uint32 bytes){
    const auto* samples=static_cast<const int16_t*>(data);last_pcm.assign(samples,samples+bytes/2);queued+=bytes;return 0;
}
extern "C" Uint32 __wrap_SDL_GetQueuedAudioSize(SDL_AudioDeviceID){return queued;}
int main(){
    SDL_setenv("SDL_VIDEODRIVER","dummy",1);SDL_setenv("SDL_AUDIODRIVER","dummy",1);
    SDL_setenv("SDL_JOYSTICK_ALLOW_BACKGROUND_EVENTS","1",1);
    dora3_prepare_sdl_input();
    #if defined(__linux__)
    if(std::getenv("WSL_INTEROP")||std::getenv("WSL_DISTRO_NAME"))assert(SDL_GetHintBoolean(SDL_HINT_LINUX_JOYSTICK_CLASSIC,SDL_FALSE));
    #endif
    assert(SDL_Init(SDL_INIT_VIDEO|SDL_INIT_GAMECONTROLLER|SDL_INIT_AUDIO)==0);
    auto* window=SDL_CreateWindow("adapter test",0,0,32,32,SDL_WINDOW_HIDDEN);assert(window);
    recomp::Configuration cfg{};dora3_configure_live_input(cfg);
    using namespace ultramodern::input;
    assert(cfg.input_callbacks.get_connected_device_info(0).connected_device==Device::Controller);
    assert(cfg.input_callbacks.get_connected_device_info(1).connected_device==Device::None);
    uint16_t buttons=123;float x=4,y=5;
    assert(!cfg.input_callbacks.get_input(1,&buttons,&x,&y) && buttons==123 && x==4 && y==5);
    keys[SDL_SCANCODE_SPACE]=keys[SDL_SCANCODE_RETURN]=keys[SDL_SCANCODE_W]=keys[SDL_SCANCODE_D]=1;
    dora3_poll_live_input(window);assert(cfg.input_callbacks.get_input(0,&buttons,&x,&y));
    assert(buttons==0x9000 && x>0 && y>0 && std::abs(x*x+y*y-1)<0.00001f);
    flags=0;dora3_poll_live_input(window);cfg.input_callbacks.get_input(0,&buttons,&x,&y);
    assert(buttons==0 && x==0 && y==0);flags=SDL_WINDOW_INPUT_FOCUS;std::memset(keys,0,sizeof(keys));
    dora3_poll_live_input(window); // Release quarantine after restoring focus.
    const int index=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,SDL_CONTROLLER_AXIS_MAX,SDL_CONTROLLER_BUTTON_MAX,0);assert(index>=0);
    auto* pad=SDL_JoystickOpen(index);assert(pad);
    SDL_JoystickSetVirtualAxis(pad,SDL_CONTROLLER_AXIS_LEFTX,32767);
    SDL_JoystickSetVirtualAxis(pad,SDL_CONTROLLER_AXIS_LEFTY,-32768);
    SDL_JoystickSetVirtualButton(pad,SDL_CONTROLLER_BUTTON_B,1);
    SDL_JoystickUpdate();dora3_poll_live_input(window);cfg.input_callbacks.get_input(0,&buttons,&x,&y);
    assert(buttons==0x4000 && x>0.7f && y>0.7f);
    SDL_JoystickSetVirtualAxis(pad,SDL_CONTROLLER_AXIS_LEFTX,7999);SDL_JoystickSetVirtualAxis(pad,SDL_CONTROLLER_AXIS_LEFTY,0);
    SDL_JoystickUpdate();dora3_poll_live_input(window);cfg.input_callbacks.get_input(0,&buttons,&x,&y);assert(x==0 && y==0);
    SDL_JoystickClose(pad);assert(SDL_JoystickDetachVirtual(index)==0);
    dora3_poll_live_input(window);cfg.input_callbacks.get_input(0,&buttons,&x,&y);assert(buttons==0 && x==0 && y==0);
    // Reconnect after removal, then apply a larger dead zone to actual SDL axes.
    const int reconnected=SDL_JoystickAttachVirtual(SDL_JOYSTICK_TYPE_GAMECONTROLLER,SDL_CONTROLLER_AXIS_MAX,SDL_CONTROLLER_BUTTON_MAX,0);assert(reconnected>=0);
    auto* second=SDL_JoystickOpen(reconnected);assert(second);
    dora3::port::set_deadzone(0.5f);
    SDL_JoystickSetVirtualAxis(second,SDL_CONTROLLER_AXIS_LEFTX,12000);SDL_JoystickUpdate();
    dora3_poll_live_input(window);cfg.input_callbacks.get_input(0,&buttons,&x,&y);assert(x==0);
    SDL_JoystickSetVirtualAxis(second,SDL_CONTROLLER_AXIS_LEFTX,32767);SDL_JoystickUpdate();
    dora3_poll_live_input(window);cfg.input_callbacks.get_input(0,&buttons,&x,&y);assert(x==1);
    SDL_JoystickClose(second);assert(SDL_JoystickDetachVirtual(reconnected)==0);dora3_poll_live_input(window);
    dora3::port::reset_bindings();
    dora3_audio_frequency(22050);
    const int16_t n64_pairs[]={-20,10,-40,30};dora3_audio_queue(n64_pairs,4);
    assert((last_pcm==std::vector<int16_t>{10,-20,30,-40}));assert(dora3_audio_remaining()==2);
    queued=4000;assert(dora3_audio_remaining()==2);queued=4;assert(dora3_audio_remaining()==1);
    using namespace dora3::port;
    auto settings_before=settings();auto changed=settings_before;changed.volume=25;set_general(changed);
    queued=0;dora3_audio_queue(n64_pairs,4);assert((last_pcm==std::vector<int16_t>{2,-5,7,-10}));
    changed.volume=0;set_general(changed);queued=0;dora3_audio_queue(n64_pairs,4);assert((last_pcm==std::vector<int16_t>{0,0,0,0}));
    set_general(settings_before);
    bind(A,false,SDL_SCANCODE_C);keys[SDL_SCANCODE_C]=1;
    dora3_poll_live_input(window);cfg.input_callbacks.get_input(0,&buttons,&x,&y);assert(buttons==0x8000);
    set_menu_open(true);cfg.input_callbacks.get_input(0,&buttons,&x,&y);assert(buttons==0&&x==0&&y==0);
    dora3_poll_live_input(window);set_menu_open(false);dora3_poll_live_input(window);
    cfg.input_callbacks.get_input(0,&buttons,&x,&y);assert(buttons==0); // Held key cannot leak through close.
    keys[SDL_SCANCODE_C]=0;dora3_poll_live_input(window);keys[SDL_SCANCODE_C]=1;dora3_poll_live_input(window);
    cfg.input_callbacks.get_input(0,&buttons,&x,&y);assert(buttons==0x8000);
    keys[SDL_SCANCODE_C]=0;dora3_poll_live_input(window);
    dora3_shutdown_live_io();SDL_DestroyWindow(window);SDL_Quit();
    std::puts("PASS: port contract; keyboard/diagonal; focus release; SDL virtual pad/deadzone/detach/reconnect; rebind/menu isolation; stereo/volume/mute; AI_LEN bound");
}
