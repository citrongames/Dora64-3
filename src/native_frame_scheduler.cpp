#include "native_frame_scheduler.hpp"
#include <limits>
#include <stdexcept>

namespace dora3::native_time {
bool WakeMailbox::prepare(WakeToken token) {
    if (token.epoch == 0 || token.serial == 0) return false;
    std::lock_guard lock(mutex_);
    if (queued_) return false;
    queued_ = token;
    return true;
}

bool WakeMailbox::reject_enqueue(WakeToken token) {
    std::lock_guard lock(mutex_);
    if (!queued_ || *queued_ != token) return false;
    queued_.reset();
    return true;
}

std::optional<WakeToken> WakeMailbox::receive(std::uint32_t message) {
    if (message != message_value) return std::nullopt;
    std::lock_guard lock(mutex_);
    const auto result = queued_;
    queued_.reset();
    return result;
}

bool WakeMailbox::occupied() const {
    std::lock_guard lock(mutex_);
    return queued_.has_value();
}

bool FrameScheduler::drained_locked() const {
    for (const auto& bank : banks_) if (bank.phase != Phase::Free) return false;
    return true;
}

void FrameScheduler::invalidate_locked() {
    if (epoch_ == std::numeric_limits<std::uint64_t>::max())
        throw std::overflow_error("Native wake generation exhausted");
    ++epoch_;
    pending_.reset();
}

bool FrameScheduler::enter(unsigned fps, Nanoseconds now, unsigned bank, bool safe) {
    if (bank >= banks_.size()) throw std::invalid_argument("Invalid native task bank");
    std::lock_guard lock(mutex_);
    if (!safe || clock_.active() || !drained_locked()) return false;
    clock_.configure(Mode::Manual, fps, now);
    clock_.resume(now);
    next_bank_ = bank;
    invalidate_locked();
    return true;
}

void FrameScheduler::change_rate(unsigned fps, Nanoseconds now) {
    std::lock_guard lock(mutex_);
    if (!clock_.active()) throw std::logic_error("Cannot change inactive native rate");
    clock_.configure(Mode::Manual, fps, now);
    invalidate_locked();
}

void FrameScheduler::leave(Nanoseconds now) {
    std::lock_guard lock(mutex_);
    clock_.configure(Mode::Original, 0, now);
    invalidate_locked();
}

std::optional<WakeToken> FrameScheduler::request(Nanoseconds now) {
    std::lock_guard lock(mutex_);
    if (!clock_.active() || pending_ || banks_[next_bank_].phase != Phase::Free) return std::nullopt;
    if (now < *clock_.deadline()) return std::nullopt;
    if (serial_ == std::numeric_limits<std::uint64_t>::max())
        throw std::overflow_error("Native wake serial exhausted");
    pending_ = WakeToken{epoch_, ++serial_};
    ++stats_.wakes;
    return pending_;
}

bool FrameScheduler::reject_enqueue(WakeToken token) {
    std::lock_guard lock(mutex_);
    if (!pending_ || *pending_ != token) return false;
    pending_.reset();
    ++stats_.queue_rejections;
    return true;
}

std::optional<FrameTicket> FrameScheduler::receive(WakeToken token, Nanoseconds now) {
    std::lock_guard lock(mutex_);
    if (!pending_ || *pending_ != token) {
        ++stats_.stale_wakes;
        return std::nullopt;
    }
    pending_.reset();
    // Rate/scene changes invalidate tokens before they can consume the clock.
    if (!clock_.active() || banks_[next_bank_].phase != Phase::Free) return std::nullopt;
    const auto step = clock_.take(now);
    if (!step) return std::nullopt;
    const unsigned bank = next_bank_;
    banks_[bank] = {Phase::Building, token, false, false};
    next_bank_ ^= 1;
    ++stats_.frames;
    return FrameTicket{token, bank, *step};
}

bool FrameScheduler::publish(const FrameTicket& ticket) {
    std::lock_guard lock(mutex_);
    if (ticket.bank >= banks_.size()) return false;
    auto& bank = banks_[ticket.bank];
    if (bank.token != ticket.token || bank.phase != Phase::Building) return false;
    bank.phase = Phase::Published;
    return true;
}

bool FrameScheduler::complete(const FrameTicket& ticket, Completion completion) {
    std::lock_guard lock(mutex_);
    if (ticket.bank >= banks_.size()) return false;
    auto& bank = banks_[ticket.bank];
    if (bank.token != ticket.token || bank.phase != Phase::Published) {
        ++stats_.stale_completions;
        return false;
    }
    bool& done = completion == Completion::GuestTask ? bank.guest_done : bank.host_done;
    if (done) return false;
    done = true;
    if (bank.guest_done && bank.host_done) bank.phase = Phase::Free;
    return true;
}

std::optional<Nanoseconds> FrameScheduler::deadline() const {
    std::lock_guard lock(mutex_);
    if (pending_ || banks_[next_bank_].phase != Phase::Free) return std::nullopt;
    return clock_.deadline();
}

bool FrameScheduler::drained() const {
    std::lock_guard lock(mutex_);
    return drained_locked();
}

SchedulerStats FrameScheduler::stats() const {
    std::lock_guard lock(mutex_);
    return stats_;
}
} // namespace dora3::native_time
