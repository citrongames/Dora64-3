## test135: боковая маска ТВ-катсцены на широком экране

2026-10-12: исходный draw801B90A8 при scene selector0x175 рисует ТВ-рамку
4DB (320x168, x0/y24) и ножку4DC (32x48, x144/y192). На widescreen мир
продолжался за боковыми границами исходного320px полотна.

Перед исходным вызовом bitmap-state801B9100 добавлены две непрозрачные чёрные
полосы. Существующая роль наружной маски0x803 расширяет только внешние края
заливок и их clipping; размеры рамки, ножки и окна телевизора сохранены.
После маски восстанавливается layout, затем оригинальный801BBB84 настраивает
bitmap pipeline. Локальный флаг вызова закрывает scope только на пути,
который действительно рисует телевизор; вне этой сцены маска не вызывается.
Путь общий для Original и Native/Manual FPS, независимо от HUD layout.
При Original aspect widescreen adapter не вмешивается.

Адреса, хеши оригинальных функций и ресурсов: research/widescreen-tv-after134.json.
Генерация из ROM: startup-tv135-v1, без ручной правки generated C.
Инкрементальная Windows Release test135, без CrashLog. Игра, тесты,
CPU checks, renderer/GPU агентом не запускались. 2026-10-12 владелец
подтвердил исправление ТВ-катсцены и разрешил коммит/push.
Архивы121/Android006 не обновлены.

# Minigame corrections through test115 accepted (2026-10-11)

User confirmed the completed fruit-game upper wall and requested commit/push.
This accepts the corrections below through115, including the previously
confirmed number-game backgrounds and fruit-field continuation. Historical
pending notes describe each build at delivery. No exhaustive scene/FPS coverage
is claimed. No additional build, tests or game execution for this Git step.

# Fruit-game upper wall after accepted114 (2026-10-10)

User accepted114 ground/checker continuation and requested completing the upper
wall. ROM B0C vertices20–23 form its vertical quad at Z=-500, Y0..400, X-750..800.
The independent material list2C80..2D08 loads them at2CF8. Added a third immutable
material-list copy with one extra quad from X800 to2800, matching the new ground.
Original wall commands and vertices remain unchanged; original height/depth,
texture repeat, texel density and right-edge UV phase are preserved. S delta is
rounded from2000*12288/1550, within signed16-bit range. ROM evidence is recorded
in research/widescreen-fruit-extension.json.

Accepted114 cells/grass, goal line, gameplay bounds and other minigame fixes
remain. Original aspect bypasses the extension; Original and Native/Manual use
the same render adapter. Existing114 generation is reused. Static source/ROM
analysis and incremental compilation only; no tests/CPU/game/emulator/renderer/
GPU execution. User wall verification pending; no commit/push yet.

# Fruit/voice minigame right-hand continuation after accepted113 (2026-10-10)

User confirmed113 ground in both number games, then authorized extending the
fruit/voice game's checker field and grass without moving the goal line.
ROM B0C (record4B2CA0) separates grass vertices64–75, red line76–99 and checker
tiles100–291. Material lists2E70..2F08 and2F10..3170 draw grass and the line/tiles.
Exact ROM hash, template vertices and scope are in widescreen-fruit-extension.json.

The minigame3 background draw801C40D8 now replaces only those two exact material
calls with immutable lists containing the original commands followed by extra
quads. Two grass strips continue the X800 edge to2800 with matching edge colors
and UV phase. Twenty new100-unit checker columns, six rows each, repeat the
original adjacent540/640 column templates with alternating colors. The original
short final column, all existing cells and the red goal line stay byte-identical.
Only appended geometry occupies the added space; no model, actor, collision,
scoring or other gameplay memory is changed. Other minigames retain accepted113.

New vertex and display-list pointers retain the extended-RDRAM tag fixed112.
Copies are immutable across pending frames and scene changes, reset with the
drained heap. Full original sublist-byte comparisons qualify replacements in
the ROM-bound background scope. Original aspect bypasses the extension;
Original and Native/Manual share this adapter. The blue underlay remains a
fallback beyond finite geometry for unusually wide views.

Generation: startup-wide-fruit114-v1. Static ROM/source analysis and incremental
compile/package only. No tests, CPU checks, game, emulator, renderer or GPU runs.
Visual acceptance pending; commit/push still requires the user's command.

