# Widescreen accepted after test100 (2026-10-10)

User reported that everything now appears normal and authorized commit/push to
main. Final implementation includes independent aspect/HUD settings, wide 3D
views, original blue background extension, corrected iris transitions and finite
minigame backgrounds, split-view HUD/start lights, grenade portraits/HP, and
centered complete puzzle compositions. The skipped-call scope crash fix is
retained. Changes share Original and Native/Manual paths.

test100 compiled and packaged successfully; user checked gameplay. No agent
tests/CPU checks/game/emulator/renderer/GPU execution. This acceptance supersedes
historical pending notes below and does not imply exhaustive scene coverage.
No new build for this documentation/commit step.

# Windows test-100 compiled, user playcheck pending

Executable: Dora64-3-test-100.exe. Generation startup-widescreen-v6 uses the existing
Windows CPU cache slot; only changed generated functions were copied. Compile/
link succeeded; no application/test/CPU/GPU execution and no commit/push.

# Puzzle composition centering after user099 screenshot (2026-10-10)

User099 screenshot shows the two puzzle backgrounds separated around the center,
while model pieces, portraits and divider lines follow different positions.
The099 per-view bitmap anchors were incorrect for this authored composition.
Both puzzle games now keep the entire original320x240 composition centered:
backgrounds use default centered ADJUST, and all three model/effect calls in each
split draw loop use the existing Center viewport role, preserving perspective
and clipping without wide-camera expansion. Mini6 final actor draw725C also uses
Center. Original ROM roots5/6A30 and6/7048, including their two-pass loops and
final bitmap HUD/dividers, were reviewed; each added direct-call scope is checked
against the existing full-body ROM hash by the generator.

The blue underlay remains full width, visible only outside the complete puzzle
canvas. No gap is inserted between participant backgrounds, and original image
proportions, puzzle positions and UI relationships are preserved. Race/plane
start-light anchors and grenade HUD099 changes are retained. Shared Original/
Native/Manual rendering path; Original aspect bypasses these scopes.

Generation startup-widescreen-v6; incremental Windows build next existing number.
No tests, CPU checks, game, emulator, renderer or GPU execution. Visual alignment
requires user playcheck; no commit/push before authorization.

# Windows test-099 compiled, user playcheck pending

Executable: Dora64-3-test-099.exe. Generation startup-widescreen-v5 uses the existing
Windows CPU cache slot; only changed generated functions were copied. Compile/
link succeeded; no application/test/CPU/GPU execution and no commit/push.

# Start lights, puzzle artwork and grenade HUD (2026-10-10)

Original race472C loads four countdown images;4820 selects phase0..3 with start
beeps,494C draws56x22 at x52/212,y30/150. Plane40A4 uses the same four-phase
draw at x52/212,y20/140 (table8508). The race already had a HUD-dependent center
anchor; the plane call was missing. Both now use fixed per-view center markers
0x804/805, following the image aspect independently of HUD Original/Fit. Original
image retains the exact authored coordinates; size and countdown timing remain.

Puzzle5/6448 and6/623C no longer stretch160x120 artwork. The images keep original
proportions at each wide view center, aligned with the existing wide3D puzzle
geometry. An original BC7 blue underlay is drawn once after the clear and before
the split loop (5/6A9C,6/70B4); only added widths remain visible around the images.
No per-cell full-frame overdraw erases preceding participants. Rect clipping
follows the image center; full-scene camera, scissor and gameplay are unchanged.

Grenade mini8:5190 renders four24x24 icons from table85F0 x21/275,y16/195.
3EF4 renders eight HP half-icons per participant, starts table84F0 x60/210/55/215,
y24/200. Added outer-edge icon scopes and whole-function HP group origins, chosen
from participant parity; every HP half/blink moves together with its portrait.
HUD Original/Fit remains honored for the corner HUD. This identifies the grenade
UI from its original draw roots, not the similar numeric-score routines in mini9.

ROM body fingerprints include new scopes; generation preserves098 local flags
for branch-skipped calls. Shared Original/Native/Manual path. No game/tests/CPU
checks/renderer/GPU execution; visual alignment awaits user check. Regenerate
startup-widescreen-v5 and incrementally compile/package next existing EXE.
No commit/push until user gameplay verification and authorization.

