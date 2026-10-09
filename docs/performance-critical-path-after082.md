# Critical path after negative082 result

User reports approximately unchanged FPS in082. The vertex/triangle batch adapter
is retained for evidence but its invocation is disabled in083; this measures the
confirmed080 functional baseline. All unrelated dirty work remains intact.

081 summaries cannot establish frame overlap or delivery latency. Static code:
prepare_runtime_timing.py moves SP completion until after send_dl; fullSync's
non-RAM path still waits for GPU uploads before completion. Two native banks wait
for both guest and host-read completion. These are lifetime/synchronization
contracts, not yet a measured bug. Simply deleting waits risks data corruption.

083 schema4 retains per-second stage/counter summaries and adds a bounded shared
steady-clock trace. Events: [kind,thread,start_us,duration_us,correlation,argument].
Header maps stage IDs and marker IDs. Wake serial+epoch relates post/receive;
frame serial+bank relates ticket and guest/host completion. Submit/parse carry DL
address and bytes; associate repeated bank addresses in chronological order.
WakeLateness is ns past scheduler deadline at accepted request, and includes bank
unavailability as well as OS scheduling delay; it is not pure sleep overshoot.
Trace NativeUpdate,NativeFrame,Source common/player,FrameBuild,Producer,HLE,
FullSync,Screen,Workload,Render. FrameBuild includes blocking original enqueue.
No command/actor event streams.16384 records per reporting interval, explicit
dropped count. Two reserved vectors swap under a short mutex; formatting occurs
outside that mutex. Same-thread nesting and cross-thread overlap are explicit.
Profiler overhead remains; no exact clean-FPS inference from diagnostic build.

Need20-30s user run in the same heavy scene. No speculative synchronization fix,
no change to gameplay or Source timing. No agent execution of game/tests/CPU
checks/renderer/GPU. Generic diagnostics and old game profiler remain disabled.

083 compile/package complete: MSVC RelWithDebInfo, 32.953s; GRAPHICS_PROFILE ON, generic PROFILE/DIAGNOSTICS OFF.
EXE D:/Games/DoraRecomp3/testbuild/Dora64-3-test-083.exe
SHA256 9fdc837c392d7c3c84201e39687cbb41d421b94736756b707004edf45d2f3e06
Preserved all261 prior non-log files; shared DLL hashes verified.
No agent game/tests, no commit/push. User profile pending.
