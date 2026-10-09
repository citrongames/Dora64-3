#pragma once
#include <cstdint>
#include <filesystem>

namespace dora3::profile {
enum class Tag : unsigned {
    NativeUpdate, NativeFrame, WorldSourceUpdate, WorldGameplay, ActorManager,
    CommonActors, Contacts, Player, Projectiles, WorldDraw, WorldGeometry,
    ModelAdvance, ModelEvaluate, FrameCommands, TaskSubmit, NativeModels,
    TextureBind, TraceSample, DistanceAdmission, RT64Parse, RT64Screen,
    RT64RenderCPU, RT64GPU, RT64GPUWait, RT64UploadWait, RT64Workload,
    PresentSwap, PresentWorkloadWait, EnemyCallback, ObjectTimeline, ModelDraw, ModelRatePlan, ModelClipBoundaries, ObjectClockRegistration,
    PreviewHookSample, EnemyAxisSample, CommonOwnershipSample, EnemyIdentitySample,
    OverlayLookupSample, EnemyPoolFinalize, NativeModelClockSample, GameplayEligibilitySample, Count
};
#if defined(DORA3_PROFILE) || defined(DORA3_PROFILE_IMPLEMENTATION)
void configure(const std::filesystem::path& path);
uint64_t begin(Tag tag);
void end(uint64_t ticket);
void record(Tag tag,uint64_t nanoseconds);
void capture_context(uint8_t* ram);
void output_frame();
void guest_frame();
void graphics_counts(uint64_t vertices,uint64_t faces,uint64_t matrices);
void report();
bool sample_trace();
bool sample_stage(Tag tag,unsigned stride);
void timeline_gap(uint64_t nanoseconds);
void timeline_input_segment(bool input_edge);
void timeline_slice(uint64_t units);
void timeline_limit(unsigned reason,uint64_t units);
#else
inline void configure(const std::filesystem::path&){}
inline uint64_t begin(Tag){return 0;}
inline void end(uint64_t){}
inline void record(Tag,uint64_t){}
inline void capture_context(uint8_t*){}
inline void output_frame(){}
inline void guest_frame(){}
inline void graphics_counts(uint64_t,uint64_t,uint64_t){}
inline void report(){}
inline bool sample_trace(){return false;}
inline bool sample_stage(Tag,unsigned){return false;}
inline void timeline_gap(uint64_t){}
inline void timeline_input_segment(bool){}
inline void timeline_slice(uint64_t){}
inline void timeline_limit(unsigned,uint64_t){}
#endif
class Scope {
    uint64_t ticket_;
public:
    explicit Scope(Tag tag,bool active=true):ticket_(active?begin(tag):0){}
    ~Scope(){end(ticket_);}
    Scope(const Scope&)=delete;
    Scope& operator=(const Scope&)=delete;
};
}
extern "C" uint64_t dora3_profile_begin(unsigned tag);
extern "C" void dora3_profile_end(uint64_t ticket);
extern "C" void dora3_profile_context(uint8_t* ram);
