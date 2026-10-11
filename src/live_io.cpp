#include "diagnostics.hpp"
// Platform adapter only: no game addresses, hooks or controller-state writes.
#include "live_io.hpp"
#include "port_menu.hpp"
#include "port_touch.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <mutex>
#include <vector>

namespace {
struct Input { uint16_t buttons=0; float x=0,y=0; bool connected=false; };
std::mutex input_mutex, audio_mutex;
std::array<Input,dora3::port::PlayerCount> input{};
bool input_enabled=true;
int applied_background=-1;
struct GamepadSlot {
    SDL_GameController* controller=nullptr;
    SDL_JoystickID instance=-1;
    std::string identity;
};
std::array<GamepadSlot,dora3::port::PlayerCount> controllers{}; // SDL thread only.
bool connected_locked(int port){return input_enabled&&port>=0&&port<dora3::port::PlayerCount&&(port==0||input[port].connected);}
void refresh_controllers() {
    for(auto& slot:controllers)if(slot.controller&&!SDL_GameControllerGetAttached(slot.controller)) {
        SDL_GameControllerClose(slot.controller);slot.controller=nullptr;slot.instance=-1;
    }
    for(int device=0;device<SDL_NumJoysticks();++device) {
        if(!SDL_IsGameController(device))continue;
        const auto instance=SDL_JoystickGetDeviceInstanceID(device);
        if(instance<0||std::any_of(controllers.begin(),controllers.end(),[&](const auto& p){return p.instance==instance;}))continue;
        if(std::none_of(controllers.begin(),controllers.end(),[](const auto& p){return !p.controller;}))break;
        auto* pad=SDL_GameControllerOpen(device);if(!pad)continue;
        char guid[33]{};SDL_JoystickGetGUIDString(SDL_JoystickGetGUID(SDL_GameControllerGetJoystick(pad)),guid,sizeof(guid));
        const char* serial=SDL_GameControllerGetSerial(pad);const char* path=SDL_GameControllerPath(pad);
        const std::string identity=std::string(guid)+":"+(serial&&*serial?serial:path?path:"");
        // Retain existing player slots; prefer a disconnected device's old slot.
        auto slot=std::find_if(controllers.begin(),controllers.end(),[&](const auto& p){return !p.controller&&p.identity==identity;});
        if(slot==controllers.end())slot=std::find_if(controllers.begin(),controllers.end(),[](const auto& p){return !p.controller&&p.identity.empty();});
        if(slot==controllers.end())slot=std::find_if(controllers.begin(),controllers.end(),[](const auto& p){return !p.controller;});
        slot->controller=pad;slot->instance=instance;slot->identity=identity;
        SDL_GameControllerSetPlayerIndex(pad,int(slot-controllers.begin()));
    }
}
SDL_AudioDeviceID audio_device=0;
uint32_t audio_rate=0;
bool audio_enabled=true,audio_suspended=false;
uint32_t deferred_audio_rate=0;
bool touch_center_held=false;
[[noreturn]] void audio_error(){DORA3_LOG(stderr,"LIVE AUDIO ERROR %s\n",SDL_GetError());std::_Exit(87);}
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
        std::lock_guard lock(input_mutex);
        return connected_device_info_t{connected_locked(port)?Device::Controller:Device::None,Pak::None};
    };
    cfg.input_callbacks.get_input=[](int port,uint16_t* buttons,float* x,float* y){
        std::lock_guard lock(input_mutex);
        *buttons=0;*x=*y=0;
        if(!connected_locked(port))return false;
        if(!dora3::port::menu_open()&&!dora3::touch::is_editing()){
            *buttons=input[port].buttons;*x=input[port].x;*y=input[port].y;
            if(port==0){
                const auto touch=dora3::touch::read_input();
                const bool center=(touch.buttons&0x2000)!=0;
                auto virtual_buttons=touch.buttons;
                if(dora3::camera::active()){
                    if(center&&!touch_center_held)dora3::camera::center();
                    virtual_buttons&=~uint16_t(0x10|0x800|0x400|0xF|0x2000);
                }
                touch_center_held=center;*buttons|=virtual_buttons;
                if(touch.stickOwned){*x=touch.x;*y=touch.y;}
            }
        }else touch_center_held=false;
        return true;
    };
    DORA3_LOG(stderr,"LIVE INPUT port0=%s; four keyboard/gamepad ports; pak=none; focus clears state\n",input_enabled?"connected":"disconnected");
}

