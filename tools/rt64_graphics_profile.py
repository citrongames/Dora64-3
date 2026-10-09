"""Coarse observation of pinned RT64 stages; no synchronization or logic changes."""
import hashlib

def instrument(root, out, receipt):
    (out/'graphics_profile.hpp').write_bytes((root/'src/graphics_profile.hpp').read_bytes())
    def replace(text, needle, replacement):
        if text.count(needle)!=1:raise SystemExit('Unknown graphics probe boundary: '+needle)
        return text.replace(needle,replacement)
    def scope(stage, name='graphics_scope'):
        return 'dora3::graphics_profile::Scope '+name+'(dora3::graphics_profile::Stage::'+stage+');'
    def wrap_calls(text, call, stage):
        if call not in text:raise SystemExit('Missing graphics probe call: '+call)
        return text.replace(call,'{ '+scope(stage)+' '+call+' }')
    p=out/'rt64_state.cpp';s='#include "graphics_profile.hpp"\n'+p.read_text()
    for signature,stage in [
        ('void State::fullSync() {','FullSync'),
        ('void State::checkRDRAM() {','RDRAMCheck'),
        ('void State::fullSyncFramebufferPairTiles(Workload &workload, FramebufferPair &fbPair, uint32_t &loadOpCursor, uint32_t &rdpTileCursor) {','Tiles'),
        ('auto renderSetup = [&]() {','EagerSetup'),
        ('auto checkRenderTargetSafety = [&](bool checkPairs) {','TargetSafety'),
        ('auto renderAndSynchronize = [&](uint32_t maxFramebufferPair) {','EagerRender')]:
        s=replace(s,signature,signature+'\n        '+scope(stage))
    s=replace(s,'        DORA64_FS_TRACE("enter");','        '+scope('Flush','flush_scope')+'\n        DORA64_FS_TRACE("enter");')
    s=replace(s,"        // Copy the current state\'s extended parameters into the workload.",
        '        flush_scope.finish();\n        dora3::graphics_profile::add(dora3::graphics_profile::Counter::Pairs,workload.fbPairCount);\n        dora3::graphics_profile::add(dora3::graphics_profile::Counter::Tiles,workload.drawData.callTiles.size());\n        // Copy the current state\'s extended parameters into the workload.')
    s=replace(s,'        const bool warningsEnabled = ext.userConfig->developerMode;',
        '        dora3::graphics_profile::add(renderToRDRAM ? dora3::graphics_profile::Counter::Eager : dora3::graphics_profile::Counter::Deferred);\n        '+scope('TileValidation','tile_scope')+'\n        const bool warningsEnabled = ext.userConfig->developerMode;')
    s=replace(s,'        // Fill out all the rendering data for the framebuffer pairs that will be uploaded.',
        '        tile_scope.finish();\n        '+scope('CallPreparation','call_scope')+'\n        // Fill out all the rendering data for the framebuffer pairs that will be uploaded.')
    s=replace(s,'        // Start uploading the entire draw data for all the framebuffer pairs that were processed.',
        '        call_scope.finish();\n        '+scope('DrawUpload','upload_scope')+'\n        // Start uploading the entire draw data for all the framebuffer pairs that were processed.')
    s=replace(s,'        if (renderToRDRAM) {\n            // Run RSP processor.',
        '        upload_scope.finish();\n        if (renderToRDRAM) {\n            '+scope('EagerPath','eager_scope')+'\n            // Run RSP processor.')
    for call,stage in [
        ('ext.framebufferGraphicsWorker->wait();','ProducerGPUWait'),
        ('ext.textureCache->waitForGPUUploads();','TextureWait'),
        ('ext.drawDataUploader->wait();','ProducerUploadWait'),
        ('ext.transformsUploader->wait();','ProducerUploadWait'),
        ('framebufferRenderer->waitForUploaders();','ProducerUploadWait'),
        ('ext.workloadQueue->advanceToNextWorkload();','QueueAdvance'),
        ('inspect();','Inspect')]:
        s=wrap_calls(s,call,stage)
    s=replace(s,'                DORA64_FS_TRACE("after render GPU wait");',
        '                DORA64_FS_TRACE("after render GPU wait");\n                '+scope('CPUReadback','readback_scope'))
    s=replace(s,'        // Evict from the texture cache that are too old and should no longer be maintained.',
        '        '+scope('TextureEvict','evict_scope')+'\n        // Evict from the texture cache that are too old and should no longer be maintained.')
    s=replace(s,'        if (renderToRDRAM) {\n            // Indicate to the texture cache',
        '        evict_scope.finish();\n        if (renderToRDRAM) {\n            // Indicate to the texture cache')
    s=replace(s,'            // Do not render anything, just wait for the data to be uploaded.',
        '            '+scope('UploadCommands','commands_scope')+'\n            // Do not render anything, just wait for the data to be uploaded.')
    s=replace(s,'            ext.framebufferGraphicsWorker->execute();\n            // Geometry/default buffers',
        '            ext.framebufferGraphicsWorker->execute();\n            commands_scope.finish();\n            // Geometry/default buffers')
    p.write_text(s);receipt['state_generated_sha256']=hashlib.sha256(p.read_bytes()).hexdigest()
    source=root/'build-deps/lib/rt64/src/hle/rt64_interpreter.cpp'
    expected='1f979869283e3679d4c9214cd747ac2b856fba28142056d43b16d299872109a4'
    data=source.read_bytes()
    if hashlib.sha256(data).hexdigest()!=expected:raise SystemExit('RT64 interpreter changed; review graphics profile adapter')
    s='#include "graphics_profile.hpp"\n'+data.decode()
    needle='    void Interpreter::processDisplayLists(uint32_t dlStartAdddress, DisplayList *dlStart) {'
    s=replace(s,needle,needle+'\n        '+scope('HLE'))
    p=out/'rt64_interpreter.cpp';p.write_text(s)
    receipt['interpreter_source_sha256']=expected;receipt['interpreter_generated_sha256']=hashlib.sha256(p.read_bytes()).hexdigest()
    p=out/'rt64_workload_queue.cpp';s='#include "graphics_profile.hpp"\n'+p.read_text()
    s=replace(s,'        rendererCPUProfiler.start();','        '+scope('Render','graphics_render_scope')+'\n        rendererCPUProfiler.start();')
    s=replace(s,'                workloadProfiler.start();','                '+scope('Workload','graphics_workload_scope')+'\n                workloadProfiler.start();')
    for call,stage in [('framebufferRenderer->waitForUploaders();','RenderUploadWait'),('ext.workloadGraphicsWorker->wait();','RenderGPUWait')]:
        s=wrap_calls(s,call,stage)
    needle='            rendererGPUProfiler.log(double(frameTimestamps[1] - frameTimestamps[0]) / 1000000.0);'
    s=replace(s,needle,needle+'\n            dora3::graphics_profile::add(dora3::graphics_profile::Counter::RenderGPUTimestampNS,frameTimestamps[1]-frameTimestamps[0]);\n            dora3::graphics_profile::add(dora3::graphics_profile::Counter::Vertices,workload.drawData.vertexCount());\n            dora3::graphics_profile::add(dora3::graphics_profile::Counter::Faces,workload.drawData.faceIndices.size()/3);\n            dora3::graphics_profile::add(dora3::graphics_profile::Counter::Matrices,workload.drawData.worldTransforms.size());')
    p.write_text(s);receipt['generated_sha256']=hashlib.sha256(p.read_bytes()).hexdigest()
    p=out/'rt64_present_queue.cpp';s='#include "graphics_profile.hpp"\n'+p.read_text()
    s=wrap_calls(s,'swapChainValid = ext.swapChain->present(swapChainIndex, &waitSemaphore, 1);','Present')
    s=wrap_calls(s,'ext.workloadQueue->waitForWorkloadId(present.workloadId);','PresentWait')
    p.write_text(s);receipt['present_generated_sha256']=hashlib.sha256(p.read_bytes()).hexdigest()
    receipt['graphics_profile']='Coarse optional scopes and counters; no per-command or per-actor tracing; synchronization preserved'

    # After078: sample hot functions without per-command journals or changing results.
    def sample(stage):
        return 'dora3::graphics_profile::SampleScope sampled_scope(dora3::graphics_profile::Stage::'+stage+');'
    p=out/'rt64_state.cpp';s=p.read_text()
    for signature,stage in [('void State::loadDrawState() {','DrawStateSample'),('void State::flush() {','FlushSample')]:
        s=replace(s,signature,signature+'\n        '+sample(stage))
    p.write_text(s);receipt['state_generated_sha256']=hashlib.sha256(p.read_bytes()).hexdigest()
    sources={
        'rt64_rsp.cpp':'848b9b35f3e76ad94fe2dbacc3a8dc25c689f870c41fd2d190c62af1511d34bc',
        'rt64_rdp.cpp':'87abb3e0bd7f7323ec6fb40318ec01617fca43f01b019461efbebc41b4430a7b',
        'rt64_rdp_tmem.cpp':'bcf1d32e6f78894901c87a63ed28419d44e27675916756c565dc3baa9335bc4a'}
    for name,expected in sources.items():
        data=(root/'build-deps/lib/rt64/src/hle'/name).read_bytes()
        if hashlib.sha256(data).hexdigest()!=expected:raise SystemExit('RT64 detailed profile input changed: '+name)
        s='#include "graphics_profile.hpp"\n'+data.decode()
        if name=='rt64_rsp.cpp':
            for signature,stage in [
                ('void RSP::setVertexCommon(uint32_t rdramAddress, uint32_t dstIndex, uint32_t dstMax) {','VertexSample'),
                ('void RSP::drawIndexedTri(uint32_t a, uint32_t b, uint32_t c, bool rawGlobalIndices) {','TriangleSample'),
                ('void RSP::matrixCommon(const hlslpp::float4x4 &floatMatrix, uint32_t address, uint8_t params) {','MatrixSample')]:
                s=replace(s,signature,signature+'\n        '+sample(stage))
        elif name=='rt64_rdp.cpp':
            for operation in ['Tile','Block','TLUT']:
                signature='void RDP::load'+operation+'Operation(const LoadTile &loadTile, const LoadTexture &loadTexture, bool deferred) {'
                s=replace(s,signature,signature+'\n        dora3::graphics_profile::SampleScope sampled_scope(deferred ? dora3::graphics_profile::Stage::LoadPlanningSample : dora3::graphics_profile::Stage::TMEMLoadSample);')
        else:
            signature='uint64_t TextureManager::uploadTexture(State *state, const LoadTile &loadTile, TextureCache *textureCache, uint64_t creationFrame, uint16_t width, uint16_t height, uint32_t tlut) {'
            s=replace(s,signature,signature+'\n        '+sample('TextureUploadSample'))
            needle='        uint64_t hash = TMEMHasher::hash(TMEM, loadTile, width, height, tlut, TMEMHasher::CurrentHashVersion);'
            s=replace(s,needle,'        dora3::graphics_profile::SampleScope hash_scope(dora3::graphics_profile::Stage::TextureHashSample);\n'+needle+'\n        hash_scope.finish();')
        if name=='rt64_rsp.cpp':s=geometry_parts(s,replace)
        p=out/name;p.write_text(s)
        receipt[name+'_source_sha256']=expected;receipt[name+'_generated_sha256']=hashlib.sha256(p.read_bytes()).hexdigest()
    receipt['graphics_profile_detail']='1/64 sampled TMEM load/hash/upload, vertex/triangle/matrix and draw-state/flush; exact observed counts; unchanged outputs'


