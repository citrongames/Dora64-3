#include "port_settings.hpp"
#include "json/json.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <mutex>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace dora3::port {
namespace {
std::mutex mutex;
Settings current=defaults();
std::filesystem::path path;
std::string error;
using Json=nlohmann::json;
void save_locked() {
    if(path.empty())return; // Component tests may use the in-memory defaults.
    try {
        const Json data={{"version",1},{"volume",current.volume},{"resolution",current.resolution},
            {"filtering",current.filtering},{"fullscreen",current.fullscreen},{"show_fps",current.show_fps},
            {"ui_scale",current.ui_scale},{"deadzone",current.deadzone},{"keys",current.keys},{"pad",current.pad}};
        auto temp=path;temp+=".tmp";
        std::ofstream out(temp,std::ios::binary|std::ios::trunc);
        out.exceptions(std::ios::failbit|std::ios::badbit);
        out<<data.dump(2)<<'\n';out.close();
        #ifdef _WIN32
        if(!MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
            throw std::runtime_error("Cannot replace port-settings.json (Windows error "+std::to_string(GetLastError())+")");
        #else
        std::filesystem::rename(temp,path);
        #endif
        error.clear();
    } catch(const std::exception& e) {
        error=e.what();std::fprintf(stderr,"PORT SETTINGS SAVE ERROR: %s\n",error.c_str());
    }
}
}
bool valid_key(int code) { return code>=0 && code<SDL_NUM_SCANCODES && code!=SDL_SCANCODE_ESCAPE && code!=SDL_SCANCODE_F1; }
bool valid_pad(int code) { return code==-1 || (code>=0 && code<SDL_CONTROLLER_BUTTON_MAX && code!=SDL_CONTROLLER_BUTTON_BACK) || (code>=100 && code<100+2*SDL_CONTROLLER_AXIS_MAX); }
Settings defaults() {
    Settings s;
    s.keys={SDL_SCANCODE_Z,SDL_SCANCODE_X,SDL_SCANCODE_SPACE,SDL_SCANCODE_RETURN,SDL_SCANCODE_Q,SDL_SCANCODE_E,
        SDL_SCANCODE_UP,SDL_SCANCODE_DOWN,SDL_SCANCODE_LEFT,SDL_SCANCODE_RIGHT,
        SDL_SCANCODE_I,SDL_SCANCODE_K,SDL_SCANCODE_J,SDL_SCANCODE_L,SDL_SCANCODE_W,SDL_SCANCODE_S,SDL_SCANCODE_A,SDL_SCANCODE_D};
    s.pad={SDL_CONTROLLER_BUTTON_A,SDL_CONTROLLER_BUTTON_B,pad_axis(SDL_CONTROLLER_AXIS_TRIGGERLEFT,true),
        SDL_CONTROLLER_BUTTON_START,SDL_CONTROLLER_BUTTON_LEFTSHOULDER,SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
        SDL_CONTROLLER_BUTTON_DPAD_UP,SDL_CONTROLLER_BUTTON_DPAD_DOWN,SDL_CONTROLLER_BUTTON_DPAD_LEFT,SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
        pad_axis(SDL_CONTROLLER_AXIS_RIGHTY,false),pad_axis(SDL_CONTROLLER_AXIS_RIGHTY,true),
        pad_axis(SDL_CONTROLLER_AXIS_RIGHTX,false),pad_axis(SDL_CONTROLLER_AXIS_RIGHTX,true),
        pad_axis(SDL_CONTROLLER_AXIS_LEFTY,false),pad_axis(SDL_CONTROLLER_AXIS_LEFTY,true),
        pad_axis(SDL_CONTROLLER_AXIS_LEFTX,false),pad_axis(SDL_CONTROLLER_AXIS_LEFTX,true)};
    return s;
}
void initialize(const std::filesystem::path& directory) {
    std::lock_guard lock(mutex);current=defaults();path=directory/"port-settings.json";error.clear();
    if(!std::filesystem::exists(path)) {save_locked();return;}
    try {
        std::ifstream in(path);Json j;in>>j;
        if(!j.is_object() || j.value("version",0)!=1)throw std::runtime_error("Unsupported settings version");
        auto integer=[&](const char* name,int& dst,int min,int max){
            if(j.contains(name)&&j[name].is_number_integer()) {const auto v=j[name].get<int64_t>();if(v>=min&&v<=max)dst=int(v);}
        };
        integer("volume",current.volume,0,100);integer("resolution",current.resolution,0,3);integer("filtering",current.filtering,0,1);
        for(auto pair:{std::pair{"fullscreen",&current.fullscreen},std::pair{"show_fps",&current.show_fps}})
            if(j.contains(pair.first)&&j[pair.first].is_boolean())*pair.second=j[pair.first].get<bool>();
        auto real=[&](const char* name,float& dst,float min,float max){
            if(j.contains(name)&&j[name].is_number()){float v=j[name].get<float>();if(std::isfinite(v)&&v>=min&&v<=max)dst=v;}
        };
        real("ui_scale",current.ui_scale,0.75f,1.5f);real("deadzone",current.deadzone,0.05f,0.5f);
        for(bool pad:{false,true}) {
            const char* name=pad?"pad":"keys";auto& dst=pad?current.pad:current.keys;
            if(j.contains(name)&&j[name].is_array()&&j[name].size()==ActionCount)
                for(size_t i=0;i<ActionCount;i++)if(j[name][i].is_number_integer()) {
                    const auto v=j[name][i].get<int64_t>();
                    if(v>=-1&&v<=SDL_NUM_SCANCODES&& (pad?valid_pad(int(v)):valid_key(int(v))))dst[i]=int(v);
                }
        }
    } catch(const std::exception& e) {
        current=defaults();
        // Preserve the bad file before a later menu change replaces settings.
        auto backup=path;backup+=".invalid";std::error_code ec;
        std::filesystem::copy_file(path,backup,std::filesystem::copy_options::none,ec);
        error=std::string("Settings could not be loaded; defaults in use. ")+e.what();
        std::fprintf(stderr,"PORT SETTINGS LOAD ERROR: %s\n",error.c_str());
    }
}
Settings settings(){std::lock_guard lock(mutex);return current;}
void set_general(const Settings& v) {
    std::lock_guard lock(mutex);
    current.volume=std::clamp(v.volume,0,100);current.resolution=std::clamp(v.resolution,0,3);current.filtering=std::clamp(v.filtering,0,1);
    current.fullscreen=v.fullscreen;current.show_fps=v.show_fps;
    if(std::isfinite(v.ui_scale))current.ui_scale=std::clamp(v.ui_scale,0.75f,1.5f);
    save_locked();
}
void set_fullscreen(bool value){std::lock_guard lock(mutex);current.fullscreen=value;save_locked();}
void set_deadzone(float v){if(!std::isfinite(v))return;std::lock_guard lock(mutex);current.deadzone=std::clamp(v,0.05f,0.5f);save_locked();}
void bind(Action a,bool gamepad,int code) {
    if(a<0||a>=ActionCount||!(gamepad?valid_pad(code):valid_key(code)))return;
    std::lock_guard lock(mutex);auto& bindings=gamepad?current.pad:current.keys;
    const int old=bindings[a],none=gamepad?-1:0;
    // Swap conflicts; one physical control never unexpectedly triggers two actions.
    if(code!=none)for(int i=0;i<ActionCount;i++)if(i!=a&&bindings[i]==code)bindings[i]=old;
    bindings[a]=code;save_locked();
}
void reset_bindings(){std::lock_guard lock(mutex);const auto d=defaults();current.keys=d.keys;current.pad=d.pad;current.deadzone=d.deadzone;save_locked();}
void profile(bool modern){std::lock_guard lock(mutex);current.resolution=modern?3:0;current.filtering=modern?1:0;save_locked();}
std::string save_error(){std::lock_guard lock(mutex);return error;}
std::string pad_name(int code) {
    if(code<0)return "Unbound";
    if(code<SDL_CONTROLLER_BUTTON_MAX){const char* n=SDL_GameControllerGetStringForButton(SDL_GameControllerButton(code));return n?n:"Unknown";}
    const int axis=(code-100)/2;const char* n=SDL_GameControllerGetStringForAxis(SDL_GameControllerAxis(axis));
    return std::string(n?n:"Unknown")+((code-100)%2?" +":" -");
}
}
