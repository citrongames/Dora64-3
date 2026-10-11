#pragma once
#include <cstdint>
// Thread-local last observed context: no RDRAM access or allocation at crash time.
namespace dora3::crash {
struct EnemyContext {uint32_t actor=0,model=0,rom=0,callback=0,site=0; int state=0,phase=0,kind=0;};
inline thread_local EnemyContext enemy;
}
