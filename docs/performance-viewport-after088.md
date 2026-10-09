# Viewport clipping after088

User result for clean088: approximately140 to160FPS (+14.3%), not a controlled
internal profile. Preserve088 overlap and080/084/086.082 remains inactive.

087 run1791480023665 measured HLE outside fullSync2.9707ms/frame.085 sampled
triangle-bounds phase estimated0.5203ms/frame with observer overhead; this
includes vertex bounds/intersections/merges, not just viewport computation.

Static disassembly of the actual088 adapted rt64_rsp.cpp.obj shows four
CRT lroundf calls inside drawIndexedTri per visible triangle, corresponding to
RSPViewport::rect. This calculation reads only scale.xy, translate.xy and the
four int16 clip ratios. It ignores the triangle, world matrices, framebuffer,
scissor and viewport Z. Recomputing it for every triangle is redundant when
those inputs match. This establishes repeated work, not its standalone cost.

089 uses a thread-local one-entry cache keyed by exact24bytes: four float
representations and four int16 ratios. No padding is read. Every call compares
all inputs, so reset, stack changes, other State instances and ratio changes
cannot reuse an incorrect entry. A miss invokes the unchanged upstream rect
with its original arithmetic/lround, then stores the result. The key retains
signed zero distinctions and all bits. Same renderer floating-point environment
is assumed, as in the existing deterministic HLE arithmetic.

All triangles, vertex bounds, scissor intersections, framebuffer merges, draw
order, culling and gameplay run unchanged. No new allocation or unbounded
cache; one small record per producer thread. The adapter pins RSP and viewport
sources and edits only build-local copies after graphics instrumentation.

No agent tests, CPU checks, game, renderer or GPU execution. Compile and static
machine-code inspection cannot establish FPS. Additional gain is pending the
user's playcheck, and this targets only part of the remaining frame expense.

## Build089

MSVC RelWithDebInfo compiled in19.015s (exit0), packaged test-089.
DIAGNOSTICS/GRAPHICS_PROFILE OFF, no generic/fine profile defines.
Static inspection of089 object confirms cache-hit return before four lroundf
calls; drawIndexedTri has no direct lroundf calls. No game or tests executed.
279 prior non-log package files preserved; shared DLL hashes match.
EXE SHA256: 98605ef3250bcf093154c857d6acee943b05f39a0fe2095bda28b7266d47524c
FPS pending user playcheck. No commit/push.