# Windows test-098 compiled, user playcheck pending

Executable: Dora64-3-test-098.exe. Generation startup-widescreen-v4 uses the existing
Windows CPU cache slot; only changed generated functions were copied. Compile/
link succeeded; no application/test/CPU/GPU execution and no commit/push.

# Crash correction after user test097 (2026-10-10)

User crash on loading four-participant racing. Read-only minidump analysis of
Dora64-3-test-097.exe.28328.dmp with matching097 EXE/PDB found C0000409/7 and
std::logic_error, dora3_wide_leave at widescreen.cpp155, called by race HUD
801C3FE0. Original ROM branch801C4258 goes directly to801C4474 when the
participant94 timer is zero. This bypasses JAL801C446C, but its unconditional
PC+8 scope-leave hook at4474 still ran. The resulting empty scope stack caused
the fail-fast. Original rendering/timer behavior itself was not the defect.

Generator now declares one local entered flag per explicit call scope, sets it
only on the JAL path, and closes/clears that scope only when entered. Applied to
all24 scopes, including files/title, racing and planes. Flags are local to each
generated C invocation, safe across loops/nesting; no guest register or game
state is changed. The strict runtime scope-stack check remains intact. No
manual generated-C edits and no rollback of097 layout fixes. Regenerate into
startup-widescreen-v4; incremental Windows build next existing number.

Static source/ROM/dump analysis only. No tests/CPU checks/game/emulator/GPU runs.
User must verify the repaired loading path. No commit/push before user approval.

# Windows test-097 compiled, user playcheck pending

Executable: Dora64-3-test-097.exe. Generation startup-widescreen-v3 uses the existing
Windows CPU cache slot; only changed generated functions were copied. Compile/
link succeeded; no application/test/CPU/GPU execution and no commit/push.

# Widescreen corrections after user test096 (2026-10-10)

User reported missing file-selection blue sides, a square iris transition,
unanchored race/plane four-view HUD, missing number-game sky sides, gray puzzle
sides, and darkness on the fruit/voice minigame right side. Fixes below use the
current096 sources and original ROM; no game/CPU/GPU/tests have been executed.

The BC7/BEC bitmap callers are now explicit generation call scopes in scene34
801BDA88/JAL801BDB34 and801BCC08/JAL801BCC94. The096 caller-ra check did not work:
N64Recomp direct host calls do not update the guest ra register. Texture identity
and repeat phase remain unchanged. Native and Original share these call scopes.

Scene1 iris801BDAF0 draws a32x32 I4 mask A04 (record493A30) plus exterior fills.
Its border texels are14/15, maskS/T0 imply tile clamp. A command-local0x803 rect
role extends the mask quad with its original texture slope and the exterior
edges of its fills, with wide clipping. The opening retains its circular shape;
band transitions in the same function extend their original full-width fills.
Animation timing, radius equation and alpha remain Original.

Finite minigame backdrop draw roots3/801C40D8 (B0C),7/801C44E0 (B84),
8/801C3CBC and9/801C4B70 (B96) use a separate viewport role0x803. Their original
projection is stretched into the wide viewport, then the ordinary perspective
role is restored before participants, targets and other geometry. No camera,
actor transforms, culling distances or simulation state change. Exact voice/
fruit module naming was not established in prior research; the isolated finite
backdrops are handled consistently rather than guessing gameplay callbacks.
Puzzle5/801C6448 and6/801C623C draw their160x120 background BMPs in four cells;
these now stretch within the same wide cells, retaining viewport scissor bounds.
The pieces/three-image gameplay geometry keeps ordinary wide perspective.

Race/plane HUD call scopes distinguish outer-corner ranks/status icons, centered
participant result times/banners, and inner-edge plane bars/indicators/icons.
Original ROM coordinate tables prove these groups: race CC69C/CC6AC ranks at
x20/282 and20/274; centered result tables CC57C..CC694; plane8528/8538 same
ranks,8560/8580 bars/indicator and85C0 icons around the shared inner edge x160.
Glyphs and texture strips inherit one call-group origin. Global race/plane
clock and race minimap remain centered. HUD Original continues to use original
coordinates through RT64 extOriginPercentage; HUD Fit moves the verified groups.