def geometry_parts(text,replace):
    def part(stage, name):
        return 'dora3::graphics_profile::PartScope '+name+'(dora3::graphics_profile::Stage::'+stage+',sampled_scope.active());'
    def region(text, start, end, changes):
        a=text.index(start);b=text.index(end,a);body=text[a:b]
        for old,new in changes:body=replace(body,old,new)
        return text[:a]+body+text[b:]
    start='    void RSP::setVertexCommon('
    scope='        dora3::graphics_profile::SampleScope sampled_scope(dora3::graphics_profile::Stage::VertexSample);'
    changes=[(scope,scope+'\n        '+part('VertexStatePart','state_part'))]
    for anchor,previous,stage,name in [
        ('        auto &posFloats = workload.drawData.posFloats;', 'state_part','VertexAppendPart','append_part'),
        ('        readExtendedVertexSegment<3, vertexSize>(rdramAddress, dstIndex, dstMax, globalIndex, G_EX_VERTEX_POSITION, posFloats);','append_part','VertexOverridePart','override_part'),
        ('        uint32_t floatIndex = globalIndex * 3;','override_part','VertexTransformPart','transform_part'),
        ('        if (usesTextureGen) {\n            const float TextureSc','transform_part','VertexTexcoordPart','texcoord_part')]:
        changes.append((anchor,'        '+previous+'.finish();\n        '+part(stage,name)+'\n'+anchor))
    text=region(text,start,'    void RSP::modifyVertex(',changes)
    scope='        dora3::graphics_profile::SampleScope sampled_scope(dora3::graphics_profile::Stage::TriangleSample);'
    changes=[(scope,scope+'\n        '+part('TriangleStatePart','state_part'))]
    for anchor,previous,stage,name in [
        ('        const bool usesShade = geometryMode & G_SHADE;','state_part','TriangleBookkeepingPart','bookkeeping_part'),
        ('        bool visibleTri = true;','bookkeeping_part','TriangleBoundsPart','bounds_part')]:
        changes.append((anchor,'        '+previous+'.finish();\n        '+part(stage,name)+'\n'+anchor))
    return region(text,'    void RSP::drawIndexedTri(uint32_t a, uint32_t b, uint32_t c, bool rawGlobalIndices)',
                  '    void RSP::drawIndexedTri(uint32_t a, uint32_t b, uint32_t c)',changes)
