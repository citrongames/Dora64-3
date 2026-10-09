#include "diagnostics.hpp"
#include "port_display.hpp"
#include <algorithm>
#include <atomic>
#include <cstdio>
#include <mutex>
#include <tuple>
#ifdef _WIN32
#include <SDL_syswm.h>
#endif
namespace dora3::port {
namespace {
std::mutex display_mutex;
std::vector<DisplayInfo> catalog;
std::atomic_bool refresh_requested{false};
std::atomic_uint output_rate{0};
bool applied=false;
Settings last;
// Adapted from the pinned Dora64 RT64 ApplicationWindow::setDisplayConfig.
// Keep SDL/window mutations on our main SDL thread; no renderer-thread SDL calls.
bool set_display_config(SDL_Window* sdlWindow,const Settings& s) {
#ifdef _WIN32
    SDL_SysWMinfo info{};SDL_VERSION(&info.version);
    if(!SDL_GetWindowWMInfo(sdlWindow,&info))return false;
    const HWND windowHandle=info.info.win.window;
#endif
        const int displayCount = SDL_GetNumVideoDisplays();
        if (displayCount <= 0) {
            fprintf(stderr, "SDL_GetNumVideoDisplays failed: %s\n", SDL_GetError());
            return false;
        }

        const int displayIndex = std::clamp(s.display_index, 0, displayCount - 1);
        int width = std::max(s.window_width, 320);
        int height = std::max(s.window_height, 240);


        const uint32_t currentFlags = SDL_GetWindowFlags(sdlWindow);
        if ((!s.fullscreen) ||
            ((currentFlags & SDL_WINDOW_FULLSCREEN) != 0)) {
            if (SDL_SetWindowFullscreen(sdlWindow, 0) != 0) {
                fprintf(stderr, "Unable to leave fullscreen mode: %s\n", SDL_GetError());
                return false;
            }
        }

        if (SDL_SetWindowDisplayMode(sdlWindow, nullptr) != 0) {
            fprintf(stderr, "Unable to reset the SDL display mode: %s\n", SDL_GetError());
            return false;
        }

        const int centered = SDL_WINDOWPOS_CENTERED_DISPLAY(displayIndex);
        SDL_SetWindowPosition(sdlWindow, centered, centered);

        if (!s.fullscreen) {
#       ifdef _WIN32
            // SDL can leave an externally hooked HWND with the popup style
            // after fullscreen. Restore the normal window chrome
            // explicitly before applying the requested client size.
            LONG_PTR windowStyle = GetWindowLongPtr(windowHandle, GWL_STYLE);
            windowStyle = (windowStyle | WS_VISIBLE | WS_OVERLAPPEDWINDOW) & ~WS_POPUP;
            SetWindowLongPtr(windowHandle, GWL_STYLE, windowStyle);
            SetWindowPos(
                windowHandle,
                HWND_NOTOPMOST,
                0,
                0,
                0,
                0,
                SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);

#       endif
            SDL_RestoreWindow(sdlWindow);
            SDL_SetWindowSize(sdlWindow, width, height);

            SDL_Rect usableBounds{};
            if (SDL_GetDisplayUsableBounds(displayIndex, &usableBounds) == 0) {
#           ifdef _WIN32
                // SDL positions the client origin for this externally hooked
                // HWND, which puts the non-client title bar above Y=0 for an
                // oversized window. Position the outer Win32 rectangle directly
                // so the title bar always remains inside the monitor work area.
                RECT outerRect{};
                GetWindowRect(windowHandle, &outerRect);
                const int outerWidth = outerRect.right - outerRect.left;
                const int outerHeight = outerRect.bottom - outerRect.top;
                const int windowX = usableBounds.x +
                    std::max((usableBounds.w - outerWidth) / 2, 0);
                const int windowY = usableBounds.y +
                    std::max((usableBounds.h - outerHeight) / 2, 0);
                SetWindowPos(
                    windowHandle,
                    nullptr,
                    windowX,
                    windowY,
                    0,
                    0,
                    SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
#           else
                int borderTop = 0;
                int borderLeft = 0;
                int borderBottom = 0;
                int borderRight = 0;
                SDL_GetWindowBordersSize(
                    sdlWindow,
                    &borderTop,
                    &borderLeft,
                    &borderBottom,
                    &borderRight);

                const int outerWidth = width + borderLeft + borderRight;
                const int outerHeight = height + borderTop + borderBottom;
                const int outerX = usableBounds.x +
                    std::max((usableBounds.w - outerWidth) / 2, 0);
                const int outerY = usableBounds.y +
                    std::max((usableBounds.h - outerHeight) / 2, 0);
                const int windowX = outerX + borderLeft;
                const int windowY = outerY + borderTop;
                SDL_SetWindowPosition(sdlWindow, windowX, windowY);
#           endif
            }
            else {
                fprintf(stderr, "Unable to query display usable bounds: %s\n", SDL_GetError());
                SDL_SetWindowPosition(sdlWindow, centered, centered);
            }


            return true;
        }

        if (SDL_SetWindowFullscreen(sdlWindow, SDL_WINDOW_FULLSCREEN_DESKTOP) != 0) {
            fprintf(stderr, "Unable to enter fullscreen mode: %s\n", SDL_GetError());
            SDL_SetWindowDisplayMode(sdlWindow, nullptr);
            SDL_SetWindowSize(sdlWindow, width, height);
            SDL_SetWindowPosition(sdlWindow, centered, centered);

            return false;
        }



        return true;
    }

}
std::vector<DisplayInfo> displays(){std::lock_guard lock(display_mutex);return catalog;}
void request_display_refresh(){refresh_requested.store(true);}
void publish_display_rate(unsigned rate){output_rate.store(rate);}
unsigned display_rate(){return output_rate.load();}
unsigned target_frame_rate(const Settings& s,unsigned rate){return s.frame_rate_mode==0?0:s.frame_rate_mode==1?rate:unsigned(s.frame_rate_limit);}
std::string output_mode_label(const OutputMode& m,bool include_refresh){
    auto label=std::to_string(m.width)+" x "+std::to_string(m.height);
    if(include_refresh&&m.refresh_rate>0)label+=" @ "+std::to_string(m.refresh_rate)+" Hz";
    return label;
}
std::vector<OutputMode> window_modes(const DisplayInfo& display){
    // Dora64: common window sizes plus sizes reported for this monitor, without Hz duplicates.
    std::vector<OutputMode> modes={{640,480,0},{800,600,0},{960,540,0},{1024,768,0},{1280,720,0},{1600,900,0},{1920,1080,0},{2560,1440,0},{3840,2160,0}};
    for(const auto& m:display.modes)modes.push_back({m.width,m.height,0});
    std::sort(modes.begin(),modes.end(),[](const auto& a,const auto& b){return std::tie(a.width,a.height)<std::tie(b.width,b.height);});
    modes.erase(std::unique(modes.begin(),modes.end()),modes.end());return modes;
}
void refresh_displays(){
    std::vector<DisplayInfo> next;
    const int count=SDL_GetNumVideoDisplays();
    if(count<=0)DORA3_LOG(stderr,"PORT display enumeration failed: %s\n",SDL_GetError());
    for(int i=0;i<count;i++){
        DisplayInfo d;const char* name=SDL_GetDisplayName(i);
        d.name=std::to_string(i+1)+": "+(name?name:"Display");
        SDL_DisplayMode m{};
        if(SDL_GetDesktopDisplayMode(i,&m)==0)d.desktop={m.w,m.h,m.refresh_rate};
        const int n=SDL_GetNumDisplayModes(i);
        for(int j=0;j<n;j++)if(SDL_GetDisplayMode(i,j,&m)==0&&m.w>=320&&m.h>=240)d.modes.push_back({m.w,m.h,m.refresh_rate});
        if(d.desktop.width>0&&d.desktop.height>0)d.modes.push_back(d.desktop);
        std::sort(d.modes.begin(),d.modes.end(),[](const auto& a,const auto& b){return std::tie(a.width,a.height,a.refresh_rate)<std::tie(b.width,b.height,b.refresh_rate);});
        d.modes.erase(std::unique(d.modes.begin(),d.modes.end()),d.modes.end());next.push_back(std::move(d));
    }
    const int index=settings().display_index;
    if(!next.empty())correct_display_index(index,std::clamp(index,0,int(next.size())-1));
    std::lock_guard lock(display_mutex);catalog=std::move(next);
}
void update_display_window(SDL_Window* window){
    const bool refresh=refresh_requested.exchange(false);
    if(refresh)refresh_displays();
    const auto s=settings();
    if(displays().empty())return;
    if(!applied||s.fullscreen!=last.fullscreen||s.display_index!=last.display_index||s.window_width!=last.window_width||s.window_height!=last.window_height||(refresh&&s.fullscreen)){
        if(!set_display_config(window,s)){
            DORA3_LOG(stderr,"PORT display change failed: %s\n",SDL_GetError());
            last=s;last.fullscreen=(SDL_GetWindowFlags(window)&SDL_WINDOW_FULLSCREEN)!=0;
            set_fullscreen(last.fullscreen);
        }else last=s; // Do not acknowledge a newer UI request that arrived during SDL calls.
        applied=true;
    }
}
}
