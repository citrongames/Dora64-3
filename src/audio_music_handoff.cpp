#include "recomp_context_copy.hpp"
#include <cstdio>
#include <optional>

extern "C" void dora3_s0_800442B0(uint8_t*, recomp_context*);

namespace {
// Only the scheduled game thread requests music and collects these handles.
// The audio thread retires voices; the Original collector owns clearing slot0.
std::optional<int32_t> pending_bank;
constexpr uint32_t music_handle=0x800DAD28, audio_disabled=0x801AA680;
int32_t word(uint8_t* rdram,uint32_t address) {return MEM_W(0,int32_t(address));}
}

extern "C" void dora3_music_reset(uint8_t*,recomp_context*) {pending_bank.reset();}

extern "C" void dora3_music_request(uint8_t* rdram,recomp_context* ctx) {
    // A newer request supersedes any delayed one, including stop IDs0..3 and
    // requests rejected by the Original disabled gate. Never revive an old scene.
    pending_bank.reset();
    std::fprintf(stderr,"MUSIC request bank=%d handle=%08X disabled=%d\n",
                 int32_t(ctx->r4),uint32_t(word(rdram,music_handle)),word(rdram,audio_disabled));
}

extern "C" void dora3_music_busy(uint8_t*,recomp_context* ctx) {
    // ROM800442F8 would discard this request while the previous bank's voices
    // still own the one shared music buffer. Do not overwrite that buffer early.
    if(ctx->r2!=0) {
        pending_bank=int32_t(ctx->r4);
        std::fprintf(stderr,"MUSIC defer bank=%d handle=%08X\n",*pending_bank,uint32_t(ctx->r2));
    }
}

extern "C" void dora3_music_stop(uint8_t* rdram,recomp_context* ctx) {
    pending_bank.reset();
    std::fprintf(stderr,"MUSIC stop duration=%d handle=%08X\n",
                 int32_t(ctx->r4),uint32_t(word(rdram,music_handle)));
}

extern "C" void dora3_music_after_collect(uint8_t* rdram,recomp_context* ctx) {
    if(!pending_bank)return;
    if(word(rdram,audio_disabled)==1) {pending_bank.reset();return;}
    if(word(rdram,music_handle)!=0)return;
    const auto bank=*pending_bank;
    pending_bank.reset(); // Consume before the normal loader's DMA can yield.
    recomp_context request;
    dora3::copy_recomp_context(request,*ctx);
    request.r4=int32_t(bank);
    std::fprintf(stderr,"MUSIC resume bank=%d after previous handle retired\n",bank);
    // Called before the collector's epilogue. A copied register context and the
    // normal downward guest stack preserve its saved registers/return address.
    // Use the real bank loader/start path, including all Original audio policy.
    dora3_s0_800442B0(rdram,&request);
}
