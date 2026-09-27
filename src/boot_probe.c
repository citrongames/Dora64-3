/* Native execution of the real ROM entry sequence, stopping before bootproc. */
#include "recomp.h"
#include <stdio.h>
#include <string.h>

extern void recomp_entrypoint(uint8_t*, recomp_context*);
static unsigned handoffs;

void boot_handoff(uint8_t* rdram, recomp_context* ctx) {
    (void)rdram;
    if ((uint32_t)ctx->r29 != 0x80085A00u) {
        fputs("Wrong entry stack\n", stderr);
        exit(2);
    }
    ++handoffs;
}

recomp_func_t* get_function(int32_t address) {
    if ((uint32_t)address == 0x80027414u) return boot_handoff;
    fprintf(stderr, "Unexpected entry handoff: %08X\n", (unsigned)address);
    exit(3);
}

int main(void) {
    const size_t size = 4u * 1024u * 1024u;
    uint8_t* memory = malloc(size);
    if (!memory) return 4;
    memset(memory, 0xA5, size);
    recomp_context context = {0};
    recomp_entrypoint(memory, &context);
    for (size_t i = 0; i < size; ++i) {
        unsigned expected = (i >= 0x7EA80u && i < 0xFECB0u) ? 0u : 0xA5u;
        if (memory[i] != expected) {
            fprintf(stderr, "Unexpected memory at %zX\n", i);
            free(memory);
            return 5;
        }
    }
    free(memory);
    if (handoffs != 1) return 6;
    puts("PASS: native ROM entry; BSS [8007EA80,800FECB0), SP=80085A00, handoff=80027414");
    puts("Scope: entry only. No game loop, frame, input or audio has run.");
    return 0;
}
