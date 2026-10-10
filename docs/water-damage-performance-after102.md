# Native damage animation boundary cost after test102

User report: first level fairy forest, jump into damaging water, falling with
damage animation drops to0–1FPS every time for all characters, then recovers.
Normal play is fine; Original FPS does not exhibit this. Default setup remains
Manual60 + Modern Camera + Widescreen. Baseline main934a8e6, clean tree, EXE102.

## Static ROM evidence

ROM SHA256410f99bdd58795639da272c9be9d1ab8c35b61f24f7c1fb92f7e83e1069a2602.
Original scene1 table801FA6B0 provides the shared player state callbacks.
C8B28 clears physics+CC atC8EBC, invokes command then CURRENT state motion,
defaults zero rate to3 atC90A8 and advances the primary model with physics+CC
atC90C8; mirrored model always receives3 atC90DC. All characters use this loop.

Nondefault writes from the player closure, inspected in Original instructions:
-37: C6084 motion writes1 atC6108.
-40: C63F8 freezes/hides through count15, kicks70 at16; after16 writes1 atC64E0.
  Existing Native C6418 guard preserves that once-per-Source counter and kick.
-43: C67C8 motion writes1 atC684C.
-46: C6B08 motion writes1 atC6B2C (already planned correctly).
-51: C729C writes1 atC72BC.
-52: C73F0 writes1 atC7410;53–56 wrappers call that same body.
Other inspected player bodies do not override+CC.40/43 use clip27 and remain
in that action until contact/ground logic selects41/44 and clip28; those fall
bodies use the default3. This is not a forest/NPC/character-specific rule.

## Defect and correction

Both Native acquisition and the per-slice model-rate plan special-cased only46;
all other player states predicted3. Hazard and heavy-damage clips actually
advance at1. Repeated clip-end predictions therefore subdivide the remaining
distance instead of reaching its boundary. Every subdivision updates the world.
Additionally, clip_boundary_limit discarded its precise model time whenever
the predicted rate differed from the observed rate. Rounded float RAM near a
clip edge can then predict that boundary already reached, repeatedly requesting
one rational time unit (1/60ns). The real animation still has distance left.
This explains a severe Native-only computational stall without rendering load
or a character-specific animation defect. No runtime profiling was performed,
so exact subdivision counts and the user's resulting FPS remain unmeasured.

A shared primary_player_model_rate now supplies all reviewed nondefault rates
to acquisition and boundary planning. State40 distinguishes the already consumed
kick period from the next counter increment using the same lifetime/run/tick
guard as C6418. The mirror retains3. Player models are excluded while D494
holds their actual update, matching the existing C8B28 call gate.

Both clip-end and frame-cue limiters retain a matching precise current position
when the NEXT planned rate changes. Expected RAM time/flags, descriptors,
channels, duration and denominator guards remain. The new rate still computes
the next boundary. No epsilon-sized fake advancement, time dropping, catch-up
cap or lowered simulation rate is used. Original motion, damage, invulnerability,
gravity, contacts, impulses, sounds and state commands remain unchanged.

## Validation and status

Static Original instruction/table and Native source inspection only, followed
by incremental compile/package. No tests, CPU checks, game, emulator, renderer
or GPU execution. Generated code unchanged; generation101 reused. User playcheck
pending; do not claim observed0–1FPS issue resolved based on compilation alone.
No commit/push until user verification and explicit authorization. Preserve102
fade and101 chair/grenade fixes, accepted camera095 and widescreen100.

## Additional slow/reverse animation audit requested by user

All production call sites of clip_boundary_limit/frame_boundary_limit were
searched: only native_gameplay_preview schedules these animation subdivisions.
The Original player closure scan covers every+CC store and state-table wrappers;
slow states37/40/43/46/51–56 are included, default falling41/44 remains3.

Common objects/enemies use E8E28..E8E54's actual physics+CC (zero means3), also
read by the plan; they do not substitute player-style constant3 for nonzero
rates. Inspected14 direct Original callers of setterE8AAC. Examples: Powan4
depart/present writes-3 at80222B14/80223424, restores default0 at80222BC4/802234C0;
Boss5 shield reverses with-3 at802207DC and the body/linked part use2 at
80220DA4/80220F28. The shared precision correction also covers such rate changes.
Projectile pool D116C passes fixed3, matching its plan. Paused models and disabled
channels are already excluded from boundary calculation; C9-paused actors are
excluded from both the common/projectile update and plan. Player D494 hold was
the additional missing exclusion, corrected here.

Minigames sample their Source-derived equations; cutscenes evaluate the current
Source motion interval. Neither calls these clip/cue limiters, so this exact
subdivision fault has no second scheduler implementation there. This is static
coverage of the shared mechanisms and identified commands, not gameplay proof
for every animation/scene or a guarantee against other performance defects.

## User acceptance on 2026-10-10

The user confirmed the water-damage fix works in test103 and explicitly
authorized commit/push to existing main. This supersedes the pending status
above. No new measured FPS result or exhaustive scene coverage is claimed.
The subsequent suspected Nobita cutscene pose glitch was withdrawn by the user
after reviewing a recording frame by frame. That investigation produced no
repository code changes. No additional build or tests/CPU/game/GPU execution.
