"""Emit ROM-checked local branches for reviewed pointer-walking jump tables.

Only LW / JR / NOP sequences are supported. The hook performs the original
load and jumps within the same generated function, preserving its C locals.
This avoids treating a local case label as a function or freezing a loop index.
"""
import re
import struct


def reviewed_local_jumps(meta, sections, compiled, rom):
    hooks = []
    for spec in meta.get('local_jump_tables', []):
        index = next(i for i, s in enumerate(sections) if s['name'] == spec['section'])
        section = sections[index]
        start, load, table = (int(spec[k], 16) for k in ('function', 'load', 'table'))
        name, size, _ = compiled[index, start]
        targets = [int(t, 16) for t in spec['targets']]
        if not targets or len(targets) != len(set(targets)):
            raise ValueError('Local jump targets must be nonempty and distinct')
        def words(address, count):
            if address % 4 or not section['vram'] <= address <= section['vram'] + section['size'] - count * 4:
                raise ValueError('Local jump evidence outside ROM section')
            return struct.unpack_from('>' + str(count) + 'I', rom, section['rom'] + address - section['vram'])
        lw, jr, delay = words(load, 3)
        base, dest = (lw >> 21) & 31, (lw >> 16) & 31
        if lw >> 26 != 35 or dest == 0 or jr != (dest << 21) | 8 or delay != 0:
            raise ValueError('Reviewed local jump requires LW, matching JR and NOP delay slot')
        if [f'0x{w:08X}' for w in (lw, jr, delay)] != spec['instructions']:
            raise ValueError('Reviewed local jump instructions changed')
        if not start <= load < start + size - 8 or any(not start <= t < start + size or t % 4 for t in targets):
            raise ValueError('Local jump or target outside owning function')
        if list(words(table, len(targets))) != targets:
            raise ValueError('Reviewed local jump table changed')
        offset = (lw & 0xFFFF) - (0x10000 if lw & 0x8000 else 0)
        lines = [f'/* Reviewed local table 0x{table:08X}; original LW/JR/NOP. */',
                 f'ctx->r{dest} = MEM_W(ctx->r{base}, {offset});',
                 f'switch ((uint32_t)ctx->r{dest}) {{']
        lines.extend(f'case 0x{target:08X}u: goto L_{target:08X};' for target in targets)
        lines.append(f'default: switch_error(__func__, 0x{load + 4:08X}, 0x{table:08X}); return;')
        lines.append('}')
        hooks.append(dict(func=name, load=load, table=table, targets=targets, text='\n'.join(lines)))
    return hooks


def verify_generated_local_jumps(directory, hooks):
    generated = '\n'.join(p.read_text() for p in directory.glob('*.c'))
    for hook in hooks:
        pattern = r'RECOMP_FUNC void ' + re.escape(hook['func']) + r'\([^\n]+\n(.*?)(?=RECOMP_FUNC void |\Z)'
        matches = re.findall(pattern, generated, re.S)
        if len(matches) != 1:
            raise ValueError('Local jump owning function missing or duplicated')
        body = matches[0]
        if body.count(hook['text']) != 1:
            raise ValueError('Local jump hook missing or duplicated')
        for target in hook['targets']:
            if body.count(f'\nL_{target:08X}:') != 1:
                raise ValueError(f'Local target label missing/duplicated: {target:08X}')
        if body.index(hook['text']) > body.index(f'// 0x{hook["load"]:08X}: lw'):
            raise ValueError('Local jump hook must precede original load')
