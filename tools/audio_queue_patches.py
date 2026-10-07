"""ROM-pinned audio queue feedback fixes shared by Original and Native modes."""
import hashlib
import struct


# Resident identity is essential: these hooks are not overlay/FPS equations.
_FUNCTIONS = {
    0x8004E560: ("dora3_audio_buffer_sizes", 276,
                 "0bcb8bc3f3f062b16fd93be63ef2d1e273f6ba426b1a691b5a1c79838c230869"),
    0x8004E674: ("dora3_audio_frame_samples", 140,
                 "52413c646e2cee411cf4a93f90d883cda8d1c628cd3eb9ec6cfe6a09555bd8b3"),
}


def audio_queue_hooks(sections, compiled, rom):
    """Keep the synth's 16-frame alignment and allocation, permit queue recovery."""
    section = sections[0]
    if (section["name"], section["rom"], section["vram"]) != (".startup", 0x1000, 0x80025C00):
        raise ValueError("Unreviewed audio resident section")
    for address, (name, size, digest) in _FUNCTIONS.items():
        entry = compiled.get((0, address))
        if entry is None:
            continue
        if tuple(entry[:2]) != (name, size):
            raise ValueError("Unreviewed audio function: " + name)
        offset = section["rom"] + address - section["vram"]
        if hashlib.sha256(rom[offset:offset + size]).hexdigest() != digest:
            raise ValueError("Unreviewed audio ROM body: " + name)
        if address == 0x8004E560:
            site, word = 0x8004E5F0, 0xAC230144
            # Source minimum = align_down(ceil(rate * retraces / VI), 16).
            # At 22050/60 that is 368 > 367.5, so even a full queue grows.
            # One lower block (352) permits feedback to straddle the true rate.
            # Only the lower bound changes; target, reserve and allocation do not.
            hook = ("/* Allow one aligned block of audio queue recovery. */ "
                    "if ((uint32_t)ctx->r3 >= 32u) ctx->r3 = SUB32(ctx->r3, 16);")
        else:
            site, word = 0x8004E69C, 0x2483FFF0
            # Source computes align_down(target - (remaining - 16), 16), then
            # applies an UNSIGNED minimum. A long SDL queue can wrap negative
            # headroom into a huge synth request. Saturate before subtraction;
            # the existing minimum branch then returns the short recovery block.
            hook = ("/* Saturate queue feedback before unsigned headroom math. */ "
                    "if ((uint32_t)ctx->r4 > (uint32_t)ctx->r2 + 16u) "
                    "ctx->r4 = (int32_t)((uint32_t)ctx->r2 + 16u);")
        offset = section["rom"] + site - section["vram"]
        if struct.unpack_from(">I", rom, offset)[0] != word:
            raise ValueError("Unreviewed audio queue hook")
        yield name, site, hook
