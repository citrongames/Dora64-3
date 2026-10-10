#include "diagnostics.hpp"
#include "port_settings.hpp"
#include "draw_distance.hpp"
#include "autosave.hpp"
#include "localization.hpp"
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
    "move_up","move_down","move_left","move_right","camera_zoom_in","camera_zoom_out"};
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
        DORA3_LOG(stderr,"PORT SETTINGS LOAD ERROR: %s\n",error.c_str());data=Json::object();return false;
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
            input_document["version"]=4;input_document["background_gamepad_input"]=current.background_gamepad_input;
            input_document["stick_dead_zone_percent"]=current.players[0].deadzone*100;
            if(!input_document["bindings"].is_object())input_document["bindings"]=Json::object();
            for(int i=0;i<ActionCount;i++)input_document["bindings"][action_keys[i]]={{"keyboard",current.players[0].keys[i]},{"gamepad",encode_pad(current.players[0].pad[i])}};
            // Keep Player 1 in the existing fields for old builds; add only P2-P4.
            if(!input_document["players"].is_object())input_document["players"]=Json::object();
            for(int player=1;player<PlayerCount;++player) {
                auto& row=input_document["players"][std::to_string(player+1)];
                if(!row.is_object())row=Json::object();
                row["stick_dead_zone_percent"]=current.players[player].deadzone*100;
                if(!row["bindings"].is_object())row["bindings"]=Json::object();
                for(int i=0;i<ActionCount;++i)row["bindings"][action_keys[i]]={
                    {"keyboard",current.players[player].keys[i]},
                    {"gamepad",encode_pad(current.players[player].pad[i])}};
            }
            write(input_path,input_document);
        }else{
            pc_document["camera"]={{"modern",current.camera.enabled},{"capture_mouse",current.camera.capture_mouse},
                {"invert_x",current.camera.invert_x},{"invert_y",current.camera.invert_y},
                {"stick_speed",current.camera.stick_speed},{"mouse_sensitivity",current.camera.mouse_sensitivity}};
            if(!pc_document["game"].is_object())pc_document["game"]=Json::object();
            pc_document["game"]["autosave"]=current.autosave;
            pc_document["language"]["code"]=current.language;
            pc_document["version"]=18;pc_document["show_fps"]=current.show_fps;
            pc_document["master_volume"]=current.volume/100.0f;pc_document.erase("menu_scale");
            if(!pc_document["graphics"].is_object())pc_document["graphics"]=Json::object();
            auto& g=pc_document["graphics"];g["window_mode"]=current.fullscreen?"Fullscreen":"Windowed";
            g["display_width"]=current.window_width;g["display_height"]=current.window_height;
            g["display_index"]=current.display_index;g["display_refresh_rate"]=current.display_refresh_rate;
            constexpr const char* frame_rates[]={"Original","Display","Manual"};
            g["frame_rate_mode"]=frame_rates[current.frame_rate_mode];g["frame_rate_limit"]=current.frame_rate_limit;
            constexpr const char* resolutions[]={"Original","Original2x","Original4x","Auto"};
            g["draw_distance"]=current.draw_distance;
            g["aspect_ratio"]=current.aspect_ratio?"Expand":"Original";
            g["hud_layout"]=current.hud_layout?"Full":"Original";
            g["render_resolution"]=resolutions[current.resolution];g["texture_filtering"]=current.filtering?"Bilinear":"ThreePoint";
            if(!pc_document["cheats"].is_object())pc_document["cheats"]=Json::object();
            for(int i=0;i<CheatCount;++i)pc_document["cheats"][cheat_keys[i]]=current.cheats[i];
            write(pc_path,pc_document);
        }
        error.clear();
    }catch(const std::exception& e){error=e.what();DORA3_LOG(stderr,"PORT SETTINGS SAVE ERROR: %s\n",error.c_str());}
}
}

