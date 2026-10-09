# After084: additional improvement, remaining cost unmeasured

User reports +20-30FPS and about150FPS. This confirms an additional practical
benefit of handoff checkpoints but does not show exact residual queue delay.
083 predates084. No360FPS claim.080 hash cache and084 checkpoint remain active;
082 geometry batching remains disabled after negative feedback.

085 observation-only keeps shared-clock timeline of083 on top084 and samples
geometry phases only when the original parent1/64 sample was selected.
Vertex parts: state/matrices/lighting; attribute array append; extended vertex
segment overrides; CPU position/screen transforms; texture-coordinate output.
Triangle parts: framebuffer/projection/draw state; index/UV/matrix bookkeeping;
culling plus screen/color/depth bounds. Same pinned source and expressions.
No per-vertex/triangle journal additions. Only whole phase timings for selected
calls; no thread CPU/cycle query in parts. Unselected calls skip begin_part.

Schema5 stages append after existing IDs. Part observed_calls=0 intentionally:
scale part accumulated wall by corresponding VertexSample/TriangleSample
observed_calls / calls to estimate across all parent calls. Do not multiply
by part calls ratio independently if an early return omitted that phase.
The parent timings include child timer/aggregation overhead; avoid using them
as precise clean cost. Parts also retain clock/branch overhead, especially tiny
regions. The shared timeline can establish how much handoff idle remains after
084; do not reuse083 gap averages as current measurements.

User run20-30seconds in same heavy scene needed. Generic game profiler and
ordinary diagnostic logging stay OFF; only GraphicsProfile ON. No agent game,
tests, CPU checks, renderer or GPU execution. Generated dependencies read-only;
build-local output generated only. No commit/push without explicit command.

085 compile/package complete: MSVC RelWithDebInfo, 32.766s successful compile; first configure helper-scope error corrected.
GRAPHICS_PROFILE ON, generic PROFILE/DIAGNOSTICS OFF.
EXE D:/Games/DoraRecomp3/testbuild/Dora64-3-test-085.exe
SHA256 0b52ca5fd1f8e8bd2bfcc658f3f80f9b7cd02155c177b66b34d063b6a6ce3b19
All267 prior non-log files preserved; shared DLL hashes verified.
No agent game/tests, no commit/push. User profile pending.
