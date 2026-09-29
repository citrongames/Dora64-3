#include "port_menu.hpp"
#include "imgui/imgui.h"
#include <algorithm>
#include <atomic>
#include <chrono>

namespace dora3::port {
namespace {
std::atomic_uint64_t frames=0;
float fps=0;
bool have_fps=false;
auto sample_time=std::chrono::steady_clock::now();
uint64_t last_frames=0;
void fps_sample(){
    const auto now=std::chrono::steady_clock::now();const float elapsed=std::chrono::duration<float>(now-sample_time).count();
    if(elapsed>=0.5f){const auto count=frames.load(std::memory_order_relaxed);fps=float(count-last_frames)/elapsed;last_frames=count;sample_time=now;have_fps=true;}
}
const char* key_name(int code){const char* n=SDL_GetScancodeName(SDL_Scancode(code));return code&&n&&*n?n:"Unbound";}
}
void record_output_frame(){frames.fetch_add(1,std::memory_order_relaxed);}
void overlay_input(){
    auto& io=ImGui::GetIO();io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard|ImGuiConfigFlags_NavEnableGamepad;
    const auto p=physical();
    if(p.connected)io.BackendFlags|=ImGuiBackendFlags_HasGamepad;else io.BackendFlags&=~ImGuiBackendFlags_HasGamepad;
    const bool enabled=p.focused&&menu_open()&&!ui_input_blocked();
    auto button=[&](ImGuiKey key,int b){io.AddKeyEvent(key,enabled&&p.pad[b]>0.5f);};
    button(ImGuiKey_GamepadFaceDown,SDL_CONTROLLER_BUTTON_A);button(ImGuiKey_GamepadFaceRight,SDL_CONTROLLER_BUTTON_B);
    button(ImGuiKey_GamepadFaceLeft,SDL_CONTROLLER_BUTTON_X);button(ImGuiKey_GamepadFaceUp,SDL_CONTROLLER_BUTTON_Y);
    button(ImGuiKey_GamepadDpadUp,SDL_CONTROLLER_BUTTON_DPAD_UP);button(ImGuiKey_GamepadDpadDown,SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    button(ImGuiKey_GamepadDpadLeft,SDL_CONTROLLER_BUTTON_DPAD_LEFT);button(ImGuiKey_GamepadDpadRight,SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    button(ImGuiKey_GamepadStart,SDL_CONTROLLER_BUTTON_START);
    io.AddKeyEvent(ImGuiKey_GamepadBack,false); // Reserved for opening/closing the port menu.
    button(ImGuiKey_GamepadL3,SDL_CONTROLLER_BUTTON_LEFTSTICK);button(ImGuiKey_GamepadR3,SDL_CONTROLLER_BUTTON_RIGHTSTICK);
    button(ImGuiKey_GamepadL1,SDL_CONTROLLER_BUTTON_LEFTSHOULDER);button(ImGuiKey_GamepadR1,SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    auto stick=[&](ImGuiKey key,int axis,bool positive){float v=enabled?p.pad[SDL_CONTROLLER_BUTTON_MAX+axis*2+int(positive)]:0;io.AddKeyAnalogEvent(key,v>0.1f,v);};
    stick(ImGuiKey_GamepadLStickLeft,SDL_CONTROLLER_AXIS_LEFTX,false);stick(ImGuiKey_GamepadLStickRight,SDL_CONTROLLER_AXIS_LEFTX,true);
    stick(ImGuiKey_GamepadLStickUp,SDL_CONTROLLER_AXIS_LEFTY,false);stick(ImGuiKey_GamepadLStickDown,SDL_CONTROLLER_AXIS_LEFTY,true);
    stick(ImGuiKey_GamepadRStickLeft,SDL_CONTROLLER_AXIS_RIGHTX,false);stick(ImGuiKey_GamepadRStickRight,SDL_CONTROLLER_AXIS_RIGHTX,true);
    stick(ImGuiKey_GamepadRStickUp,SDL_CONTROLLER_AXIS_RIGHTY,false);stick(ImGuiKey_GamepadRStickDown,SDL_CONTROLLER_AXIS_RIGHTY,true);
    stick(ImGuiKey_GamepadL2,SDL_CONTROLLER_AXIS_TRIGGERLEFT,true);stick(ImGuiKey_GamepadR2,SDL_CONTROLLER_AXIS_TRIGGERRIGHT,true);
}
void draw_overlay(){
    fps_sample();auto s=settings();
    const auto* viewport=ImGui::GetMainViewport();const ImVec2 size=viewport->WorkSize;
    if(size.x<=0||size.y<=0)return;
    // Scale up from 720p; keep default text readable in small windows. Fit the window separately.
    const auto original_style=ImGui::GetStyle();const float original_font=ImGui::GetIO().FontGlobalScale;
    const float base=std::min(size.x/960.0f,size.y/720.0f);
    const float scale=std::max(1.0f,base)*s.ui_scale;
    ImGui::GetStyle().ScaleAllSizes(scale);ImGui::GetIO().FontGlobalScale=original_font*scale;
    ImGui::PushFont(ImGui::GetFont());
    if(menu_open()) {
        const ImVec2 pos{viewport->WorkPos.x+size.x*0.5f,viewport->WorkPos.y+size.y*0.5f};
        ImGui::SetNextWindowPos(pos,ImGuiCond_Always,ImVec2(0.5f,0.5f));
        ImGui::SetNextWindowSize(ImVec2(std::min(880*scale,size.x*0.96f),size.y*0.9f),ImGuiCond_Always);
        if(ImGui::Begin("Dora64-3 | Port menu",nullptr,ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoCollapse|ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::TextWrapped("Esc / F1 / controller Back: open or close. Arrows / D-pad: navigate. Enter / A: select.");
            ImGui::TextDisabled("The game keeps running while this menu is open.");
            const auto capture_text=capture_label();if(!capture_text.empty())ImGui::TextWrapped("%s",capture_text.c_str());
            const auto error=save_error();if(!error.empty())ImGui::TextWrapped("Settings error: %s",error.c_str());
            ImGui::Separator();
            ImGui::BeginDisabled(ui_input_blocked());
            if(ImGui::BeginTabBar("PortTabs")) {
                if(ImGui::BeginTabItem("Settings")) {
                    bool changed=false;
                    ImGui::PushItemWidth(-200*scale);
                    changed|=ImGui::SliderInt("Volume",&s.volume,0,100,"%d%%");
                    changed|=ImGui::Checkbox("Fullscreen",&s.fullscreen);
                    changed|=ImGui::Combo("Render resolution",&s.resolution,"Original (1x)\0Original 2x\0Original 4x\0Fit window (integer scale)\0");
                    changed|=ImGui::Combo("Texture filtering",&s.filtering,"N64 three-point\0Bilinear\0");
                    changed|=ImGui::SliderFloat("Menu scale",&s.ui_scale,0.75f,1.5f,"%.2fx");
                    changed|=ImGui::Checkbox("Show output FPS",&s.show_fps);
                    ImGui::PopItemWidth();
                    if(changed)set_general(s);
                    ImGui::Separator();ImGui::TextUnformatted("Graphics profiles");
                    if(ImGui::Button("Original"))profile(false);ImGui::SameLine();if(ImGui::Button("Modern"))profile(true);
                    ImGui::TextWrapped("Original: 1x, N64 filtering. Modern: fit window, bilinear. Both preserve 4:3 and original game timing. Controls are configured separately.");
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Controls")) {
                    const auto p=physical();
                    ImGui::TextWrapped("Gamepad: %s",p.connected?p.controller_name.c_str():"not connected (keyboard available)");
                    float deadzone=s.deadzone*100;
                    if(ImGui::SliderFloat("Stick dead zone",&deadzone,5,50,"%.0f%%"))set_deadzone(deadzone/100);
                    ImGui::TextWrapped("Select a binding, release controls, then press a new key/button or move a stick. Conflicting bindings swap. Esc / F1 and controller Back are reserved for this menu.");
                    if(ImGui::Button("Reset all controls"))ImGui::OpenPopup("Reset controls?");
                    if(ImGui::BeginPopupModal("Reset controls?",nullptr,ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::TextUnformatted("Restore default keyboard, gamepad and dead zone?");
                        if(ImGui::Button("Restore")){reset_bindings();ImGui::CloseCurrentPopup();}ImGui::SameLine();
                        if(ImGui::Button("Cancel"))ImGui::CloseCurrentPopup();ImGui::EndPopup();
                    }
                    if(ImGui::BeginTable("Bindings",5,ImGuiTableFlags_BordersInnerH|ImGuiTableFlags_RowBg|ImGuiTableFlags_ScrollY,
                        ImVec2(0,std::max(100*scale,ImGui::GetContentRegionAvail().y-45*scale)))) {
                        ImGui::TableSetupColumn("Action",ImGuiTableColumnFlags_WidthStretch,1);
                        ImGui::TableSetupColumn("Keyboard",ImGuiTableColumnFlags_WidthStretch,1.3f);
                        ImGui::TableSetupColumn("##ClearKey",ImGuiTableColumnFlags_WidthFixed,45*scale);
                        ImGui::TableSetupColumn("Gamepad",ImGuiTableColumnFlags_WidthStretch,1.6f);
                        ImGui::TableSetupColumn("##ClearPad",ImGuiTableColumnFlags_WidthFixed,45*scale);
                        ImGui::TableSetupScrollFreeze(0,1);ImGui::TableHeadersRow();
                        for(int i=0;i<ActionCount;i++) {
                            ImGui::PushID(i);ImGui::TableNextRow();ImGui::TableNextColumn();ImGui::TextUnformatted(action_names[i]);
                            ImGui::TableNextColumn();if(ImGui::Button((std::string(key_name(s.keys[i]))+"###keyboard").c_str(),ImVec2(-1,0)))begin_capture(Action(i),false);
                            ImGui::TableNextColumn();if(ImGui::Button("X##key"))bind(Action(i),false,0);
                            ImGui::TableNextColumn();ImGui::BeginDisabled(!p.connected);
                            if(ImGui::Button((pad_name(s.pad[i])+"###gamepad").c_str(),ImVec2(-1,0)))begin_capture(Action(i),true);
                            ImGui::EndDisabled();ImGui::TableNextColumn();if(ImGui::Button("X##pad"))bind(Action(i),true,-1);
                            ImGui::PopID();
                        }
                        ImGui::EndTable();
                    }
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::Separator();if(ImGui::Button("Return to game"))set_menu_open(false);
            ImGui::EndDisabled();
        }
        ImGui::End();
    }
    if(s.show_fps) {
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x+size.x-10*scale,viewport->WorkPos.y+10*scale),ImGuiCond_Always,ImVec2(1,0));
        ImGui::SetNextWindowBgAlpha(0.7f);
        if(ImGui::Begin("##OutputFPS",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoInputs|ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoFocusOnAppearing)) {
            if(have_fps)ImGui::Text("FPS %.1f",fps);else ImGui::TextUnformatted("FPS --");
        }
        ImGui::End();
    }
    ImGui::GetIO().FontGlobalScale=original_font;ImGui::PopFont();ImGui::GetStyle()=original_style;
}
}
