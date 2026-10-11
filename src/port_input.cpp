#include "port_menu.hpp"
#include "port_touch.hpp"
#include <atomic>
#include <mutex>
#include <algorithm>

namespace dora3::port {
namespace {
std::mutex mutex;
PhysicalInput latest,previous;
std::atomic_bool open=false,exit_requested=false;
int capture=-1,capture_player=0,menu_player=0;
SDL_JoystickID capture_instance=-1;
bool capture_pad=false,armed=false,game_release=false,ui_release=false;
bool keyboard_focus_return=false;
std::array<bool,SDL_NUM_SCANCODES> held_keys{};
std::array<std::array<bool,SDL_CONTROLLER_BUTTON_MAX+SDL_CONTROLLER_AXIS_MAX*2>,PlayerCount> held_pad{};
bool neutral_keys(const PhysicalInput& p) {
    return std::none_of(p.keys.begin(),p.keys.end(),[](unsigned char v){return v!=0;});
}
bool neutral_pad(const PhysicalGamepad& p,float threshold=0.35f) {
    return std::none_of(p.pad.begin(),p.pad.end(),[threshold](float v){return v>threshold;});
}
void open_locked(bool value){open.store(value);capture=-1;armed=false;game_release=true;ui_release=true;}
int navigation_player(const PhysicalInput& p) {
    if(p.gamepads[menu_player].connected)return menu_player;
    for(int i=0;i<PlayerCount;++i)if(p.gamepads[i].connected)return i;
    return 0;
}
}
void request_exit(){exit_requested.store(true);}
bool take_exit_request(){return exit_requested.exchange(false);}
void publish_physical(const PhysicalInput& p){std::lock_guard lock(mutex);latest=p;}
PhysicalInput physical(){std::lock_guard lock(mutex);return latest;}
int menu_gamepad(){std::lock_guard lock(mutex);return navigation_player(latest);}
bool menu_open(){return open.load();}
void set_menu_open(bool value){{std::lock_guard lock(mutex);open_locked(value);}dora3::touch::cancel();dora3::camera::clear_input();}
bool capture_active(){std::lock_guard lock(mutex);return capture>=0;}
bool ui_input_blocked(){std::lock_guard lock(mutex);return capture>=0||ui_release;}
void begin_capture(Action a,bool gamepad,int player){
    std::lock_guard lock(mutex);
    if(a<0||a>=ActionCount||player<0||player>=PlayerCount)return;
    if(gamepad&&!latest.gamepads[player].connected)return;
    capture=a;capture_pad=gamepad;capture_player=player;
    capture_instance=latest.gamepads[player].instance;armed=false;ui_release=true;
}
void cancel_capture(){std::lock_guard lock(mutex);capture=-1;armed=false;ui_release=true;}
std::string capture_label(){
    std::lock_guard lock(mutex);if(capture<0)return {};
    return std::string(armed?"Press a new ":"Release all controls, then press a ")+
        (capture_pad?"gamepad button / move a stick":"key")+" for Player "+std::to_string(capture_player+1)+
        " "+action_names[capture]+". Esc / Back cancels.";
}
bool process_port_input(const PhysicalInput& p,PhysicalInput* game_input) {
    std::lock_guard lock(mutex);latest=p;
    const bool available=p.focused||p.background_gamepad;
    bool toggle=p.focused&&p.keys[SDL_SCANCODE_ESCAPE]&&!previous.keys[SDL_SCANCODE_ESCAPE];
    for(int player=0;player<PlayerCount;++player) {
        const auto& pad=p.gamepads[player];const auto& prior=previous.gamepads[player];
        if(available&&pad.connected&&pad.instance==prior.instance&&
           pad.pad[SDL_CONTROLLER_BUTTON_BACK]>0.5f&&prior.pad[SDL_CONTROLLER_BUTTON_BACK]<=0.5f) {
            if(!toggle)menu_player=player;
            toggle=true;
        }
    }
    if(!available){game_release=true;ui_release=true;capture=-1;}
    else if(toggle){if(capture>=0){capture=-1;ui_release=true;}else open_locked(!open.load());}
    if(!p.focused&&capture>=0&&!capture_pad){capture=-1;ui_release=true;}
    if(capture>=0&&capture_pad&&(!p.gamepads[capture_player].connected||
       p.gamepads[capture_player].instance!=capture_instance)){capture=-1;ui_release=true;}
    const bool keys_neutral=!p.focused||neutral_keys(p);
    // Quarantine held controls independently for every player. A drifting
    // controller must not block new keyboard presses or another player's pad.
    const bool reset_keys=game_release||open.load()||keyboard_focus_return||!p.focused;
    for(size_t i=0;i<held_keys.size();++i)held_keys[i]=p.keys[i]&&(reset_keys||held_keys[i]);
    for(int player=0;player<PlayerCount;++player) {
        const bool reset_pad=game_release||open.load()||!available||
            p.gamepads[player].instance!=previous.gamepads[player].instance;
        for(size_t i=0;i<held_pad[player].size();++i)
            held_pad[player][i]=p.gamepads[player].pad[i]>0.01f&&(reset_pad||held_pad[player][i]);
    }
    keyboard_focus_return=!p.focused;game_release=false;
    if(available&&keys_neutral&&neutral_pad(p.gamepads[navigation_player(p)]))ui_release=false;
    if(available&&capture>=0&&!toggle) {
        const auto& pad=p.gamepads[capture_player];const auto& prior=previous.gamepads[capture_player];
        if(!armed){if(capture_pad?neutral_pad(pad):keys_neutral)armed=true;}
        else {
            int code=-1;
            if(capture_pad) {
                for(size_t i=0;i<pad.pad.size();i++)if(pad.pad[i]>0.65f&&prior.pad[i]<=0.65f) {
                    const int candidate=i<SDL_CONTROLLER_BUTTON_MAX?int(i):100+int(i)-SDL_CONTROLLER_BUTTON_MAX;
                    if(valid_pad(candidate)){code=candidate;break;}
                }
            } else for(int i=1;i<SDL_NUM_SCANCODES;i++)if(p.keys[i]&&!previous.keys[i]&&valid_key(i)){code=i;break;}
            if(code>=0){bind(Action(capture),capture_pad,code,capture_player);capture=-1;ui_release=true;}
        }
    }
    if(game_input) {
        *game_input=p;
        for(size_t i=0;i<held_keys.size();++i)if(held_keys[i]||!p.focused)game_input->keys[i]=0;
        for(int player=0;player<PlayerCount;++player)for(size_t i=0;i<held_pad[player].size();++i)
            if(held_pad[player][i])game_input->gamepads[player].pad[i]=0;
    }
    previous=p;
    return open.load()||!available;
}
}
