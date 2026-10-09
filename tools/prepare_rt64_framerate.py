"""Build-local RT64 adaptation: Manual FPS may exceed display refresh rate.
Pinned dependency checkout stays untouched. Unknown input aborts configuration.
"""
from pathlib import Path
import hashlib,json,sys
root=Path(__file__).resolve().parents[1]
source=root/'build-deps/lib/rt64/src/hle/rt64_workload_queue.cpp'
expected='7fe61533ff8d1ffa475eb90a49310028e10dd59552c0f2a47a3a662cbfd7757a'
data=source.read_bytes()
if hashlib.sha256(data).hexdigest()!=expected:raise SystemExit('RT64 workload queue changed; review Manual FPS adaptation')
text=data.decode()
start=text.index('            // Limit the target rate to the rate detected by the swap chain.')
end=text.index('            break;',start)
removed=text[start:end]
assert 'workloadConfig.targetRate = ext.sharedResources->swapChainRate;' in removed
text=text[:start]+'            // Dora64-3: Manual keeps the requested rate, even above display Hz.\n\n'+text[end:]
out=Path(sys.argv[1]);out.mkdir(parents=True,exist_ok=True)
(out/'performance_profile.hpp').write_bytes((root/'src/performance_profile.hpp').read_bytes())
# Observe the pinned renderer through the existing build-local adapter.
# Rendering order, synchronization and timestamp queries remain unchanged.
text='#include "performance_profile.hpp"\n'+text
def profile_replace(text,needle,replacement):
    if text.count(needle)!=1:raise SystemExit('Unreviewed RT64 profiler boundary: '+needle)
    return text.replace(needle,replacement)
text=profile_replace(text,'        rendererCPUProfiler.start();',
    '        rendererCPUProfiler.start();\n        dora3::profile::Scope renderProfile(dora3::profile::Tag::RT64RenderCPU);')
text=profile_replace(text,'            framebufferRenderer->waitForUploaders();',
    '            { dora3::profile::Scope uploadProfile(dora3::profile::Tag::RT64UploadWait); framebufferRenderer->waitForUploaders(); }')
text=profile_replace(text, """            ext.workloadGraphicsWorker->execute();
            ext.workloadGraphicsWorker->wait();""",
    """            ext.workloadGraphicsWorker->execute();
            { dora3::profile::Scope gpuWaitProfile(dora3::profile::Tag::RT64GPUWait); ext.workloadGraphicsWorker->wait(); }""")
text=profile_replace(text,'            rendererGPUProfiler.log(double(frameTimestamps[1] - frameTimestamps[0]) / 1000000.0);',
    '            rendererGPUProfiler.log(double(frameTimestamps[1] - frameTimestamps[0]) / 1000000.0);\n'
    '            dora3::profile::record(dora3::profile::Tag::RT64GPU,frameTimestamps[1]-frameTimestamps[0]);\n'
    '            dora3::profile::graphics_counts(workload.drawData.vertexCount(),workload.drawData.faceIndices.size()/3,workload.drawData.worldTransforms.size());')
text=profile_replace(text,'                workloadProfiler.start();',
    '                workloadProfiler.start();\n                dora3::profile::Scope workloadProfile(dora3::profile::Tag::RT64Workload);')
target=out/'rt64_workload_queue.cpp';target.write_text(text)
receipt={'source_sha256':expected,'generated_sha256':hashlib.sha256(target.read_bytes()).hexdigest(),
         'scope':'Manual target rate only; Original/Display, VI timing and interpolation pacing unchanged'}
