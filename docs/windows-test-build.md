# test-105: World-map autosave

Game > Autosave matches Dora64 placement and notice style. Default and Modern
profile enable it; Original profile disables it. One save per ready map visit,
using the active slot and original packing/EEPROM functions. Manual save remains.
Implementation and ROM evidence: docs/autosave.md.

Incremental Windows compile/link succeeded. No tests, CPU checks, game, emulator,
renderer or GPU execution. The user confirmed autosave works on 2026-10-10 and
authorized commit/push to main. This feedback is not exhaustive scenario coverage.
No additional build or execution was performed for the acceptance/Git step.
Previous EXEs, ROM, settings, saves and shared DLLs preserved.
