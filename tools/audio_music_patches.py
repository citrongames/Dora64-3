"""Resident music-request handoff, shared by all scenes and FPS modes."""
import hashlib
import struct

_FUNCTIONS = {
    0x80044164: ("dora3_audio_init", 332,
                 "182ff373d05453e63cccc22d18bc4205190bc846c1668c932b35a6a6b690cf26",
                 [(0x80044164, 0x27BDFF98, "dora3_music_reset")]),
    0x800442B0: ("dora3_s0_800442B0", 128,
                 "4f52edb3094a75ad60fc3daad56f0f12582344b7cd63e839d3abdba400ec524a",
                 [(0x800442B0, 0x3C03801B, "dora3_music_request"),
                  (0x800442F8, 0x14400008, "dora3_music_busy")]),
    0x80044330: ("dora3_s0_80044330", 44,
                 "e66326ca5705f4fcaf0e936f981aa7fe66a754811fdd7209d456beef4ef7a69e",
                 [(0x80044330, 0x00802821, "dora3_music_stop")]),
    0x800446C0: ("dora3_sound_handles_collect", 212,
                 "56c6215fd42d93f5ba893dcb2b17bf3493e6e9ed43af7db9254dca3b5306d79d",
                 [(0x80044778, 0x8FBF0020, "dora3_music_after_collect")]),
}


def audio_music_hooks(sections, compiled, rom):
    section = sections[0]
    if (section["name"], section["rom"], section["vram"]) != (".startup", 0x1000, 0x80025C00):
        raise ValueError("Unreviewed music resident section")
    for address, (name, size, digest, hooks) in _FUNCTIONS.items():
        entry = compiled.get((0, address))
        if entry is None:
            raise ValueError("Missing shared music function: " + name)
        offset = section["rom"] + address - section["vram"]
        if tuple(entry[:2]) != (name, size) or hashlib.sha256(rom[offset:offset + size]).hexdigest() != digest:
            raise ValueError("Unreviewed music ROM body: " + name)
        for site, word, hook in hooks:
            offset = section["rom"] + site - section["vram"]
            if struct.unpack_from(">I", rom, offset)[0] != word:
                raise ValueError("Unreviewed music handoff hook")
            yield name, site, hook + "(rdram, ctx);"
