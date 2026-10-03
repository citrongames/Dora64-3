#ifdef DORA3_STORY_EFFECT_DELAY_CHECK
extern "C" void reference_dora3_s3_core_801ED4AC(uint8_t*,recomp_context*);
static unsigned flash_delay_cases=0,flash_delay_intervals=0,flash_delay_duplicates=0;
struct FlashDelaySnapshot {
    std::vector<int32_t> words;
    std::vector<CityCommand> commands;
    bool operator==(const FlashDelaySnapshot&)const=default;
};
static FlashDelaySnapshot flash_delay_snapshot(uint8_t* rdram,uint32_t actor) {
    const auto physical=uint32_t(MEM_W(0x98,int32_t(actor)));
    const auto model=uint32_t(MEM_W(0x74,int32_t(actor)));
    assert(model&&game_model_animation().lifetime_token(model));
    FlashDelaySnapshot value{{},city_commands};
    for(auto [address,size]:std::array<std::pair<uint32_t,unsigned>,3>{{{actor,0xCC},{physical,0xF4},{model,0x84}}})
        for(unsigned offset=0;offset<size;offset+=4)value.words.push_back(MEM_W(int32_t(offset),int32_t(address)));
    return value;
}
[[maybe_unused]] static void check_flash_delay(uint8_t* rdram,recomp_context caller,unsigned fps,unsigned divider) {
    const auto baseline_heap=allocations;const auto baseline_refs=resource_references;
    const auto baseline_models=game_model_animation().model_count();const auto baseline_next=next_address;
    for(unsigned selector=0;selector<6;++selector) {
        std::vector<FlashDelaySnapshot> oracle;
        auto create=[&] {
            caller.r4=int32_t(0x801FC950u);caller.r5=-500;caller.r6=80;caller.r7=-300;
            MEM_W(0x10,caller.r29)=0;dora3_s3_801E6968(rdram,&caller);
            const auto actor=uint32_t(caller.r2),ph=uint32_t(MEM_W(0x98,int32_t(actor)));
            assert(actor>=prop&&MEM_H(0x20,int32_t(ph))==130&&MEM_W(0x94,int32_t(actor))==1);
            MEM_H(0x44,int32_t(ph))=int16_t(selector);
            MEM_W(0,int32_t(0x8021BDEC))=int32_t(actor);MEM_W(0,int32_t(0x8021BCDC))=int32_t(ph);
            city_commands.clear();return actor;
        };
        auto retire=[&](uint32_t actor) {
            caller.r4=int32_t(actor);dora3_s3_801E656C(rdram,&caller);
            caller.r4=int32_t(actor);caller.r5=0xFFFF;dora3_s0_8002EA4C(rdram,&caller);
            caller.r4=int32_t(actor);dora3_scene_object_init(rdram,&caller);
            assert(allocations==baseline_heap&&resource_references==baseline_refs&&game_model_animation().model_count()==baseline_models);
        };
        const auto actor=create();
        for(unsigned tick=1;MEM_W(0x94,int32_t(actor))==1;++tick) {
            assert(tick<=3);const auto units=uint64_t(second)*divider*tick;
            input_now=Nanoseconds((units+59)/60);city_source_stamp={Nanoseconds(units/60),unsigned(units%60)};
            reference_dora3_s3_core_801ED4AC(rdram,&caller);oracle.push_back(flash_delay_snapshot(rdram,actor));
        }
        retire(actor);next_address=baseline_next;
        preview_live=true;native_gameplay_preview_begin(rdram,0,divider);
        const auto actual=create();assert(actual==actor);
        SimulationTimeline timeline(divider);Nanoseconds cursor=0;unsigned frame=1;
        while(timeline.ticks()<oracle.size()) {
            const auto stamp=Nanoseconds(second*frame/fps);++frame;
            const auto remaining=Nanoseconds((uint64_t(second)*divider*oracle.size()+59)/60)-cursor;
            const auto elapsed=std::min(stamp-cursor,remaining);assert(elapsed>0);
            timeline.advance_while(elapsed,[&](const SimulationSlice& slice) {
                const auto tick=slice.tick;assert(tick<oracle.size());
                const auto units=uint64_t(second)*divider*tick+slice.phase+slice.numerator;
                input_now=Nanoseconds((units+59)/60);city_source_stamp={Nanoseconds(uint64_t(second)*divider*(tick+1)/60),unsigned((uint64_t(second)*divider*(tick+1))%60)};
                const auto before_count=MEM_H(0x34,MEM_W(0x98,int32_t(actual)));
                SimulationScope scope(slice);dora3_s3_core_801ED4AC(rdram,&caller);++flash_delay_intervals;
                if(slice.ends_tick()) {
                    const auto value=flash_delay_snapshot(rdram,actual);
                    if(value!=oracle[tick]) {
                        std::cerr<<"Flash delay mismatch fps="<<fps<<" divider="<<divider<<" selector="<<selector<<" tick="<<tick<<" commands="<<value.commands.size()<<"/"<<oracle[tick].commands.size()<<'\n';
                        for(unsigned i=0;i<value.words.size();++i)if(value.words[i]!=oracle[tick].words[i])std::cerr<<" word="<<i<<" actual="<<std::hex<<value.words[i]<<" expected="<<oracle[tick].words[i]<<std::dec<<'\n';
                        std::abort();
                    }
                    if(MEM_W(0x94,int32_t(actual))==1) {
                        const auto before=flash_delay_snapshot(rdram,actual);
                        dora3_s3_core_801ED4AC(rdram,&caller);
                        assert(flash_delay_snapshot(rdram,actual)==before);++flash_delay_duplicates;
                    }
                } else assert(MEM_H(0x34,MEM_W(0x98,int32_t(actual)))==(slice.phase==0&&tick==0?0:before_count));
            },[](uint64_t){},[&] {return timeline.ticks()<oracle.size();});
            cursor+=elapsed;
        }
        assert(MEM_W(0x94,int32_t(actual))==10);retire(actual);
        native_gameplay_preview_reset(rdram);preview_live=false;next_address=baseline_next;++flash_delay_cases;
    }
}
#endif
