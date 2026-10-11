#include "port_menu.hpp"
#include "port_touch.hpp"
#include "game_reset.hpp"
#include "autosave.hpp"
#include "localization.hpp"
#include "port_display.hpp"
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
    if(elapsed>=0.5f){const auto count=frames.load(std::memory_order_relaxed);const float current=float(count-last_frames)/elapsed;fps=have_fps?fps*0.25f+current*0.75f:current;last_frames=count;sample_time=now;have_fps=true;}
}
const char* key_name(int code){const char* n=SDL_GetScancodeName(SDL_Scancode(code));return code&&n&&*n?n:"Unbound";}
}
void record_output_frame(){frames.fetch_add(1,std::memory_order_relaxed);}
void overlay_input(){
    auto& io=ImGui::GetIO();
    // RT64 calls this before ImGui::NewFrame, including the first frame after
    // context creation. Port layout is fixed; persistent settings use our JSON.
    // Disable both automatic ini loading and saving (also on context shutdown).
    io.IniFilename=nullptr;
    io.ConfigFlags|=ImGuiConfigFlags_NavEnableKeyboard|ImGuiConfigFlags_NavEnableGamepad;
    const auto p=menu_physical();const auto& gamepad=p.gamepads[menu_gamepad()];
    // The SDL backend supplies keyboard events. Release only navigation keys
    // held across opening/capture; fresh keys keep their normal SDL events.
    if(menu_open()) {
        const auto raw=physical();
        const struct { SDL_Scancode scan; ImGuiKey key; } navigation_keys[]{
            {SDL_SCANCODE_TAB,ImGuiKey_Tab},{SDL_SCANCODE_RETURN,ImGuiKey_Enter},
            {SDL_SCANCODE_KP_ENTER,ImGuiKey_KeypadEnter},{SDL_SCANCODE_SPACE,ImGuiKey_Space},
            {SDL_SCANCODE_UP,ImGuiKey_UpArrow},{SDL_SCANCODE_DOWN,ImGuiKey_DownArrow},
            {SDL_SCANCODE_LEFT,ImGuiKey_LeftArrow},{SDL_SCANCODE_RIGHT,ImGuiKey_RightArrow},
            {SDL_SCANCODE_HOME,ImGuiKey_Home},{SDL_SCANCODE_END,ImGuiKey_End},
            {SDL_SCANCODE_PAGEUP,ImGuiKey_PageUp},{SDL_SCANCODE_PAGEDOWN,ImGuiKey_PageDown}};
        for(const auto& key:navigation_keys)
            if(raw.keys[key.scan]&&!p.keys[key.scan])io.AddKeyEvent(key.key,false);
    }
    if(gamepad.connected)io.BackendFlags|=ImGuiBackendFlags_HasGamepad;else io.BackendFlags&=~ImGuiBackendFlags_HasGamepad;
    const bool enabled=gamepad.connected&&(p.focused||p.background_gamepad)&&menu_open()&&!ui_input_blocked();
    auto button=[&](ImGuiKey key,int b){io.AddKeyEvent(key,enabled&&gamepad.pad[b]>0.5f);};
    button(ImGuiKey_GamepadFaceDown,SDL_CONTROLLER_BUTTON_A);button(ImGuiKey_GamepadFaceRight,SDL_CONTROLLER_BUTTON_B);
    button(ImGuiKey_GamepadFaceLeft,SDL_CONTROLLER_BUTTON_X);button(ImGuiKey_GamepadFaceUp,SDL_CONTROLLER_BUTTON_Y);
    button(ImGuiKey_GamepadDpadUp,SDL_CONTROLLER_BUTTON_DPAD_UP);button(ImGuiKey_GamepadDpadDown,SDL_CONTROLLER_BUTTON_DPAD_DOWN);
    button(ImGuiKey_GamepadDpadLeft,SDL_CONTROLLER_BUTTON_DPAD_LEFT);button(ImGuiKey_GamepadDpadRight,SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
    button(ImGuiKey_GamepadStart,SDL_CONTROLLER_BUTTON_START);
    io.AddKeyEvent(ImGuiKey_GamepadBack,false); // Reserved for opening/closing the port menu.
    button(ImGuiKey_GamepadL3,SDL_CONTROLLER_BUTTON_LEFTSTICK);button(ImGuiKey_GamepadR3,SDL_CONTROLLER_BUTTON_RIGHTSTICK);
    button(ImGuiKey_GamepadL1,SDL_CONTROLLER_BUTTON_LEFTSHOULDER);button(ImGuiKey_GamepadR1,SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
    auto stick=[&](ImGuiKey key,int axis,bool positive){float v=enabled?gamepad.pad[SDL_CONTROLLER_BUTTON_MAX+axis*2+int(positive)]:0;io.AddKeyAnalogEvent(key,v>0.1f,v);};
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
#ifdef __ANDROID__
    constexpr float menu_height=620.f;
#else
    constexpr float menu_height=500.f;
#endif
    if(menu_open()) {
        ImGui::SetNextFrameWantCaptureKeyboard(true);
        ImGui::SetNextFrameWantCaptureMouse(true);
        ImGui::GetBackgroundDrawList()->AddRectFilled(viewport->Pos,
            ImVec2(viewport->Pos.x+viewport->Size.x,viewport->Pos.y+viewport->Size.y),IM_COL32(0,0,0,145));
        ImGui::SetNextWindowPos(viewport->GetCenter(),ImGuiCond_Always,ImVec2(0.5f,0.5f));
        ImGui::SetNextWindowSize(ImVec2(std::max(1.0f,std::min(760*scale,size.x-48*scale)),std::max(1.0f,std::min(menu_height*scale,size.y-48*scale))),ImGuiCond_Always);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,10*scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(22*scale,18*scale));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding,5*scale);
        ImGui::PushStyleVar(ImGuiStyleVar_TabRounding,5*scale);
        ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(0.025f,0.035f,0.055f,0.98f));
        ImGui::PushStyleColor(ImGuiCol_TitleBg,ImVec4(0.05f,0.18f,0.28f,1));
        ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.08f,0.38f,0.56f,1));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(0.11f,0.52f,0.72f,1));
        ImGui::PushStyleColor(ImGuiCol_CheckMark,ImVec4(0.45f,0.95f,0.75f,1));
