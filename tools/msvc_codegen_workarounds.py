"""ROM-pinned compiler workarounds; never change guest instructions or dependencies."""
import hashlib
import re


def apply_msvc_codegen_workarounds(directory, sections, compiled, rom):
    # MSVC 19.44 /O2 in test053 turns base + 3*index into
    # 3*(index + 0xC011FE50), producing 0x4035FAF3 for index1.
    # Preserve this tiny, infrequent sequence matcher instruction by instruction.
    # Keep the rest of the generated translation unit at its configured settings.
    for index, section in enumerate(sections):
        if section["name"] != ".remaining_slot2_08":
            continue
        entry = compiled.get((index, 0x8023D730))
        if entry is None:
            return
        name = "dora3_s47_remaining_8023D730"
        if ((section["rom"], section["vram"]) != (0x15A960, 0x8023D6D0)
                or tuple(entry[:2]) != (name, 200)):
            raise ValueError("Unreviewed sound-color sequence function")
        offset = section["rom"] + 0x8023D730 - section["vram"]
        if hashlib.sha256(rom[offset:offset + 200]).hexdigest() != (
                "4c22dc09208a87d613f1469cb6bb8e72614ba17a22ccba01958c85b0c542044d"):
            raise ValueError("Unreviewed sound-color sequence ROM body")
        pattern = re.compile(r"^RECOMP_FUNC void " + name + r"\([^\n]+\) \{.*?^;}", re.M | re.S)
        matches = [(path, match) for path in sorted(directory.glob("*.c"))
                   for match in pattern.finditer(path.read_text())]
        if len(matches) != 1:
            raise ValueError("Expected one generated sound-color sequence body")
        path, match = matches[0]
        source = path.read_text()
        before = ('/* MSVC /O2 miscompiles the ROM table address in this function. */\n'
                  '#if defined(_MSC_VER) && !defined(__clang__)\n'
                  '#pragma optimize("", off)\n#endif\n')
        after = '\n#if defined(_MSC_VER) && !defined(__clang__)\n#pragma optimize("", on)\n#endif'
        path.write_text(source[:match.start()] + before + match.group() + after + source[match.end():])
        return
