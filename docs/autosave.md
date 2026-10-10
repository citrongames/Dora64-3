# World-map autosave

Implementation after accepted test104. On 2026-10-10 the user confirmed that
test105 autosave works and explicitly authorized commit/push to existing main.
This is user gameplay feedback, not exhaustive scenario coverage.

Game > Autosave precedes Modern camera, matching the read-only Dora64 donor.
Default and Modern profile enable it; Original profile disables it. The setting
persists as game.autosave. FPS selection is independent. A green two-second
Autosave notice appears at the upper left using the donor's style and input flags.

## Locked-ROM evidence and lifecycle

ROM SHA256:410f99bdd58795639da272c9be9d1ab8c35b61f24f7c1fb92f7e83e1069a2602.
Scene30 uses scene34 overlay ROM0x635C0. Map entry801BB704 installs update801BB52C
and draw801BB648 before its epilogue801BB9D0. Process install800280DC/80028040
sets state3; dispatcher800281B8 promotes it to state2. Both valid states are
accepted, including a deferred draw phase during the first map update. The generator arms one visit there.
Normal map handler801BAF64 first handles forced progress flags, messages and
story departures. Instruction801BB108 begins user input after those branches;
the second hook consumes the pending visit before immediate input can leave.
No host timer or Native visual frame can repeat the write.

The guards require the same overlay load generation, original map process pair,
unchanged active slot801B3698 in0..2, occupied slot byte80087A28+slot*0x200,
normal map state801C636C, no departure801C63A0 or movement801C638C, and no debug
preset801B3763. Every map initialization replaces the pending visit. Internal
reset clears visit and notification state. Disabling consumes the visit without
saving; re-enabling on an already ready map waits for the next visit.

Manual confirmation801BAB90 calls resident80043800 to pack live progress, then
80043F1C(slot) to calculate checksum and perform the original staged EEPROM
write. Autosave calls this same pair with a private recomp_context, including
rebinding its floating-point storage pointer. It does not reconstruct save fields,
change slots, synthesize buttons, open the confirmation, or alter manual saving.
Slot RAM checksum can lag live progress, so it is not used as an entry gate.

EEPROM return0 publishes the green notice; a nonzero return publishes Autosave
failed instead, without automatic repeated writes. As with ordinary manual save,
the runtime buffers EEPROM synchronously and flushes to disk asynchronously;
the existing backup replacement and disk-error reporting remain responsible for
durability. The notice is not an fsync guarantee. Save format/location unchanged.

## Verification scope

ROM/source analysis, guarded generation and incremental Windows compilation only.
No agent tests, CPU checks, game, emulator, renderer or GPU execution. The user
reported autosave works; specific slot/reset/FPS scenario coverage was not stated.
Compilation alone does not establish correct gameplay behavior. Donor and
sources/ untouched. No new build was made for the acceptance/Git step.
