# test-107: localization corrections after test106

File selection/copy/erase, mode/difficulty/confirmation prompts now resolve from
their original temporary text buffers. Added the separately drawn save-card
GAMES / ИГРЫ label and shared Shoot / Стрелять action for minigame rules.
Choice labels retain the bell cursor anchors. Both English and Russian updated.

Reflowed all230 contextual item/weapon description lines in each language into
12 original cells (24 translated characters), retaining the original line count
and native glyph size. Shortened phrasing where needed. Static atlas measurement
gives maximum148px ink extent from x112; right edge x260, leaving frame padding.

Static catalog diagnostics:0 errors;2152 translated records per language plus12
intentional alien-symbol records. Incremental Windows compile/link succeeded;
existing generated code reused unchanged. No tests, CPU checks, game, emulator,
renderer or GPU execution. User gameplay verification is pending; compilation
does not confirm in-game correctness. No commit/push before user authorization.
Keep assets/localization beside the EXE. Changed packs are backed up by packaging.
Previous EXEs, ROM, settings, saves and shared DLLs are retained.

## User acceptance — 2026-10-10

The owner confirmed test107 corrections work and authorized commit/push of the
localization through107. The compilation receipt remains unchanged as built;
no new build or execution was performed for this acceptance step.
