# Deferred texture eviction regression after077

User comparison:076 is normal;077 introduces the flower flicker.085 loses the
flower for one frame, while092 can show black instead of its transparent region.
The user identifies Fairy Forest, first level. No new game run by the agent.

## Source evidence

077 enabled deferred rendering for real 3D workloads by correcting the depth
address and accepting fill-only depth clears.076 normally retained eager RAM
rendering. Historical build receipts identify framebuffer_readback.cpp as the
only changed executable input between those builds.

In pinned RT64 State::fullSync, the deferred branch uploads/processes tiles and
waits for uploads, then evicts textures BEFORE handing the workload to the display
worker. TextureManager::uploadTexture/uploadTMEM skip uploads on hashSet hits but
do not mark an existing texture used. Only FramebufferRenderer::createGPUTiles
calls useTexture, which refreshes TextureMap's access age. The eager RAM-render
path performs that call before eviction; deferred rendering performs it later.
Thus a returning texture can be found, skipped for upload, evicted as old, and
then fail lookup for the very workload that requested it. The missing lookup
leaves texture index zero. This is a concrete lifetime defect matching the077
boundary; the precise black-versus-invisible pixel output has not been measured.
A loop boundary is not required: any texture returning near its eviction age is
exposed, including other animated materials or objects returning into view.

##093 correction

The generated RT64 State adapter refreshes all valid non-framebuffer-copy texture
hashes of the deferred workload immediately before eviction, after existing
texture-upload completion. Refresh and eviction share one textureMapMutex lock.
List splice preserves iterators without per-tile allocation; duplicate hashes at
the current timestamp need no update. The timestamp is never moved backward by
this refresh. Existing uploads, hash memoization, GPU deletion locks, replacement
reference handling, eviction age bounds and eager behavior remain unchanged.
Current workloads receive the same age protection previously supplied by eager
createGPUTiles; the original minimum age is twice the four-slot workload queue.
The display consumer may still refresh older queued frames, as on the eager path;
the current workload is refreshed atomically before each producer eviction.

No texture-content, alpha, material timing, animation, gameplay or extra render
changes. No flower-ID exception. Unused textures still expire normally; cache
retention is not globally disabled. Pinned dependency sources are unmodified.
The adapter pins the cache layout and relevant producer/consumer source hashes.

Static analysis only. No tests, CPU checks, game, emulator, renderer or GPU run.
Windows compile/package follows; visual confirmation and FPS impact remain for
the user. Existing092 camera/lava fixes and performance improvements retained.
No commit/push without a new direct command after user validation.

## Build result

Clean Windows093 incremental MSVC RelWithDebInfo compile/link exit0,22.312s.
Package exit0;291 previous non-log files kept size/mtime, shared DLL hashes
matched. EXE SHA256 40eaadefc4b40dbbefe57cfa2f5ece583d8c950dd5f60fcf905957cb7e48cacb.
Diagnostics and all profiler compile flags absent. No game or tests launched.
User visual validation pending; no commit/push.

## User acceptance,2026-10-09

User confirmed test093 worked and explicitly authorized commit/push. The flower
flicker is resolved by user playcheck. No separate FPS measurement or exhaustive
all-material validation was reported. Agent validation remains compilation and
package preservation only; no tests, CPU checks or game/GPU execution.
