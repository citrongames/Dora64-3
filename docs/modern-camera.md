# Test-095 accepted by the user (2026-10-10)

The user reported the build appears to work after the zone-reentry crash fix
and explicitly authorized commit/push. This supersedes the historical pending
playcheck restrictions below for the completed camera work. It does not claim
exhaustive verification of every room or obstacle. Delivered EXE remains test095;
no agent game/tests/CPU/GPU execution and no additional compilation for this
documentation update.

# Windows test-095 — compiled, user playcheck pending

Incremental MSVC compile/link succeeded. EXE: Dora64-3-test-095.exe. No game/tests
or CPU checks run. Unchanged source generation startup-modern-camera-v2 is staged in
the existing startup-clean-after074-v1 cache slot to compile only changed units;
prior differing generated files and receipt are backed up in the task workspace.
No commit/push before the user's gameplay check.

## 2026-10-10: test094 zone-reentry crash diagnosed and source fix pending build

User reported a crash after reentering zones. Read-only analysis of local dump
Dora64-3-test-094.exe.32628.dmp found fail-fast C0000409/7 and a C++ logic_error.
Saved stack addresses resolve against test094 PDB to SimulationSuspension
constructor, modern_camera.cpp update line171, camera::dispatch, and native
camera dispatch. Source review identifies the constructor's explicit exception
"Subsystem suspension outside simulation". Camera's Source shake path wrongly
constructed a Native suspension with no active slice after a zone handoff.
Fixed by suspending only when a slice exists; Original calls the same shake
handler directly. UI fallback remains geometry-only and does not advance shake
commands. No generation changes; no tests/game/CPU/GPU runs. User asked to check;
no new Windows EXE without a build command. No commit/push before user playcheck.

# Windows test-094 — compiled, user playcheck pending

Incremental MSVC compile/link succeeded. EXE: Dora64-3-test-094.exe. No game/tests
or CPU checks run. New source generation startup-modern-camera-v2 is staged in
the existing startup-clean-after074-v1 cache slot to compile only changed units;
prior differing generated files and receipt are backed up in the task workspace.
No commit/push before the user's gameplay check.

# Modern camera — 2026-10-10

Implementation based on main1155643 / accepted test093. User gameplay check is
pending; compilation does not establish camera correctness in actual rooms.

## Original evidence and permission

Locked Japanese ROM SHA256410f99bdd58795639da272c9be9d1ab8c35b61f24f7c1fb92f7e83e1069a2602.
Scene1 ROM70A50, dispatcher801DBD58,16 entries at80210F98 /15 roots. Z2000
tests: mode3/801D91B8, mode4/801D7944, mode5/801D5F14. Logical permission
must not require a current Z edge or the >5-degree error that skips an already
aligned turn. State67 blocks Original Z; modes4/5 also check dialogue8021710D.
Modern also excludes scripted state66, dialogue in all modes, room transitions,
world freeze, game menus, cutscenes, nonworld callback owners and minigames.
Mode7 is fixed-room geometry; mode8 is player-relative geometry. Neither grants
free camera permission. Modes0/1 accept manual C input but are not Z-follow.

Original prefix R10 enters old camera mode2 through801DBAA0; modern zoom must
not run this prefix. Generator modern_camera_patches.py verifies full dispatcher
hash and instruction before masking R/C-up/C-down in its selected-input register.
Automatic mode3/4 swimming transitions and BE84 Native ownership join remain.

## Settings and input

Game tab, immediately before Show FPS: Modern camera(false); Camera stick speed
120, range30..360 deg/s; Capture mouse(false); Camera mouse sensitivity0.15,
range0.02..1 deg/pixel; Invert camera X/Y(false). Same Dora64 labels/order/help.
Modern profile enables it; Original profile disables it. Camera JSON keys match
Dora64. Existing bindings/unknown settings survive; PageUp/PageDown zoom actions
are appended, preserving old action indexes and18-action legacy migration.

Right stick/C bindings rotate. Dpad up/down, wheel, PageUp/PageDown or RB/E+
vertical camera stick zoom. R/C/Dpad up/down/Z are intercepted only while modern
camera is published active. Z recenters over five original periods without
leaving Original's control+1 turn lock set. Mouse capture requires foreground,
active camera and closed port menu; acquisition delta is discarded. Focus/menu
and unavailable-owner transitions discard pending deltas. SDL never reads RAM.

## Camera ownership and timing

modern_camera.cpp owns yaw/elevation, preferred distance80..600 (initial300),
focus follow14/s and collision ease-out8/s. Zoom240 units/s; wheel30 units/step.
Eye/target/yaw use the actual resident camera record801A99F8; player movement
reads its yaw, so rotation also changes movement's camera-relative basis.
Focus is held in world coordinates and published relative to current world
origin800DAE54/800FE878/800DD3F0. Resident8002F064 derives eye/up. Initial height
comes from the original target, clamped20..100, as in Dora64.

All roots use native_camera_timeline dispatch. Modern consumes actual native
slice.seconds(), outside the cumulative Original equation anchor. Disabled or
ineligible roots retain their original timing path. UI fallback evaluates into
its private RAM image and publishes camera geometry only: elapsed increments
are subtracted from the following Source period, avoiding double motion/input.
Original camera shake commands/counters run at Source boundaries, and the last
shake offset remains visible across Native frames. Scene/menu/script ownership
revokes the orbit; boot/Reset resets transient state and pending input.

## Obstacle adaptation

Original camera D5378 calls D52F0 ->80040104. This game queries room grid surface
records DACB8/DACD4 and inside/outside bits DACF0; it is not Dora64's sweep ABI.
Modern queries the entire focus-to-eye segment in10-unit increments (Original
Z distance search uses20), refines the first blocked interval and retains12-unit
clearance. Floor/ceiling clearance is checked and all15 query-result words are
restored afterward. Pull-in is immediate; extension is smoothed. Actual thin,
sloped and moving obstacles, water rooms and visual near-plane clearance still
require the user's gameplay check; no claim of visual validation is made.

## Build and validation scope

Source and ROM analysis, necessary generation, C++ compilation and Windows
packaging only. No tests, CPU checks, game, emulator, renderer or GPU launched.
Generated code is changed exclusively through generator sources. Current changes
remain uncommitted until the user checks the packaged build.
