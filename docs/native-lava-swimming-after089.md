## 091 follow-up:090 failed user playcheck

User reports both lava and underwater camera still wrong. The090 hypotheses
were not sufficient; the analysis below is retained as history.

Static control-flow finding: BB080 resets camera_allowed=false. With control+28
==1 and camera mode3, DBE5C branches directly to DBE84, bypassing the BE78
ownership hook. The Source camera executes, but no Native camera follows until
the next Source update. Move the hook to BE84, the shared join after all mode
commands. Native still skips the Source command prelude via DBD7C->BE78 and
then reaches BE84. Original mode executes untouched because the hook requires
a live Native owner. Remove090 incremental-pitch/ApproachTimeScope changes.

Static material finding: sampled texels/TLUT allocated by allocate() live in
the extended display-list bank from recomp::alloc, but FD rewrites stripped
bit31 with &0x1fffffff. RT64 RSP::fromSegmented only accepts a raw extended
address with bit31; otherwise it applies segment lookup and24-bit offset.
RDP::maskAddress then applies the N64 address mask without the tag as well.
Therefore the renderer did not necessarily read the generated material bytes.
Use the existing distance::display_address convention for both pixel and TLUT
FD commands, as already used for cloned DE lists. Ordinary ROM endpoints keep
the N64 representation; extended samples retain their tag through both stages.
Both CI4 palette/texels and IA8 texels are covered; exact UV scrolling unchanged.
090 phase-independent palette grouping remains, but no visual success claimed.

Static source/ROM/dependency inspection only. No tests, game, renderer or GPU.

---

# Lava fade and swimming camera after089

2026-10-09 user paused FPS work (city can reach360; mountain remains heavy),
then authorized committing/pushing the previous stage.08d2760 is on origin/main.
New reports: lava fade appears unstable on Manual; vertical swimming camera
looks Source-rate in world1 lake/tower level (user thinks level3).

## Material analysis and change

Original ROM SHA256410f99bdd58795639da272c9be9d1ab8c35b61f24f7c1fb92f7e83e1069a2602.
The actual world material audit links02AA..2AF (32x32 red CI4 fades) and
02B0..2B5 (64x64 CI4 patterned frames). Static decoding of the first family
finds38/38/38/39/40 distinct adjacent texel-index pairs, all with16-entry TLUTs.
These are not pure equal-index palette changes or exact UV translations.

The old sampler rounded each current interpolated pixel toRGB555 and rebuilt
weighted median-cut clusters and nearest-palette membership at every fractional
phase. Rounding/cluster/split changes can move many unrelated pixels between
palette entries in the middle of an otherwise smooth color fade.

The new sampler partitions six-component endpoint color trajectories. Both
cluster membership and the packedCI4 pixel map depend only on the two source
images, never phase. Only their palette colors change with phase. Original
sequence indices, repeated holds, durations, UV translation handling, IA8 and
transparency rejection remain unchanged. The existing source-image endpoint
paths are unchanged. CI4 still has at most16 colors and the original TMEM size;
intermediate color quantization remains an approximation, now temporally stable
within each transition. Visual fidelity and transition seams need playcheck.
No claim that a static audit alone reproduces the user's exact rendered artifact.

## Swimming camera analysis and change

DBE3C..DBE70 selects mode3 when player control+28==1. D8FF0 reads the current
player physical+C pitch atD9318; D9474 applies the0.03 follow coefficient, and
D94C0..D94CC updates camera pitch. This is the only generic exponential-approach
hook in that root. Yaw has a separate capped approach/recenter path. Native
player flight/swim updates already publish fractional pitch from C1ADC.

OrdinaryCameraTimeline restored tick-start pitch and evaluated cumulative time
h against the already-advanced player target. A linearly moving target from
zero produces approximately h*h response near the tick start, restarting its
response curve every authored tick. This is a concrete temporal coupling defect;
its contribution to the user's visible judder is pending playcheck.

Mode3 now preserves current camera pitch and uses the actual movement slice's
time for that follow gain. Source0.03 becomes1-pow(1-0.03,dt), as before, but
dt is the current interval rather than cumulative age. Yaw, other camera fields,
shake/control counters and event boundaries retain their existing anchored time.
Collision probes consume the same updated pitch inside the original callback;
there is no after-the-fact rendered-pose blend. Single full-tick/Original behavior
is unchanged; moving-target Native results intentionally no longer restart each
Source tick. Fixed targets retain the exponential composition law, modulo float
rounding. ApproachTimeScope is thread-local RAII; inactive Original calls bypass
it and all other camera roots receive the same dt they previously used.

