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
std::filesystem::path pc_path,input_path;
std::string error;
using Json=nlohmann::json;
Json pc_document=Json::object(),input_document=Json::object();
constexpr const char* action_keys[]={"n64_a","n64_b","n64_z","n64_start","n64_l","n64_r",
    "dpad_up","dpad_down","dpad_left","dpad_right","c_up","c_down","c_left","c_right",
    "move_up","move_down","move_left","move_right"};
// Dora64 GamepadInput serialization. Extra SDL buttons use a separate range.
constexpr int pad_codes[]={-1,SDL_CONTROLLER_BUTTON_A,SDL_CONTROLLER_BUTTON_B,SDL_CONTROLLER_BUTTON_X,SDL_CONTROLLER_BUTTON_Y,
    SDL_CONTROLLER_BUTTON_START,SDL_CONTROLLER_BUTTON_LEFTSHOULDER,SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
    pad_axis(SDL_CONTROLLER_AXIS_TRIGGERLEFT,true),pad_axis(SDL_CONTROLLER_AXIS_TRIGGERRIGHT,true),
    SDL_CONTROLLER_BUTTON_LEFTSTICK,SDL_CONTROLLER_BUTTON_RIGHTSTICK,SDL_CONTROLLER_BUTTON_DPAD_UP,SDL_CONTROLLER_BUTTON_DPAD_DOWN,
    SDL_CONTROLLER_BUTTON_DPAD_LEFT,SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
    pad_axis(SDL_CONTROLLER_AXIS_LEFTY,false),pad_axis(SDL_CONTROLLER_AXIS_LEFTY,true),pad_axis(SDL_CONTROLLER_AXIS_LEFTX,false),pad_axis(SDL_CONTROLLER_AXIS_LEFTX,true),
    pad_axis(SDL_CONTROLLER_AXIS_RIGHTY,false),pad_axis(SDL_CONTROLLER_AXIS_RIGHTY,true),pad_axis(SDL_CONTROLLER_AXIS_RIGHTX,false),pad_axis(SDL_CONTROLLER_AXIS_RIGHTX,true)};
int encode_pad(int code){for(int i=0;i<int(std::size(pad_codes));i++)if(pad_codes[i]==code)return i;return 1000+code;}
int decode_pad(int code){return code>=0&&code<int(std::size(pad_codes))?pad_codes[code]:code>=1000?code-1000:-2;}
void integer(const Json& j,const char* name,int& dst,int min,int max){
    if(j.contains(name)&&j[name].is_number_integer()){const auto v=j[name].get<int64_t>();if(v>=min&&v<=max)dst=int(v);}
}
void real(const Json& j,const char* name,float& dst,float min,float max){
    if(j.contains(name)&&j[name].is_number()){float v=j[name].get<float>();if(std::isfinite(v)&&v>=min&&v<=max)dst=v;}
}
void boolean(const Json& j,const char* name,bool& dst){if(j.contains(name)&&j[name].is_boolean())dst=j[name].get<bool>();}
bool read(const std::filesystem::path& path,Json& data){
    try {std::ifstream in(path);in>>data;if(!data.is_object())throw std::runtime_error("Expected a JSON object");return true;}
    catch(const std::exception& e){
        auto backup=path;backup+=".invalid";std::error_code ec;std::filesystem::copy_file(path,backup,std::filesystem::copy_options::none,ec);
        error="Settings could not be loaded; defaults in use. "+std::string(e.what());
        std::fprintf(stderr,"PORT SETTINGS LOAD ERROR: %s\n",error.c_str());data=Json::object();return false;
    }
}
void write(const std::filesystem::path& path,const Json& data){
    auto temp=path;temp+=".tmp";std::ofstream out(temp,std::ios::binary|std::ios::trunc);
    out.exceptions(std::ios::failbit|std::ios::badbit);out<<data.dump(4)<<'\n';out.close();
    #ifdef _WIN32
    if(!MoveFileExW(temp.c_str(),path.c_str(),MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH))
        throw std::runtime_error("Cannot replace settings (Windows error "+std::to_string(GetLastError())+")");
    #else
    std::filesystem::rename(temp,path);
    #endif
}
void save_locked(bool controls=false){
    if(pc_path.empty())return;
    try {
        if(controls){
            input_document["version"]=3;input_document["background_gamepad_input"]=false;
            input_document["stick_dead_zone_percent"]=current.deadzone*100;
            if(!input_document["bindings"].is_object())input_document["bindings"]=Json::object();
            for(int i=0;i<ActionCount;i++)input_document["bindings"][action_keys[i]]={{"keyboard",current.keys[i]},{"gamepad",encode_pad(current.pad[i])}};
            write(input_path,input_document);
        }else{
            pc_document["version"]=14;pc_document["show_fps"]=current.show_fps;
            pc_document["master_volume"]=current.volume/100.0f;pc_document["menu_scale"]=current.ui_scale;
            if(!pc_document["graphics"].is_object())pc_document["graphics"]=Json::object();
            auto& g=pc_document["graphics"];g["window_mode"]=current.fullscreen?"Fullscreen":"Windowed";
            g["display_width"]=current.window_width;g["display_height"]=current.window_height;
            constexpr const char* resolutions[]={"Original","Original2x","Original4x","Auto"};
            g["render_resolution"]=resolutions[current.resolution];g["texture_filtering"]=current.filtering?"Bilinear":"ThreePoint";
            write(pc_path,pc_document);
        }
        error.clear();
    }catch(const std::exception& e){error=e.what();std::fprintf(stderr,"PORT SETTINGS SAVE ERROR: %s\n",error.c_str());}
}
}

