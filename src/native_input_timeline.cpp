#include "native_input_timeline.hpp"
#include "native_frame_bridge.hpp"
#include <stdexcept>
#include <deque>
#include <limits>
namespace {
using dora3::native_time::Nanoseconds;
struct Port {
    uint16_t held=0,directions=0,pressed=0,direction_pressed=0;
    uint16_t consumed_held=0,consumed_directions=0;
    int8_t x=0,y=0;
    bool connected=false;
};
std::array<Port,4> ports{};
struct Sample {Nanoseconds time;std::array<Port,4> ports;};
std::deque<Sample> samples;
Nanoseconds sampled_at=0;
bool running=false,scoped=false;
uint64_t epoch=0;
// Every output of the original 800305E0 latch, excluding unrelated neighbors.
constexpr std::array<uint32_t,9> fields{
    0x801A99AC,0x801A9534,0x801A953C,0x801AC6C4,0x801A9504,
    0x801A48EC,0x801A48B8,0x801A39E2,0x801A48CC};
constexpr std::array<uint32_t,2> axes{0x80171578,0x801AC6D4};
std::array<Port,4> read(uint8_t* rdram) {
    std::array<Port,4> result{};
    const auto connected=MEM_BU(0,int32_t(0x801A48B4));
    for(unsigned i=0;i<ports.size();++i) {
        auto& port=result[i];const auto pad=0x80171560+i*6;
        const bool valid=(connected&(1u<<i)) && MEM_BU(4,int32_t(pad))==0;
        const uint16_t held=valid?MEM_HU(0,int32_t(pad)):0;
        const int8_t x=valid?MEM_B(2,int32_t(pad)):0,y=valid?MEM_B(3,int32_t(pad)):0;
        const uint16_t direction=(x>=50?0x100:x<=-50?0x200:0)|(y>=50?0x800:y<=-50?0x400:0);
        port.held=held;port.directions=direction;port.x=x;port.y=y;port.connected=valid;
    }
    return result;
}
void accept(Nanoseconds time) {
    while(!samples.empty() && samples.front().time<=time) {
        for(unsigned i=0;i<ports.size();++i) {
            auto& p=ports[i];const auto& next=samples.front().ports[i];
            if(next.connected) {
                p.pressed|=uint16_t(next.held&~p.held);
                p.direction_pressed|=uint16_t(next.directions&~p.directions);
            } else p.pressed=p.direction_pressed=0;
            p.held=next.held;p.directions=next.directions;p.x=next.x;p.y=next.y;p.connected=next.connected;
        }
        samples.pop_front();
    }
}
}
void native_input_reset() {++epoch;running=false;ports={};samples.clear();sampled_at=0;}
void native_input_end(uint8_t* rdram) {
    if(!running)return;
    accept(std::numeric_limits<Nanoseconds>::max());
    for(unsigned i=0;i<ports.size();++i) {
        const auto& p=ports[i];
        // Reconcile the original accumulator/history before it resumes. An
        // edge consumed by native must not be replayed by the next VI latch;
        // an edge still pending belongs to that next original update.
        MEM_H(int32_t(2*i),int32_t(0x801A9548))=int16_t(p.held&~p.pressed);
        MEM_H(int32_t(2*i),int32_t(0x801A44E0))=int16_t(p.held|p.pressed);
        MEM_H(int32_t(2*i),int32_t(0x801A9AC8))=int16_t(p.directions&~p.direction_pressed);
        MEM_H(int32_t(2*i),int32_t(0x801AA610))=int16_t(p.directions|p.direction_pressed);
    }
    native_input_reset();
}
void native_input_begin(uint8_t* ram,Nanoseconds now) {
    native_input_reset();running=true;ports=read(ram);sampled_at=now;
    for(auto& port:ports) {port.consumed_held=port.held;port.consumed_directions=port.directions;}
}
extern "C" void dora3_native_input_sample(uint8_t* ram,recomp_context*) {
    if(!running)return;
    const auto now=dora3::native_time::monotonic_time();
    if(now<sampled_at)throw std::logic_error("Input sample time moved backwards");
    sampled_at=now;samples.push_back({now,read(ram)});
}
Nanoseconds native_input_next_sample(Nanoseconds after,Nanoseconds before) {
    for(const auto& sample:samples)if(sample.time>after && sample.time<before)return sample.time;
    return before;
}
NativeInputScope::NativeInputScope(uint8_t* rdram):NativeInputScope(rdram,std::numeric_limits<Nanoseconds>::max()) {}
NativeInputScope::NativeInputScope(uint8_t* rdram,Nanoseconds time):ram_(rdram),epoch_(epoch) {
    if(!running || scoped)throw std::logic_error("Invalid native input ownership");
    scoped=true;
    accept(time);
    for(unsigned i=0;i<ports.size();++i) {
        auto& port=ports[i];
        const std::array<uint16_t,9> values{port.pressed,port.direction_pressed,uint16_t(port.connected),
            port.consumed_held,port.held,port.pressed,port.consumed_directions,port.directions,port.direction_pressed};
        for(unsigned f=0;f<fields.size();++f) {
            words_[i*fields.size()+f]=MEM_H(int32_t(2*i),int32_t(fields[f]));
            MEM_H(int32_t(2*i),int32_t(fields[f]))=int16_t(values[f]);
        }
        for(unsigned axis=0;axis<axes.size();++axis) {
            sticks_[i*axes.size()+axis]=MEM_B(int32_t(i),int32_t(axes[axis]));
            MEM_B(int32_t(i),int32_t(axes[axis]))=axis?port.y:port.x;
        }
        port.consumed_held=port.held;port.consumed_directions=port.directions;
        port.pressed=port.direction_pressed=0;
    }
}
NativeInputScope::~NativeInputScope() {
    uint8_t* rdram=ram_;
    // A replacement scene owns its new input state; never roll it back.
    if(epoch_==epoch)for(unsigned i=0;i<ports.size();++i) {
        for(unsigned f=0;f<fields.size();++f)MEM_H(int32_t(2*i),int32_t(fields[f]))=words_[i*fields.size()+f];
        for(unsigned axis=0;axis<axes.size();++axis)MEM_B(int32_t(i),int32_t(axes[axis]))=sticks_[i*axes.size()+axis];
    }
    scoped=false;
}
