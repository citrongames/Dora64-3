# Geometry bookkeeping after profile081

User run1791476020198, schema3:43samples, scope_errors0. Stable scene165 excludes
entry/eager samples:28samples,3860frames,137.38FPS, 3860deferred/0eager/0materialized.
Memo cache1903258hits/212misses =99.9889% among eligible lookups.397631 bypasses.
Texture stage0.734ms/frame vs3.982 in079; producer5.085ms/frame; HLE self2.870ms;
NativeUpdate2.088ms, NativeFrame1.269ms (includes submit wait). Do not sum these
nested/concurrent measurements. Rendering/GPU waits remain, not the only cost.

Wall-only detail estimates: setVertexCommon1.064ms/frame (5781744 batches,
~21k vertices/frame), drawIndexedTri1.178ms/frame (26583024 calls). These are
sampled estimates, not exact exclusive durations. Geometry is now a measured
large expense; whole remaining cost is not attributed to one source line.

Static source establishes20 individual vector appends per ordinary vertex:
position3, color4, attribute indices6, zero velocities5, transformed positions2;
texture coordinates add2. Each append updates size/checks capacity. Every triangle
also loops over identical drawCall tiles three times to reduce integer UV bounds,
and individually appends its3 indices. This is repeated bookkeeping on the
measured path.082 batches it, without removing vertices or changing gameplay.

The pinned build-local adapter tools/rt64_geometry_batch.py grows each vertex
array once per command; fills constant attributes and zero velocities in bulk;
writes position/color/texture/transformed values by index into the live range.
It records each vector's own previous size. EXV1's pre-appended velocities remain
untouched when addEmptyVelocity=false. Extended segment overrides still happen
before transformation, with identical globalIndex and float expressions.
modifyVertex, branchZ/W, lighting, fog, matrices, projection and clipping unchanged.

Triangles reduce the same truncation/ceil integer texture bounds over all3
vertices before updating each tile once. Integer min/max is associative; no
floating-point reassociation. tileCount0 still performs no conversions. Face
indices append together in original order. Culling/raw indices/material state,
color/depth bounds and triangle counts unchanged. Pinned RSP/RDP source hashes
reject upstream changes; generated/dependency files not manually edited.

Allocation failure behavior is not equivalent mid-function; as in the existing
renderer it is not a recoverable gameplay path. Bulk resize value-initializes
some overwritten scalar storage; actual speedup must be checked by user.
No tests, CPU checks, game, renderer or GPU execution by agent.082 will be clean
DIAGNOSTICS OFF / GRAPHICS_PROFILE OFF / generic profiler OFF. No360FPS promise.

Build082 complete: MSVC RelWithDebInfo compile/link exit0, 34.329s.
All three profiling/diagnostic definitions absent; CPU generation clean reused.
EXE D:/Games/DoraRecomp3/testbuild/Dora64-3-test-082.exe
SHA256 56e863ab9d3120e344184b7717ffcdb4c437fc1c0b4b67a2b56beff07d4c5d3f
Preserved all258 prior non-log files; sharedDLL hashes verified.
No agent launch/tests, no commit/push. User FPS/visual check pending.
