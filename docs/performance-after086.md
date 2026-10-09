# After086: little user-visible change;087 completion delivery profile

User086 reports approximately same FPS, perhaps10FPS better. This is not a
confirmed improvement. Keep functional080/084 and086 boundaries for observation;
082 batching stays inactive. No further speculative performance fix here.

087 schema6 traces SP/DP host enqueue (ID plus1=SP/2=DP and guest message),
external delivery success/failure with retained ID on requeue, scheduler receive
(queue/message), audio task submit and actual RSP audio duration, graphics RAM
mutex wait, producer/render GPU waits and present waits. Existing frame ledger,
producer/update/build/common/player timeline remains. Individual SP/DP deliveries
can now be separated from audio execution and render processing. Guest receipt
has queue/message rather than host ID: correlate FIFO deliveries respecting jams
and actual queue consumption; never pair solely by nearest time.

Fine sampled hot-function and nested geometry timers compile to no-ops in087.
DORA3_GRAPHICS_FINE_PROFILE is absent; no per-vertex sampling atomics or child
timer overhead. Header declares detail_sample_stride0. Existing coarse timers
retain observer overhead and Windows thread CPU granularity. Trace capacity65536
per reporting interval; dropped count remains explicit. No exact clean FPS claim.

Only diagnostics changed: host profile ID exists under GraphicsProfile; original
enqueue policy, three delivery paths, guest receive, locks, SP/DP completions and
reset order are retained. Failure markers do not cause retry beyond old policy.
No changes to gameplay, time, AI, contact, animation or rendering output.
Generic PROFILE and DIAGNOSTICS remain OFF, GraphicsProfile ON. No agent tests,
CPU checks, game/emulator/renderer/GPU execution. User runs same heavy area20-30s.
Next fix must follow actual evidence. Commit/push only on explicit command.

087 MSVC RelWithDebInfo compile/package exit0, 36.313s.
EXE D:/Games/DoraRecomp3/testbuild/Dora64-3-test-087.exe
SHA256 e02d088f903b15a04578c5622baba33e072bc5fb01906f09b693fe8ec9b909fe
20 profile TUs; generic/fine profiler and Diagnostics off.
All273 prior non-log files preserved, shared DLL hash checks passed.
No agent game/tests, no commit/push. User profile pending.
