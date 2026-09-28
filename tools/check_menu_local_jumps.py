"""Check the two slot-detail local dispatches without running the game."""
import argparse
import json
import struct
import subprocess
from pathlib import Path
from inspect_rom import ROOT, read_rom
from overlay_layout import prepare_layout
from local_jump_tables import reviewed_local_jumps, verify_generated_local_jumps


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--rom', required=True, type=Path)
    parser.add_argument('--generated', required=True, type=Path)
    args = parser.parse_args()
    rom = read_rom(args.rom)
    meta = json.loads((ROOT / 'recomp/startup.json').read_text())
    sections, compiled, _, _ = prepare_layout(meta, rom)
    hooks = reviewed_local_jumps(meta, sections, compiled, rom)
    assert len(hooks) == 2
    verify_generated_local_jumps(args.generated / 'RecompiledFuncs', hooks)
    # Reject changed table entries and non-NOP delay slots before generating C.
    for spec in meta['local_jump_tables']:
        for address in (int(spec['load'], 16) + 8, int(spec['table'], 16)):
            altered = bytearray(rom)
            offset = 0x635C0 + address - 0x801B8ED0
            struct.pack_into('>I', altered, offset, 1)
            try:
                reviewed_local_jumps(meta, sections, compiled, altered)
            except ValueError:
                pass
            else:
                raise AssertionError('Changed ROM evidence accepted')
    # Compile the exact hook text used in the generated bodies. No renderer,
    # runtime threads, audio, saves or game entrypoint are linked into this test.
    text = '''#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <setjmp.h>
typedef struct { int64_t r2, r16, r18; } recomp_context;
static uint32_t words[5], table_base;
static jmp_buf invalid_target;
static int64_t read_word(int64_t base, int offset) {
    uint32_t addr = (uint32_t)(base + offset);
    assert(addr >= table_base && addr < table_base + 20 && !(addr & 3));
    return (int32_t)words[(addr-table_base)/4];
}
#define MEM_W(base, offset) read_word(base, offset)
static void switch_error(const char* name, uint32_t pc, uint32_t table) {
    (void)name; (void)pc; (void)table; longjmp(invalid_target, 1);
}
static uint32_t chosen;
'''
    for index, hook in enumerate(hooks):
        text += f'static void dispatch{index}(recomp_context* ctx) {{\n{hook["text"]}\n'
        text += '\n'.join(f'L_{target:08X}: chosen = 0x{target:08X}u; return;' for target in hook['targets']) + '\n}\n'
    text += 'int main(void) {\n'
    for index, hook in enumerate(hooks):
        text += '{ const uint32_t targets[5] = {' + ','.join(hex(t) for t in hook['targets']) + '};\n'
        text += f'table_base = 0x{hook["table"]:08X}u; memcpy(words, targets, sizeof(words));\n'
        text += '''for (unsigned i=0;i<5;++i) {
            recomp_context ctx = {1234, (int32_t)(table_base+i*4), (int32_t)(table_base+i*4)};
            recomp_context before = ctx;
'''
        text += f'dispatch{index}(&ctx);\n'
        text += '''assert(chosen == targets[i]);
            assert(ctx.r2 == (int64_t)(int32_t)targets[i]);
            assert(ctx.r16 == before.r16 && ctx.r18 == before.r18);
        }
        words[0] = 0xDEADBEEF;
        recomp_context bad = {0, (int32_t)table_base, (int32_t)table_base};
'''
        text += f'if (!setjmp(invalid_target)) {{ dispatch{index}(&bad); assert(0); }}\n}}\n'
    text += 'return 0; }\n'
    out = ROOT / 'build-local/check-menu-local-jumps'
    out.mkdir(exist_ok=True)
    source = out / 'dispatch.c'
    source.write_text(text)
    binary = out / 'dispatch'
    subprocess.run(['cc', '-std=c11', '-Wall', '-Wextra', '-Werror', '-O2', str(source), '-o', str(binary)], check=True)
    subprocess.run([str(binary)], check=True)
    print('PASS: 10 local branches; loaded target and register preservation; invalid targets fail; changed table/delay rejected; generated labels/hooks verified. No game launch.')


if __name__ == '__main__':
    main()
