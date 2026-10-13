# Manual FPS audit after test146 — work remains

Static review only. No game, emulator, tests, CPU checks or renderer executed.
User requested no diagnostic logging. Do not describe this as completed coverage
or a proven fix for the reported final boss attack freeze.

## Changes in test147

- Common registered enemy/scene-object command adapter now uses the animation's
  own Source cadence, previously restricted to boss kinds399–477 and Powan387–390.
  Ordinary enemy exact-frame attack consumers (Bironn/Brobo/Arizinogu among others)
  share the same interruption/rebase path and otherwise can read shifted fractional
  frames forever. Cue deduplication is extended to these registered owners too;
  exact equality and deliberately skipped Source frames are preserved.
- Boss4 fireball442 growth and crash445 shrink: ROM stores F134/F700 target actor1C,
  F138/F708 target actor20. The adapter had these reversed, capturing the already
  changed Y value as the start of one axis and producing a stepped size change.
- Brobo linked-part collision rewind used pool+absoluteSlot*CC, but pool already
  skips30 reserved slots. Use804B1CD0+absoluteSlot*CC, as the ROM and all neighboring
  linked-slot lookups do. The old lookup could miss the intended part entirely.

## Inspection evidence and limits

The static table scan inspected619 Store/SceneStore rows for their instruction
operand and byte offset. Four mismatched offsets above were found. Four SW(a2)
rows are valid: preceding MFC1(a2,f0) preserves the same bits before the delay-slot
store; no register fix was needed there.

Inventory across4081 compiled overlay functions:129 model-time getter calls and
362 model-end getter calls. This is an inventory, NOT evidence that every path
and every fractional transition has been validated. Existing exact-frame rule
coverage and the common adapter were inspected; custom equations still need a
complete semantic pass beyond table operand matching.

Boss7 bank11D570, kind472 and linked473–477: reviewed all31 functions, entry,
homing/dive/rise, mask loss, jump/landing, death, shield and reward. No proven
cause of the reported attack freeze was established. Its root has no exact-frame
attack cue; the unmasked jump waits on an end flag. Therefore common cue changes
must NOT be presented as a demonstrated fix for this boss. Remaining leads are
fractional command interruption and the actual active phase/owner. Do not force
phase changes, add timeout teleports or claim the freeze fixed from compilation.

The requested exhaustive enemy/NPC audit remains open. Earlier documents claiming
complete mathematical coverage are historical implementation descriptions, not
fresh runtime confirmation. City/slot09 NPCs also have a separate scheduling path;
the common-adapter change does not claim to audit all of that path.

Build147: Windows Release, CrashLog/GraphicsProfile/Diagnostics OFF; gameplay
verification pending. No commit/push authorized for this new block.
