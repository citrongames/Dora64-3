// Leaf fixtures never admit an enemy owner or execute its Source command scope.
// The connected enemy and whole-world fixtures link native_enemy.cpp instead.
#include "recomp.h"
extern "C" {
int dora3_native_enemy_callback(uint8_t*,recomp_context*,uint32_t,uint32_t,void (*)(uint8_t*,recomp_context*)){return 0;}
void dora3_native_enemy_gravity(uint8_t*,recomp_context*,unsigned){}
void dora3_native_enemy_turn(uint8_t*,recomp_context*,unsigned){}
int dora3_native_enemy_blink_skip(uint8_t*){return 0;}
void dora3_native_enemy_equation(uint8_t*,recomp_context*,uint32_t){}
int dora3_native_enemy_attachment_only(){return 0;}
void dora3_native_enemy_clip_end(uint8_t*,recomp_context*){}
int dora3_native_enemy_commands_active(){return 0;}
int dora3_native_enemy_model_event(uint8_t*,uint32_t,float observed,float target,uint64_t){return observed==target;}
void dora3_native_enemy_model_read(uint8_t*,uint32_t){}
int dora3_native_enemy_model_zero(uint8_t*,uint32_t,float observed,uint64_t){return observed<=0;}
void dora3_native_enemy_projection(uint8_t*,recomp_context*,uint32_t){}
}
