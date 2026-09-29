#include "native_present.hpp"
#include "native_game_loop.hpp"
#ifdef DORA3_LIVE_RT64
#include "port_settings.hpp"
#endif
unsigned dora3_requested_native_fps() {
#ifdef DORA3_LIVE_RT64
    const auto settings=dora3::port::settings();
    return settings.frame_rate_mode==2?unsigned(settings.frame_rate_limit):0;
#else
    return 0;
#endif
}

unsigned dora3_native_presentation_fps() {
    return dora3::native_time::native_presentation_owned()?dora3_requested_native_fps():0;
}