namespace { float camera_wheel=0; bool camera_center_held=false; }
void dora3_camera_mouse_wheel(float value){if(dora3::camera::active()&&!dora3::port::menu_open())camera_wheel+=value;}
void dora3_live_input_event(const SDL_Event& event){
    // Only P1 controller activity hides P1 touch controls.
    if((event.type==SDL_CONTROLLERBUTTONDOWN&&event.cbutton.which!=controllers[0].instance)||
       (event.type==SDL_CONTROLLERAXISMOTION&&event.caxis.which!=controllers[0].instance))return;
    dora3::touch::process_event(event);
}
void dora3_poll_live_input(SDL_Window* window){
    refresh_controllers();dora3::touch::update();
    using namespace dora3::port;
    const auto config=settings();
    if(applied_background!=int(config.background_gamepad_input)){
        SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS,config.background_gamepad_input?"1":"0");
        applied_background=int(config.background_gamepad_input);
    }
    PhysicalInput physical;
    physical.background_gamepad=config.background_gamepad_input;
#ifdef __ANDROID__
    physical.background_gamepad=false;
#endif
    physical.focused=(SDL_GetWindowFlags(window)&SDL_WINDOW_INPUT_FOCUS)!=0;
    const auto* keys=SDL_GetKeyboardState(nullptr);
    if(physical.focused)std::copy_n(keys,SDL_NUM_SCANCODES,physical.keys.begin());
    std::array<Input,PlayerCount> next{};
    for(int player=0;player<PlayerCount;++player) {
        const auto& slot=controllers[player];auto& pad=physical.gamepads[player];
        pad.connected=slot.controller!=nullptr;pad.instance=slot.instance;
        const auto& controls=config.players[player];
        // P1 always has the keyboard. Other ports appear with a pad or assigned keys.
        next[player].connected=player==0||pad.connected||
            std::any_of(controls.keys.begin(),controls.keys.begin()+CameraZoomIn,[](int key){return key>0;});
        if(!slot.controller)continue;
        const char* name=SDL_GameControllerName(slot.controller);pad.name=name?name:"Gamepad";
        if(!physical.focused&&!config.background_gamepad_input)continue;
        for(int i=0;i<SDL_CONTROLLER_BUTTON_MAX;i++)pad.pad[i]=SDL_GameControllerGetButton(slot.controller,SDL_GameControllerButton(i))?1.f:0.f;
        for(int i=0;i<SDL_CONTROLLER_AXIS_MAX;i++) {
            const float v=axis(SDL_GameControllerGetAxis(slot.controller,SDL_GameControllerAxis(i)),controls.deadzone);
            pad.pad[SDL_CONTROLLER_BUTTON_MAX+i*2]=std::max(0.f,-v);
            pad.pad[SDL_CONTROLLER_BUTTON_MAX+i*2+1]=std::max(0.f,v);
        }
    }
    PhysicalInput gameplay;
    const bool blocked=process_port_input(physical,&gameplay)||dora3::touch::is_editing();
    const bool modern=input_enabled&&!blocked&&!capture_active()&&config.camera.enabled&&dora3::camera::active();
    #ifdef __ANDROID__
    const bool mouse_capture=false;
#else
    const bool mouse_capture=modern&&physical.focused&&config.camera.capture_mouse;