bool valid_key(int code) { return code>=0 && code<SDL_NUM_SCANCODES && code!=SDL_SCANCODE_ESCAPE; }
bool valid_pad(int code) { return code==-1 || (code>=0 && code<SDL_CONTROLLER_BUTTON_MAX && code!=SDL_CONTROLLER_BUTTON_BACK) || (code>=100 && code<100+2*SDL_CONTROLLER_AXIS_MAX); }
Settings defaults() {
    Settings s;
    s.keys={SDL_SCANCODE_SPACE,SDL_SCANCODE_C,SDL_SCANCODE_Z,SDL_SCANCODE_RETURN,SDL_SCANCODE_Q,SDL_SCANCODE_E,
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
void initialize(const std::filesystem::path& directory){
    std::lock_guard lock(mutex);current=defaults();error.clear();pc_document=Json::object();input_document=Json::object();
    pc_path=directory/"port_pc_settings.json";input_path=directory/"port_input_settings.json";
    const bool missing_pc=!std::filesystem::exists(pc_path),missing_input=!std::filesystem::exists(input_path);
    // Import test-021 only for missing destinations. Leave its original file intact.
    const auto legacy=directory/"port-settings.json";
    if((missing_pc||missing_input)&&std::filesystem::exists(legacy)){
        Json j;if(read(legacy,j)){
            if(missing_pc){
                integer(j,"volume",current.volume,0,100);integer(j,"resolution",current.resolution,0,3);integer(j,"filtering",current.filtering,0,1);
                boolean(j,"fullscreen",current.fullscreen);boolean(j,"show_fps",current.show_fps);real(j,"ui_scale",current.ui_scale,0.75f,1.5f);
                integer(j,"window_width",current.window_width,480,7680);integer(j,"window_height",current.window_height,360,4320);
            }
            if(missing_input){
                real(j,"deadzone",current.deadzone,0.05f,0.5f);
                for(bool pad:{false,true}){
                    const char* name=pad?"pad":"keys";auto& dst=pad?current.pad:current.keys;
                    if(j.contains(name)&&j[name].is_array()&&j[name].size()==ActionCount)
                        for(int i=0;i<ActionCount;i++)if(j[name][i].is_number_integer()){
                            int v=j[name][i].get<int>();if(pad?valid_pad(v):valid_key(v))dst[i]=v;
                        }
                }
            }
        }
    }
    // Upgrade an untouched test-021 keyboard layout; preserve customized layouts.
    if(missing_input){
        auto old_keys=defaults().keys;old_keys[A]=SDL_SCANCODE_Z;old_keys[B]=SDL_SCANCODE_X;old_keys[Z]=SDL_SCANCODE_SPACE;
        if(current.keys==old_keys)current.keys=defaults().keys;
    }
    if(!missing_pc&&read(pc_path,pc_document)){
        boolean(pc_document,"show_fps",current.show_fps);float volume=current.volume/100.0f;
        real(pc_document,"master_volume",volume,0,1);current.volume=int(volume*100+0.5f);
        real(pc_document,"menu_scale",current.ui_scale,0.75f,1.5f);
        const auto it=pc_document.find("graphics");if(it!=pc_document.end()&&it->is_object()){
            const auto& g=*it;integer(g,"display_width",current.window_width,480,7680);integer(g,"display_height",current.window_height,360,4320);
            if(g.contains("window_mode")&&g["window_mode"].is_string())current.fullscreen=g["window_mode"]=="Fullscreen";
            constexpr const char* resolutions[]={"Original","Original2x","Original4x","Auto"};
            for(int i=0;i<4;i++)if(g.contains("render_resolution")&&g["render_resolution"]==resolutions[i])current.resolution=i;
            if(g.contains("texture_filtering"))current.filtering=g["texture_filtering"]=="Bilinear"?1:0;
        }
    }
    if(!missing_input&&read(input_path,input_document)){
        float percent=current.deadzone*100;real(input_document,"stick_dead_zone_percent",percent,5,50);current.deadzone=percent/100;
        const auto it=input_document.find("bindings");if(it!=input_document.end()&&it->is_object())for(int i=0;i<ActionCount;i++){
            const auto entry=it->find(action_keys[i]);if(entry==it->end()||!entry->is_object())continue;
            int key=current.keys[i],pad=encode_pad(current.pad[i]);integer(*entry,"keyboard",key,0,SDL_NUM_SCANCODES-1);integer(*entry,"gamepad",pad,0,1200);
            if(valid_key(key))current.keys[i]=key;pad=decode_pad(pad);if(valid_pad(pad))current.pad[i]=pad;
        }
    }
    const auto load_error=error;
    if(missing_pc)save_locked();if(missing_input)save_locked(true);
    if(!load_error.empty())error=load_error;
}

Settings settings(){std::lock_guard lock(mutex);return current;}
void set_general(const Settings& v) {
    std::lock_guard lock(mutex);
    current.volume=std::clamp(v.volume,0,100);current.resolution=std::clamp(v.resolution,0,3);current.filtering=std::clamp(v.filtering,0,1);
    current.window_width=std::clamp(v.window_width,480,7680);current.window_height=std::clamp(v.window_height,360,4320);
    current.fullscreen=v.fullscreen;current.show_fps=v.show_fps;
    if(std::isfinite(v.ui_scale))current.ui_scale=std::clamp(v.ui_scale,0.75f,1.5f);
    save_locked();
}
void set_fullscreen(bool value){std::lock_guard lock(mutex);current.fullscreen=value;save_locked();}
void set_deadzone(float v){if(!std::isfinite(v))return;std::lock_guard lock(mutex);current.deadzone=std::clamp(v,0.05f,0.5f);save_locked(true);}
void bind(Action a,bool gamepad,int code) {
    if(a<0||a>=ActionCount||!(gamepad?valid_pad(code):valid_key(code)))return;
    std::lock_guard lock(mutex);auto& bindings=gamepad?current.pad:current.keys;
    const int old=bindings[a],none=gamepad?-1:0;
    // Swap conflicts; one physical control never unexpectedly triggers two actions.
    if(code!=none)for(int i=0;i<ActionCount;i++)if(i!=a&&bindings[i]==code)bindings[i]=old;
    bindings[a]=code;save_locked(true);
}
void reset_bindings(){std::lock_guard lock(mutex);const auto d=defaults();current.keys=d.keys;current.pad=d.pad;current.deadzone=d.deadzone;save_locked(true);}
void profile(bool modern){std::lock_guard lock(mutex);current.resolution=modern?3:0;current.filtering=modern?1:0;save_locked();}
std::string save_error(){std::lock_guard lock(mutex);return error;}
std::string pad_name(int code){
    constexpr const char* names[]={"Unbound","A","B","X","Y","Start","LB","RB","LT","RT","Left stick click","Right stick click",
        "D-pad up","D-pad down","D-pad left","D-pad right","Left stick up","Left stick down","Left stick left","Left stick right",
        "Right stick up","Right stick down","Right stick left","Right stick right"};
    const int serialized=encode_pad(code);if(serialized>=0&&serialized<int(std::size(names)))return names[serialized];
    if(code<SDL_CONTROLLER_BUTTON_MAX){const char* n=SDL_GameControllerGetStringForButton(SDL_GameControllerButton(code));return n?n:"Unknown";}
    const char* n=SDL_GameControllerGetStringForAxis(SDL_GameControllerAxis((code-100)/2));
    return std::string(n?n:"Unknown")+((code-100)%2?" +":" -");
}
}
