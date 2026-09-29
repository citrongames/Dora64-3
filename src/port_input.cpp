#include "port_menu.hpp"
#include <atomic>
#include <mutex>
#include <algorithm>

namespace dora3::port {
namespace {
std::mutex mutex;
PhysicalInput latest,previous;
std::atomic_bool open=false,exit_requested=false;
int capture=-1;
bool capture_pad=false,armed=false,game_release=false,ui_release=false;
bool neutral(const PhysicalInput& p,bool pad,float threshold=0.01f) {
    if(pad)return std::none_of(p.pad.begin(),p.pad.end(),[threshold](float v){return v>threshold;});
    return std::none_of(p.keys.begin(),p.keys.end(),[](unsigned char v){return v!=0;});
}
void open_locked(bool value){open.store(value);capture=-1;armed=false;game_release=true;ui_release=true;}
}
void request_exit(){exit_requested.store(true);}
bool take_exit_request(){return exit_requested.exchange(false);}
void publish_physical(const PhysicalInput& p){std::lock_guard lock(mutex);latest=p;}
PhysicalInput physical(){std::lock_guard lock(mutex);return latest;}
bool menu_open(){return open.load();}
void set_menu_open(bool value){std::lock_guard lock(mutex);open_locked(value);}
bool capture_active(){std::lock_guard lock(mutex);return capture>=0;}
bool ui_input_blocked(){std::lock_guard lock(mutex);return capture>=0||ui_release;}
void begin_capture(Action a,bool gamepad){std::lock_guard lock(mutex);if(a<0||a>=ActionCount)return;capture=a;capture_pad=gamepad;armed=false;ui_release=true;}
void cancel_capture(){std::lock_guard lock(mutex);capture=-1;armed=false;ui_release=true;}
std::string capture_label(){
    std::lock_guard lock(mutex);if(capture<0)return {};
    return std::string(armed?"Press a new ":"Release all controls, then press a ")+
        (capture_pad?"gamepad button / move a stick":"key")+" for "+action_names[capture]+". Esc / Back cancels.";
}
bool process_port_input(const PhysicalInput& p) {
    std::lock_guard lock(mutex);latest=p;
    const bool available=p.focused||p.background_gamepad;
    const bool toggle=(p.focused&&p.keys[SDL_SCANCODE_ESCAPE]&&!previous.keys[SDL_SCANCODE_ESCAPE]) ||
        (available&&p.pad[SDL_CONTROLLER_BUTTON_BACK]>0.5f&&previous.pad[SDL_CONTROLLER_BUTTON_BACK]<=0.5f);
    if(!available){game_release=true;ui_release=true;capture=-1;}
    else if(toggle){if(capture>=0){capture=-1;ui_release=true;}else open_locked(!open.load());}
    if(!p.focused&&capture>=0&&!capture_pad){capture=-1;ui_release=true;}
    const bool keys_neutral=!p.focused||neutral(p,false),pad_neutral=neutral(p,true);
    if(available&&keys_neutral&&pad_neutral)game_release=false;
    // Small drift must not lock the menu and prevent increasing the dead zone.
    if(available&&keys_neutral&&neutral(p,true,0.35f))ui_release=false;
    if(available&&capture>=0&&!toggle) {
        if(!armed){if(capture_pad?neutral(p,true,0.35f):keys_neutral)armed=true;}
        else {
            int code=-1;
            if(capture_pad) {
                if(!p.connected){capture=-1;ui_release=true;}
                else for(size_t i=0;i<p.pad.size();i++)if(p.pad[i]>0.65f && previous.pad[i]<=0.65f) {
                    const int candidate=i<SDL_CONTROLLER_BUTTON_MAX?int(i):100+int(i)-SDL_CONTROLLER_BUTTON_MAX;
                    if(valid_pad(candidate)){code=candidate;break;}
                }
            } else for(int i=1;i<SDL_NUM_SCANCODES;i++)if(p.keys[i]&&!previous.keys[i]&&valid_key(i)){code=i;break;}
            if(code>=0&&capture>=0){bind(Action(capture),capture_pad,code);capture=-1;ui_release=true;}
        }
    }
    previous=p;
    return open.load()||game_release||!available;
}
}
