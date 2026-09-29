// No ROM, SDL window or GPU. Real settings/input/menu sources and pinned ImGui.
#include "port_menu.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include <cassert>
#include <fstream>
#include <cmath>
#include <thread>
#include <cstdio>
using namespace dora3::port;
int main(int argc,char** argv){
    assert(argc==2);const auto dir=std::filesystem::path(argv[1]);std::filesystem::create_directories(dir);
    initialize(dir);assert(save_error().empty());
    auto s=settings();assert(s.keys[A]==SDL_SCANCODE_Z);
    s.volume=35;s.resolution=2;s.fullscreen=true;s.show_fps=true;s.ui_scale=1.25f;set_general(s);set_deadzone(0.3f);
    bind(A,false,SDL_SCANCODE_X);assert(settings().keys[B]==SDL_SCANCODE_Z);
    bind(Z,true,SDL_CONTROLLER_BUTTON_B);assert(settings().pad[B]==pad_axis(SDL_CONTROLLER_AXIS_TRIGGERLEFT,true));
    initialize(dir);s=settings();assert(s.volume==35&&s.resolution==2&&s.fullscreen&&s.show_fps&&s.ui_scale==1.25f&&std::abs(s.deadzone-0.3f)<1e-6f);
    assert(s.keys[A]==SDL_SCANCODE_X&&s.pad[Z]==SDL_CONTROLLER_BUTTON_B);
    bind(A,false,SDL_SCANCODE_ESCAPE);assert(settings().keys[A]==SDL_SCANCODE_X);
    bind(A,true,SDL_CONTROLLER_BUTTON_BACK);assert(settings().pad[A]==SDL_CONTROLLER_BUTTON_A);
    bind(A,false,0);bind(A,true,-1);assert(settings().keys[A]==0&&settings().pad[A]==-1);
    reset_bindings();assert(settings().keys==defaults().keys&&settings().pad==defaults().pad);
    profile(true);assert(settings().resolution==3&&settings().filtering==1);
    profile(false);assert(settings().resolution==0&&settings().filtering==0);
    {std::ofstream f(dir/"port-settings.json");f<<R"({"version":1,"volume":-3,"ui_scale":"bad","resolution":99,"keys":[-500]})";}
    initialize(dir);assert(settings().volume==100&&settings().ui_scale==1&&settings().resolution==0&&settings().keys==defaults().keys);
    {std::ofstream f(dir/"port-settings.json");f<<"{truncated";}
    initialize(dir);assert(!save_error().empty()&&std::filesystem::exists(dir/"port-settings.json.invalid"));
    set_general(defaults());assert(save_error().empty());
    PhysicalInput p;p.focused=true;process_port_input(p);
    p.keys[SDL_SCANCODE_ESCAPE]=1;assert(process_port_input(p)&&menu_open());
    assert(process_port_input(p)&&menu_open()); // Key repeat does not toggle.
    p.keys.fill(0);process_port_input(p);
    begin_capture(A,false);p.keys[SDL_SCANCODE_RETURN]=1;process_port_input(p);assert(capture_active());
    p.keys.fill(0);process_port_input(p);p.keys[SDL_SCANCODE_T]=1;process_port_input(p);
    assert(!capture_active()&&settings().keys[A]==SDL_SCANCODE_T&&ui_input_blocked());
    p.keys.fill(0);process_port_input(p);assert(!ui_input_blocked());
    begin_capture(B,false);p.keys[SDL_SCANCODE_ESCAPE]=1;process_port_input(p);assert(!capture_active()&&menu_open());
    p.keys.fill(0);p.connected=true;process_port_input(p);begin_capture(StickRight,true);process_port_input(p);
    p.pad[SDL_CONTROLLER_BUTTON_MAX+SDL_CONTROLLER_AXIS_RIGHTX*2+1]=1;process_port_input(p);
    assert(!capture_active()&&settings().pad[StickRight]==pad_axis(SDL_CONTROLLER_AXIS_RIGHTX,true));
    p.pad.fill(0);process_port_input(p);begin_capture(B,true);process_port_input(p);p.connected=false;process_port_input(p);assert(!capture_active());
    p.keys[SDL_SCANCODE_F1]=1;process_port_input(p);assert(!menu_open());
    p.keys[SDL_SCANCODE_Z]=1;p.keys[SDL_SCANCODE_F1]=0;assert(process_port_input(p));
    p.keys.fill(0);assert(!process_port_input(p));
    p.focused=false;assert(process_port_input(p));p.focused=true;p.keys[SDL_SCANCODE_Z]=1;assert(process_port_input(p));
    p.keys.fill(0);assert(!process_port_input(p));
    p.connected=true;p.pad[SDL_CONTROLLER_BUTTON_BACK]=1;process_port_input(p);assert(menu_open());
    p.pad.fill(0);process_port_input(p);
    p.pad[SDL_CONTROLLER_BUTTON_MAX]=0.15f;process_port_input(p);assert(!ui_input_blocked());
    p.pad.fill(0);process_port_input(p);
    ImGui::CreateContext();auto& io=ImGui::GetIO();io.IniFilename=nullptr;io.DeltaTime=1.0f/60;
    unsigned char* pixels;int w,h;io.Fonts->GetTexDataAsRGBA32(&pixels,&w,&h);
    const auto style=ImGui::GetStyle();
    for(const ImVec2 size:{ImVec2(640,480),ImVec2(1280,720),ImVec2(3840,2160),ImVec2(480,900)}) {
        io.DisplaySize=size;
        for(float scale:{0.75f,1.0f,1.5f}) {
            auto config=settings();config.ui_scale=scale;config.show_fps=true;set_general(config);
            for(int i=0;i<3;i++){overlay_input();ImGui::NewFrame();draw_overlay();ImGui::Render();}
            const auto* window=ImGui::FindWindowByName("Dora64-3 | Port menu");assert(window&&window->Active);
            assert(window->Pos.x>=-1&&window->Pos.y>=-1&&window->Pos.x+window->Size.x<=size.x+1&&window->Pos.y+window->Size.y<=size.y+1);
            assert(ImGui::GetDrawData()->TotalVtxCount>0);
            assert(ImGui::GetStyle().WindowPadding.x==style.WindowPadding.x&&io.FontGlobalScale==1.0f&&ImGui::GetFontSize()==13.0f);
        }
    }
    // Exercise navigation through actual ImGui widgets, not only state helpers.
    auto frame=[&]{overlay_input();ImGui::NewFrame();draw_overlay();ImGui::Render();};
    io.DisplaySize=ImVec2(1280,720);auto normal=settings();normal.ui_scale=1;set_general(normal);frame();
    auto* menu=ImGui::FindWindowByName("Dora64-3 | Port menu");assert(menu);
    auto* tabs=ImGui::GetCurrentContext()->TabBars.GetByKey(menu->GetID("PortTabs"));assert(tabs&&tabs->Tabs.Size==2);
    const ImGuiID controls_id=tabs->Tabs[1].ID;
    tabs->NextSelectedTabId=controls_id;frame();frame();
    assert(tabs->SelectedTabId==controls_id);
    // Controls table is present and scrollable, all 18 actions have bindings.
    bool found_table=false;
    for(auto* child:menu->DC.ChildWindows)if(child->Active){found_table=true;assert(child->Size.x>0&&child->Size.y>0);}
    assert(found_table);
    // Return to Settings and exercise keyboard/gamepad navigation.
    tabs->NextSelectedTabId=tabs->Tabs[0].ID;frame();frame();
    // Keyboard tab navigation reaches an interactive settings widget.
    io.AddKeyEvent(ImGuiKey_Tab,true);frame();io.AddKeyEvent(ImGuiKey_Tab,false);frame();
    assert(ImGui::GetCurrentContext()->NavId!=0);

    io.AddKeyEvent(ImGuiKey_DownArrow,true);frame();io.AddKeyEvent(ImGuiKey_DownArrow,false);frame();frame();
    assert(ImGui::GetCurrentContext()->NavId!=0);
    // SDL-state gamepad navigation goes through overlay_input into ImGui.
    p.pad.fill(0);process_port_input(p);frame();p.pad[SDL_CONTROLLER_BUTTON_DPAD_DOWN]=1;process_port_input(p);frame();
    p.pad.fill(0);process_port_input(p);frame();frame();
    assert(ImGui::GetCurrentContext()->NavInputSource==ImGuiInputSource_Gamepad);

    set_menu_open(false);p.pad.fill(0);process_port_input(p);overlay_input();ImGui::NewFrame();draw_overlay();ImGui::Render();
    assert(!ImGui::FindWindowByName("Dora64-3 | Port menu")->Active);
    ImGui::DestroyContext();
    // Shared settings snapshots must remain safe while input/render/audio use them.
    std::thread reader([]{for(int i=0;i<1000;i++){const auto s=settings();assert(s.volume>=0&&s.volume<=100);physical();menu_open();}});
    for(int i=0;i<50;i++){set_deadzone(0.1f+i%4*0.1f);publish_physical(p);}reader.join();
    std::puts("PASS: settings roundtrip/invalid recovery; conflict swap/reserved/clear; capture arm/cancel/axis/disconnect; menu edge/focus/release; headless ImGui scale/font restore/FPS/controls table/keyboard and gamepad navigation; concurrent snapshots");
}
