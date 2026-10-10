# World fade/dialogue performance after test101

User confirmed opening-chair and minigame grenade motion corrected in101.
Remaining report: Manual60 + Modern Camera + Widescreen, first city Shizuka
interaction, camera approach and white fade; displayed FPS reaches35 or lower.

## Static evidence and scope

Original ROM SHA256410f99bdd58795639da272c9be9d1ab8c35b61f24f7c1fb92f7e83e1069a2602.
City overlay132F80 contact8022DA68 calls world pause801BBD5C, resumes its
NPC actor, sets white RGB/step16 via80028878, then800288EC starts the fade.
State2 at8022DB34 requests script351 after16 Source ticks. The common world
pause sets player state66 via801C8478; resident289D8 draws the common fade.
These original commands and their timing remain unchanged.

frame_lights.cpp formerly allowed deferred framebuffer readback only for the
Native gameplay simulation owner. UI-owned world fades/dialogues and scripted
world frames were rejected even though the acquired draw pair remains
801BB730/801BB848 in module70A50. This restores the eager native-resolution
render/wait/RAM copy in addition to display rendering on every such frame.

The request now follows the acquired world presentation identity, independent
of its simulation policy. Snapshot stays on the producing guest thread and
travels with its exact bank. Native build, generation-bearing acquired identity,
320x240 dimensions and no CPU framebuffer copy remain required. Workload
validation still rejects offscreen images, framebuffer operations, tile copies,
reinterpretation and unreviewed depth targets. Actual CPU-copy/resource/Original
handoff barriers still materialize before access. Original FPS, minigame/menu
overlays, scheduler ownership and original fade/camera equations are unchanged.
Historical profiling counter GateNotGameplay now means non-world presentation.

This fixes an identified common performance discontinuity, not a Shizuka-specific
branch. Static analysis alone does NOT prove it accounts for all of the observed
35FPS dip: player state66 itself remains gameplay-eligible, and no runtime profile
establishes exactly when this user's transition changes policy. Native session
draining/resource loads can still stall. Do not report the measured issue resolved
until the user checks102. No runtime instrumentation or behavior shortcuts added.

## Validation

Static source/ROM analysis and incremental compilation/package only. No tests,
CPU checks, game, emulator, renderer or GPU execution. Existing generation101
reused unchanged. No commit/push until explicit user authorization after playcheck.
Preserve101 motion fixes and accepted camera095/widescreen100.

## Acceptance on2026-10-10

User reports test102 appears to work and explicitly authorizes commit/push to
existing main. Chair and minigame grenade motion were confirmed after101. This
supersedes pending verification/Git restrictions above for changes through102.
No new measured FPS result or exhaustive scene coverage claimed. No additional
build, tests, CPU checks, game, emulator, renderer or GPU execution.