#endif
    const auto touch_camera=dora3::touch::take_camera();
    const bool captured=SDL_GetRelativeMouseMode()==SDL_TRUE;
    if(mouse_capture!=captured)SDL_SetRelativeMouseMode(mouse_capture?SDL_TRUE:SDL_FALSE);
    int mouse_x=0,mouse_y=0;SDL_GetRelativeMouseState(&mouse_x,&mouse_y);
    if(!mouse_capture||!captured)mouse_x=mouse_y=0;
    if(!modern){dora3::camera::clear_input();camera_center_held=false;}
    if(input_enabled&&!blocked)for(int player=0;player<PlayerCount;++player) {
        auto& state=next[player];const auto& controls=config.players[player];
        const auto& pad=gameplay.gamepads[player].pad;
        constexpr uint16_t masks[]={0x8000,0x4000,0x2000,0x1000,0x20,0x10,0x800,0x400,0x200,0x100,8,4,2,1};
        const auto amount=[&](int action){
            const int key=controls.keys[action],binding=controls.pad[action];
            const float keyboard=key>0&&gameplay.keys[key]?1.0f:0.0f;
            const int index=binding<100?binding:SDL_CONTROLLER_BUTTON_MAX+binding-100;
            const float gamepad=index>=0&&index<int(pad.size())?pad[index]:0.0f;
            return std::max(keyboard,gamepad);
        };
        for(int i=0;i<StickUp;i++)if(amount(i)>(i>=CUp?0.32f:0.01f))state.buttons|=masks[i];
        const bool center=amount(Z)>.01f;
        if(player==0&&modern) {
            const bool zoom_mode=amount(R)>.01f;
            const float vertical=amount(CDown)-amount(CUp);
            dora3::camera::submit(amount(CRight)-amount(CLeft),zoom_mode?0.f:vertical,float(mouse_x)+touch_camera.dx,float(mouse_y)+touch_camera.dy,
                std::max(amount(CameraZoomOut),amount(DDown))-std::max(amount(CameraZoomIn),amount(DUp))+(zoom_mode?vertical:0.f)+touch_camera.zoom,
                physical.focused?camera_wheel:0.f,center&&!camera_center_held);
            state.buttons&=~uint16_t(0x10|0x800|0x400|0xF|0x2000);
        }
        if(player==0)camera_center_held=center;
        state.x=amount(StickRight)-amount(StickLeft);state.y=amount(StickUp)-amount(StickDown);
        const float length=std::sqrt(state.x*state.x+state.y*state.y);
        if(length>1){state.x/=length;state.y/=length;}
    }
    camera_wheel=0;
    std::lock_guard lock(input_mutex);input=next;
}

void dora3_audio_frequency(uint32_t frequency){
    if(!audio_enabled)return;
    if(frequency<8000 || frequency>96000){DORA3_LOG(stderr,"LIVE AUDIO unsupported rate %u\n",frequency);std::_Exit(87);}
    std::lock_guard lock(audio_mutex);
    if(audio_suspended){deferred_audio_rate=frequency;return;}
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
    DORA3_LOG(stderr,"LIVE AUDIO OPEN rate=%u channels=2 format=S16 driver=%s\n",frequency,SDL_GetCurrentAudioDriver());
}

void dora3_audio_queue(const int16_t* samples,size_t count){
    if(!audio_enabled)return;
    std::lock_guard lock(audio_mutex);
    if(audio_suspended)return;
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
    return audio_device&&!audio_suspended ? size_t(SDL_GetQueuedAudioSize(audio_device)/(2*sizeof(int16_t))) : 0;
}

uint8_t dora3_live_connected_mask(){
    std::lock_guard lock(input_mutex);uint8_t mask=0;
    for(int player=0;player<dora3::port::PlayerCount;++player)if(connected_locked(player))mask|=uint8_t(1u<<player);
    return mask;
}

Dora3LiveInputSnapshot dora3_live_input_snapshot(){
    std::lock_guard lock(input_mutex);
    return {input[0].buttons,input[0].x,input[0].y,input_enabled};
}

void dora3_audio_suspend(bool value){
    uint32_t resume_rate=0;
    {
        std::lock_guard lock(audio_mutex);
        if(audio_suspended==value)return;
        audio_suspended=value;
        // Called only after SDL_PollEvent returns, outside its lifecycle lock.
        if(audio_device){SDL_PauseAudioDevice(audio_device,1);SDL_ClearQueuedAudio(audio_device);}
        if(!value){resume_rate=deferred_audio_rate;deferred_audio_rate=0;
            if(audio_device)SDL_PauseAudioDevice(audio_device,0);}
    }
    if(resume_rate)dora3_audio_frequency(resume_rate);
}
void dora3_reset_live_io(){
    // Keep device/controller handles and settings; discard the old session PCM.
    std::scoped_lock lock(input_mutex,audio_mutex);
    input={};touch_center_held=false;
    if(audio_device&&!audio_suspended){
        SDL_PauseAudioDevice(audio_device,1);
        SDL_ClearQueuedAudio(audio_device);
        SDL_PauseAudioDevice(audio_device,0);
    }
}

void dora3_shutdown_live_io(){
    for(auto& slot:controllers){if(slot.controller)SDL_GameControllerClose(slot.controller);slot={};}
    std::lock_guard lock(audio_mutex);
    if(audio_device){SDL_CloseAudioDevice(audio_device);audio_device=0;}
}