(out/'generation-receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
present_source=root/'build-deps/lib/rt64/src/hle/rt64_present_queue.cpp'
present_expected='ee1dcde2815041174d5ac56e83a24f53f7ec7586bfa84cd4ebcc8093d136820c'
data=present_source.read_bytes()
if hashlib.sha256(data).hexdigest()!=present_expected:raise SystemExit('RT64 present queue changed; review Manual VSync adaptation')
text="""// Optional application target query; standalone replay keeps the null default.
#include <atomic>
static std::atomic<unsigned(*)()> dora3_native_target_query{nullptr};
void dora3_set_native_present_target(unsigned(*query)()) {
    dora3_native_target_query.store(query, std::memory_order_release);
}
"""+data.decode()
needle='                const bool needsResize = ext.swapChain->needsResize() || !swapChainValid;'
assert text.count(needle)==1
policy="""                // Dora64-3 Manual VSync policy: present thread owns swap-chain changes.
                const auto nativeQuery = dora3_native_target_query.load(std::memory_order_acquire);
                const auto nativeTarget = nativeQuery ? nativeQuery() : 0;
                bool manualAboveDisplay;
                {
                    std::scoped_lock<std::mutex> configurationLock(ext.sharedResources->configurationMutex);
                    const auto& config = ext.sharedResources->userConfig;
                    manualAboveDisplay =
                        (nativeTarget != 0 && (ext.sharedResources->swapChainRate == 0 || nativeTarget > ext.sharedResources->swapChainRate)) ||
                        (config.refreshRate == UserConfiguration::RefreshRate::Manual &&
                         (ext.sharedResources->swapChainRate == 0 || config.refreshRateTarget > ext.sharedResources->swapChainRate));
                }
                ext.swapChain->setVsyncEnabled(!manualAboveDisplay);
                // End Dora64-3 Manual VSync policy.
"""
text=text.replace(needle,policy+needle)
text='#include "performance_profile.hpp"\n'+text
text=profile_replace(text,'                swapChainValid = ext.swapChain->present(swapChainIndex, &waitSemaphore, 1);',
    '                { dora3::profile::Scope presentProfile(dora3::profile::Tag::PresentSwap); swapChainValid = ext.swapChain->present(swapChainIndex, &waitSemaphore, 1); }')
text=profile_replace(text,'                ext.workloadQueue->waitForWorkloadId(present.workloadId);',
    '                { dora3::profile::Scope workloadWaitProfile(dora3::profile::Tag::PresentWorkloadWait); ext.workloadQueue->waitForWorkloadId(present.workloadId); }')
present=out/'rt64_present_queue.cpp';present.write_text(text)
receipt['present_source_sha256']=present_expected
receipt['present_generated_sha256']=hashlib.sha256(present.read_bytes()).hexdigest()
receipt['scope']='Native Manual target is independent of RT64 Original/no-interpolation; present-thread VSync disabled above display Hz while owned native frames exist; prior interpolated Manual policy retained; guest VI unchanged'
(out/'generation-receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
print('Prepared isolated RT64 Manual FPS/VSync adaptation')

# Isolate the framebuffer policy adapter too; never edit the dependency tree.
state_source=root/'build-deps/lib/rt64/src/hle/rt64_state.cpp'
state_expected='3a695dfdaa6defa114b15a748b384a3b1f7d41f7b28393c7dbcf61664339f7d1'
data=state_source.read_bytes()
if hashlib.sha256(data).hexdigest()!=state_expected:raise SystemExit('RT64 state changed; review framebuffer readback adaptation')
prefix="""#include <atomic>
namespace RT64 {struct State;struct Workload;}
using Dora3FramebufferPolicy=bool(*)(RT64::State*,const RT64::Workload*);
static std::atomic<Dora3FramebufferPolicy> dora3_framebuffer_policy{nullptr};
void dora3_set_framebuffer_ram_policy(Dora3FramebufferPolicy query) {
    dora3_framebuffer_policy.store(query,std::memory_order_release);
}
static bool dora3_defer_framebuffer_ram(RT64::State* state,const RT64::Workload* workload) {
    const auto query=dora3_framebuffer_policy.load(std::memory_order_acquire);
    return query&&query(state,workload);
}
"""
text=prefix+data.decode()
needle='        const bool renderToRDRAM = extendedRenderToRAMSet ? (extended.renderToRAM != 0) : ext.emulatorConfig->framebuffer.renderToRAM;'
replacement='        const bool renderToRDRAM = (extendedRenderToRAMSet ? (extended.renderToRAM != 0) : ext.emulatorConfig->framebuffer.renderToRAM) && !dora3_defer_framebuffer_ram(this,&workload);'
text=profile_replace(text,needle,replacement)
state=out/'rt64_state.cpp';state.write_text(text)
receipt['state_source_sha256']=state_expected
receipt['state_generated_sha256']=hashlib.sha256(state.read_bytes()).hexdigest()
receipt['framebuffer_scope']='Optional Native gameplay permanent framebuffer readback policy; null/default leaves eager renderer behavior unchanged'
(out/'generation-receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')

native_source=root/'build-deps/lib/rt64/src/render/rt64_native_target.cpp'
native_expected='f05f81c02beb686d8f652d70e01d3a0b3387b0debe36239d1d466260580b8c5f'
data=native_source.read_bytes()
if hashlib.sha256(data).hexdigest()!=native_expected:raise SystemExit('RT64 native target changed; review framebuffer coordinates')
prefix="""#include <utility>
#include <cmath>
static thread_local bool dora3_scaled_framebuffer_readback=false;
bool dora3_set_scaled_framebuffer_readback(bool enabled) {
    return std::exchange(dora3_scaled_framebuffer_readback,enabled);
}
"""
text=prefix+data.decode()
text=text.replace('#include "rt64_native_target.h"','#include "render/rt64_native_target.h"').replace('#include "rt64_render_target.h"','#include "render/rt64_render_target.h"').replace('#include "rt64_shader_library.h"','#include "render/rt64_shader_library.h"').replace('#include "rt64_render_worker.h"','#include "render/rt64_render_worker.h"')
needle='        nativeCB.offset = { 0, rowStart };'
replacement="""        // High bit selects deferred capture coordinates; low bits are
        // unsigned16.16 source scale. Ordinary eager copies retain offset.x0.
        const uint32_t deferredScale = dora3_scaled_framebuffer_readback ?
            (0x80000000u | uint32_t(std::lround(float(srcTarget->resolutionScale.y) * 65536.f))) : 0;
        nativeCB.offset = { deferredScale, rowStart };"""
text=profile_replace(text,needle,replacement)
native=out/'rt64_native_target.cpp';native.write_text(text)
receipt['native_source_sha256']=native_expected
receipt['native_generated_sha256']=hashlib.sha256(native.read_bytes()).hexdigest()
(out/'generation-receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')

from rt64_upload_overlap import adapt as overlap_upload
overlap_upload(root,out,receipt)

# Optional scopes compile away unless DORA3_GRAPHICS_PROFILE is selected.
from rt64_graphics_profile import instrument
instrument(root,out,receipt)
(out/"generation-receipt.json").write_text(json.dumps(receipt,indent=2)+"\n")

from rt64_tmem_hash_cache import adapt
adapt(root,out,receipt)
(out/"generation-receipt.json").write_text(json.dumps(receipt,indent=2)+"\n")

#082 batching showed no user-visible gain;083 measures confirmed080 baseline.
# Adapter retained as source evidence, not applied pending further measurements.

from rt64_viewport_cache import adapt as cache_viewport
cache_viewport(root,out,receipt)
(out/"generation-receipt.json").write_text(json.dumps(receipt,indent=2)+"\n")

from rt64_texture_lifetime import adapt as retain_workload_textures
retain_workload_textures(root,out,receipt)
(out/"generation-receipt.json").write_text(json.dumps(receipt,indent=2)+"\n")
