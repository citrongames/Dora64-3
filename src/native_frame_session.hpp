#pragma once
#include "native_frame_bridge.hpp"
#include "native_presentation_gate.hpp"

namespace dora3::native_time {
// Called only by the scheduled game thread (reset by the quiescent coordinator).
// Caller stops legacy builds/scene changes from begin until Original is returned.
// The legacy boundary must include task retirement, not merely its game message.
class FrameSession {
public:
    enum class Phase { Original, Acquiring, Native, Draining };
    FrameSession(FrameBridge& bridge,PresentationGate& gate):bridge_(bridge),gate_(gate) {}
    bool begin(unsigned fps,unsigned bank,bool legacy_drained,FrameConsumer consumer) {
        if (phase_!=Phase::Original || !legacy_drained || !consumer || bridge_.active() || !bridge_.drained()) return false;
        if (fps<60 || fps>360 || bank>=2) throw std::invalid_argument("Invalid native session");
        token_=gate_.reserve();if (!token_) return false;
        fps_=fps;bank_=bank;consumer_=consumer;phase_=Phase::Acquiring;return true;
    }
    Phase poll(Nanoseconds now) {
        if (phase_==Phase::Acquiring && gate_.ready(*token_)) {
            if (!bridge_.enter(fps_,now,bank_,true,consumer_)) throw std::logic_error("Native session lost entry boundary");
            phase_=Phase::Native;
        }
        if (phase_==Phase::Native && !bridge_.active()) phase_=Phase::Draining;
        if (phase_==Phase::Draining && bridge_.drained()) release();
        return phase_;
    }
    void change_rate(unsigned fps,Nanoseconds now) {
        if (fps<60 || fps>360) throw std::invalid_argument("Invalid native session rate");
        if (phase_==Phase::Acquiring) fps_=fps;
        else if (phase_==Phase::Native) bridge_.change_rate(fps,now);
        else throw std::logic_error("Rate change outside native session");
    }
    void leave(Nanoseconds now) {
        if (phase_==Phase::Acquiring) {release();return;}
        if (phase_==Phase::Native) {bridge_.leave(now);phase_=Phase::Draining;}
        if (phase_==Phase::Draining && bridge_.drained()) release();
    }
    Phase phase() const { return phase_; }
    void reset_after_runtime_barrier() {
        if (bridge_.active() || !bridge_.drained()) throw std::logic_error("Session reset before frame barrier");
        gate_.reset_after_runtime_barrier();token_.reset();consumer_=nullptr;phase_=Phase::Original;
    }
private:
    void release() {
        if (!token_ || !gate_.release(*token_)) throw std::logic_error("Native session lost reservation");
        token_.reset();consumer_=nullptr;phase_=Phase::Original;
    }
    FrameBridge& bridge_;
    PresentationGate& gate_;
    std::optional<uint64_t> token_;
    unsigned fps_=60,bank_=0;
    FrameConsumer consumer_=nullptr;
    Phase phase_=Phase::Original;
};
inline FrameSession& game_frame_session() {
    static FrameSession session(game_frame_bridge(),presentation_gate());return session;
}
}
