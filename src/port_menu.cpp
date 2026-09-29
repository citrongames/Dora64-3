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
    const bool enabled=(p.focused||p.background_gamepad)&&menu_open()&&!ui_input_blocked();
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
    // Same output-height scale as Dora64; never tied to game render resolution.
    const auto original_style=ImGui::GetStyle();const float original_font=ImGui::GetIO().FontGlobalScale;
    const float scale=size.y/720.0f;
    ImGui::GetStyle().ScaleAllSizes(scale);ImGui::GetIO().FontGlobalScale=original_font*scale;
    ImGui::PushFont(ImGui::GetFont());
    if(menu_open()) {
        ImGui::SetNextFrameWantCaptureKeyboard(true);
        ImGui::SetNextFrameWantCaptureMouse(true);
        ImGui::GetBackgroundDrawList()->AddRectFilled(viewport->Pos,
            ImVec2(viewport->Pos.x+viewport->Size.x,viewport->Pos.y+viewport->Size.y),IM_COL32(0,0,0,145));
        ImGui::SetNextWindowPos(viewport->GetCenter(),ImGuiCond_Always,ImVec2(0.5f,0.5f));
        ImGui::SetNextWindowSize(ImVec2(std::min(760*scale,size.x-24),std::min(500*scale,size.y-24)),ImGuiCond_Always);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,10*scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(22*scale,18*scale));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,5*scale);
        ImGui::PushStyleVar(ImGuiStyleVar_TabRounding,5*scale);
        ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(0.025f,0.035f,0.055f,0.98f));
        ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.08f,0.38f,0.56f,1));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(0.11f,0.52f,0.72f,1));
        ImGui::PushStyleColor(ImGuiCol_CheckMark,ImVec4(0.45f,0.95f,0.75f,1));
        if(ImGui::Begin("##Dora3PcMenu",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::SetWindowFontScale(1.18f);ImGui::TextUnformatted("Dora64-3");ImGui::SetWindowFontScale(1);
            ImGui::SameLine();ImGui::TextDisabled("PC menu");ImGui::Separator();ImGui::Spacing();
            const auto error=save_error();if(!error.empty())ImGui::TextWrapped("Settings error: %s",error.c_str());
            static Action capture_action=A;
            static bool capture_pad=false;
            bool open_capture=false;
            ImGui::BeginDisabled(ui_input_blocked());
            if(ImGui::BeginTabBar("##Dora3PcMenuTabs")) {
                if(ImGui::BeginTabItem("Game")) {
                    if(ImGui::BeginChild("##GameContent",ImVec2(0,-66*scale))) {
                        ImGui::TextUnformatted("Settings profiles");
                        const ImVec2 profile_size((ImGui::GetContentRegionAvail().x-ImGui::GetStyle().ItemSpacing.x)*0.5f,ImGui::GetFrameHeight()*1.8f);
                        if(ImGui::Button("Modern",profile_size))profile(true);ImGui::SameLine();
                        if(ImGui::Button("Original",profile_size))profile(false);
                        ImGui::Spacing();ImGui::Separator();ImGui::Spacing();
                        ImGui::TextWrapped("Original: original resolution and N64 filtering. Modern: fit to window and bilinear filtering.");
                        ImGui::TextWrapped("Both profiles preserve the original game speed and 4:3 picture.");
                        if(ImGui::Checkbox("Show FPS counter",&s.show_fps))set_general(s);
                        ImGui::Spacing();ImGui::TextWrapped("The game keeps running while this menu is open.");
                    }
                    ImGui::EndChild();ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Graphics")) {
                    if(ImGui::BeginChild("##GraphicsContent",ImVec2(0,-66*scale))) {
                        bool changed=false;int mode=s.fullscreen?1:0;
                        ImGui::PushItemWidth(-195*scale);
                        if(ImGui::Combo("Display mode",&mode,"Windowed\0Borderless fullscreen\0")){s.fullscreen=mode!=0;changed=true;}
                        if(!s.fullscreen) {
                            const auto preview=std::to_string(s.window_width)+" x "+std::to_string(s.window_height);
                            if(ImGui::BeginCombo("Output resolution",preview.c_str())) {
                                constexpr int sizes[][2]={{640,480},{800,600},{960,540},{1024,768},{1280,720},{1600,900},{1920,1080},{2560,1440},{3840,2160}};
                                for(const auto& dimensions:sizes) {
                                    const bool selected=s.window_width==dimensions[0]&&s.window_height==dimensions[1];
                                    const auto label=std::to_string(dimensions[0])+" x "+std::to_string(dimensions[1]);
                                    if(ImGui::Selectable(label.c_str(),selected)){s.window_width=dimensions[0];s.window_height=dimensions[1];changed=true;}
                                    if(selected)ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }
                        } else ImGui::TextUnformatted("Output resolution: desktop/native");
                        changed|=ImGui::Combo("Game render resolution",&s.resolution,"Original (1x)\0Original (2x)\0Original (4x)\0Fit to window\0");
                        changed|=ImGui::Combo("Texture filtering",&s.filtering,"Original N64 (three-point)\0Bilinear\0");
                        ImGui::PopItemWidth();if(changed)set_general(s);
                    }
                    ImGui::EndChild();ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Audio")) {
                    if(ImGui::BeginChild("##AudioContent",ImVec2(0,-66*scale)))
                        if(ImGui::SliderInt("Master volume",&s.volume,0,100,"%d%%"))set_general(s);
                    ImGui::EndChild();ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Controls")) {
                    if(ImGui::BeginChild("##ControlsContent",ImVec2(0,-66*scale))) {
                        const auto p=physical();ImGui::TextWrapped("Gamepad: %s",p.connected?p.controller_name.c_str():"not connected");
                        if(ImGui::Checkbox("Background gamepad input",&s.background_gamepad_input))set_background_gamepad_input(s.background_gamepad_input);
                        ImGui::TextWrapped("Gamepad only; keyboard input still requires focus.");
                        int deadzone=int(s.deadzone*100+0.5f);
                        if(ImGui::SliderInt("Stick dead zone",&deadzone,5,50,"%d%%"))set_deadzone(deadzone/100.0f);
                        ImGui::TextWrapped("Escape and Back / Select are reserved for the PC menu.");ImGui::Spacing();
                        if(ImGui::BeginTable("##InputBindings",3,ImGuiTableFlags_Borders|ImGuiTableFlags_RowBg|ImGuiTableFlags_ScrollY|ImGuiTableFlags_SizingStretchProp,ImVec2(0,252*scale))) {
                            ImGui::TableSetupScrollFreeze(0,1);
                            ImGui::TableSetupColumn("Action",ImGuiTableColumnFlags_WidthStretch,2);
                            ImGui::TableSetupColumn("Keyboard",ImGuiTableColumnFlags_WidthStretch,1);
                            ImGui::TableSetupColumn("Gamepad",ImGuiTableColumnFlags_WidthStretch,1);ImGui::TableHeadersRow();
                            for(int i=0;i<ActionCount;i++) {
                                ImGui::PushID(i);ImGui::TableNextRow();ImGui::TableNextColumn();ImGui::AlignTextToFramePadding();ImGui::TextUnformatted(action_names[i]);
                                ImGui::TableNextColumn();if(ImGui::Button((std::string(key_name(s.keys[i]))+"###keyboard").c_str(),ImVec2(-1,0))){capture_action=Action(i);capture_pad=false;begin_capture(capture_action,false);open_capture=true;}
                                ImGui::TableNextColumn();ImGui::BeginDisabled(!p.connected);
                                if(ImGui::Button((pad_name(s.pad[i])+"###gamepad").c_str(),ImVec2(-1,0))){capture_action=Action(i);capture_pad=true;begin_capture(capture_action,true);open_capture=true;}
                                ImGui::EndDisabled();ImGui::PopID();
                            }
                            ImGui::EndTable();
                        }
                        if(ImGui::Button("Restore default controls"))reset_bindings();
                    }
                    ImGui::EndChild();ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::Separator();ImGui::TextDisabled("Escape / Back: close menu");
            if(ImGui::GetContentRegionAvail().x>470*scale)ImGui::SameLine(ImGui::GetWindowContentRegionMax().x-208*scale);
            if(ImGui::Button("Continue",ImVec2(100*scale,0)))set_menu_open(false);
            ImGui::SameLine();if(ImGui::Button("Exit game",ImVec2(100*scale,0)))ImGui::OpenPopup("Exit game?");
            ImGui::EndDisabled();
            if(open_capture)ImGui::OpenPopup("Bind control");
            ImGui::SetNextWindowPos(viewport->GetCenter(),ImGuiCond_Always,ImVec2(0.5f,0.5f));
            ImGui::SetNextWindowSize(ImVec2(std::min(420*scale,size.x-24),0),ImGuiCond_Always);
            if(ImGui::BeginPopupModal("Bind control",nullptr,ImGuiWindowFlags_AlwaysAutoResize)) {
                if(!capture_active())ImGui::CloseCurrentPopup();
                else {
                    ImGui::TextUnformatted(action_names[capture_action]);ImGui::Spacing();
                    ImGui::TextWrapped("%s",capture_pad?"Release the current control, then press a button, trigger, or stick direction.":"Release the current key, then press the new key.");
                    if(ImGui::Button("Clear binding")){bind(capture_action,capture_pad,capture_pad?-1:0);cancel_capture();ImGui::CloseCurrentPopup();}
                    ImGui::SameLine();if(ImGui::Button("Cancel")){cancel_capture();ImGui::CloseCurrentPopup();}
                }
                ImGui::EndPopup();
            }
            ImGui::SetNextWindowPos(viewport->GetCenter(),ImGuiCond_Always,ImVec2(0.5f,0.5f));
            if(ImGui::BeginPopupModal("Exit game?",nullptr,ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextUnformatted("Exit Dora64-3?");ImGui::Spacing();
                if(ImGui::Button("Exit",ImVec2(100*scale,0))){request_exit();ImGui::CloseCurrentPopup();}
                ImGui::SameLine();if(ImGui::Button("Cancel",ImVec2(100*scale,0)))ImGui::CloseCurrentPopup();
                ImGui::EndPopup();
            }
        }
        ImGui::End();ImGui::PopStyleColor(4);ImGui::PopStyleVar(4);
    }
    if(s.show_fps) {
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x+size.x-12*scale,viewport->WorkPos.y+12*scale),ImGuiCond_Always,ImVec2(1,0));
        ImGui::SetNextWindowBgAlpha(0.72f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,5*scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(8*scale,5*scale));
        ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(0.025f,0.03f,0.04f,1));
        if(ImGui::Begin("##OutputFPS",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoInputs|ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoFocusOnAppearing)) {
            ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.72f,1.0f,0.76f,1.0f));
            if(have_fps)ImGui::Text("FPS %.1f",fps);else ImGui::TextUnformatted("FPS --");
            ImGui::PopStyleColor();
        }
        ImGui::End();ImGui::PopStyleColor();ImGui::PopStyleVar(2);
    }
    ImGui::GetIO().FontGlobalScale=original_font;ImGui::PopFont();ImGui::GetStyle()=original_style;
}
}