## Validation boundary

Static source/ROM analysis only. No tests, CPU checks, game, renderer or GPU run.
Compile is permitted and does not prove either visual fix. Old CPU PASS reports
are historical and were not rerun. New changes remain uncommitted for playcheck.

## Build090

MSVC RelWithDebInfo compile/link exit0 in37.562s. Clean diagnostics/profileOFF.
Packaged D:/Games/DoraRecomp3/testbuild/Dora64-3-test-090.exe.
282 prior non-log package files preserved; shared DLL hashes match.
EXE SHA256: 62593cb44063c437c77e9d34c2d78ec6f3e51e9dd63b1202e03e46c42118fc11
No runtime/CPU tests executed. User visual check pending.

## Build091

Clean MSVC RelWithDebInfo compilation/link succeeded in23.266s. Diagnostics and
graphics profilerOFF. New generator output startup-clean-test091-v1 copied
by exact file differences into the existing Windows CPU cache slot to retain
incremental object reuse; only funcs_32.c changed. Prior generated files archived.
No hand-edited generated code; no agent tests/CPU checks/game/renderer/GPU.
Packaged D:/Games/DoraRecomp3/testbuild/Dora64-3-test-091.exe.
All285 pre-existing non-log package files unchanged; shared DLL hashes match.
EXE SHA256: 709cb1f7daa03651f5ca0d142330d762e0d024f65acc96791bd976c59782c25f
User visual validation pending. No new commit/push.

## After091: confirmed camera; residual lava flash and092

User: camera fixed; lava appears improved, but a single-frame flash at the end
of the loop. No fresh runtime profile, game or CPU execution by the agent.

ROM sequences801FD54C and801FD5C4 end02AB->02AA and wrap into two02AA holds.
Sequence801FD5D0 similarly ends02B1->02B0 and holds02B0. Whole-cycle asset
color trajectories number177 for02AA..02AF and50 for02B0..02B5, versus16 CI4
palette entries. The090 pair partition is stable *inside* each transition, but
its approximated endpoint does not equal either the next pair's approximation
or the literal ROM image used at phase0/1 and identical holds. That is a static
discontinuity even with perfect timing. It differs from earlier half-tile UV
wraps, whose exact-scroll fixes stay intact.

092 partitions displayed color trajectories once across all distinct images
in a compatible opaque CI4 loop. One pixel-index map and one palette per
source image are shared by every transition, hold and wrap. Thus the ending
palette of stepN is exactly the starting palette of stepN+1, including modulo
wrap; holds never switch back to a separately quantized ROM image. Fractional
colors use lerp and one final RGB555 rounding. Equal endpoints bypass arithmetic
so a half-integer mean cannot alternate due to floating-point cancellation.

This applies to compatible world CI4 deformation/fade loops, not a hardcoded
lava ID. Exact nonzero texture translations keep the prior UV-scroll path.
IA8, clamped/nonlooping sequences, cutout materials, Source channel timing,
repeat counts and Original rendering are unchanged. Original resource bytes
are untouched. Native still has the original CI4/TMEM budget; its fixed cycle
partition can approximate colors even at keyframes/holds. This quality tradeoff
needs user visual review; no claim of pixel-exact original Native keyframes.

Layouts are cached by all bitmap content signatures, bounded to16 layouts and
64 distinct images per optional layout; incompatible/oversized cycles retain
the prior renderer path. Per-frame samples stay in the existing leased arena
with091 extended addresses and the existing submit overlap fallback.

Static inspection only, no tests/CPU checks/game/emulator/renderer/GPU.
Compilation/package does not confirm visual seam removal.

### Build092

Clean incremental MSVC compile/link exit0 in21.203s; diagnostics/profileOFF.
Generation091 reused with verified input hashes. Packaged
D:/Games/DoraRecomp3/testbuild/Dora64-3-test-092.exe.
All288 prior non-log package files unchanged; shared DLL hashes match.
EXE SHA256: 04a35a90b4cbf152d90d7b4ac0bbf19b737c1bfb416f34b4ab0a1e99239ed394
No agent game/tests/CPU checks/GPU; visual result awaits user.

## User acceptance and publication

2026-10-09: user confirmed091 fixed the swimming camera, then confirmed092
worked for the remaining lava-loop flash and explicitly requested commit/push.
The four implementation files match the exact092 compile receipt hashes.
Documentation acceptance updates postdate that immutable build snapshot.
No agent runtime/CPU tests were run; visual acceptance comes from the user.
