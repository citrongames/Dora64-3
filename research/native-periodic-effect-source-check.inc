#ifdef DORA3_PERIODIC_EFFECT_SOURCE_CHECK
static void check_periodic_effect_source(const std::vector<uint32_t>& image) {
    std::map<std::array<unsigned,4>,std::vector<FlashPoolSnapshot>> oracles;
    unsigned source_cases=0,native_cases=0,endpoints=0,intervals=0,source_births=0,motion_samples=0;
    for(unsigned playback:{0u,1u})for(unsigned camera:{12u,2u})for(unsigned divider:{2u,3u})
    for(unsigned fps:{60u,73u,120u,144u,165u,360u})for(unsigned height:{80u,250u,400u}) {
        auto memory=image;auto* rdram=reinterpret_cast<uint8_t*>(memory.data());seed(rdram,0,true);
        std::memset(rdram+(0x80171CD0&0xFFFFFF),0,230*0xCC);
        MEM_W(0x94,int32_t(player))=1;MEM_W(0,int32_t(0x8021D640))=int32_t(other);
        wf(rdram,player+0xA0,0);wf(rdram,physics+0x14,0);MEM_W(0x30,int32_t(control))=-1;
        wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;MEM_H(0x18,int32_t(control))=0;
        MEM_W(0,int32_t(0x801B36E0))=0;MEM_B(0,int32_t(0x801B36EC))=0;
        for(unsigned i=0;i<8;++i) {
            const auto slot=int32_t(0x801A44F0+i*20);MEM_W(0,slot)=i<2?2:0;
            MEM_W(12,slot)=i;MEM_W(16,slot)=int32_t(i?0x801BB848:0x801BB730);
        }
        for(auto a:{0x8021D484u,0x8021D494u,0x80218AD8u,0x801A9C88u})MEM_W(0,int32_t(a))=0;
        MEM_W(0,int32_t(0x801AA648))=MEM_W(0,int32_t(0x801A9500))=1;
        MEM_B(0,int32_t(0x801A48B4))=1;MEM_H(0,int32_t(0x80171560))=0;
        MEM_B(2,int32_t(0x80171560))=MEM_B(3,int32_t(0x80171560))=MEM_B(4,int32_t(0x80171560))=0;
        recomp_context c{};c.r29=int32_t(0x80300000);c.f_odd=&c.f0.u32h;c.r4=1;
        game_model_animation().reset();allocations.clear();resource_references={};next_address=0x802A0000;
        const auto owners=models(rdram,c);c.r4=1;wf(rdram,params+0x10,15);
        MEM_W(0,int32_t(0x801A39DC))=int(divider);MEM_B(0,int32_t(0x801B361A))=int8_t(camera);
        c.f12.fl=rf(rdram,player+0x9C);c.f14.fl=rf(rdram,player+0xA0);c.r6=MEM_W(0xA4,int32_t(player));
        dora3_s3_801BBDDC(rdram,&c);if(camera==2)dora3_s3_801D7134(rdram,&c);
        c.r4=1;const auto caller=c;
        native_gameplay_preview_reset(rdram);preview_live=false;
        room(rdram);city_loaded=false;slot09_loaded=linked_construct_watch=linked_watch=true;
        city_watch=patrol_watch=city_dialogue_watch=false;city_commands.clear();city_source_initial_stamp.reset();
        std::copy(slot09_overlay_words.begin(),slot09_overlay_words.end(),memory.begin()+slot09_overlay_vram_offset/4);
        MEM_W(0,int32_t(0x801AEC00+20))=int32_t(asset);MEM_W(0,int32_t(0x801AEC00+40))=int32_t(clip);
        MEM_W(0,int32_t(0x801AEC00+80))=int32_t(0x80102000u);
        for(unsigned slot=30;slot<230;++slot) {
            c.r4=int32_t(0x80171CD0u+slot*0xCC);dora3_scene_object_init(rdram,&c);MEM_H(0xA8,c.r4)=-1;
        }
        const auto before_heap=allocations;const auto before_refs=resource_references;
        const auto before_models=game_model_animation().model_count();assert(before_models==2);
        input_now=0;city_source_stamp={0,0};flash_pool_reference=playback==0;
        if(playback==1) {
            assert(native_gameplay_preview_eligible(rdram));preview_live=true;
            native_gameplay_preview_begin(rdram,0,divider);dora3_native_preview_hook(rdram,&c,0x801BB080);
        }
        c=caller;c.f_odd=&c.f0.u32h;c.r4=int32_t(0x801FC964u);
        c.r5=std::bit_cast<int32_t>(500.f);c.r6=std::bit_cast<int32_t>(float(height));c.r7=std::bit_cast<int32_t>(-300.f);
        MEM_W(0x10,int32_t(c.r29))=0;dora3_s3_801E6968(rdram,&c);
        const auto effect=uint32_t(c.r2);assert(effect==0x801734B8u);
        const auto ph=uint32_t(MEM_W(0x98,int32_t(effect))),model=uint32_t(MEM_W(0x74,int32_t(effect)));
        assert(MEM_H(0x20,int32_t(ph))==131&&MEM_W(0x94,int32_t(effect))==1);
        assert(game_model_animation().lifetime_token(model));if(playback==1){assert(game_model_animation().native_birth(model));++source_births;}
        auto snapshot=[&] {
            FlashPoolSnapshot value{{},city_commands,allocations,resource_references,game_model_animation().model_count()};
            value.words.push_back(MEM_W(0x94,int32_t(effect)));
            if(MEM_W(0x94,int32_t(effect)))for(auto [a,size]:std::array<std::pair<uint32_t,unsigned>,3>{{{effect,0xCC},{ph,0xF4},{model,0x84}}})
                for(unsigned off=0;off<size;off+=4)value.words.push_back(MEM_W(int32_t(off),int32_t(a)));
            return value;
        };
        unsigned frame=1;bool retired=false;
        for(unsigned tick=1;tick<=96;++tick) {
            const auto units=uint64_t(second)*divider*tick;const auto boundary=Nanoseconds((units+59)/60);
            c=caller;c.f_odd=&c.f0.u32h;
            if(playback==1) {
                auto native_to=[&](Nanoseconds stamp) {
                    auto command_snapshot=[&] {
                        auto result=snapshot();
                        if(MEM_W(0x94,int32_t(effect))) {
                            result.words.at(1+0xA0/4)=0;
                            // Common publication marker2A is cleared by the
                            // genuine common tail; private latch28 stays fixed.
                            result.words.at(1+0xCC/4+0x28/4)&=~int32_t(0x0000FF00u);
                            result.words.at(1+0xCC/4+0xF4/4+0x64/4)=0;
                            result.words.at(1+0xCC/4+0xF4/4+0x7C/4)&=~int32_t(0x02000000u);
                        }
                        return result;
                    };
                    const auto before=command_snapshot();const auto context=c;input_now=stamp;
                    const auto old_y=MEM_W(0xA0,int32_t(effect));
                    native_gameplay_preview_update(rdram,&c,stamp);
                    if(MEM_W(0x94,int32_t(effect))&&MEM_W(0xA0,int32_t(effect))!=old_y)++motion_samples;
                    const auto after=command_snapshot();
                    if(after!=before) {
                        std::cerr<<"Native periodic command mutation fps="<<fps<<" divider="<<divider<<" height="<<height<<" tick="<<tick<<" stamp="<<stamp<<'\n';
                        for(unsigned i=0;i<std::min(after.words.size(),before.words.size());++i)
                            if(after.words[i]!=before.words[i])std::cerr<<" word="<<i<<" before="<<std::hex<<before.words[i]<<" after="<<after.words[i]<<std::dec<<'\n';
                    }
                    assert(after==before&&std::memcmp(&context,&c,sizeof(c))==0);++intervals;
                };
                while(second*frame/fps<boundary){native_to(second*frame/fps);++frame;}native_to(boundary);
            }
            input_now=boundary;city_source_stamp={Nanoseconds(units/60),unsigned(units%60)};
            c=caller;c.f_odd=&c.f0.u32h;
            if(playback==0)reference_dora3_s3_801E8BDC(rdram,&c);else dora3_s3_801E8BDC(rdram,&c);
            const auto value=snapshot();const auto key=std::array<unsigned,4>{fps,divider,camera,height};
            if(playback==0)oracles[key].push_back(value);else {
                if(value!=oracles.at(key).at(tick-1)) {
                    std::cerr<<"Periodic effect Source mismatch fps="<<fps<<" divider="<<divider<<" camera="<<camera<<" height="<<height<<" tick="<<tick<<'\n';
                    for(unsigned i=0;i<std::min(value.words.size(),oracles.at(key).at(tick-1).words.size());++i)
                        if(value.words[i]!=oracles.at(key).at(tick-1).words[i])std::cerr<<" word="<<i<<" actual="<<std::hex<<value.words[i]<<" expected="<<oracles.at(key).at(tick-1).words[i]<<std::dec<<'\n';
                    std::abort();
                }
                ++endpoints;
            }
            if(!MEM_W(0x94,int32_t(effect)))retired=true;
        }
        assert(retired&&allocations==before_heap&&resource_references==before_refs&&game_model_animation().model_count()==before_models);
        native_gameplay_preview_reset(rdram);preview_live=false;slot09_loaded=linked_construct_watch=linked_watch=false;
        for(auto m:owners){c.r4=int32_t(m);dora3_s0_8003B77C(rdram,&c);}
        if(playback==0)++source_cases;else ++native_cases;
    }
    assert(source_cases==72&&native_cases==72&&endpoints==72*96&&source_births==72&&intervals>10000&&motion_samples>72);
    std::cout<<"PASS: 72 Original and72 mixed Native/Source periodic effect131 descriptor/lifetime schedules; "<<intervals
        <<" Native intervals retain private Source commands with "+std::to_string(motion_samples)+" intermediate Y changes; "<<endpoints
        <<" exact live actor/physics/model/heap/resource/command frozen Original endpoints; actual descriptor, first Source init/RNG/clip/seek30/scale1.5/Y velocity-5 and genuine retirement at pre-step Y<=50; Native pure initialized kind131 drift/model time with ordered Source retirement; no game/GPU.\n";
}
#endif
