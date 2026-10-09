# Ready-frame handoff after083

Evidence:research/test083-critical-path-analysis.json. During30..39s producer
busy68.0%; matched1163frames have mean1.996ms after both NativeFrame end and
previous producer completion before next parse.576frames exceed0.5ms.
Runtime submit-to-parse median50.6us, wake post-to-receive median35.5us;
most queue delay lies before the graphics task reaches runtime, not inside its
host action queue. Native build end to runtime submit mean4.871ms includes time
when previous SP still runs, so cannot all be removed. Other rendering/update
costs remain and no360FPS claim is made.

Source chain: native task hook publishes retained external graphics wake AFTER
original command enqueue. Native game updates/building are cooperative guest
code: pending host SP/DP/pump events are drained at SDK boundaries; idle wait
also schedules after only one external message. A wake can therefore remain
undelivered while long Native computation runs even though a task is ready.
The trace proves delayed handoff; it does not identify exact SP-vs-DP enqueue
race on every frame.084 fixes explicit lack of event-delivery safe points.

Runtime adapter exports dora3_native_scheduler_checkpoint. On guest threads with
interrupts enabled it uses the same dequeue_external_messages and
check_running_queue as SDK calls. No wait for new events, no sleeps, no forced
thread priority or synthetic VI/SP/DP. Full guest queues use existing retained
requeue semantics. Called after graphics wake publication (command already
queued), before Native update and after update before frame reservation/draw.
All calls hold no mailbox/scheduler locks. Guest priority switching may yield
normally to an existing higher-priority thread. It cannot run another instance
of the paused game thread; scheduler may block on full game queue and return
execution to game as before. Checkpoint writes no recomp_context registers.

Pre-submit bank/light registration and temporary pose lifetime are unchanged;
post-submit boundary already allowed scheduler preemption in osSendMesg.
SP remains completed only after send_dl; DP/readback/GPU synchronization and
bank guest+host completion requirements unchanged. Simulation now/elapsed,
AI/contact/animation boundaries, draw distance, vertices unchanged. CPU-only
standalone configuration has no-op checkpoint; runtime build provides the real
implementation via pinned generator, never hand-edited generated sources.

084 clean diagnostics/profilers OFF. Agent only static source/profile review,
necessary generation/compile/package. No tests/game/CPU checks/renderer/GPU
execution. Actual FPS and gameplay verification remains with user.

084 compile/package complete: MSVC RelWithDebInfo, 34.36s.
DIAGNOSTICS/GRAPHICS_PROFILE/generic PROFILE OFF.
EXE D:/Games/DoraRecomp3/testbuild/Dora64-3-test-084.exe
SHA256 b72d8969e983ddfaee7d5ab63bfd19b4dc2451febf01e3b8c299bc05de01dd65
All264 prior non-log files preserved; shared DLL hashes verified.
No agent game/tests, no commit/push. User performance/stability check pending.
