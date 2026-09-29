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
target=out/'rt64_workload_queue.cpp';target.write_text(text)
receipt={'source_sha256':expected,'generated_sha256':hashlib.sha256(target.read_bytes()).hexdigest(),
         'scope':'Manual target rate only; Original/Display, VI timing and interpolation pacing unchanged'}
(out/'generation-receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
present_source=root/'build-deps/lib/rt64/src/hle/rt64_present_queue.cpp'
present_expected='ee1dcde2815041174d5ac56e83a24f53f7ec7586bfa84cd4ebcc8093d136820c'
data=present_source.read_bytes()
if hashlib.sha256(data).hexdigest()!=present_expected:raise SystemExit('RT64 present queue changed; review Manual VSync adaptation')
text=data.decode()
needle='                const bool needsResize = ext.swapChain->needsResize() || !swapChainValid;'
assert text.count(needle)==1
policy="""                // Dora64-3 Manual VSync policy: present thread owns swap-chain changes.
                bool manualAboveDisplay;
                {
                    std::scoped_lock<std::mutex> configurationLock(ext.sharedResources->configurationMutex);
                    const auto& config = ext.sharedResources->userConfig;
                    manualAboveDisplay = config.refreshRate == UserConfiguration::RefreshRate::Manual &&
                        (ext.sharedResources->swapChainRate == 0 ||
                         config.refreshRateTarget > ext.sharedResources->swapChainRate);
                }
                ext.swapChain->setVsyncEnabled(!manualAboveDisplay);
                // End Dora64-3 Manual VSync policy.
"""
text=text.replace(needle,policy+needle)
present=out/'rt64_present_queue.cpp';present.write_text(text)
receipt['present_source_sha256']=present_expected
receipt['present_generated_sha256']=hashlib.sha256(present.read_bytes()).hexdigest()
receipt['scope']='Manual target is not clamped; present-thread VSync disabled above display Hz and restored for Original/Display; guest VI unchanged'
(out/'generation-receipt.json').write_text(json.dumps(receipt,indent=2)+'\n')
print('Prepared isolated RT64 Manual FPS/VSync adaptation')
