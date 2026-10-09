#pragma once
#include <cstdio>
namespace dora3::diagnostics {
#ifdef DORA3_DIAGNOSTICS
inline constexpr bool enabled=true;
#else
inline constexpr bool enabled=false;
#endif
}
// Disabled diagnostics do not evaluate formatting arguments or take CRT locks.
#ifdef DORA3_DIAGNOSTICS
#define DORA3_LOG(...) std::fprintf(__VA_ARGS__)
#define DORA3_LOG_TEXT(...) std::fputs(__VA_ARGS__)
#else
#define DORA3_LOG(...) ((void)0)
#define DORA3_LOG_TEXT(...) ((void)0)
#endif
