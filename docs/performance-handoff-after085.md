# After085: world catch-up delays ready graphics work

User085 profile1791478659712:51 samples, scope_errors0, trace_dropped0.
Shared-clock30..39s:1054 matched frames, producer duty65.48%; ready-frame plus
producer-idle mean2.142ms, median1.416ms, p90 4.431ms.40..51s agrees:2.151ms.
Submit-to-parse median48us; the multi-ms delay precedes runtime submission.
These are diagnostic timings, not a clean084/086 FPS comparison.

Concrete example: frame6895 is ready35.005384s; previous producer ends
35.0068703s. Game thread continues common/player slices through35.0111108s;
next runtime submit35.0116946s, parse35.0117685s. Source BB080 calls the same
native_gameplay_preview_update outside FrameBridge::dispatch; long chains also
occur inside dispatch.084 checkpoints around whole updates cannot service
completions that arrive during these chains. Runtime sends actual SP after
send_dl, DP after unlocking RAM; external messages only enter guest queues at
SDK calls/checkpoints. Original SP/DP handlers select/launch pending tasks.
This establishes a missed cooperative scheduling boundary. The trace does not
record each individual SP/DP enqueue/delivery or audio task, so not all2.14ms
is proven recoverable and no360FPS prediction is justified.

086 keeps084/080 and adds a timeline-only current_at_boundary predicate.
It runs the existing checkpoint once before the first slice and once after each
continuous callback has returned and phase accounting has committed. All RAII
SimulationScope/GameplaySliceScope/NativeInputScope and temporary poses unwind
before it runs. In-callback current() checks remain pure and never schedule.
The same original guest priority scheduler/interrupt mask/queue retention is
used, followed by epoch/live revalidation. Invalid sessions are not advanced.
The game thread cannot consume another game message reentrantly; other ready
guest threads may handle completion/audio as at existing SDK yields. No locks
or borrowed render bank held at the added boundary. Source BB080 and Native
frame wake paths both get this policy through their shared update function.

No slice is skipped or merged, fixed now/elapsed/clip limits/catch-up and
AI/contact/animation math are unchanged. No synthetic SP/DP completion or
resource lifetime wait is removed. Two-bank guest+host ownership unchanged.
No manual generated code/dependency edit;082 geometry batching stays inactive.
085 fine geometry phases are small, observer-biased; sampled parent durations
include nested instrumentation and must not be used as clean execution costs.

Build086 clean GraphicsProfileOFF/DiagnosticsOFF, existing clean CPU/audio
generation reused only after input hash comparison. No agent tests/CPU checks,
game/emulator/renderer/GPU execution. User playcheck establishes actual FPS.
Commit/push only on explicit command; preserve all prior dirty work and packages.

086 compile/package complete: MSVC RelWithDebInfo, 31.969s, exit0.
Clean modes verified in cache and compiler definitions.
EXE D:/Games/DoraRecomp3/testbuild/Dora64-3-test-086.exe
SHA256 5bbbbe38dbb3a956567045046d7fc458694f6da1af51652e4da0912220c7c56d
All270 prior non-log files preserved; shared DLL hashes verified.
No agent game/tests and no commit/push. User FPS check pending.
