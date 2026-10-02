"""Persistent, ROM-qualified names for generated game C without changing its ABI.

Semantic identifiers expand to the original symbol/register tokens. Checkers
read the reversible canonical view so old references remain valid. A generation
fails if a ROM identity, register scope or the exact round trip differs.
"""
import hashlib
import json
import re
import struct
from pathlib import Path

_IDENTIFIER = re.compile(r'[A-Za-z_]\w*\Z')
_BODY = re.compile(r'RECOMP_FUNC void (\w+)\([^\n]+\n.*?(?=RECOMP_FUNC void |\Z)', re.S)
_LEX = re.compile(r'//[^\n]*|/\*.*?\*/|"(?:\\.|[^"\\])*"|\'(?:\\.|[^\'\\])*\'|ctx->(?:r|f)\d+\b|\b[A-Za-z_]\w*\b', re.S)
_BLOCK = re.compile(r'/\* DORA3_DECOMP_(NOTE|LOCALS|UNDEFS)_BEGIN \*/\n.*?/\* DORA3_DECOMP_\1_END \*/\n(?:#line \d+\n)?', re.S)
_INCLUDE = '#include "decomp_names.h"\n'


def sha_bytes(data):
    return hashlib.sha256(data).hexdigest()


def rewrite_tokens(source, names):
    """Do not rename comments, diagnostic strings or substrings of identifiers."""
    return _LEX.sub(lambda m: names.get(m[0], m[0]), source)


def canonical_source(source, function_aliases):
    locals_map = dict(re.findall(r'^#define (\w+) (ctx->(?:r|f)\d+)\s*$', source, re.M))
    source = _BLOCK.sub('', source).replace(_INCLUDE+'#line 1\n', '').replace(_INCLUDE, '')
    return rewrite_tokens(source, {**function_aliases, **locals_map})


def read_decompiled_source(path):
    """Read an old or annotated generation by its stable original symbol names."""
    source = path.read_text()
    if _INCLUDE not in source:
        return source
    report = json.loads((path.parent/'decomp-annotations.json').read_text())
    canonical = canonical_source(source, report['function_aliases'])
    if sha_bytes(canonical.encode()) != report['canonical_sha256'][path.name]:
        raise ValueError('Decomp naming round trip changed: '+str(path))
    return canonical


