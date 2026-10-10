# test-103: Native water-damage animation boundary correction

Shared player animation rate planning now matches Original slow damage/death
states37/40/43/46/51–56, with the state40 kick-period guard. Frame/clip boundary
calculation preserves precise current animation time across rate changes.
Held player models no longer request boundaries while their update is skipped.
Includes the requested related slow/reverse animation audit; see
docs/water-damage-performance-after102.md for ROM evidence and limitations.

Incremental compile/link succeeded. The user confirmed the water-damage
FPS fix works in test103 and authorized commit/push to main. No tests, CPU checks, game, emulator, renderer or
GPU execution. Generation101 reused unchanged; accepted102 fixes preserved.
Old EXEs, ROM, settings, saves and DLLs preserved. No additional build for
this acceptance/documentation/Git step. The suspected cutscene pose glitch was
withdrawn after the user reviewed a recording; no corresponding code change.