#ifdef __ANDROID__
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,ImVec2(10*scale,14*scale));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,ImVec2(10*scale,10*scale));
        ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize,24*scale);
        ImGui::PushStyleVar(ImGuiStyleVar_GrabMinSize,28*scale);
        ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign,ImVec2(0,.5f));
#endif
        if(ImGui::Begin("##Dora3PcMenu",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings)) {
            ImGui::SetWindowFontScale(1.18f);ImGui::TextUnformatted("Dora64-3");ImGui::SetWindowFontScale(1);
            ImGui::SameLine();
#ifdef __ANDROID__
            ImGui::TextDisabled("Port menu");
#else
            ImGui::TextDisabled("PC menu");
#endif
            ImGui::Separator();ImGui::Spacing();
            const auto error=save_error();if(!error.empty())ImGui::TextWrapped("Settings error: %s",error.c_str());
            static Action capture_action=A;
            static bool capture_pad=false;
            static int capture_player=0;
            bool open_capture=false;
            ImGui::BeginDisabled(ui_input_blocked());
            const auto draw_tab=[&](const char* id,const auto& draw){
#ifdef __ANDROID__
                if(ImGui::BeginChild(id,ImVec2(0,-72*scale)))draw();
                ImGui::EndChild();
#else
                (void)id;draw();
#endif
            };
            if(ImGui::BeginTabBar("##Dora3PcMenuTabs")) {
                if(ImGui::BeginTabItem("Game")) {
                    draw_tab("##GameContent",[&]{
                    {
                        ImGui::TextUnformatted("Settings profiles");
                        const ImVec2 profile_size((ImGui::GetContentRegionAvail().x-ImGui::GetStyle().ItemSpacing.x)*0.5f,ImGui::GetFrameHeight()*1.8f);
                        if(ImGui::Button("Modern",profile_size)){profile(true);s=settings();}ImGui::SameLine();
                        if(ImGui::Button("Original",profile_size)){profile(false);s=settings();}
                        ImGui::Spacing();ImGui::Separator();ImGui::Spacing();
                        const auto& languages=dora3::localization::languages();
                        const char* language_name="Original Japanese";
                        for(const auto& language:languages)if(language.code==s.language)language_name=language.name.c_str();
                        if(ImGui::BeginCombo("Language",language_name)) {
                            for(const auto& language:languages) {
                                const bool active=language.code==s.language;
                                if(ImGui::Selectable(language.name.c_str(),active)){s.language=language.code;set_general(s);}
                                if(active)ImGui::SetItemDefaultFocus();
                            }
                            ImGui::EndCombo();
                        }
                        if(ImGui::Checkbox("Autosave",&s.autosave))set_general(s);
                        ImGui::Separator();
                        bool camera_changed=ImGui::Checkbox("Modern camera",&s.camera.enabled);
                        if(s.camera.enabled) {
#ifdef __ANDROID__
                            ImGui::TextWrapped("Rotate with the right stick or swipe the free area on the right. Adjust swipe sensitivity in Touch. Scripted and fixed cameras keep their original control.");
#else
                            ImGui::TextWrapped("Rotate with the right stick or camera keys. Enable mouse capture to rotate with the mouse. Scripted and boss cameras retain their original control.");
#endif
                            camera_changed|=ImGui::SliderFloat("Camera stick speed",&s.camera.stick_speed,30.f,360.f,"%.0f deg/s");
#ifndef __ANDROID__
                            camera_changed|=ImGui::Checkbox("Capture mouse",&s.camera.capture_mouse);
                            camera_changed|=ImGui::SliderFloat("Camera mouse sensitivity",&s.camera.mouse_sensitivity,.02f,1.f,"%.2f deg/pixel");
#endif
                            camera_changed|=ImGui::Checkbox("Invert camera X",&s.camera.invert_x);
                            camera_changed|=ImGui::Checkbox("Invert camera Y",&s.camera.invert_y);
#ifdef __ANDROID__
                            ImGui::TextWrapped("Zoom: touch + / -, D-pad up / down, or hold Camera mode and move the camera stick vertically. The gear or Back / Select opens this menu.");
#else
                            ImGui::TextWrapped("Zoom: D-pad up / down (your configured bindings), mouse wheel, Page Up / Page Down, or hold Camera mode (RB / E by default) and move the camera stick vertically.");
                            ImGui::TextDisabled("Esc opens this menu and releases the mouse.");
#endif
                        }
                        if(camera_changed)set_general(s);
                        ImGui::Separator();
                        if(ImGui::Checkbox("Show FPS counter",&s.show_fps))set_general(s);
                    }
                    });
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Graphics")) {
                    draw_tab("##GraphicsContent",[&]{
                    {
                        bool changed=false;
#ifndef __ANDROID__
                        int mode=s.fullscreen?1:0;
                        if(ImGui::Combo("Display mode",&mode,"Windowed\0Borderless fullscreen\0")){s.fullscreen=mode!=0;changed=true;}
                        const auto catalog=displays();
                        if(catalog.empty())ImGui::TextDisabled("No displays were reported by SDL.");
                        else {
                            s.display_index=std::clamp(s.display_index,0,int(catalog.size())-1);
                            if(ImGui::BeginCombo("Monitor",catalog[s.display_index].name.c_str())){
                                for(int i=0;i<int(catalog.size());i++){
                                    const bool selected=s.display_index==i;
                                    if(ImGui::Selectable(catalog[i].name.c_str(),selected)){
                                        s.display_index=i;const auto& desktop=catalog[i].desktop;
                                        if(desktop.width>0&&desktop.height>0){s.window_width=desktop.width;s.window_height=desktop.height;s.display_refresh_rate=desktop.refresh_rate;}
                                        changed=true;
                                    }
                                    if(selected)ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }
                            const auto& display=catalog[s.display_index];
                            if(s.fullscreen){
                                if(display.desktop.width>0&&display.desktop.height>0)ImGui::Text("Output resolution: %s (desktop/native)",output_mode_label(display.desktop,true).c_str());
                            } else if(ImGui::BeginCombo("Output resolution",output_mode_label({s.window_width,s.window_height,s.display_refresh_rate},false).c_str())){
                                for(const auto& m:window_modes(display)){
                                    const bool selected=s.window_width==m.width&&s.window_height==m.height;
                                    if(ImGui::Selectable(output_mode_label(m,false).c_str(),selected)){s.window_width=m.width;s.window_height=m.height;s.display_refresh_rate=0;changed=true;}
                                    if(selected)ImGui::SetItemDefaultFocus();
                                }
                                ImGui::EndCombo();
                            }
                        }
                        ImGui::SameLine();if(ImGui::SmallButton("Refresh"))request_display_refresh();
                        #endif
                        int frame_rate_selection=s.frame_rate_mode==0?0:1;
                        if(ImGui::Combo("Frame rate",&frame_rate_selection,"Original\0Manual\0")){
                            s.frame_rate_mode=frame_rate_selection==0?0:2;changed=true;
                        }
                        if(s.frame_rate_mode==2)changed|=ImGui::SliderInt("Frame rate limit",&s.frame_rate_limit,60,360,"%d FPS",ImGuiSliderFlags_AlwaysClamp);
                        if(s.frame_rate_mode!=0){
                            const unsigned rate=display_rate();
                            if(rate>0)ImGui::Text("Display: %u Hz | Target: %u FPS",rate,target_frame_rate(s,rate));
                            ImGui::TextWrapped("Smoother motion with the original game speed. Manual can exceed the display refresh rate.");
                        }
                        changed|=ImGui::Combo("Game render resolution",&s.resolution,"Original (1x)\0Original (2x)\0Original (4x)\0Fit to window\0");
                        changed|=ImGui::Combo("Game aspect ratio",&s.aspect_ratio,"Original (4:3)\0Fit to window / fullscreen\0");
                        changed|=ImGui::Combo("Gameplay HUD layout",&s.hud_layout,"Original (4:3)\0Fit to window / fullscreen\0");
                        changed|=ImGui::Combo("Texture filtering",&s.filtering,"Original N64 (three-point)\0Bilinear\0");
                        ImGui::Spacing();
                        changed|=ImGui::SliderFloat("Object / NPC draw distance",&s.draw_distance,1.f,5.f,"%.1fx",ImGuiSliderFlags_AlwaysClamp);
                        ImGui::TextDisabled("1x = Original. Higher values show objects farther away.");
                        if(changed)set_general(s);
                    }
                    });
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Audio")) {
                    draw_tab("##AudioContent",[&]{
                    {
                        if(ImGui::SliderInt("Master volume",&s.volume,0,100,"%d%%"))set_general(s);
                    }
                    });
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Controls")) {
                    draw_tab("##ControlsContent",[&]{
                    const auto p=physical();
#ifndef __ANDROID__
                    if(ImGui::Checkbox("Background gamepad input",&s.background_gamepad_input))set_background_gamepad_input(s.background_gamepad_input);
                    ImGui::TextDisabled("Gamepad only; keyboard input still requires focus.");
#endif
                    if(ImGui::BeginTabBar("##PlayerControlsTabs")) {
                        constexpr const char* player_names[]={"Player 1","Player 2","Player 3","Player 4"};
                        for(int player=0;player<PlayerCount;++player)if(ImGui::BeginTabItem(player_names[player])) {
                            ImGui::PushID(player);
                            const auto& pad=p.gamepads[player];auto& controls=s.players[player];
                            if(pad.connected)ImGui::Text("Gamepad: %s",pad.name.empty()?"SDL controller":pad.name.c_str());
                            else ImGui::TextDisabled(
#ifdef __ANDROID__
                                "Gamepad: not connected."
#else
                                "Gamepad: not connected. Assign keys to play on the keyboard."
#endif
                            );
                            int deadzone=int(controls.deadzone*100+0.5f);
                            if(ImGui::SliderInt("Stick dead zone",&deadzone,5,50,"%d%%"))set_deadzone(deadzone/100.0f,player);
#ifdef __ANDROID__
                            ImGui::TextDisabled("Back / Select opens the port menu. Touch controls are in Touch.");
                            constexpr int binding_columns=2;
#else
                            ImGui::TextDisabled("Escape and Back / Select are reserved for the PC menu.");
                            constexpr int binding_columns=3;
#endif
                            ImGui::Spacing();
                            // Player tabs need one more row. Shrink only the binding
                            // table, keeping Restore defaults and the footer visible.
                            const float table_height=std::max(1.f,std::min(252*scale,ImGui::GetContentRegionAvail().y-76*scale));
                            if(ImGui::BeginTable("##InputBindings",binding_columns,ImGuiTableFlags_Borders|ImGuiTableFlags_RowBg|ImGuiTableFlags_ScrollY|ImGuiTableFlags_SizingStretchProp,ImVec2(0,table_height))) {
                                ImGui::TableSetupScrollFreeze(0,1);
                                ImGui::TableSetupColumn("Action",ImGuiTableColumnFlags_WidthStretch,2);
#ifndef __ANDROID__
                                ImGui::TableSetupColumn("Keyboard",ImGuiTableColumnFlags_WidthStretch,1);
#endif
                                ImGui::TableSetupColumn("Gamepad",ImGuiTableColumnFlags_WidthStretch,1);ImGui::TableHeadersRow();
                                const int count=player==0?ActionCount:CameraZoomIn;
                                for(int i=0;i<count;i++) {
                                    ImGui::PushID(i);ImGui::TableNextRow();ImGui::TableNextColumn();ImGui::AlignTextToFramePadding();ImGui::TextUnformatted(action_names[i]);
#ifndef __ANDROID__
                                    ImGui::TableNextColumn();
                                    if(ImGui::Button((std::string(key_name(controls.keys[i]))+"###keyboard").c_str(),ImVec2(-1,0))) {
                                        capture_action=Action(i);capture_pad=false;capture_player=player;
                                        begin_capture(capture_action,false,player);open_capture=true;
                                    }
#endif
                                    ImGui::TableNextColumn();ImGui::BeginDisabled(!pad.connected);
                                    if(ImGui::Button((pad_name(controls.pad[i])+"###gamepad").c_str(),ImVec2(-1,0))) {
                                        capture_action=Action(i);capture_pad=true;capture_player=player;
                                        begin_capture(capture_action,true,player);open_capture=true;
                                    }
                                    ImGui::EndDisabled();ImGui::PopID();
                                }
                                ImGui::EndTable();
                            }
                            if(ImGui::Button("Restore defaults for this player"))reset_bindings(player);
                            ImGui::PopID();ImGui::EndTabItem();
                        }
                        ImGui::EndTabBar();
                    }
                    });
                    ImGui::EndTabItem();
                }
#ifdef __ANDROID__
                if(ImGui::BeginTabItem("Touch")) {
                    draw_tab("##TouchContent",dora3::touch::draw_settings);
                    ImGui::EndTabItem();
                }
#endif
                if(ImGui::BeginTabItem("Cheats")) {
                    draw_tab("##CheatsContent",[&]{
                    {
                        bool changed=ImGui::Checkbox("Infinite HP",&s.cheats[InfiniteHealth]);
                        ImGui::TextWrapped("Keep the selected character at their own maximum HP and prevent HP loss.");
                        ImGui::Spacing();ImGui::Separator();ImGui::Spacing();
                        ImGui::TextDisabled("Each enabled item stays at 99, including after use.");
                        constexpr const char* labels[]={"Full Recovery Fruit (99)","Revival Capsule (99)",
                            "Energy Stone (99)","Gas Spray (99)","Mysterious Fruit (99)",
                            "Silver Key (99)","Gold Key (99)"};
                        for(int i=FullRecoveryFruit;i<CheatCount;++i)
                            changed|=ImGui::Checkbox(labels[i-FullRecoveryFruit],&s.cheats[i]);
                        if(changed)set_cheats(s.cheats);
                    }
                    });
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
#ifdef __ANDROID__
            ImGui::Separator();ImGui::AlignTextToFramePadding();ImGui::TextDisabled("Back / Select: close menu");
            ImGui::SameLine(ImGui::GetWindowContentRegionMax().x-200*scale-ImGui::GetStyle().ItemSpacing.x);
#else
            ImGui::SetCursorPosY(std::max(ImGui::GetCursorPosY(),ImGui::GetWindowHeight()-54*scale));
            ImGui::Separator();ImGui::TextDisabled("Escape / Back: close menu");
            ImGui::SameLine(ImGui::GetWindowWidth()-330*scale);
#endif
            if(ImGui::Button("Continue",ImVec2(100*scale,0)))set_menu_open(false);
            ImGui::SameLine();if(ImGui::Button("Reset game",ImVec2(100*scale,0)))ImGui::OpenPopup("Reset game?");
#ifndef __ANDROID__
            ImGui::SameLine();if(ImGui::Button("Exit game",ImVec2(100*scale,0)))ImGui::OpenPopup("Exit game?");
#endif
            ImGui::EndDisabled();
            if(open_capture)ImGui::OpenPopup("Bind control");
            ImGui::SetNextWindowPos(viewport->GetCenter(),ImGuiCond_Always,ImVec2(0.5f,0.5f));
            if(ImGui::BeginPopupModal("Bind control",nullptr,ImGuiWindowFlags_AlwaysAutoResize)) {
                if(!capture_active())ImGui::CloseCurrentPopup();
                else {
                    ImGui::Text("Player %d: %s",capture_player+1,action_names[capture_action]);ImGui::Spacing();
                    ImGui::TextWrapped("%s",capture_pad?"Release the current control, then press a button, trigger, or stick direction.":"Release the current key, then press the new key.");
                    ImGui::Spacing();
                    if(ImGui::Button("Clear binding",ImVec2(120*scale,0))){bind(capture_action,capture_pad,capture_pad?-1:0,capture_player);cancel_capture();ImGui::CloseCurrentPopup();}
                    ImGui::SameLine();if(ImGui::Button("Cancel",ImVec2(100*scale,0))){cancel_capture();ImGui::CloseCurrentPopup();}
                }
                ImGui::EndPopup();
            }
            ImGui::SetNextWindowPos(viewport->GetCenter(),ImGuiCond_Always,ImVec2(0.5f,0.5f));
            if(ImGui::BeginPopupModal("Reset game?",nullptr,ImGuiWindowFlags_AlwaysAutoResize)) {
                ImGui::TextUnformatted("Return to the opening screen?");
                ImGui::TextDisabled("Unsaved progress will be lost.");ImGui::Spacing();
                if(ImGui::Button("Reset",ImVec2(100*scale,0))){
                    set_menu_open(false);cancel_capture();
                    dora3::request_game_reset();ImGui::CloseCurrentPopup();
                }
                ImGui::SameLine();if(ImGui::Button("Cancel",ImVec2(100*scale,0)))ImGui::CloseCurrentPopup();
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
#ifdef __ANDROID__
        const auto menu_pos=ImGui::GetWindowPos(),menu_size=ImGui::GetWindowSize();
        const bool dismiss=!ImGui::IsPopupOpen(nullptr,ImGuiPopupFlags_AnyPopup);
        dora3::touch::set_port_menu_bounds({(menu_pos.x-viewport->Pos.x)/viewport->Size.x,
            (menu_pos.y-viewport->Pos.y)/viewport->Size.y,
            menu_size.x/viewport->Size.x,menu_size.y/viewport->Size.y},dismiss);
        const auto& io=ImGui::GetIO();const auto mouse=io.MousePos;
        const bool outside=mouse.x<menu_pos.x||mouse.y<menu_pos.y||mouse.x>menu_pos.x+menu_size.x||mouse.y>menu_pos.y+menu_size.y;
        if(dismiss&&outside&&io.MouseSource!=ImGuiMouseSource_TouchScreen&&ImGui::IsMouseClicked(ImGuiMouseButton_Left))set_menu_open(false);
#endif
        ImGui::End();
#ifdef __ANDROID__
        ImGui::PopStyleVar(5);
#endif
        ImGui::PopStyleColor(5);ImGui::PopStyleVar(4);
    }
    const auto autosave_notice=dora3::autosave::notification();
    if(autosave_notice!=dora3::autosave::Notification::None) {
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x+12*scale,viewport->WorkPos.y+12*scale),ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.72f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,5*scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(8*scale,5*scale));
        ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(0.025f,0.03f,0.04f,1));
        if(ImGui::Begin("##Dora3Autosave",nullptr,ImGuiWindowFlags_NoDecoration|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoFocusOnAppearing|ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoInputs)) {
            const bool saved=autosave_notice==dora3::autosave::Notification::Saved;
            ImGui::PushStyleColor(ImGuiCol_Text,saved?ImVec4(0.72f,1,0.76f,1):ImVec4(1,0.55f,0.5f,1));
            ImGui::TextUnformatted(saved?"Autosave":"Autosave failed");ImGui::PopStyleColor();
        }
        ImGui::End();ImGui::PopStyleColor();ImGui::PopStyleVar(2);
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
    if(cheats_enabled()) {
        ImGui::SetNextWindowPos(ImVec2(viewport->WorkPos.x+size.x-12*scale,
            viewport->WorkPos.y+size.y-12*scale),ImGuiCond_Always,ImVec2(1,1));
        ImGui::SetNextWindowBgAlpha(0.72f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,5*scale);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(8*scale,5*scale));
        ImGui::PushStyleColor(ImGuiCol_WindowBg,ImVec4(0.025f,0.03f,0.04f,1));
        if(ImGui::Begin("##Dora3CheatsIndicator",nullptr,ImGuiWindowFlags_NoDecoration|
            ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoSavedSettings|
            ImGuiWindowFlags_NoInputs|ImGuiWindowFlags_NoNav|ImGuiWindowFlags_NoFocusOnAppearing)) {
            ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.72f,1.0f,0.76f,1));
            ImGui::TextUnformatted("Cheats On");ImGui::PopStyleColor();
        }
        ImGui::End();ImGui::PopStyleColor();ImGui::PopStyleVar(2);
    }
    dora3::touch::draw();
    ImGui::GetIO().FontGlobalScale=original_font;ImGui::PopFont();ImGui::GetStyle()=original_style;
}
}