bool valid_key(int code) { return code>=0 && code<SDL_NUM_SCANCODES && code!=SDL_SCANCODE_ESCAPE; }
bool valid_pad(int code) { return code==-1 || (code>=0 && code<SDL_CONTROLLER_BUTTON_MAX && code!=SDL_CONTROLLER_BUTTON_BACK) || (code>=100 && code<100+2*SDL_CONTROLLER_AXIS_MAX); }
Settings defaults() {
    Settings s;
    s.players[0].keys={SDL_SCANCODE_SPACE,SDL_SCANCODE_C,SDL_SCANCODE_Z,SDL_SCANCODE_RETURN,SDL_SCANCODE_Q,SDL_SCANCODE_E,
        SDL_SCANCODE_UP,SDL_SCANCODE_DOWN,SDL_SCANCODE_LEFT,SDL_SCANCODE_RIGHT,
        SDL_SCANCODE_I,SDL_SCANCODE_K,SDL_SCANCODE_J,SDL_SCANCODE_L,SDL_SCANCODE_W,SDL_SCANCODE_S,SDL_SCANCODE_A,SDL_SCANCODE_D,SDL_SCANCODE_PAGEUP,SDL_SCANCODE_PAGEDOWN};
    s.players[0].pad={SDL_CONTROLLER_BUTTON_A,SDL_CONTROLLER_BUTTON_B,pad_axis(SDL_CONTROLLER_AXIS_TRIGGERLEFT,true),
        SDL_CONTROLLER_BUTTON_START,SDL_CONTROLLER_BUTTON_LEFTSHOULDER,SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
        SDL_CONTROLLER_BUTTON_DPAD_UP,SDL_CONTROLLER_BUTTON_DPAD_DOWN,SDL_CONTROLLER_BUTTON_DPAD_LEFT,SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
        pad_axis(SDL_CONTROLLER_AXIS_RIGHTY,false),pad_axis(SDL_CONTROLLER_AXIS_RIGHTY,true),
        pad_axis(SDL_CONTROLLER_AXIS_RIGHTX,false),pad_axis(SDL_CONTROLLER_AXIS_RIGHTX,true),
        pad_axis(SDL_CONTROLLER_AXIS_LEFTY,false),pad_axis(SDL_CONTROLLER_AXIS_LEFTY,true),
        pad_axis(SDL_CONTROLLER_AXIS_LEFTX,false),pad_axis(SDL_CONTROLLER_AXIS_LEFTX,true),-1,-1};
    for(int player=1;player<PlayerCount;++player)s.players[player].pad=s.players[0].pad;
    return s;
}
void initialize(const std::filesystem::path& directory){
    std::lock_guard lock(mutex);current=defaults();error.clear();pc_document=Json::object();input_document=Json::object();
    current.language=dora3::localization::system_language();
    pc_path=directory/"port_pc_settings.json";input_path=directory/"port_input_settings.json";
    const bool missing_pc=!std::filesystem::exists(pc_path),missing_input=!std::filesystem::exists(input_path);
    // Import test-021 only for missing destinations. Leave its original file intact.
    const auto legacy=directory/"port-settings.json";
    if((missing_pc||missing_input)&&std::filesystem::exists(legacy)){
        Json j;if(read(legacy,j)){
            if(missing_pc){
                integer(j,"volume",current.volume,0,100);integer(j,"resolution",current.resolution,0,3);integer(j,"filtering",current.filtering,0,1);
                boolean(j,"fullscreen",current.fullscreen);boolean(j,"show_fps",current.show_fps);
                integer(j,"window_width",current.window_width,480,7680);integer(j,"window_height",current.window_height,360,4320);
            }
            if(missing_input){
                real(j,"deadzone",current.players[0].deadzone,0.05f,0.5f);
                for(bool pad:{false,true}){
                    const char* name=pad?"pad":"keys";auto& dst=pad?current.players[0].pad:current.players[0].keys;
                    if(j.contains(name)&&j[name].is_array()&&j[name].size()>=18&&j[name].size()<=ActionCount)
                        for(int i=0;i<int(j[name].size());i++)if(j[name][i].is_number_integer()){
                            int v=j[name][i].get<int>();if(pad?valid_pad(v):valid_key(v))dst[i]=v;
                        }
                }
            }
        }
    }
    // Upgrade an untouched test-021 keyboard layout; preserve customized layouts.
    if(missing_input){
        auto old_keys=defaults().players[0].keys;old_keys[A]=SDL_SCANCODE_Z;old_keys[B]=SDL_SCANCODE_X;old_keys[Z]=SDL_SCANCODE_SPACE;
        if(current.players[0].keys==old_keys)current.players[0].keys=defaults().players[0].keys;
    }
    if(!missing_pc&&read(pc_path,pc_document)){
        if(pc_document.contains("language")&&pc_document["language"].is_object()&&pc_document["language"].contains("code")&&pc_document["language"]["code"].is_string())current.language=pc_document["language"]["code"].get<std::string>();
        boolean(pc_document,"show_fps",current.show_fps);float volume=current.volume/100.0f;
        real(pc_document,"master_volume",volume,0,1);current.volume=int(volume*100+0.5f);
        const auto game=pc_document.find("game");
        if(game!=pc_document.end()&&game->is_object())boolean(*game,"autosave",current.autosave);
        const auto camera=pc_document.find("camera");
        if(camera!=pc_document.end()&&camera->is_object()) {
            boolean(*camera,"modern",current.camera.enabled);boolean(*camera,"capture_mouse",current.camera.capture_mouse);
            boolean(*camera,"invert_x",current.camera.invert_x);boolean(*camera,"invert_y",current.camera.invert_y);
            real(*camera,"stick_speed",current.camera.stick_speed,30,360);real(*camera,"mouse_sensitivity",current.camera.mouse_sensitivity,.02f,1.f);
        }
        const auto cheats=pc_document.find("cheats");
        if(cheats!=pc_document.end()&&cheats->is_object())
            for(int i=0;i<CheatCount;++i)boolean(*cheats,cheat_keys[i],current.cheats[i]);
        const auto it=pc_document.find("graphics");if(it!=pc_document.end()&&it->is_object()){
            const auto& g=*it;integer(g,"display_width",current.window_width,320,16384);integer(g,"display_height",current.window_height,240,16384);
            integer(g,"display_index",current.display_index,0,1024);integer(g,"display_refresh_rate",current.display_refresh_rate,0,1000);
            integer(g,"frame_rate_limit",current.frame_rate_limit,60,360);
            real(g,"draw_distance",current.draw_distance,1,5);
            if(g.contains("aspect_ratio"))current.aspect_ratio=g["aspect_ratio"]=="Expand";
            if(g.contains("hud_layout"))current.hud_layout=g["hud_layout"]=="Full";
            constexpr const char* frame_rates[]={"Original","Display","Manual"};
            for(int i=0;i<3;i++)if(g.contains("frame_rate_mode")&&g["frame_rate_mode"]==frame_rates[i])current.frame_rate_mode=i;
            if(g.contains("window_mode")&&g["window_mode"].is_string())current.fullscreen=g["window_mode"]=="Fullscreen";
            constexpr const char* resolutions[]={"Original","Original2x","Original4x","Auto"};
            for(int i=0;i<4;i++)if(g.contains("render_resolution")&&g["render_resolution"]==resolutions[i])current.resolution=i;
            if(g.contains("texture_filtering"))current.filtering=g["texture_filtering"]=="Bilinear"?1:0;
        }
    }
    if(!missing_input&&read(input_path,input_document)){
        boolean(input_document,"background_gamepad_input",current.background_gamepad_input);
        float percent=current.players[0].deadzone*100;real(input_document,"stick_dead_zone_percent",percent,5,50);current.players[0].deadzone=percent/100;
        const auto it=input_document.find("bindings");if(it!=input_document.end()&&it->is_object())for(int i=0;i<ActionCount;i++){
            const auto entry=it->find(action_keys[i]);if(entry==it->end()||!entry->is_object())continue;
            int key=current.players[0].keys[i],pad=encode_pad(current.players[0].pad[i]);integer(*entry,"keyboard",key,0,SDL_NUM_SCANCODES-1);integer(*entry,"gamepad",pad,0,1200);
            if(valid_key(key))current.players[0].keys[i]=key;pad=decode_pad(pad);if(valid_pad(pad))current.players[0].pad[i]=pad;
        }
    }
    const auto players=input_document.find("players");
    if(players!=input_document.end()&&players->is_object())for(int player=1;player<PlayerCount;++player) {
        const auto row=players->find(std::to_string(player+1));
        if(row==players->end()||!row->is_object())continue;
        auto& controls=current.players[player];float percent=controls.deadzone*100;
        real(*row,"stick_dead_zone_percent",percent,5,50);controls.deadzone=percent/100;
        const auto bindings=row->find("bindings");
        if(bindings==row->end()||!bindings->is_object())continue;
        for(int i=0;i<ActionCount;++i) {
            const auto entry=bindings->find(action_keys[i]);
            if(entry==bindings->end()||!entry->is_object())continue;
            int key=controls.keys[i],pad=encode_pad(controls.pad[i]);
            integer(*entry,"keyboard",key,0,SDL_NUM_SCANCODES-1);integer(*entry,"gamepad",pad,0,1200);
            if(valid_key(key))controls.keys[i]=key;
            pad=decode_pad(pad);if(valid_pad(pad))controls.pad[i]=pad;
        }
    }
    current.language=dora3::localization::select(current.language);
    configure_cheats(current.cheats);
    dora3::autosave::configure(current.autosave);
    dora3::camera::configure(current.camera);
    current.draw_distance=dora3::distance::configure(current.draw_distance);
    const auto load_error=error;
    if(missing_pc||pc_document.contains("menu_scale"))save_locked();if(missing_input)save_locked(true);
    if(!load_error.empty())error=load_error;
}

