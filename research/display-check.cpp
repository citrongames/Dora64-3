// SDL display calls are wrapped: no actual window, renderer, ROM or monitor changes.
#include "port_display.hpp"
#include <cassert>
#include <algorithm>
#include <thread>
#include <cstdio>
using namespace dora3::port;
int count=2,enumerations=0,positions=0,sizes=0;Uint32 flags=0;bool fail_fullscreen=false;
extern "C" {
int __wrap_SDL_GetNumVideoDisplays(){++enumerations;return count;}
const char* __wrap_SDL_GetDisplayName(int i){return i==0?"Primary":"Secondary";}
int __wrap_SDL_GetDesktopDisplayMode(int i,SDL_DisplayMode* m){*m={};m->w=i?2560:1920;m->h=i?1440:1080;m->refresh_rate=i?165:60;return 0;}
int __wrap_SDL_GetNumDisplayModes(int){return 4;}
int __wrap_SDL_GetDisplayMode(int i,int j,SDL_DisplayMode* m){*m={};m->w=j==3?100:(i?2560:1920);m->h=j==3?100:(i?1440:1080);m->refresh_rate=j==1?75:60;return 0;}
Uint32 __wrap_SDL_GetWindowFlags(SDL_Window*){return flags;}
int __wrap_SDL_SetWindowFullscreen(SDL_Window*,Uint32 value){if(fail_fullscreen&&value)return -1;flags=value;return 0;}
int __wrap_SDL_SetWindowDisplayMode(SDL_Window*,const SDL_DisplayMode* mode){assert(!mode);return 0;}
void __wrap_SDL_SetWindowPosition(SDL_Window*,int,int){++positions;}
void __wrap_SDL_RestoreWindow(SDL_Window*){}
void __wrap_SDL_SetWindowSize(SDL_Window*,int w,int h){assert(w>=320&&h>=240);++sizes;}
int __wrap_SDL_GetDisplayUsableBounds(int i,SDL_Rect* b){*b={i?1920:0,0,i?2560:1920,i?1400:1040};return 0;}
int __wrap_SDL_GetWindowBordersSize(SDL_Window*,int* t,int* l,int* b,int* rr){*t=30;*l=*b=*rr=1;return 0;}
}
int main(int argc,char** argv){
    assert(argc==2);initialize(argv[1]);refresh_displays();auto d=displays();assert(d.size()==2&&d[1].name=="2: Secondary");
    assert(d[0].modes.size()==2&&d[1].modes.size()==3); // deduplication, desktop insertion, undersized rejection
    auto modes=window_modes(d[1]);assert(std::count(modes.begin(),modes.end(),OutputMode{2560,1440,0})==1);
    assert(std::find(modes.begin(),modes.end(),OutputMode{1280,720,0})!=modes.end());
    assert(output_mode_label(d[1].desktop,true)=="2560 x 1440 @ 165 Hz");
    SDL_Window* window=reinterpret_cast<SDL_Window*>(1);update_display_window(window);const int initial=sizes;
    update_display_window(window);assert(sizes==initial); // manual drag/resize isn't undone every frame
    auto s=settings();s.display_index=1;s.window_width=2560;s.window_height=1440;set_general(s);update_display_window(window);assert(sizes==initial+1);
    s=settings();s.fullscreen=true;set_general(s);update_display_window(window);assert(flags==SDL_WINDOW_FULLSCREEN_DESKTOP);
    s.fullscreen=false;set_general(s);update_display_window(window);assert(flags==0);
    fail_fullscreen=true;s.fullscreen=true;set_general(s);update_display_window(window);assert(!settings().fullscreen);fail_fullscreen=false;
    const int n=enumerations;request_display_refresh();assert(enumerations==n);update_display_window(window);assert(enumerations>n);
    count=1;request_display_refresh();update_display_window(window);assert(settings().display_index==0&&displays().size()==1);
    count=0;request_display_refresh();update_display_window(window);assert(displays().empty());
    count=2;request_display_refresh();update_display_window(window);assert(displays().size()==2);
    std::thread reader([]{for(int i=0;i<1000;i++){auto copy=displays();assert(copy.size()==2);display_rate();settings();}});
    for(int i=0;i<20;i++){refresh_displays();publish_display_rate(i%2?60:165);}reader.join();
    std::puts("PASS: two displays, actual modes/desktop/dedup, common window sizes, SDL-thread refresh, monitor changes, fullscreen/failure, disconnect fallback, no repeated resizing, concurrent snapshots");
}
