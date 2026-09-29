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

text='#include "n64_hardware.hpp"\n#include "graphics_sync.hpp"\n'+text
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
# Bound periodic VI presentation work without blocking VI/AI or dropping tasks.
text=replace_once(text,'    bool cpu_changes_only = false;','    bool cpu_changes_only = false;\n    bool periodic = false;')
text=replace_once(text,"        // If the game hasn't started yet, set a dummy VI mode and origin.",
    "        const bool queue_periodic = !events_context.scheduler_screen_update_active.load(std::memory_order_acquire) && dora3::sync::periodic_present.acquire();\n\n        // If the game hasn't started yet, set a dummy VI mode and origin.")
text=replace_once(text,'            events_context.action_queue.enqueue(DummyWorkloadAction{events_context.vi.get_next_state()->framebuffer});',
    '            if (queue_periodic) events_context.action_queue.enqueue(DummyWorkloadAction{events_context.vi.get_next_state()->framebuffer});')
text=replace_once(text,'            if (!events_context.scheduler_screen_update_active.load(std::memory_order_acquire)) {\n                events_context.action_queue.enqueue(ScreenUpdateAction{ events_context.vi.regs });\n            }',
    '            if (queue_periodic) {\n                if (!events_context.action_queue.enqueue(ScreenUpdateAction{ events_context.vi.regs, false, true })) {\n                    dora3::sync::periodic_present.complete();\n                    throw std::bad_alloc{};\n                }\n            }')
text=replace_once(text,'                renderer_context->update_screen(screen_update_action->cpu_changes_only);',
    '                renderer_context->update_screen(screen_update_action->cpu_changes_only);\n                if (screen_update_action->periodic) dora3::sync::periodic_present.complete();')
# VI state is shared with SDK setters/getters under message_mutex. regs_mutex
# alone only protects the register snapshot, not cur_state or states[].
text=replace_once(text,'        if (!ultramodern::is_game_started()) {\n            static bool odd = false;',
    '        if (!ultramodern::is_game_started()) {\n            std::lock_guard state_lock{ events_context.message_mutex };\n            static bool odd = false;')
text=replace_once(text,'            std::lock_guard lock{ events_context.vi.regs_mutex };\n\n            // Before Doraemon',
    '            std::scoped_lock lock{ events_context.message_mutex, events_context.vi.regs_mutex };\n\n            // Before Doraemon')
for getter in ['osViGetNextFramebuffer','osViGetCurrentFramebuffer']:
    signature='extern "C" PTR(void) '+getter+'() {'
    text=replace_once(text,signature,signature+'\n    std::lock_guard lock{ events_context.message_mutex };')
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
# Keep the native request pending if the GUEST queue is full. The public
# enqueue_external_message API returns void and cannot report initial host OOM.
adapted='#include "native_frame_scheduler.hpp"\n'+adapted
adapted=replace_once(adapted,'bool do_send(RDRAM_ARG PTR(OSMesgQueue) mq_, OSMesg msg, bool jam, bool block);',
    '''bool dora3_enqueue_native_frame_wake() {
    return external_messages.enqueue({int32_t(dora3::native_time::game_queue),
        int32_t(dora3::native_time::WakeMailbox::message_value), false, true});
}
bool dora3_enqueue_native_graphics_wake() {
    // N3DJ scheduler801A69D0 + event queue40. Not a synthetic VI666.
    return external_messages.enqueue({int32_t(0x801A6A10u), int32_t(0xFFFFFF04u), false, true});
}
static void requeue_external_message(const QueuedMessage& message) {
    const bool accepted = external_messages.enqueue(message);
    const bool native = (uint32_t(message.mq) == dora3::native_time::game_queue &&
        uint32_t(message.mesg) == dora3::native_time::WakeMailbox::message_value) ||
        (uint32_t(message.mq) == 0x801A6A10u && uint32_t(message.mesg) == 0xFFFFFF04u);
    if (!accepted && native) {
        // Continuing after dropping the sole retained wake would hang native
        // delivery permanently. Do not silently report success on host OOM.
        throw std::bad_alloc{};
    }
}
bool do_send(RDRAM_ARG PTR(OSMesgQueue) mq_, OSMesg msg, bool jam, bool block);''')
adapted=replace_once(adapted,'        external_messages.enqueue(cur_mesg);',
    '        requeue_external_message(cur_mesg);')
if adapted.count('external_messages.enqueue(to_send);')!=2:
    raise RuntimeError('Unreviewed blocking receive requeue sites')
adapted=adapted.replace('external_messages.enqueue(to_send);','requeue_external_message(to_send);')
# Queue reset is called after wait_for_game_threads_stopped and the SP/gfx
# reset barriers, before RAM is cleared. Pin the caller as well as events.cpp.
reset_caller=ROOT/'build-deps/lib/N64ModernRuntime/librecomp/src/recomp.cpp'
if hashlib.sha256(reset_caller.read_bytes()).hexdigest()!='1b7ffe7b2d8993d851ac2a47753251245c0df18dfe3f01724a881ac1678ecfaa':
    raise RuntimeError('Unreviewed game reset ordering')
adapted='void dora3_reset_native_frame_delivery();\n'+adapted
adapted=replace_once(adapted,'void ultramodern::reset_message_queues_for_game_reset() {',
    'void ultramodern::reset_message_queues_for_game_reset() {\n    dora3_reset_native_frame_delivery();')
write('mesgqueue.cpp',adapted)
print('Pinned runtime: SP lifecycle, raw RDP queue, interrupt gating; original dependencies unchanged')
