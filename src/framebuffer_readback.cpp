#include "graphics_profile.hpp"
#include "framebuffer_readback.hpp"
#include "hle/rt64_state.h"
#include "hle/rt64_workload_queue.h"
#include "hle/rt64_present_queue.h"
#include <algorithm>
#include <cstdlib>
#include <mutex>
#include <vector>

bool dora3_set_scaled_framebuffer_readback(bool enabled);
namespace dora3::graphics {
namespace {
// Original 80031588/8003158C stores 80000400 at 801AA650 (800315C8).
// The 320x240x16-bit Z image ends exactly at resident code 80025C00.
constexpr uint32_t game_depth_address=0x400u;
}
bool FramebufferReadback::defer(RT64::State& state,const RT64::Workload& workload) {
    using namespace dora3::graphics_profile;
    std::vector<Copy> writes;
    if(!requested_)add(Counter::RequestOff);
    if(!workload.fbPairCount)add(Counter::NoPairs);
    bool supported=requested_ && workload.fbPairCount>0;
    for(uint32_t i=0;i<workload.fbPairCount&&supported;++i) {
        const auto& pair=workload.fbPairs[i];const auto& color=pair.colorImage;
        // CPU-dependent/offscreen framebuffer work retains RT64's ordinary
        // eager path. The fast path owns only the game's permanent screen/Z.
        supported=pair.startFbOperations.empty()&&pair.endFbOperations.empty()&&
            pair.startFbDiscards.empty()&&color.width==320&&
            color.siz==G_IM_SIZ_16b&&color.fmt==G_IM_FMT_RGBA;
        if(!supported){
            if(!pair.startFbOperations.empty()||!pair.endFbOperations.empty()||!pair.startFbDiscards.empty())add(Counter::RejectOperations);
            if(color.width!=320||color.siz!=G_IM_SIZ_16b||color.fmt!=G_IM_FMT_RGBA)add(Counter::RejectFormat);
            break;
        }
        if(pair.drawColorRect.isEmpty())continue;
        const auto height=uint32_t(pair.drawColorRect.bottom(true));
        supported=height>0&&height<=240;
        if(!supported){add(Counter::RejectHeight);break;}

        const bool depth_clear=pair.fastPaths.clearDepthOnly;
        if(depth_clear) {
            // 8002F594 clears Z by setting it as the color image and filling
            // it, before 8002F6A4 restores the real screen color image.
            // RT64 marks this pair during submitFramebufferPair and renders
            // it into a Depth target, WITHOUT allocating a Color target.
            // Track that actual target for later RAM materialization.
            supported=pair.fillRectOnly&&depth_address_==game_depth_address&&
                color.address==depth_address_&&pair.depthImage.address==depth_address_;
            if(!supported){add(Counter::RejectDepthClear);break;}
            writes.push_back({depth_address_,320,height,G_IM_SIZ_16b,G_IM_FMT_DEPTH,true});
            continue;
        }
        supported=color.address==0x3B5000u||color.address==0x3DA800u;
        if(!supported){add(Counter::RejectColorAddress);break;}
        writes.push_back({color.address,320,height,G_IM_SIZ_16b,G_IM_FMT_RGBA,false});
        if(pair.depthRead||pair.depthWrite) {
            supported=depth_address_==game_depth_address&&pair.depthImage.address==depth_address_;
            if(!supported)add(Counter::RejectDepthAddress);
            if(supported&&!pair.drawDepthRect.isEmpty()) {
                writes.push_back({depth_address_,320,height,G_IM_SIZ_16b,G_IM_FMT_DEPTH,true});
            }
        }
    }
    for(const auto& tile:workload.drawData.callTiles)
        if(tile.tileCopyUsed||tile.reinterpretTile){supported=false;add(Counter::RejectTileCopy);}
    if(!supported||writes.empty()) {
        if(writes.empty())add(Counter::NoWrites);
        materialize(state);return false;
    }
    for(auto copy:writes) {
        copy.seed=state.ditherRandomSeed++;
        const auto key=std::make_pair(copy.address,copy.depth);
        auto [it,inserted]=pending_.try_emplace(key,copy);
        if(!inserted) {
            copy.height=std::max(it->second.height,copy.height);
            it->second=copy;
        }
    }
    return true;
}

void FramebufferReadback::materialize(RT64::State& state) {
    if(pending_.empty())return;
    using namespace dora3::graphics_profile;
    Scope profile(Stage::Materialize);add(Counter::Materializations);
    auto& queue=*state.ext.workloadQueue;auto& shared=*queue.ext.sharedResources;
    // These IDs were already published by the producer. Wait before taking
    // the target lock needed by either consumer; no future VI is required.
    Scope wait_profile(Stage::MaterializeWait);
    queue.waitForWorkloadId(state.workloadId);
    state.ext.presentQueue->waitForPresentId(state.presentId);
    wait_profile.finish();
    std::scoped_lock targets(shared.workloadMutex,shared.managerMutex);
    auto* worker=state.ext.framebufferGraphicsWorker;
    struct Transfer {Copy copy;RT64::Framebuffer* fb;};
    std::vector<Transfer> transfers;transfers.reserve(pending_.size());
    struct CopyCoordinates {
        bool previous=dora3_set_scaled_framebuffer_readback(true);
        ~CopyCoordinates(){dora3_set_scaled_framebuffer_readback(previous);}
    } coordinate_mode;
    worker->commandList->begin();
    for(const auto& [key,copy]:pending_) {
        const auto type=copy.depth?RT64::Framebuffer::Type::Depth:RT64::Framebuffer::Type::Color;
        const RT64::RenderTargetKey targetKey(copy.address,copy.width,copy.siz,type);
        auto& target=shared.renderTargetManager.get(targetKey);
        // Coordinates use the target's actual scale, including Auto and a
        // retained larger allocation after a window resize. No top-left crop.
        const float scale=target.resolutionScale.y;
        if(target.isEmpty()||!(scale>0.f)||scale>64.f||target.resolutionScale.x!=scale||
           target.width<uint32_t(copy.width*scale)||target.height<uint32_t(copy.height*scale))std::abort();
        auto& fb=state.framebufferManager.get(copy.address,copy.siz,copy.width,copy.height);
        if(const auto* gpuFb=shared.framebufferManager.find(copy.address))fb.ditherPatterns=gpuFb->ditherPatterns;
        fb.nativeTarget.resetBufferHistory();
        fb.copyRenderTargetToNative(worker,&target,copy.width,0,copy.height,
            copy.fmt,copy.seed,state.ext.shaderLibrary);
        transfers.push_back({copy,&fb});
    }
    worker->commandList->end();worker->execute();worker->wait();
    for(const auto& transfer:transfers) {
        const auto& copy=transfer.copy;auto& fb=*transfer.fb;
        fb.copyNativeToRAM(state.RDRAM+copy.address,copy.width,0,copy.height);
        fb.lastWriteType=copy.depth?RT64::Framebuffer::Type::Depth:RT64::Framebuffer::Type::Color;
        fb.lastWriteFmt=copy.fmt;fb.lastWriteTimestamp=state.framebufferManager.nextWriteTimestamp();
        RT64::FixedRect written;
        written.ulx=written.uly=0;written.lrx=int32_t(copy.width*4);written.lry=int32_t(copy.height*4);
        fb.lastWriteRect.merge(written);
        fb.everUsedAsDepth|=copy.depth;
    }
    state.framebufferManager.hashTracking(state.RDRAM);
    state.rdramCheckPending=true;
    pending_.clear();
}
}
