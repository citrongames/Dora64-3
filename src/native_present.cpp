#include "native_present.hpp"
#include "native_frame_bridge.hpp"
#include "frame_lights.hpp"
#include <stdexcept>

namespace dora3::native_time {
void PresentLedger::bind(const FrameTicket& ticket, FrameScheduler& owner, uint32_t task, uint32_t fb, uint32_t message) {
    if (ticket.bank >= 2 || task != dora3::frame::bank0 + ticket.bank*dora3::frame::bank_stride ||
        message != task+0xC0 || (fb != 0x803B5000u && fb != 0x803DA800u) || !owner.published(ticket))
        throw std::logic_error("Unreviewed native frame publication");
    std::lock_guard lock(mutex_);
    if (records_[ticket.bank].owner) throw std::logic_error("Native present bank reused while held");
    for (const auto& record: records_)
        if (record.owner && record.frame.framebuffer == fb) throw std::logic_error("Native framebuffer still owned");
    records_[ticket.bank] = {{ticket,task,fb,message}, &owner};
}
bool PresentLedger::owns_task(uint32_t task) const {
    std::lock_guard lock(mutex_);
    for (const auto& r: records_) if (r.owner && r.frame.task==task) return true;
    return false;
}
bool PresentLedger::awaiting_message(uint32_t message) const {
    std::lock_guard lock(mutex_);
    for (const auto& r: records_) if (r.owner && !r.consumed && r.frame.message==message) return true;
    return false;
}
bool PresentLedger::occupied() const {
    std::lock_guard lock(mutex_);
    for (const auto& r: records_) if (r.owner) return true;
    return false;
}
void PresentLedger::reconcile(Record& r) {
    if (r.retired && r.consumed && !r.guest_notified) {
        if (!r.owner->complete(r.frame.ticket, Completion::GuestTask))
            throw std::logic_error("Native guest completion lost ownership");
        r.guest_notified=true;
    }
    if (r.guest_notified && r.host_done) r={};
}
std::optional<PresentFrame> PresentLedger::retire(uint32_t task) {
    std::lock_guard lock(mutex_);
    for (auto& r: records_) if (r.owner && r.frame.task==task) {
        if (r.retired) return std::nullopt;
        r.retired=true;
        const auto frame=r.frame;
        reconcile(r);
        return frame;
    }
    return std::nullopt;
}
bool PresentLedger::consumed(uint32_t message) {
    std::lock_guard lock(mutex_);
    for (auto& r: records_) if (r.owner && r.frame.message==message) {
        if (r.consumed) return false;
        r.consumed=true;
        reconcile(r);
        return true;
    }
    return false;
}
bool PresentLedger::host_read_done(const FrameTicket& ticket) {
    std::lock_guard lock(mutex_);
    if (ticket.bank>=2) return false;
    auto& r=records_[ticket.bank];
    if (!r.owner || r.frame.ticket.token!=ticket.token || !r.retired || r.host_done) return false;
    if (!r.owner->complete(ticket,Completion::HostRead)) throw std::logic_error("Native host completion lost ownership");
    r.host_done=true;
    reconcile(r);
    return true;
}
void PresentLedger::reset_after_runtime_barrier() {
    std::lock_guard lock(mutex_);
    records_={};
}
PresentLedger& native_present_ledger() { static PresentLedger ledger;return ledger; }
uint64_t native_presentation_epoch() { return game_frame_bridge().generation(); }
bool native_presentation_owned() { return presentation_gate().reserved() || game_frame_bridge().active() || native_present_ledger().occupied(); }
}
