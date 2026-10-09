#pragma once
#include <cstdint>
#include <filesystem>
namespace dora3::graphics_profile {
// Coarse frame stages plus1/64 wall-only samples of selected hot functions.
enum class Stage : unsigned {
 NativeUpdate, NativeFrame, Producer, Screen, HLE, RDRAMCheck, FullSync,
 Flush, TileValidation, CallPreparation, DrawUpload, EagerPath, Tiles,
 EagerSetup, TargetSafety, EagerRender, ProducerUploadWait, ProducerGPUWait,
 TextureWait, CPUReadback, QueueAdvance, Inspect, Materialize, MaterializeWait,
 Workload, Render, RenderUploadWait, RenderGPUWait, Present, PresentWait,
 VertexSample, TriangleSample, MatrixSample, DrawStateSample, FlushSample,
 LoadPlanningSample, TMEMLoadSample, TextureUploadSample, TextureHashSample,
 ObjectTimelineSample, ObjectRegistrationSample, ModelRatePlanSample,
 EnemyCallbackSample, EnemyFinalizeSample, ModelBoundarySample, ModelPoseSample, NativeCommon, NativePlayer, FrameBuild,
 VertexStatePart, VertexAppendPart, VertexOverridePart, VertexTransformPart, VertexTexcoordPart,
 TriangleStatePart, TriangleBookkeepingPart, TriangleBoundsPart, AudioTask, GraphicsRAMWait, TextureEvict, UploadCommands, Count
};
enum class Counter : unsigned {
 Output, Guest, Vertices, Faces, Matrices, Pairs, Tiles, RenderGPUTimestampNS,
 GateNotNativeBuild, GateNotGameplay, GateDimensions, GateCPUFramebuffer,
 RequestOff, NoPairs, RejectOperations, RejectFormat, RejectHeight,
 RejectDepthClear, RejectColorAddress, RejectDepthAddress, RejectTileCopy,
 NoWrites, Deferred, Eager, Materializations, HashMemoHit, HashMemoMiss, HashMemoBypass, Count
};
enum class Mark : unsigned { TaskSubmit, ParseStart, WakePost, WakeReceive, FrameTicket, GuestDone, HostDone, WakeLateness, ExternalQueued, ExternalDelivered, ExternalBlocked, SchedulerReceived, AudioSubmit };
#ifdef DORA3_GRAPHICS_PROFILE
void mark(Mark,uint64_t correlation,uint64_t argument=0);
void configure(const std::filesystem::path&);
void report();
void context(unsigned scene,unsigned fps,unsigned mode,float distance);
void add(Counter,uint64_t value=1);
uint64_t begin(Stage);
uint64_t begin_sampled(Stage);
uint64_t begin_part(Stage,bool);
void end(uint64_t);
class Scope {
 uint64_t ticket_;
public:
 explicit Scope(Stage stage):ticket_(begin(stage)){}
 ~Scope(){finish();}
 void finish(){const auto ticket=ticket_;ticket_=0;end(ticket);}
 Scope(const Scope&)=delete;
 Scope& operator=(const Scope&)=delete;
};
#ifdef DORA3_GRAPHICS_FINE_PROFILE
class SampleScope {
 uint64_t ticket_;
public:
 explicit SampleScope(Stage stage):ticket_(begin_sampled(stage)){}
 ~SampleScope(){finish();}
 bool active() const {return ticket_!=0;}
 void finish(){const auto ticket=ticket_;ticket_=0;end(ticket);}
 SampleScope(const SampleScope&)=delete;
 SampleScope& operator=(const SampleScope&)=delete;
};
class PartScope {
 uint64_t ticket_;
public:
 PartScope(Stage stage,bool active):ticket_(active?begin_part(stage,true):0){}
 ~PartScope(){finish();}
 void finish(){const auto ticket=ticket_;ticket_=0;end(ticket);}
 PartScope(const PartScope&)=delete;
 PartScope& operator=(const PartScope&)=delete;
};
#else
//087 isolates coarse scheduling without hot-function sampling overhead.
class SampleScope {public: explicit SampleScope(Stage){} bool active()const{return false;} void finish(){};};
class PartScope {public: PartScope(Stage,bool){} void finish(){};};
#endif
#else
inline void mark(Mark,uint64_t,uint64_t=0){}
inline void report(){}
inline void context(unsigned,unsigned,unsigned,float){}
inline void add(Counter,uint64_t=1){}
class Scope {
public:
 explicit Scope(Stage){}
 bool active() const {return false;}
 void finish(){}
};
using SampleScope=Scope;
class PartScope {public: PartScope(Stage,bool){} void finish(){};};
#endif
}
