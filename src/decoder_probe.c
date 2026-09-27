/* Differential reference: original decoder + local synchronous ROM-read adapter.
 * No libultra scheduling, GPU, audio, or real game DMA behavior is claimed here. */
#include "recomp.h"
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif
extern void recomp_entrypoint(uint8_t*, recomp_context*);
static unsigned char* rom;
static const size_t rom_size = 16u * 1024u * 1024u;
static const size_t ram_size = 8u * 1024u * 1024u;

void read_rom_dma(uint8_t* rdram, recomp_context* ctx) {
    uint32_t source=(uint32_t)ctx->r4, dest=(uint32_t)ctx->r5, count=(uint32_t)ctx->r6;
    /* The game's reader requests 0x3FF but consumes 0x400 bytes per refill.
     * Its DMA wrapper splits into <=256 byte transfers; the final 255-byte
     * PI transfer rounds to 256. Model that observed case, not general PI I/O. */
    if (count == 0x3ffu) ++count;
    if (source>rom_size || count>rom_size-source || dest<0x80000000u ||
        dest-0x80000000u>ram_size || count>ram_size-(dest-0x80000000u)) exit(7);
    for (uint32_t i=0;i<count;++i) rdram[((dest-0x80000000u)+i)^3]=rom[source+i];
}

recomp_func_t* get_function(int32_t address) {
    fprintf(stderr,"Unexpected decoder callback: %08X\n",(unsigned)address);exit(8);
}

static uint32_t be32(const unsigned char* p) {
    return ((uint32_t)p[0]<<24)|((uint32_t)p[1]<<16)|((uint32_t)p[2]<<8)|p[3];
}

int main(int argc,char** argv) {
    if(argc!=3) {fputs("Use the validating tools/verify_decoder.py wrapper.\n",stderr);return 1;}
    char* end=NULL;unsigned long off=strtoul(argv[2],&end,0);
    if(!end || *end || off>rom_size-16) return 2;
    FILE* input=fopen(argv[1],"rb");if(!input)return 3;
    rom=malloc(rom_size);if(!rom)return 4;
    if(fread(rom,1,rom_size,input)!=rom_size || fgetc(input)!=EOF)return 5;
    fclose(input);
    uint32_t packed=be32(rom+off+8),unpacked=be32(rom+off+12);
    if(be32(rom+off)!=1 || packed>rom_size-off-16 || unpacked>0x400000)return 6;
    uint8_t* memory=calloc(ram_size,1);if(!memory)return 4;
    recomp_context context={0};
    context.r4=off;context.r5=(gpr)(int32_t)0x80400000u;context.r29=(gpr)(int32_t)0x803ff000u;
    recomp_entrypoint(memory,&context);
    if((uint32_t)context.r2!=unpacked) {fprintf(stderr,"Output size mismatch: %u/%u\n",(uint32_t)context.r2,unpacked);return 9;}
    for(uint32_t i=0;i<unpacked;i+=4) {
        unsigned char temp[4];uint32_t n=unpacked-i<4?unpacked-i:4;
        for(uint32_t j=0;j<n;++j)temp[j]=memory[(0x400000u+i+j)^3];
#ifdef _WIN32
        if(i==0)_setmode(_fileno(stdout),_O_BINARY);
#endif
        if(fwrite(temp,1,n,stdout)!=n)return 10;
    }
    free(memory);free(rom);return 0;
}
