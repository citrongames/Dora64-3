#ifdef DORA3_STORY_EFFECT_MOTION_CHECK
extern "C" void reference_dora3_s3_core_801ED5F4(uint8_t*,recomp_context*);
extern "C" void reference_flash_common_tail(uint8_t*,recomp_context*);
extern "C" void native_flash_common_tail(uint8_t*,recomp_context*);
static unsigned flash_motion_cases=0,flash_motion_intervals=0,flash_motion_endpoints=0,flash_motion_samples=0;
struct FlashMotionSnapshot {
    std::vector<int32_t> words;
    std::vector<CityCommand> commands;
    decltype(allocations) heap;
    decltype(resource_references) references;
    std::size_t models;
    bool operator==(const FlashMotionSnapshot&)const=default;
};
static FlashMotionSnapshot flash_motion_snapshot(uint8_t* rdram,uint32_t actor) {
    FlashMotionSnapshot value{{MEM_W(0x94,int32_t(actor))},city_commands,allocations,resource_references,game_model_animation().model_count()};
    if(!MEM_W(0x94,int32_t(actor)))return value; // Freed model/physics bytes are no longer observable owners.
    const auto physical=uint32_t(MEM_W(0x98,int32_t(actor)));
    const auto model=uint32_t(MEM_W(0x74,int32_t(actor)));
    assert(model&&game_model_animation().lifetime_token(model));
    for(auto [address,size]:std::array<std::pair<uint32_t,unsigned>,3>{{{actor,0xCC},{physical,0xF4},{model,0x84}}})
        for(unsigned offset=0;offset<size;offset+=4)value.words.push_back(MEM_W(int32_t(offset),int32_t(address)));
    return value;
}
static void check_flash_motion(uint8_t* rdram,recomp_context caller,unsigned fps,unsigned divider) {
    const auto baseline_heap=allocations;const auto baseline_refs=resource_references;
    const auto baseline_models=game_model_animation().model_count();const auto baseline_next=next_address;
    for(unsigned selector=0;selector<6;++selector)for(unsigned source_prefix:{0u,3u}) {
        std::vector<FlashMotionSnapshot> oracle;
        auto stamp=[&](unsigned tick) {
            const auto units=uint64_t(second)*divider*tick;
            input_now=Nanoseconds((units+59)/60);city_source_stamp={Nanoseconds(units/60),unsigned(units%60)};
        };
        auto create=[&] {
            caller.r4=int32_t(0x801FC950u);caller.r5=-500;caller.r6=80;caller.r7=-300;
            MEM_W(0x10,caller.r29)=0;dora3_s3_801E6968(rdram,&caller);
            const auto actor=uint32_t(caller.r2),physical=uint32_t(MEM_W(0x98,int32_t(actor)));
            assert(MEM_H(0x20,int32_t(physical))==130&&MEM_W(0x94,int32_t(actor))==1);
            MEM_H(0x44,int32_t(physical))=int16_t(selector);
            MEM_W(0,int32_t(0x8021BDEC))=int32_t(actor);MEM_W(0,int32_t(0x8021BCDC))=int32_t(physical);
            for(unsigned tick=1;MEM_W(0x94,int32_t(actor))==1;++tick) {assert(tick<=3);stamp(tick);reference_dora3_s3_core_801ED4AC(rdram,&caller);}
            for(unsigned tick=1;tick<=source_prefix;++tick) {stamp(tick);reference_dora3_s3_core_801ED5F4(rdram,&caller);reference_flash_common_tail(rdram,&caller);}
            city_commands.clear();return actor;
        };
        preview_live=false;
        const auto actor=create();
        for(unsigned tick=1;MEM_W(0x94,int32_t(actor));++tick) {
            assert(tick<64);stamp(tick);
            reference_dora3_s3_core_801ED5F4(rdram,&caller);reference_flash_common_tail(rdram,&caller);
            oracle.push_back(flash_motion_snapshot(rdram,actor));
        }
        assert(allocations==baseline_heap&&resource_references==baseline_refs&&game_model_animation().model_count()==baseline_models);
        next_address=baseline_next;preview_live=true;native_gameplay_preview_begin(rdram,0,divider);
        const auto actual=create();assert(actual==actor);
        SimulationTimeline timeline(divider);Nanoseconds cursor=0;unsigned frame=1;
        while(timeline.ticks()<oracle.size()) {
            const auto stamp_time=Nanoseconds(second*frame/fps);++frame;
            const auto remaining=Nanoseconds((uint64_t(second)*divider*oracle.size()+59)/60)-cursor;
            const auto elapsed=std::min(stamp_time-cursor,remaining);assert(elapsed>0);
            timeline.advance_while(elapsed,[&](const SimulationSlice& slice) {
                const auto tick=slice.tick;assert(tick<oracle.size());
                const auto units=uint64_t(second)*divider*tick+slice.phase+slice.numerator;
                input_now=Nanoseconds((units+59)/60);
                city_source_stamp={Nanoseconds(uint64_t(second)*divider*(tick+1)/60),unsigned((uint64_t(second)*divider*(tick+1))%60)};
                const auto previous_y=rf(rdram,actual+0xA0);
                SimulationScope scope(slice);
                dora3_s3_core_801ED5F4(rdram,&caller);native_flash_common_tail(rdram,&caller);++flash_motion_intervals;
                if(slice.ends_tick()) {
                    const auto value=flash_motion_snapshot(rdram,actual);
                    if(value!=oracle[tick]) {
                        std::cerr<<"Flash motion mismatch fps="<<fps<<" divider="<<divider<<" selector="<<selector<<" prefix="<<source_prefix<<" tick="<<tick<<" commands="<<value.commands.size()<<"/"<<oracle[tick].commands.size()<<" words="<<value.words.size()<<"/"<<oracle[tick].words.size()<<'\n';
                        for(unsigned i=0;i<std::min(value.words.size(),oracle[tick].words.size());++i)
                            if(value.words[i]!=oracle[tick].words[i])std::cerr<<" word="<<i<<" actual="<<std::hex<<value.words[i]<<" expected="<<oracle[tick].words[i]<<std::dec<<'\n';
                        std::abort();
                    }
                    ++flash_motion_endpoints;
                } else if(MEM_W(0x94,int32_t(actual))) {
                    assert(std::isfinite(rf(rdram,actual+0x9C))&&std::isfinite(rf(rdram,actual+0xA0))&&std::isfinite(rf(rdram,actual+0xA4)));
                    if(rf(rdram,actual+0xA0)!=previous_y)++flash_motion_samples;
                }
            },[](uint64_t){},[&] {return timeline.ticks()<oracle.size();});
            cursor+=elapsed;
        }
        assert(!MEM_W(0x94,int32_t(actual))&&allocations==baseline_heap&&resource_references==baseline_refs&&game_model_animation().model_count()==baseline_models);
        native_gameplay_preview_reset(rdram);preview_live=false;next_address=baseline_next;++flash_motion_cases;
    }
}
#endif