def annotate_generated_sources(directory, sections, compiled, rom, catalog_path):
    catalog = json.loads(catalog_path.read_text())
    if catalog['schema'] != 1 or catalog['rom_sha256'] != sha_bytes(rom):
        raise ValueError('Unreviewed decomp symbol catalog/ROM')
    by_section = {s['name']: (i, s) for i, s in enumerate(sections)}
    existing = {v[0] for v in compiled.values()}
    entries = {}
    aliases = {}
    for entry in catalog['functions']:
        index, section = by_section[entry['section']]
        addr, size = int(entry['vram'], 16), entry['size']
        if compiled[index, addr][:2] != [entry['legacy_name'], size]:
            # prepare_layout currently preserves list values; accept tuple callers.
            if tuple(compiled[index, addr][:2]) != (entry['legacy_name'], size):
                raise ValueError('Function identity changed: '+entry['legacy_name'])
        offset = section['rom']+addr-section['vram']
        if sha_bytes(rom[offset:offset+size]) != entry['rom_sha256']:
            raise ValueError('Function ROM changed: '+entry['legacy_name'])
        legacy, name = entry['legacy_name'], entry['name']
        if not _IDENTIFIER.fullmatch(name) or legacy in entries or name in aliases:
            raise ValueError('Duplicate/invalid decomp name')
        if name != legacy and name in existing:
            raise ValueError('Decomp alias collides with original symbol')
        if any('*/' in entry[k] or '\n' in entry[k] for k in ('description','evidence')):
            raise ValueError('Invalid decomp comment')
        for role in entry.get('locals', []):
            start, end = (int(role[k], 16) for k in ('start','end'))
            if not addr <= start < end <= addr+size or start%4 or end%4:
                raise ValueError('Register role outside function')
            if not re.fullmatch(r'ctx->(?:r(?:[1-9]|[12]\d|3[01])|f(?:\d|[12]\d|3[01]))', role['expression']):
                raise ValueError('Register role must preserve an exact GPR/FPR storage lvalue')
            if not _IDENTIFIER.fullmatch(role['name']) or '*/' in role['description'] or '\n' in role['description']:
                raise ValueError('Invalid register alias/comment')
            for site, word in role['instructions'].items():
                actual = struct.unpack_from('>I', rom, section['rom']+int(site,16)-section['vram'])[0]
                if actual != int(word,16):
                    raise ValueError('Register role instruction changed')
        entries[legacy] = entry
        if name != legacy:
            aliases[name] = legacy
    forward = {old: new for new, old in aliases.items()}
    canonical_hashes = {}
    found = set()
    roles_used = 0
    for path in sorted(directory.glob('*.c')):
        original = path.read_text()
        canonical_hashes[path.name] = sha_bytes(original.encode())
        def annotate(match):
            nonlocal roles_used
            legacy = match[1]
            body = match[0]
            if legacy not in entries:
                return body
            entry = entries[legacy]
            found.add(legacy)
            macros = {}
            for role in entry.get('locals', []):
                alias, expr = role['name'], role['expression']
                if alias in macros and macros[alias] != expr:
                    raise ValueError('A register alias has two different lvalues')
                if re.search(r'\b'+re.escape(alias)+r'\b', body):
                    raise ValueError('Register alias collides with generated local')
                macros[alias] = expr
            pc = None
            replaced = {i:0 for i in range(len(entry.get('locals', [])))}
            lines = []
            for line in body.splitlines(keepends=True):
                address = re.search(r'// 0x([0-9A-F]{8}):', line)
                if address:
                    pc = int(address[1],16)
                if pc is not None:
                    substitutions = {}
                    for i, role in enumerate(entry.get('locals', [])):
                        if int(role['start'],16) <= pc < int(role['end'],16):
                            expr = role['expression']
                            if expr in substitutions:
                                raise ValueError('Overlapping roles for a register')
                            substitutions[expr] = role['name']
                            replaced[i] += sum(m[0]==expr for m in _LEX.finditer(line))
                    line = rewrite_tokens(line, substitutions)
                lines.append(line)
            if any(count==0 for count in replaced.values()):
                raise ValueError('Register role matched no generated accesses: '+legacy)
            roles_used += len(replaced)
            body = ''.join(lines)
            note = ('/* DORA3_DECOMP_NOTE_BEGIN */\n'
                    f'    /* {entry["description"]}\n'
                    f'       ROM: {entry["section"]} {entry["vram"]}; ABI: {legacy}.\n'
                    f'       Основание: {entry["evidence"]} */\n'
                    '/* DORA3_DECOMP_NOTE_END */\n')
            if macros:
                note += '/* DORA3_DECOMP_LOCALS_BEGIN */\n'
                for alias, expr in macros.items():
                    note += f'#define {alias} {expr}\n'
                for role in entry.get('locals', []):
                    note += f'    /* {role["name"]}: {role["description"]}; {role["start"]}..{role["end"]} (end excluded). */\n'
                note += '/* DORA3_DECOMP_LOCALS_END */\n'
            first_line, rest = body.split('\n',1)
            # Preserve __LINE__ and diagnostic source positions despite added
            # descriptions/macros. No filename is changed in the generated C.
            original_line = original.count('\n',0,match.start())+1
            body = first_line+'\n'+note+f'#line {original_line+1}\n'+rest
            if macros:
                body += '/* DORA3_DECOMP_UNDEFS_BEGIN */\n'
                body += ''.join(f'#undef {alias}\n' for alias in macros)
                body += '/* DORA3_DECOMP_UNDEFS_END */\n'
                body += f'#line {original.count(chr(10),0,match.end())+1}\n'
            return body
        annotated = _INCLUDE+'#line 1\n'+rewrite_tokens(_BODY.sub(annotate, original), forward)
        if canonical_source(annotated, aliases) != original:
            raise ValueError('Naming altered original C: '+path.name)
        path.write_text(annotated)
    if found != set(entries):
        raise ValueError('Catalog functions absent from generated C: '+str(set(entries)-found))
    header = '#pragma once\n/* Semantic names expand to unchanged original ABI symbols. */\n'
    header += ''.join(f'#define {name} {legacy}\n' for name,legacy in sorted(aliases.items()))
    (directory/'decomp_names.h').write_text(header)
    report = dict(schema=1, function_aliases=aliases, canonical_sha256=canonical_hashes,
                  catalog_sha256=sha_bytes(catalog_path.read_bytes()),
                  annotated_functions=len(entries), renamed_functions=len(aliases),
                  register_role_scopes=roles_used)
    (directory/'decomp-annotations.json').write_text(json.dumps(report,indent=2)+'\n')
    print(f'Decomp names: {len(entries)} descriptions; {len(aliases)} semantic function names; {roles_used} register scopes; exact canonical C preserved')