# Number-game ground extension after accepted test112 (2026-10-10)

User confirmed test112 fixes the corrupt background and requested stretching
only the ground as well as the sky in both number-shooting games. The ROM ground
is B84 vertices372–435 (16 flat quads), B96 vertices100–103 (one quad). Their
G_VTX loads and coordinates were read directly from the locked ROM; exact vertex
data is recorded in research/widescreen-arena-after110.json.

Multiply X by4 in those explicit ranges only, widening ±700 to ±2800. Keep Y/Z,
UVs and colors: the ground texture stretches horizontally with its polygons.
The corrected immutable vertex-copy/extended-address path from112 is retained.
Buildings, targets, actors, movement/collision bounds and the grenade arena are
unchanged. Sky keeps its accepted112 extension. Original aspect uses original
vertices; Original and Native/Manual share the same wide render path.

No new generation required. Static ROM/source inspection and incremental
compile/package only; no tests/CPU checks/game/emulator/renderer/GPU execution.
User verification of the ground pending; no commit/push before authorization.

# Number-game sky address correction after test111 (2026-10-10)

User reports background polygons covering the screen in both number games.
test111 incorrectly stripped kseg0 from the immutable vertex copy when writing
G_MW_SEGMENT2. That copy is in the recompiler heap, beyond the original DMA
address range. RT64 F3DEX2 moveWord passes w1 unchanged to RSP::setSegment;
fromSegmented adds the vertex offset, then maskPhysicalAddress applies the
24-bit DMA mask unless bit31 is present. The stripped pointer therefore aliases
unrelated RAM and corrupts all geometry using that vertex segment.

Keep sky.address intact, as required by the already-enabled ExtendedRDRAM path.
Both number games share this correction. The sky-only coordinates/UV extension,
normal arena projection, original collision bounds and blue underlay remain.
No regenerated guest functions are needed. Static source inspection and one
incremental Windows compile/package only; no tests/CPU/game/emulator/renderer/
GPU execution. User visual verification pending; no commit/push yet.

# Finite minigame arenas after test110 (2026-10-10)

User reports the grenade squares look wider than their movement boundaries.
The test097 whole-backdrop projection stretch was wrong: B95 contains the actual
four arena squares, not just decoration. Actors used normal wide perspective,
while their floor was stretched. The same mismatch affected the street/floor
models B84/B96 in both number-shooting games, and B0C in minigame3.

Removed all four role15 whole-model stretches and its renderer exception.
Environment, participants, projectiles and targets now share normal widescreen
perspective. No collision dimensions, movement limits, target hitboxes, cameras,
HUD anchors or timing are changed. Original aspect bypasses these new scopes;
Original and Native/Manual use the same rendering adapters.

ROM evidence: minigame8 init801C3C2C loads B95 at actor804B2000 and translates
Y by25. B95 has four square beds with X/Z edges at25 and350; original boundary
routine801C4334 and object update801C4794 use those same25/350 magnitudes.
Full function/asset hashes are in research/widescreen-arena-after110.json.

Both street models include a separate four-vertex distant sky plane: B84
vertices100–103, B96 vertices372–375. Immutable vertex copies extend only sky X
from ±1000 to ±4000; S expands around2048 at the same rate, preserving the
original central cloud scale/phase. Y/Z, T, colors and every other vertex stay
unchanged. GIMO vertex segment2 is rebound only in the just-built background
commands, with a full original vertex-array comparison. Original model/CPU
resources stay untouched. Copies survive scene changes/pending frames and reset
only with the drained game heap, matching existing widescreen blue assets.

The existing blue underlay fills uncovered space beyond finite geometry in
these four minigames. It is inserted before projection load, below all world
draws, without distorting walkable floors or inventing extra playable space.
The sky extension is finite; very wide/custom views can still reveal this
underlay. No renderer execution or visual result has been claimed.

Necessary generation: startup-wide-arena111-v1. Static ROM/source inspection and
one incremental Windows compile/package only; no tests/CPU checks/game/emulator/
renderer/GPU execution. User verification pending; no commit/push yet.

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

Test142: см. [подарок Oppi, ТВ и диагностика корабля](oppi-ship-tv-after141.md).