research/widescreen-hooks.json records full ROM-body fingerprints and direct-JAL
scope boundaries. tools/widescreen_patches.py validates call opcode and bounds.
Generated C and pinned dependency/donor sources are not edited. Regeneration:
startup-widescreen-v3; Windows incremental compile/package next existing number.
No commit/push before user playcheck. These fixes need visual validation by user.

# Windows test-096 compiled, user playcheck pending

Executable: Dora64-3-test-096.exe. Generation startup-widescreen-v2 uses the existing
Windows CPU cache slot; only changed generated functions were copied. Compile/
link succeeded; no application/test/CPU/GPU execution and no commit/push.

# Widescreen — implementation, user playcheck pending

Base: accepted main 7df7634 / Windows095. No game, renderer, GPU, tests or CPU
checks executed. Compilation is not gameplay validation. Commit/push only after
the user's check. Donor Dora64 and sources/ remain untouched.

Graphics follows Dora64 order: Game render resolution, Game aspect ratio,
Gameplay HUD layout, Texture filtering. Aspect: Original (4:3) / Fit to window
/ fullscreen. HUD is independently Original / Fit. Existing configs default
to Original; Original profile selects both Original, Modern selects both Fit.
RT64 clamps narrow windows to the original aspect; resize/fullscreen use drawable
dimensions. JSON uses donor graphics.aspect_ratio and graphics.hud_layout.

Original projection matrices/FOV/cameras are unchanged. A pinned-source adapter
enables horizontal perspective expansion for every ordinary and split viewport,
including the nine minigames. It preserves each view's scissor and removes the
first game's whole-screen 3D scissor overrides and texture-hash HUD heuristics.
Map composition explicitly retains its centered viewport. Animated game-menu
backdrop projection1 stretches independently from the item/character projection0.
Game camera permission, Native timing, physics and distance gates are unchanged.

ROM-qualified generation hooks attach layout directly to each display list:
resident bitmap/fade/projection routines, scene34 map, and scene1 HUD/menu roots.
Generated functions are regenerated from ROM. Hook body hashes are recorded in
research/widescreen-hooks.json; no manual edits to generated C/dependencies.
Native and Original call the same hooks. Each bank captures aspect/HUD settings.
Ordinary HUD groups (portrait/hearts, weapon/charge, counters) move as complete
groups; dialogue, centered meters, minigame UI and unknown compositions retain
their original centered layout. Split 3D views remain separate; minigame HUD does
not acquire unverified corner anchors. Fill rectangles within HUD retain group
origins; full-width clears and scoped fade effects cover the expanded frame.

BC7 is the original blue logo/bell BMP, repeated every160 source pixels. BEC
title background repeats every162; both palettes are opaque, so overlapping
320-pixel source strips cover clipping seams without changing texture phase.
Doors, letters and menu controls remain centered at their original proportions.
Map retains its original scrolling320x240 composition with a BC7 underlay.
The two other full-canvas artworks BA0/C2D are identified by full original ROM
palette/texel equality and get the same blue underlay. No AI assets, screenshot
textures, external image files or ROM data are bundled. Immutable blue texels
are decoded once into runtime-owned RAM and survive overlays until reset.

Renderer repeats are command-local origins0x801/0x802; other layers use standard
left/center/right origins. Repeated textures preserve CI palette and sampling.
Layer command emission is deduplicated; there is no per-frame asset decoding.
Whole-image equality is limited to full-canvas bitmap candidates.

Native deferred framebuffer readback remains enabled. GPU-to-guest materializing
uses the center320 pixels with the actual last-writer target scale, including
fractional crop and resized retained allocations. Color/depth/packed shader
paths share this addressing; ordinary eager copies are untouched. No queue,
resource-lifetime, DMA or completion synchronization was removed.

Visual acceptance remains with the user: Original/Manual, files/title/map,
ordinary/fixed cameras, menu backgrounds, transitions, split views and resize.
Scene geometry outside original4:3 may have authored gaps; wider projection
cannot create geometry. Unknown 2D compositions deliberately remain centered.
See docs/widescreen-analysis.md and research/widescreen-analysis.json for analysis.