Settings settings(){std::lock_guard lock(mutex);return current;}
void set_general(const Settings& v) {
    std::lock_guard lock(mutex);
    current.volume=std::clamp(v.volume,0,100);current.resolution=std::clamp(v.resolution,0,3);current.filtering=std::clamp(v.filtering,0,1);
    current.window_width=std::clamp(v.window_width,320,16384);current.window_height=std::clamp(v.window_height,240,16384);
    current.display_index=std::clamp(v.display_index,0,1024);current.display_refresh_rate=std::clamp(v.display_refresh_rate,0,1000);
    current.frame_rate_mode=std::clamp(v.frame_rate_mode,0,2);current.frame_rate_limit=std::clamp(v.frame_rate_limit,60,360);
    current.aspect_ratio=std::clamp(v.aspect_ratio,0,1);current.hud_layout=std::clamp(v.hud_layout,0,1);
    current.language=dora3::localization::select(v.language);
    current.camera=v.camera;
    current.camera.stick_speed=std::isfinite(v.camera.stick_speed)?std::clamp(v.camera.stick_speed,30.f,360.f):120.f;
    current.camera.mouse_sensitivity=std::isfinite(v.camera.mouse_sensitivity)?std::clamp(v.camera.mouse_sensitivity,.02f,1.f):.15f;
    dora3::camera::configure(current.camera);
    current.fullscreen=v.fullscreen;current.show_fps=v.show_fps;
    current.autosave=v.autosave;dora3::autosave::configure(current.autosave);
    current.draw_distance=dora3::distance::configure(v.draw_distance);
    save_locked();
}
void set_cheats(const CheatSettings& value){std::lock_guard lock(mutex);current.cheats=value;configure_cheats(current.cheats);save_locked();}
void set_background_gamepad_input(bool value){std::lock_guard lock(mutex);current.background_gamepad_input=value;save_locked(true);}
void correct_display_index(int expected,int value){std::lock_guard lock(mutex);if(current.display_index==expected&&expected!=value){current.display_index=value;save_locked();}}
void set_fullscreen(bool value){std::lock_guard lock(mutex);current.fullscreen=value;save_locked();}
void set_deadzone(float v,int player){if(!std::isfinite(v)||player<0||player>=PlayerCount)return;std::lock_guard lock(mutex);current.players[player].deadzone=std::clamp(v,0.05f,0.5f);save_locked(true);}
void bind(Action a,bool gamepad,int code,int player) {
    if(player<0||player>=PlayerCount||a<0||a>=ActionCount||!(gamepad?valid_pad(code):valid_key(code)))return;
    std::lock_guard lock(mutex);auto& bindings=gamepad?current.players[player].pad:current.players[player].keys;
    const int old=bindings[a],none=gamepad?-1:0;
    // Swap conflicts; one physical control never unexpectedly triggers two actions.
    if(code!=none)for(int i=0;i<ActionCount;i++)if(i!=a&&bindings[i]==code)bindings[i]=old;
    bindings[a]=code;save_locked(true);
}
void reset_bindings(int player){if(player<0||player>=PlayerCount)return;std::lock_guard lock(mutex);current.players[player]=defaults().players[player];save_locked(true);}
void profile(bool modern) {
    std::lock_guard lock(mutex);
    // Profiles cover game/presentation enhancements. Keep personal language,
    // camera tuning, controls, audio, display selection and cheats as configured.
    current.autosave=modern;
    current.camera.enabled=modern;
    current.resolution=modern?3:0;
    current.frame_rate_mode=modern?1:0;
    current.aspect_ratio=modern?1:0;
    current.hud_layout=modern?1:0;
    current.filtering=modern?1:0;
    current.draw_distance=dora3::distance::configure(modern?5.f:1.f);
    dora3::autosave::configure(current.autosave);
    dora3::camera::configure(current.camera);
    save_locked();
}
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
