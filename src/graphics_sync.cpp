#include "graphics_sync.hpp"
#ifdef DORA3_LIVE_RT64
#include "live_graphics.hpp"
#endif
#include "ultramodern/ultramodern.hpp"
#include <cstdlib>

namespace { thread_local bool copying_framebuffer=false; }
// These hooks enclose only the original call-free copy loop (and its zero-size
// branch). Never extend the lock across guest scheduling or PI calls.
extern "C" void dora3_begin_cpu_fb_copy() {
    if(copying_framebuffer) std::abort();
    ultramodern::get_graphics_rdram_mutex().lock();
    copying_framebuffer=true;
#ifdef DORA3_LIVE_RT64
    dora3_materialize_native_framebuffers_locked();
#endif
}
extern "C" void dora3_end_cpu_fb_copy() {
    // Earlier exits from the containing function also reach this epilogue.
    if(copying_framebuffer) {
        copying_framebuffer=false;
        ultramodern::get_graphics_rdram_mutex().unlock();
    }
}
