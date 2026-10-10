# test-102: Common world dialogue/fade readback optimization

Native world presentation keeps deferred framebuffer readback across gameplay,
UI-owned fade/dialogue and scripted camera modes. The request follows the acquired
world draw identity; workload/CPU-copy/resource/Original handoff guards remain.
No NPC or location-specific conditions. See docs/fade-performance-after101.md.

User confirmed101 opening-chair and minigame grenade fixes; those are preserved.
User reports the fade fix appears to work in102 and authorized commit/push.
No new numerical FPS measurement was supplied.
Incremental compile/link succeeded; no tests, CPU checks, game, emulator, renderer
or GPU execution. Existing EXEs/ROM/settings/saves/shared DLLs preserved.
No additional build for this acceptance/documentation/Git step.
