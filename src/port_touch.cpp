#include "port_touch.hpp"

#if defined(__ANDROID__)
#include "port_touch_layout.hpp"
#include "modern_camera.hpp"
#include "port_menu.hpp"
#include "imgui/imgui.h"
#include <cstdio>
#include <fstream>
#include <mutex>
#include <string>

namespace dora3::touch {
namespace {
    State state;
    std::mutex mutex;
    std::filesystem::path settingsPath;
    std::string saveError;
    std::array<float,SDL_CONTROLLER_AXIS_MAX> axisAnchor{}, axisValue{};
    bool suspended=false;
    Rect portMenuBounds{};
    bool portMenuDismissible=false;

    void context() {
        state.set_context(port::settings().camera.enabled, camera::dialogue_active(),
            suspended || (port::menu_open() && !state.editing));
    }
    bool save() {
        try {
            auto temporary=settingsPath;
            temporary += ".tmp";
            std::ofstream output(temporary,std::ios::trunc);
            output.exceptions(std::ios::failbit | std::ios::badbit);
            output << encode_layout(state.layout).dump(2) << '\n';
            output.close();
            std::filesystem::rename(temporary,settingsPath);
            saveError.clear();
            return true;
        }
        catch(const std::exception& e) {
            saveError="Could not save touch layout. Please try again.";
            std::fprintf(stderr,"Dora64-3 touch settings: %s\n",e.what());
            return false;
        }
    }
}

void initialize(const std::filesystem::path& directory) {
    std::lock_guard lock(mutex);
    settingsPath=directory/"port_touch_settings.json";
    try {
        std::ifstream input(settingsPath);
        if(input) { nlohmann::json j; input >> j; state.layout=decode_layout(j); }
    }
    catch(const std::exception& e) { std::fprintf(stderr,"Dora64-3 touch settings: %s\n",e.what()); }
}

void process_event(const SDL_Event& event) {
    bool openMenu=false, closeMenu=false;
    {
        std::lock_guard lock(mutex);
        context();
        switch(event.type) {
        case SDL_FINGERDOWN:
            if(!suspended && port::menu_open()) {
                closeMenu=portMenuDismissible && !portMenuBounds.contains({event.tfinger.x,event.tfinger.y});
                // Consume the down event even when closing, never forward it to a game button.
                break;
            }
            axisAnchor=axisValue; // A held stick must not immediately hide a newly revealed HUD.
            state.down(event.tfinger.touchId,event.tfinger.fingerId,{event.tfinger.x,event.tfinger.y});
            openMenu=state.menuRequested; state.menuRequested=false;
            break;
        case SDL_FINGERMOTION:
            state.move(event.tfinger.touchId,event.tfinger.fingerId,{event.tfinger.x,event.tfinger.y});
            break;
        case SDL_FINGERUP: state.up(event.tfinger.touchId,event.tfinger.fingerId); break;
        case SDL_CONTROLLERBUTTONDOWN: if(!suspended) state.gamepad_activity(); break;
        case SDL_CONTROLLERAXISMOTION: {
            const int axis=event.caxis.axis;
            if(axis>=SDL_CONTROLLER_AXIS_MAX) break;
            const float value=event.caxis.value/32767.0f;
            axisValue[axis]=value;
            if(std::abs(value)<.20f) axisAnchor[axis]=0;
            else if(std::abs(value)>.35f && std::abs(value-axisAnchor[axis])>.25f) {
                axisAnchor[axis]=value;
                if(!suspended) state.gamepad_activity();
            }
            break;
        }
        case SDL_CONTROLLERDEVICEREMOVED: axisAnchor={}; axisValue={}; state.disconnected(); break;
        case SDL_APP_WILLENTERBACKGROUND:
        case SDL_APP_DIDENTERBACKGROUND: suspended=true; state.clear(); break;
        case SDL_APP_DIDENTERFOREGROUND: suspended=false; state.clear(); break;
        case SDL_WINDOWEVENT:
            if(event.window.event==SDL_WINDOWEVENT_FOCUS_LOST) { suspended=true; state.clear(); }
            if(event.window.event==SDL_WINDOWEVENT_FOCUS_GAINED) { suspended=false; state.clear(); }
            break;
        default: break;
        }
    }
    if((openMenu && !port::menu_open()) || (closeMenu && port::menu_open()))
        port::set_menu_open(!port::menu_open());
}

void update() {
    // SDL window and DPI queries stay on the event thread; draw only uses a snapshot.
    SDL_Window* window=SDL_GetKeyboardFocus();
    int w=0,h=0;
    if(window) SDL_GetWindowSize(window,&w,&h);
    static int previousW=0,previousH=0;
    static float density=1;
    if(window && (w!=previousW || h!=previousH)) {
        float dpi=160;
        if(SDL_GetDisplayDPI(SDL_GetWindowDisplayIndex(window),&dpi,nullptr,nullptr)==0 && std::isfinite(dpi))
            density=dpi/160;
        previousW=w; previousH=h;
    }
    std::lock_guard lock(mutex);
    context();
    if(w>0 && h>0) state.resize(float(w),float(h),density);
}
void cancel() { std::lock_guard lock(mutex); state.clear(); portMenuDismissible=false; }
void set_port_menu_bounds(Rect bounds, bool canDismiss) {
    std::lock_guard lock(mutex);
    portMenuBounds=bounds;
    portMenuDismissible=canDismiss;
}
bool is_editing() { std::lock_guard lock(mutex); return state.editing; }
GameInput read_input() { std::lock_guard lock(mutex); context(); return state.read(camera::active()); }
CameraInput take_camera() { std::lock_guard lock(mutex); context(); return state.take_camera(); }

void draw() {
    State view;
    {
        std::lock_guard lock(mutex);
        context();
        // Opening the port menu from a physical button also cancels editing safely.
        if(state.editing && port::menu_open()) { state.finish_edit(false); context(); }
        if(state.blocked || !state.visible) return;
        view=state;
    }
    const auto* viewport=ImGui::GetMainViewport();
    auto* list=ImGui::GetBackgroundDrawList();
    const auto screen=[&](Point p) { return ImVec2(viewport->Pos.x+p.x*viewport->Size.x,viewport->Pos.y+p.y*viewport->Size.y); };
    const float unit=viewport->Size.y/720.0f;
    const char* labels[count]={"","A","B","Z","START","L","R","","+","-"};
    for(int i=0;i<count;++i) {
        const auto control=static_cast<Control>(i);
        if(!view.shown(control)) continue;
        const ImVec2 center=screen(view.layout.positions[i]);
        const Point normalized=view.halfSize(control);
        const ImVec2 radius(normalized.x*viewport->Size.x,normalized.y*viewport->Size.y);
        const bool pressed=view.held(control);
        const float alpha=view.editing ? .8f : pressed ? .9f : view.dialogue ? view.layout.dialogueOpacity : view.layout.opacity;
        const ImU32 outline=IM_COL32(238,246,255,int(255*alpha));
        const ImU32 fill=IM_COL32(18,29,42,int(125*alpha));
        const float line=std::max(1.0f,1.5f*unit);
        if(control==Control::Start || control==Control::L || control==Control::R || control==Control::Menu) {
            const ImVec2 a(center.x-radius.x,center.y-radius.y),b(center.x+radius.x,center.y+radius.y);
            list->AddRectFilled(a,b,fill,8*unit);
            list->AddRect(a,b,outline,8*unit,0,line);
        }
        else {
            list->AddCircleFilled(center,radius.y,fill,48);
            list->AddCircle(center,radius.y,outline,48,line);
        }
        if(control==Control::Stick) {
            const auto offset=view.stick_offset();
            const ImVec2 knob(center.x+offset.x*radius.x*.60f,center.y+offset.y*radius.y*.60f);
            list->AddCircleFilled(knob,radius.y*.36f,IM_COL32(235,245,255,int(90*alpha)),32);
            list->AddCircle(knob,radius.y*.36f,outline,32,line);
        }
        else if(control==Control::Menu) {
            const float r=radius.y*.38f;
            list->AddCircle(center,r,outline,24,line*1.4f);
            list->AddCircle(center,r*.38f,outline,16,line);
            for(int tooth=0;tooth<8;++tooth) {
                const float a=tooth*3.14159265f/4;
                list->AddLine({center.x+std::cos(a)*r,center.y+std::sin(a)*r},
                    {center.x+std::cos(a)*r*1.4f,center.y+std::sin(a)*r*1.4f},outline,line*2);
            }
        }
        else {
            const float fontSize=radius.y*(control==Control::Start ? .65f : 1.0f);
            const ImVec2 textSize=ImGui::GetFont()->CalcTextSizeA(fontSize,10000,0,labels[i]);
            list->AddText(ImGui::GetFont(),fontSize,{center.x-textSize.x/2,center.y-textSize.y/2},outline,labels[i]);
        }
    }
    if(!view.editing) return;

    ImGui::SetNextWindowPos(screen({.5f,.96f}),ImGuiCond_Always,{.5f,1});
    ImGui::SetNextWindowBgAlpha(.94f);
    const ImGuiWindowFlags flags=ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove;
    ImGui::Begin("Touch layout editor",nullptr,flags);
    ImGui::TextUnformatted("Drag buttons to move them. Game input is disabled.");
    const bool saveClicked=ImGui::Button("Save",{100*unit,40*unit});
    ImGui::SameLine();
    const bool cancelClicked=ImGui::Button("Cancel",{100*unit,40*unit});
    ImGui::SameLine();
    const bool resetClicked=ImGui::Button("Reset positions",{150*unit,40*unit});
    bool returnToMenu=false;
    {
        std::lock_guard lock(mutex);
        const auto p=ImGui::GetWindowPos(),s=ImGui::GetWindowSize();
        state.editorToolbar={(p.x-viewport->Pos.x)/viewport->Size.x,(p.y-viewport->Pos.y)/viewport->Size.y,s.x/viewport->Size.x,s.y/viewport->Size.y};
        if(resetClicked) { state.clear(); state.layout.positions=Layout{}.positions; state.resize(state.width,state.height,state.density); }
        if(cancelClicked || (saveClicked && save())) { state.finish_edit(!cancelClicked); returnToMenu=true; }
        if(!saveError.empty()) ImGui::TextUnformatted(saveError.c_str());
    }
    ImGui::End();
    if(returnToMenu && !port::menu_open()) port::set_menu_open(!port::menu_open());
}

void draw_settings() {
    bool edit=false;
    {
        std::lock_guard lock(mutex);
        ImGui::TextWrapped("Controls appear on touch and hide when you use the gamepad. Touch once to reveal hidden controls.");
        ImGui::Spacing();
        bool changed=false;
        ImGui::SliderFloat("Button size",&state.layout.scale,.7f,1.6f,"%.2fx");
        changed |= ImGui::IsItemDeactivatedAfterEdit();
        ImGui::SliderFloat("Opacity",&state.layout.opacity,.15f,.85f,"%.2f");
        changed |= ImGui::IsItemDeactivatedAfterEdit();
        ImGui::SliderFloat("Dialogue opacity",&state.layout.dialogueOpacity,.05f,.5f,"%.2f");
        changed |= ImGui::IsItemDeactivatedAfterEdit();
        ImGui::SliderFloat("Swipe sensitivity",&state.layout.sensitivity,.25f,3.0f,"%.2fx");
        changed |= ImGui::IsItemDeactivatedAfterEdit();
        if(changed) { state.resize(state.width,state.height,state.density); save(); }
        ImGui::Spacing();
        if(ImGui::Button("Edit button positions")) { state.begin_edit(); edit=true; }
        ImGui::TextWrapped("Swipe the free area on the right to control the camera. Modern: look around; + / - zoom. Classic: swipe left / right to rotate, up / down to zoom.");
        ImGui::TextWrapped("A or START advances dialogue. Buttons stay in place and become more transparent.");
        if(!saveError.empty()) {
            ImGui::TextUnformatted(saveError.c_str());
            if(ImGui::Button("Retry saving")) save();
        }
    }
    if(edit && port::menu_open()) port::set_menu_open(!port::menu_open());
}
}
#else
namespace dora3::touch {
void initialize(const std::filesystem::path&) {}
void process_event(const SDL_Event&) {}
void update() {}
void cancel() {}
void set_port_menu_bounds(Rect, bool) {}
bool is_editing() { return false; }
GameInput read_input() { return {}; }
CameraInput take_camera() { return {}; }
void draw() {}
void draw_settings() {}
}
#endif
