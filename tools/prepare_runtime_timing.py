"""Generate a pinned research-only runtime source; never edit dependency checkouts."""
import hashlib, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
source=ROOT/'build-deps/lib/N64ModernRuntime/ultramodern/src/events.cpp'
data=source.read_bytes()
expected='cd47bb9b332e0c907131310bb3ec8b551b7fb7b2160538ba7fe5aa7554ba2e97'
if hashlib.sha256(data).hexdigest()!=expected:
    raise RuntimeError('Unreviewed events.cpp revision; refusing timing adaptation')
text=data.decode()
before='''                // Keep the original RSP timing; delaying this until parsing completes
                // did not prevent Doraemon's nested-list corruption and stalls the game.
                sp_complete();'''
after='''                // Dora64-3 research only: keep the original scheduler waiting for
                // graphics SP completion until RT64 has consumed this task.
                // This is not a complete memory-lifetime fix or a timing guarantee.'''
point='                renderer_context->send_dl(&task_action->task);'
assert text.count(before)==1 and text.count(point)==1
text=text.replace(before,after).replace(point,point+'\n                sp_complete();')

def replace_once(text, before, after):
    if text.count(before)!=1: raise RuntimeError('Unreviewed runtime source layout: '+before)
    return text.replace(before,after)

text='#include "n64_hardware.hpp"\n'+text
text=replace_once(text,'struct SpTaskAction {','struct RawRdpAction { uint32_t address, size; };\n\nstruct SpTaskAction {')
text=replace_once(text,'using Action = std::variant<SpTaskAction,','using Action = std::variant<RawRdpAction, SpTaskAction,')
text=replace_once(text,'void sp_complete() {','void sp_complete() {\n    dora3_sp_completed();')
text=replace_once(text,'    OSTask* task = TO_PTR(OSTask, task_);','    OSTask* task = TO_PTR(OSTask, task_);\n    dora3_sp_submitted();')
text=replace_once(text,'            if (const auto* task_action = std::get_if<SpTaskAction>(&action)) {','''            if (const auto* raw = std::get_if<RawRdpAction>(&action)) {
                std::unique_lock rdram_lock{ events_context.graphics_rdram_mutex };
                OSTask task{};
                task.t.type=dora3::hw::raw_rdp_task;
                task.t.data_ptr=raw->address;
                task.t.data_size=raw->size;
                renderer_context->send_dl(&task);
            }
            else if (const auto* task_action = std::get_if<SpTaskAction>(&action)) {''')
text+='\nvoid dora3_enqueue_raw_rdp(uint32_t address, uint32_t size) { events_context.action_queue.enqueue(RawRdpAction{address,size}); }\nvoid dora3_raw_dp_event() { dp_complete(); }\n'
output=Path(sys.argv[1]).resolve()
if not output.is_relative_to(ROOT/'build-local'): raise ValueError('Output must stay in ignored build-local')
output.mkdir(parents=True,exist_ok=True)
def write(name,content):
    dest=output/name
    if not dest.exists() or dest.read_text()!=content: dest.write_text(content)
write('events-after-parse.cpp',text)
data=(source.parent/'scheduling.cpp').read_bytes()
if hashlib.sha256(data).hexdigest()!='c722d615e94bee97d87f158249b6d75429fa381f875cde2db7110dba21ca2f7e': raise RuntimeError('Unreviewed scheduling.cpp')
adapted='#include "n64_hardware.hpp"\n'+data.decode()
adapted=replace_once(adapted,'void ultramodern::check_running_queue(RDRAM_ARG1) {','void ultramodern::check_running_queue(RDRAM_ARG1) {\n    if (!dora3_interrupts_enabled()) return;')
write('scheduling.cpp',adapted)
data=(source.parent/'mesgqueue.cpp').read_bytes()
if hashlib.sha256(data).hexdigest()!='68fb77dd5d630866a30b3d0a8dc5dcc0cc9d9c9184cc71b3a7bebc5f486c2978': raise RuntimeError('Unreviewed mesgqueue.cpp')
adapted='#include "n64_hardware.hpp"\n'+data.decode()
adapted=replace_once(adapted,'void dequeue_external_messages(RDRAM_ARG1) {','void dequeue_external_messages(RDRAM_ARG1) {\n    if (!dora3_interrupts_enabled()) return;')
for wait_entry in ['void ultramodern::wait_for_external_message(RDRAM_ARG1) {',
                   'void ultramodern::wait_for_external_message_timed(RDRAM_ARG u32 millis) {']:
    adapted=replace_once(adapted,wait_entry,wait_entry+'\n    if (!dora3_interrupts_enabled()) return;')
write('mesgqueue.cpp',adapted)
print('Pinned runtime: SP lifecycle, raw RDP queue, interrupt gating; original dependencies unchanged')
