# After087: overlap independent geometry upload and TMEM preparation

087 run1791480023665 schema6:53 sample rows, scope_errors0, dropped0.
Late scene165 (t_ms>=30000),3326 frames, Native5x/Manual360:
producer5.057ms/frame, HLE5.048 including fullSync2.077; HLE excluding fullSync
2.971. CPU tiles0.743, producer GPU upload wait0.682, upload command preparation
0.224. Graphics RAM lock0.0001 and texture eviction0.004 are negligible.
SP host enqueue-to-delivery mean262us/median45us (includes audio), DP142/37us;
delivery-to-scheduler receive19/10us. Audio total0.297ms/output frame; concurrent
stages must not be added. Windows thread CPU readings are quantized. Prior086
user result only perhaps10FPS and approximately same, not a confirmed speedup.

Pinned non-renderToRDRAM fullSync serializes CPU tile/TMEM replay and geometry
GPU copy. Geometry arrays and transforms are finalized and their CPU uploads
already submitted before the branch. Workload::uploadDrawData copies vertices,
indices, RDP params/tiles, lights/viewports and raw triangles; it does not copy
callTiles or fbPair operation lists. The transforms uploader similarly consumes
finalized transforms. fullSyncFramebufferPairTiles deferred=false operations
write TMEM/Rice state, texture cache/hash metadata, callTile IDs and possibly
framebuffer operation descriptions. They neither change geometry/transforms nor
submit to the framebufferGraphicsWorker. Texture uploads use their own cache
queue. Existing BufferPair objects remain owned by the same workload throughout.

088 moves the unchanged CPU tile loop after the geometry command-list execute,
before the unchanged GPU fence wait. This overlaps tile CPU work with that same
frame's GPU copy. CPU upload readiness is still awaited before execute. The same
worker/fence, geometry GPU wait, texture upload wait and queue publication remain.
No completion is sent early, no next-frame resource ownership is introduced.
An exceptional tile-processing path also waits on the submitted copy before
rethrowing, so buffer/command-list destruction cannot race an in-flight copy.
Eager RAM rendering remains byte-for-byte unchanged by this adapter. All tile
loads, hashes, texture queue operations and upload copies keep their own order.

This addresses a measured avoidable serialization; actual benefit is unmeasured
and cannot establish360FPS. About3ms HLE plus Native work still remain. Do not
present it as the sole remaining bottleneck. New pinned build-local adapter
checks State, Workload, RDP, TMEM and BufferUploader sources; no dependency or
generated output edited by hand.080/084/086 retained,082 batching still disabled.

Clean088: GraphicsProfileOFF, DiagnosticsOFF, no generic profiler. Existing CPU
and audio generation reused only with matching inputs. No agent tests/CPU checks,
game/emulator/renderer/GPU runs. User checks performance and graphics. Preserve
all dirty work and previous packages; no commit/push without direct command.

088 compile/package exit0, MSVC RelWithDebInfo, 35.640s.
EXE D:/Games/DoraRecomp3/testbuild/Dora64-3-test-088.exe
SHA256 6e50b8ad56f8e10e91b9e04fa7c30bc72f2b6dd567eb63924be918790db87433
Clean flags verified. All276 prior non-log files preserved; DLL hashes match.
No agent game/tests and no commit/push. FPS/graphics user check pending.
