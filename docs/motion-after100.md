# Motion corrections after test100

User setup: Manual60 FPS, Modern Camera, Widescreen. Reports: rough camera
handoff to dialogue/script, Nobita's chair movement in the first cutscene,
grenade flight in minigame8. The user did not inspect the FPS counter; this
does not establish a measured performance drop. Main-game grenade motion was
reported smooth and is outside the new projectile adapter.

## Source evidence and changes

ROM SHA256:410f99bdd58795639da272c9be9d1ab8c35b61f24f7c1fb92f7e83e1069a2602.

Modern camera previously discarded its orbit immediately on dialogue/script
ownership loss. Native frames now apply a200ms draw-only handoff to the actual
Original target, with shortest yaw/elevation/distance motion and world-space
target coordinates. Original controls, counters and source camera fields are
restored after the draw. Overlay generation, process pair, menu/load/fade and
graphics gates cancel a stale handoff. Subsequent script cuts are not smoothed.
No free-camera input is allowed while Original owns dialogue/script. Original
FPS does not use this Native frame scope. Reset clears retained handoff state.

Opening chair scene is script350 at ROM167280..168630, NOT script376 (the latter
is the later opening sequence with kinds155/156). Chair150 is created for
participant5 at ROM1675A2; Nobita143 uses participant1. Opcode65 issues matched
short position/rotation bursts around ROM16778E onward, including durations1/2.
E4484 runs commands before E3768 participant timers/motion and E0760 cameras.
The previous cutscene anchor was captured AFTER these equations; fractional
frames predicted the next interval, whose timers can clear a short command
before its intermediate poses were displayed. Models already sample their
current Source interval from the pre-advance clip time.

A ROM-qualified E3768 entry hook now captures the reached command interval
after commands and before equations. Native recomputes that interval's motion
in the existing private8MiB image, capped at its end, on the model frame clock.
This is equation evaluation, not interpolation of stored rendered poses or
RT64 interpolation. Source still executes commands once at20/30Hz and owns its
completed state. Published transforms are restored before Source. Complete
controller comparisons use the POST-Source baseline, separately from the
pre-equation image; actor/model/driver replacement rejects a stale anchor.
Cutscene and model/UI clocks now share the same top-level Source completion.
Visual motion may trail command execution by up to one Source interval, as
the existing model sampler does. No interpreter/audio/input/allocation replay.

Minigame8 ROM1BC460:4E24 dispatches slots10..59; type0 invokes4D74.4218 derives
horizontal velocity,42C8 applies gravity/terminal-50,4300 advances worldXYZ;
449C owns contacts and4940 spawns explosions.4EC8 copies worldXYZ and rebuilds
the model matrix. The new adapter binds type0 world equations explicitly at
Source completion to the actual completed driver velocities and gravity,
including its first recorded interval. It avoids relying on the float-register
expression graph across indirect projectile calls. XZ uses Original projection;
Y uses the discrete kick/drift extension v*h-g*h*(h+1)/2 with the terminal cap.
Actor/component/allocation and driver identity bound the publication. Contacts,
damage, explosion, retirement, spawn timing and Original mode are unchanged.
This is a targeted robustness correction; static analysis alone has not proven
which runtime capture loss produced the user's minigame symptom.

## Validation and status

Static original ROM/source inspection, necessary generation and incremental
compilation/packaging only. No tests, CPU checks, game, emulator, renderer or
GPU execution. Build status is recorded in docs/windows-test-build.md and
research/windows-build-check.json. User gameplay verification pending. No
commit/push until the user checks this build and explicitly authorizes Git.
Preserve accepted camera095, widescreen100 and all unrelated changes.

## User feedback after101

Opening chair and minigame grenade now smooth by user report. Camera approach
and white fade at the first city Shizuka interaction still drop displayed FPS
to35 or lower. Follow-up: docs/fade-performance-after101.md. No Git authorization.

## Acceptance on2026-10-10

User reports test102 appears to work and explicitly authorizes commit/push to
existing main. Chair and minigame grenade motion were confirmed after101. This
supersedes pending verification/Git restrictions above for changes through102.
No new measured FPS result or exhaustive scene coverage claimed. No additional
build, tests, CPU checks, game, emulator, renderer or GPU execution.
