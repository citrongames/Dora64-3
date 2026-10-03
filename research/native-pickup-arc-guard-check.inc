#ifdef DORA3_PICKUP_ARC_GUARD_CHECK
static void check_pickup_arc_guards(const std::vector<uint32_t>& image) {
    std::map<std::array<unsigned,7>,std::vector<FlashPoolSnapshot>> oracles;
    unsigned source_cases=0,native_cases=0,endpoints=0,intervals=0,source_births=0,spin_yaw=0,spin_rate=0,spin_alpha=0,spin_states=0,arc_phases=0,arc_returns=0;
    unsigned arc_first_xyz=0,arc_second_xz=0,arc_scale=0,arc_rejected_intervals=0;
    unsigned rejected_worlds=0;
    unsigned idle_yaw_samples=0,collected_yaw_samples=0,flash_height_samples=0,periodic_height_samples=0,rise_samples=0,shrink_samples=0;
    for(unsigned playback:{0u,1u})for(unsigned guard:{0u,1u,2u,3u,4u,5u,6u,7u})for(unsigned placement:{0u,1u})for(unsigned camera:{12u,2u})for(unsigned divider:{2u,3u})
    for(unsigned fps:{60u,73u,120u,144u,165u,360u})for(unsigned target:{45u,120u,270u})for(unsigned item_kind:{118u,123u,127u})for(unsigned initial_state:{8u}) {
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
        room(rdram);
        // The shared player fixture's ceiling is250; the explicit height400
        // item starts outside that room. Keep the genuine floor/collision code,
        // but place the ceiling above all three producer trajectories.
        for(unsigned n=2;n<4;++n)wf(rdram,0x80280000u+0x20+n*44+36,-1000.f);
        city_loaded=false;slot09_loaded=linked_construct_watch=linked_watch=true;
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
        c=caller;c.f_odd=&c.f0.u32h;c.r4=int32_t(item_kind==118?0x801FC818u:item_kind==123?0x801FC894u:0x801FC904u);
        c.r5=std::bit_cast<int32_t>(500.f);c.r6=std::bit_cast<int32_t>(400.f);c.r7=std::bit_cast<int32_t>(-300.f);
        MEM_W(0x10,int32_t(c.r29))=0;dora3_s3_801E6968(rdram,&c);
        const auto effect=uint32_t(c.r2);assert(effect==0x801734B8u);
        const auto ph=uint32_t(MEM_W(0x98,int32_t(effect))),model=uint32_t(MEM_W(0x74,int32_t(effect)));
        assert(MEM_H(0x20,int32_t(ph))==int16_t(item_kind)&&MEM_W(0x94,int32_t(effect))!=0);
        MEM_W(0x94,int32_t(effect))=int(initial_state);MEM_B(0x28,int32_t(ph))=0;
        // Explicit state/input parameters; actual descriptor ctor and state
        // initializers remain Original. The state8 entry sets latch28=1.
        MEM_H(0xBE,int32_t(ph))=320;MEM_H(0xC0,int32_t(ph))=400;MEM_H(0x44,int32_t(ph))=int16_t(target);
        wf(rdram,effect+0x18,3.5f);wf(rdram,effect+0x1C,3.5f);wf(rdram,effect+0x20,3.5f);
        assert(game_model_animation().lifetime_token(model));if(playback==1){assert(game_model_animation().native_birth(model));++source_births;}
        auto snapshot=[&] {
            FlashPoolSnapshot value{{},city_commands,allocations,resource_references,game_model_animation().model_count()};
            for(unsigned slot=30;slot<230;++slot) {
                const auto actor=0x80171CD0u+slot*0xCC;value.words.push_back(MEM_W(0x94,int32_t(actor)));
                if(!MEM_W(0x94,int32_t(actor)))continue;
                const auto physical=uint32_t(MEM_W(0x98,int32_t(actor))),m=uint32_t(MEM_W(0x74,int32_t(actor)));
                const auto kind=MEM_H(0x20,int32_t(physical));assert(kind==int16_t(item_kind)||kind==130||kind==131);
                assert(game_model_animation().lifetime_token(m));
                for(auto [a,size]:std::array<std::pair<uint32_t,unsigned>,3>{{{actor,0xCC},{physical,0xF4},{m,0x84}}})
                    for(unsigned off=0;off<size;off+=4)value.words.push_back(MEM_W(int32_t(off),int32_t(a)));
            }
            return value;
        };
        auto source_owned=[&] {
            std::vector<int32_t> words;
            for(unsigned slot=30;slot<230;++slot) {
                const auto actor=0x80171CD0u+slot*0xCC;const auto state=MEM_W(0x94,int32_t(actor));if(!state)continue;
                const auto physical=uint32_t(MEM_W(0x98,int32_t(actor))),m=uint32_t(MEM_W(0x74,int32_t(actor)));
                const auto kind=MEM_H(0x20,int32_t(physical));
                const bool idle=kind==int16_t(item_kind)&&state==1&&MEM_W(0x10,int32_t(physical))==0&&MEM_W(0x14,int32_t(physical))==0&&MEM_W(0x18,int32_t(physical))==0;
                if(idle||(kind==int16_t(item_kind)&&state==10&&MEM_BU(0x28,int32_t(physical))==1)||(kind==130&&state==10))continue;
                for(auto [a,size]:std::array<std::pair<uint32_t,unsigned>,3>{{{actor,0xCC},{physical,0xF4},{m,0x84}}})
                    for(unsigned off=0;off<size;off+=4) {
                        auto value=MEM_W(int32_t(off),int32_t(a));
#ifdef DORA3_PICKUP_ARC_NATIVE_CHECK
                        if(kind==int16_t(item_kind)&&state==8) {
                            if((a==actor&&(off==0x9C||off==0xA0||off==0xA4||off==0x18||off==0x1C||off==0x20))||
                               (a==physical&&off==0xC4)||(a==m&&(off==0x64||off==0x74||off==0x78)))value=0;
                            if(a==physical&&off==0x28)value&=~int32_t(0x0000FF00u);
                            if(a==physical&&off==0xC8)value&=~int32_t(0xFF000000u);
                            if(a==m&&off==0x7C)value&=~int32_t(0x02000000u);
                        }
#endif
#ifdef DORA3_PICKUP_SPIN_NATIVE_CHECK
                        if(kind==int16_t(item_kind)&&state==7) {
                            if((a==actor&&off==0x10)||(a==physical&&(off==0x58||off==0x5C||off==0xC4))||(a==m&&(off==0x64||off==0x74||off==0x78)))value=0;
                            if(a==physical&&off==0x28)value&=~int32_t(0x0000FF00u);
                            if(a==physical&&off==0xC8)value&=~int32_t(0xFF000000u);
                            if(a==m&&off==0x7C)value&=~int32_t(0x02000000u);
                        }
#endif
                        if(kind==131&&((a==actor&&off==0xA0)||(a==m&&off==0x64)))value=0;
                        if(kind==131&&a==m&&off==0x7C)value&=~int32_t(0x02000000u);
                        if(kind==131&&a==physical&&off==0x28)value&=~int32_t(0x0000FF00u);
                        words.push_back(value);
                    }
            }
            return words;
        };
        auto external_command=[&](unsigned tick) {
            if(!MEM_W(0x94,int32_t(effect)))return;
            if(guard==0) {
                if(tick==5)MEM_B(0xC9,int32_t(ph))=1;
                if(tick==10)MEM_B(0xC9,int32_t(ph))=0;
            } else if(guard==1) {
                if(tick==5)wf(rdram,ph+0x10,11);
                if(tick==10)wf(rdram,ph+0x10,0);
            } else if(guard==2) {
                if(tick==5)MEM_W(0,int32_t(0x80202064u+item_kind*84u))=int32_t(0x801E5D84u);
                if(tick==10)MEM_W(0,int32_t(0x80202064u+item_kind*84u))=int32_t(item_kind==118?0x801ECC6Cu:item_kind==123?0x801ED020u:0x801ED244u);
            } else if(guard==3) {
                if(tick==5||tick==23) {
                    wf(rdram,effect+0x9C,-50);wf(rdram,effect+0xA0,175);wf(rdram,effect+0xA4,75);
                    for(unsigned off:{0x18u,0x1Cu,0x20u})wf(rdram,effect+off,3.5f);
                }
            } else if(guard==4) {
                if(tick==5)MEM_H(0xD8,int32_t(ph))=6;
                if(tick==10)MEM_H(0xD8,int32_t(ph))=1;
            } else if(guard==5) {
                if(tick==5||tick==18){c.r4=int32_t(model);c.r5=0;dora3_s0_80039564(rdram,&c);}
            } else if(guard==6) {
                if(tick==5)MEM_H(0x36,int32_t(ph))=359;
                if(tick==6){MEM_H(0x34,int32_t(ph))=1;MEM_H(0x36,int32_t(ph))=359;}
                if(tick==7)MEM_H(0x44,int32_t(ph))=int16_t(target-2);
            } else if(guard==7) {
                if(tick==11||tick==30){c.r4=int32_t(model);c.r5=0;c.r6=0;dora3_s0_80039578(rdram,&c);}
                if(tick==16||tick==35){c.r4=int32_t(model);c.r5=0;c.r6=1;dora3_s0_80039578(rdram,&c);}
            }
            c=caller;c.f_odd=&c.f0.u32h;
        };
        unsigned frame=1;bool retired=false;
        std::array<unsigned,2> rejected_phases{};
        Nanoseconds last_native_stamp=0;
        for(unsigned tick=1;tick<=256;++tick) {
            const auto units=uint64_t(second)*divider*tick;const auto boundary=Nanoseconds((units+59)/60);
            c=caller;c.f_odd=&c.f0.u32h;
            if(placement==0) {
                const auto start_units=uint64_t(second)*divider*(tick-1);input_now=Nanoseconds((start_units+59)/60);
                city_source_stamp={Nanoseconds(start_units/60),unsigned(start_units%60)};external_command(tick);
            }
            if(playback==1) {
                auto native_to=[&](Nanoseconds stamp) {
                    const bool positive_interval=stamp>last_native_stamp;last_native_stamp=stamp;
                    const auto before=source_owned();const auto context=c;input_now=stamp;
                    const auto old_state=MEM_W(0x94,int32_t(effect)),old_yaw=MEM_W(0x10,int32_t(effect));
                    const auto old_height=MEM_W(0xA0,int32_t(effect)),old_scale=MEM_W(0x18,int32_t(effect));
                    const auto old_x=MEM_W(0x9C,int32_t(effect)),old_z=MEM_W(0xA4,int32_t(effect));
                    const auto old_arc_phase=MEM_H(0x34,int32_t(ph));
                    const auto old_speed=MEM_W(0x58,int32_t(ph)),old_alpha=MEM_W(0x5C,int32_t(ph));
                    std::map<uint32_t,int32_t> flash_heights,periodic_heights;
                    for(unsigned slot=30;slot<230;++slot) {
                        const auto a=0x80171CD0u+slot*0xCC;if(!MEM_W(0x94,int32_t(a)))continue;
                        if(MEM_H(0x20,MEM_W(0x98,int32_t(a)))==130)flash_heights[a]=MEM_W(0xA0,int32_t(a));
                        if(MEM_H(0x20,MEM_W(0x98,int32_t(a)))==131)periodic_heights[a]=MEM_W(0xA0,int32_t(a));
                    }
                    auto raw_item=[&] {
                        std::vector<int32_t> value;
                        for(auto [address,size]:std::array<std::pair<uint32_t,unsigned>,3>{{{effect,0xCC},{ph,0xF4},{model,0x84}}})
                            for(unsigned off=0;off<size;off+=4)value.push_back(MEM_W(int32_t(off),int32_t(address)));
                        return value;
                    };
                    const auto arc_angle=MEM_H(0x36,int32_t(ph)),arc_target=MEM_H(0x44,int32_t(ph));
                    const bool rejected_arc=old_state==8&&((old_arc_phase==0&&arc_angle>=90)||(old_arc_phase==1&&arc_angle>=arc_target));
                    const auto rejected_before=rejected_arc?raw_item():std::vector<int32_t>{};
                    native_gameplay_preview_update(rdram,&c,stamp);
                    if(rejected_arc){assert(raw_item()==rejected_before);++arc_rejected_intervals;if(positive_interval)++rejected_phases.at(unsigned(old_arc_phase));}
                    if(old_state==8&&(old_x!=MEM_W(0x9C,int32_t(effect))||old_z!=MEM_W(0xA4,int32_t(effect)))) {
                        if(old_arc_phase==0)++arc_first_xyz;else ++arc_second_xz;
                    }
                    if(old_state==8&&old_scale!=MEM_W(0x18,int32_t(effect)))++arc_scale;
                    if(old_state==7) {spin_yaw+=old_yaw!=MEM_W(0x10,int32_t(effect));spin_rate+=old_speed!=MEM_W(0x58,int32_t(ph));spin_alpha+=old_alpha!=MEM_W(0x5C,int32_t(ph));}
                    if(old_state==5&&MEM_W(0xA0,int32_t(effect))!=old_height)++rise_samples;
                    if(old_state==6&&MEM_W(0x18,int32_t(effect))!=old_scale)++shrink_samples;
                    if(MEM_W(0x94,int32_t(effect))&&MEM_W(0x10,int32_t(effect))!=old_yaw) {
                        if(old_state==1)++idle_yaw_samples;
                        if(old_state==10)++collected_yaw_samples;
                    }
                    for(auto [a,y]:flash_heights)if(MEM_W(0x94,int32_t(a))&&MEM_W(0xA0,int32_t(a))!=y)++flash_height_samples;
                    for(auto [a,y]:periodic_heights)if(MEM_W(0x94,int32_t(a))&&MEM_W(0xA0,int32_t(a))!=y)++periodic_height_samples;
                    assert(source_owned()==before&&std::memcmp(&context,&c,sizeof(c))==0);++intervals;
                };
                while(second*frame/fps<boundary){native_to(second*frame/fps);++frame;}native_to(boundary);
            }
            input_now=boundary;city_source_stamp={Nanoseconds(units/60),unsigned(units%60)};
            c=caller;c.f_odd=&c.f0.u32h;
            if(placement==1)external_command(tick);
            if(initial_state==8&&tick==140) {
                assert(MEM_W(0x94,int32_t(effect))==1);
                const auto old_actor=MEM_W(0,int32_t(0x8021BDEC)),old_physical=MEM_W(0,int32_t(0x8021BCDC)),old_mode=MEM_W(0,int32_t(0x8021BCE4));
                MEM_W(0,int32_t(0x8021BDEC))=int32_t(effect);MEM_W(0,int32_t(0x8021BCDC))=int32_t(ph);MEM_W(0,int32_t(0x8021BCE4))=3;
                if(item_kind==118){if(playback==0)reference_dora3_s3_core_801ECBE4(rdram,&c);else dora3_s3_core_801ECBE4(rdram,&c);}
                else if(item_kind==123){if(playback==0)reference_dora3_s3_core_801ECFB4(rdram,&c);else dora3_s3_core_801ECFB4(rdram,&c);}
                else {if(playback==0)reference_dora3_s3_core_801ED228(rdram,&c);else dora3_s3_core_801ED228(rdram,&c);}
                assert(MEM_W(0x94,int32_t(effect))==10&&MEM_BU(0x28,int32_t(ph))==0);
                MEM_W(0,int32_t(0x8021BDEC))=old_actor;MEM_W(0,int32_t(0x8021BCDC))=old_physical;MEM_W(0,int32_t(0x8021BCE4))=old_mode;
                c=caller;c.f_odd=&c.f0.u32h;
            }
            if(playback==0)reference_dora3_s3_801E8BDC(rdram,&c);else dora3_s3_801E8BDC(rdram,&c);
            const auto value=snapshot();const auto key=std::array<unsigned,7>{fps,divider,camera,target,item_kind,guard,placement};
            if(playback==0)oracles[key].push_back(value);else {
                if(value!=oracles.at(key).at(tick-1)) {
                    std::cerr<<"Periodic producer mismatch fps="<<fps<<" divider="<<divider<<" camera="<<camera<<" target="<<target<<" guard="<<guard<<" placement="<<placement<<" item_kind="<<item_kind<<" tick="<<tick<<'\n';
                    for(unsigned i=0;i<std::min(value.words.size(),oracles.at(key).at(tick-1).words.size());++i)
                        if(value.words[i]!=oracles.at(key).at(tick-1).words[i])std::cerr<<" word="<<i<<" actual="<<std::hex<<value.words[i]<<" expected="<<oracles.at(key).at(tick-1).words[i]<<std::dec<<'\n';
                    std::abort();
                }
                ++endpoints;
            }
            if(playback==0&&MEM_W(0x94,int32_t(effect))==7&&rf(rdram,ph+0x58)>1)++spin_states;
            if(playback==0&&MEM_W(0x94,int32_t(effect))==8&&MEM_H(0x34,int32_t(ph))==1)++arc_phases;
            if(playback==0&&initial_state==8&&MEM_W(0x94,int32_t(effect))==1)++arc_returns;
            if(!MEM_W(0x94,int32_t(effect)))retired=true;
        }
        if(!(retired&&allocations==before_heap&&resource_references==before_refs&&game_model_animation().model_count()==before_models)) {
            std::cerr<<"Periodic producer lifetime mismatch playback="<<playback<<" fps="<<fps<<" divider="<<divider<<" camera="<<camera<<" target="<<target<<" guard="<<guard<<" placement="<<placement<<" kind="<<item_kind
                <<" retired="<<retired<<" heap_equal="<<(allocations==before_heap)<<" refs_equal="<<(resource_references==before_refs)<<" models="<<game_model_animation().model_count()<<" expected_models="<<before_models<<'\n';
            for(unsigned slot=30;slot<230;++slot) {
                const auto a=0x80171CD0u+slot*0xCC;const auto state=MEM_W(0x94,int32_t(a));if(!state)continue;
                const auto physical=MEM_W(0x98,int32_t(a));
                std::cerr<<" live_slot="<<slot<<" kind="<<MEM_H(0x20,physical)<<" state="<<state<<" y="<<rf(rdram,a+0xA0)<<" vy="<<rf(rdram,uint32_t(physical)+0x14)
                    <<" latch="<<int(MEM_BU(0x28,physical))<<" count="<<MEM_H(0x34,physical)<<" phase="<<MEM_H(0x36,physical)<<'\n';
            }
            std::abort();
        }
        if(playback==1) {
            if(guard==6&&placement==0) {
                // All24 settings *3 targets *3 kinds must reject each of the
                // two actual incompatible commands for at least one interval.
                assert(rejected_phases[0]>0&&rejected_phases[1]>0);++rejected_worlds;
            } else assert(rejected_phases[0]==0&&rejected_phases[1]==0);
        }
        native_gameplay_preview_reset(rdram);preview_live=false;slot09_loaded=linked_construct_watch=linked_watch=false;
        for(auto m:owners){c.r4=int32_t(m);dora3_s0_8003B77C(rdram,&c);}
        if(playback==0)++source_cases;else ++native_cases;
        // CPU-only diagnostic progress; no game state or oracle changes.
        if((source_cases+native_cases)%12==0)
            std::ofstream("build-local/native-pickup-arc-guard-check/progress.txt")
                <<source_cases<<' '<<native_cases<<' '<<endpoints<<'\n';
    }
    std::ofstream("build-local/native-pickup-arc-guard-check/coverage.txt")
        <<arc_first_xyz<<' '<<arc_second_xz<<' '<<arc_scale<<' '<<arc_phases<<' '<<arc_returns<<' '
        <<arc_rejected_intervals<<' '<<rejected_worlds<<' '<<source_cases<<' '<<native_cases<<' '<<endpoints<<'\n';
    assert(source_cases==3456&&native_cases==3456&&endpoints==3456*256&&source_births==3456&&intervals>10000);
    assert(arc_first_xyz>3456&&arc_second_xz>3456&&arc_scale>3456&&arc_phases>3456&&arc_returns>3456);
    assert(rejected_worlds==24*3*3&&arc_rejected_intervals>=2*rejected_worlds);
    assert(idle_yaw_samples>3456&&collected_yaw_samples>3456&&flash_height_samples>3456);
    std::cout<<"PASS: 3456 Original and3456 mixed Native/Source guarded pickup8 schedules;8 profiles before/after Native: actor pause/resume, velocity rejection/reentry, update replacement, absolute XYZ/uniform scale, real blink6/reentry1, actual seek0, angle wrap/phase/target clamp commands, model pause/resume near phase edges; "<<intervals<<" Native intervals; "<<endpoints<<" exact full live actor/physics/model/heap/resource/command frozen Original endpoints; Native firstXYZ/secondXZ/scale samples "<<arc_first_xyz<<'/'<<arc_second_xz<<'/'<<arc_scale<<"; rejected incompatible phase-angle intervals="<<arc_rejected_intervals<<" across="<<rejected_worlds<<" paired phase0/phase1 rejection worlds; actual initialization/Source half-angle phase/latch/clamp/state1, genuine mode3 collection/six-flash retirement; external fixture commands/synthetic assets, no real renderer/script or pool exhaustion proof; no game/GPU.\n";
}
#endif
