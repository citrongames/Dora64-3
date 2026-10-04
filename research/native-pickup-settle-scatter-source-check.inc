#ifdef DORA3_PICKUP_SETTLE_SCATTER_SOURCE_CHECK
static void check_pickup_settle_scatter_source(const std::vector<uint32_t>& image) {
    std::map<std::array<unsigned,6>,std::vector<FlashPoolSnapshot>> oracles;
    unsigned source_cases=0,native_cases=0,endpoints=0,intervals=0,source_births=0,spin_states=0,arc_phases=0,arc_returns=0;
    unsigned settle_steps=0,scatter_steps=0;
    unsigned scatter_yaw_samples=0,scatter_position_samples=0,scatter_speed_samples=0,scatter_rollback_samples=0;
    unsigned settle_height_samples=0,settle_velocity_samples=0,query_rollback_samples=0;
    unsigned idle_yaw_samples=0,collected_yaw_samples=0,flash_height_samples=0,periodic_height_samples=0,rise_samples=0,shrink_samples=0;
    for(unsigned playback:{0u,1u})for(unsigned camera:{12u,2u})for(unsigned divider:{2u,3u})
    for(unsigned fps:{60u,73u,120u,144u,165u,360u})for(unsigned target:{80u,250u,400u})for(unsigned item_kind:{118u,123u,127u})for(unsigned initial_state:{2u,3u}) {
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
        c.r5=std::bit_cast<int32_t>(500.f);c.r6=std::bit_cast<int32_t>(float(target));c.r7=std::bit_cast<int32_t>(-300.f);
        MEM_W(0x10,int32_t(c.r29))=0;dora3_s3_801E6968(rdram,&c);
        const auto effect=uint32_t(c.r2);assert(effect==0x801734B8u);
        const auto ph=uint32_t(MEM_W(0x98,int32_t(effect))),model=uint32_t(MEM_W(0x74,int32_t(effect)));
        assert(MEM_H(0x20,int32_t(ph))==int16_t(item_kind)&&MEM_W(0x94,int32_t(effect))!=0);
        MEM_W(0x94,int32_t(effect))=int(initial_state);MEM_B(0x28,int32_t(ph))=0;
        MEM_H(0x7C,int32_t(ph))=-1; // Explicit no-parent branch for Source2 baseline.

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
            value.words.push_back(MEM_W(0,int32_t(0x801AA620)));
            value.words.push_back(MEM_W(0,int32_t(0x8021BDFC)));
            for(unsigned i=0;i<15;++i)value.words.push_back(MEM_W(int32_t(4*i),int32_t(0x800DACB8u)));
            return value;
        };
        auto source_owned=[&] {
            std::vector<int32_t> words;
            for(unsigned slot=30;slot<230;++slot) {
                const auto actor=0x80171CD0u+slot*0xCC;const auto state=MEM_W(0x94,int32_t(actor));if(!state)continue;
                const auto physical=uint32_t(MEM_W(0x98,int32_t(actor))),m=uint32_t(MEM_W(0x74,int32_t(actor)));
                const auto kind=MEM_H(0x20,int32_t(physical));
                const bool idle=kind==int16_t(item_kind)&&state==1&&(uint32_t(MEM_W(0x10,int32_t(physical)))&0x7FFFFFFFu)==0&&(uint32_t(MEM_W(0x14,int32_t(physical)))&0x7FFFFFFFu)==0&&(uint32_t(MEM_W(0x18,int32_t(physical)))&0x7FFFFFFFu)==0;
                const bool settle=kind==int16_t(item_kind)&&state==2&&MEM_BU(0x28,int32_t(physical))==1&&MEM_H(0x34,int32_t(physical))==1&&MEM_H(0x7C,int32_t(physical))==-1;
                const bool scatter=kind==int16_t(item_kind)&&state==3&&MEM_BU(0x28,int32_t(physical))==1&&MEM_H(0x7C,int32_t(physical))==-1&&MEM_W(0x14,int32_t(physical))==0;
                if(idle||(kind==int16_t(item_kind)&&state==10&&MEM_BU(0x28,int32_t(physical))==1)||(kind==130&&state==10))continue;
                for(auto [a,size]:std::array<std::pair<uint32_t,unsigned>,3>{{{actor,0xCC},{physical,0xF4},{m,0x84}}})
                    for(unsigned off=0;off<size;off+=4) {
                        auto value=MEM_W(int32_t(off),int32_t(a));
#ifdef DORA3_PICKUP_SCATTER_NATIVE_CHECK
                        if(scatter&&((a==actor&&(off==0x10||off==0x9C||off==0xA0||off==0xA4))||(a==physical&&(off==0||off==0x10||off==0x14||off==0x18||off==0xC4))||(a==m&&(off==0x64||off==0x78||off==0x74))))value=0;
                        if(scatter&&a==m&&off==0x7C)value&=~int32_t(0x02000000u);
                        if(scatter&&a==physical&&off==0xC8)value&=~int32_t(0xFF000000u);
                        if(scatter&&a==physical&&off==0x28)value&=~int32_t(0x0000FF00u);
#endif
#ifdef DORA3_PICKUP_SETTLE_NATIVE_CHECK
                        if(settle&&((a==actor&&off==0xA0)||(a==physical&&off==0x14)||(a==m&&off==0x64)||(a==physical&&off==0xC4)||(a==m&&(off==0x78||off==0x74))))value=0;
                        if(settle&&a==m&&off==0x7C)value&=~int32_t(0x02000000u);
                        if(settle&&a==physical&&off==0xC8)value&=~int32_t(0xFF000000u);
                        if(settle&&a==physical&&off==0x28)value&=~int32_t(0x0000FF00u);
#endif
                        if(kind==131&&((a==actor&&off==0xA0)||(a==m&&off==0x64)))value=0;
                        if(kind==131&&a==m&&off==0x7C)value&=~int32_t(0x02000000u);
                        if(kind==131&&a==physical&&off==0x28)value&=~int32_t(0x0000FF00u);
                        words.push_back(value);
                    }
            }
            return words;
        };
        unsigned frame=1;bool retired=false;
        for(unsigned tick=1;tick<=256;++tick) {
            const auto units=uint64_t(second)*divider*tick;const auto boundary=Nanoseconds((units+59)/60);
            c=caller;c.f_odd=&c.f0.u32h;
            if(playback==1) {
                auto native_to=[&](Nanoseconds stamp) {
                    const auto before=source_owned();const auto context=c;input_now=stamp;
                    const auto old_state=MEM_W(0x94,int32_t(effect)),old_yaw=MEM_W(0x10,int32_t(effect));
                    const auto old_height=MEM_W(0xA0,int32_t(effect)),old_scale=MEM_W(0x18,int32_t(effect));
                    std::map<uint32_t,int32_t> flash_heights,periodic_heights;
                    for(unsigned slot=30;slot<230;++slot) {
                        const auto a=0x80171CD0u+slot*0xCC;if(!MEM_W(0x94,int32_t(a)))continue;
                        if(MEM_H(0x20,MEM_W(0x98,int32_t(a)))==130)flash_heights[a]=MEM_W(0xA0,int32_t(a));
                        if(MEM_H(0x20,MEM_W(0x98,int32_t(a)))==131)periodic_heights[a]=MEM_W(0xA0,int32_t(a));
                    }
                    const auto old_velocity=MEM_W(0x14,int32_t(ph));
                    std::array<int32_t,15> candidates{};for(unsigned i=0;i<candidates.size();++i)candidates[i]=MEM_W(int32_t(4*i),int32_t(0x800DACB8u));
                    std::array<int32_t,52> terrain{};for(unsigned i=0;i<terrain.size();++i)terrain[i]=MEM_W(int32_t(4*i),int32_t(0x80280000u));
                    const auto old_scatter_x=MEM_W(0x9C,int32_t(effect)),old_scatter_z=MEM_W(0xA4,int32_t(effect)),old_scatter_speed=MEM_W(0,int32_t(ph));
                    const auto random_word=MEM_W(0,int32_t(0x801AA620));
                    native_gameplay_preview_update(rdram,&c,stamp);
#ifdef DORA3_PICKUP_SCATTER_NATIVE_CHECK
                    if(old_state==3) {
                        if(MEM_W(0x10,int32_t(effect))!=old_yaw)++scatter_yaw_samples;
                        if(MEM_W(0x9C,int32_t(effect))!=old_scatter_x||MEM_W(0xA4,int32_t(effect))!=old_scatter_z||MEM_W(0xA0,int32_t(effect))!=old_height)++scatter_position_samples;
                        if(MEM_W(0,int32_t(ph))!=old_scatter_speed)++scatter_speed_samples;
                        assert(random_word==MEM_W(0,int32_t(0x801AA620)));
                        for(unsigned i=0;i<candidates.size();++i)assert(candidates[i]==MEM_W(int32_t(4*i),int32_t(0x800DACB8u)));
                        for(unsigned i=0;i<terrain.size();++i)assert(terrain[i]==MEM_W(int32_t(4*i),int32_t(0x80280000u)));
                        ++scatter_rollback_samples;
                    }
#endif
                    if(old_state==2) {
                        if(MEM_W(0xA0,int32_t(effect))!=old_height)++settle_height_samples;
                        if(MEM_W(0x14,int32_t(ph))!=old_velocity)++settle_velocity_samples;
                        for(unsigned i=0;i<candidates.size();++i)assert(candidates[i]==MEM_W(int32_t(4*i),int32_t(0x800DACB8u)));
                        for(unsigned i=0;i<terrain.size();++i)assert(terrain[i]==MEM_W(int32_t(4*i),int32_t(0x80280000u)));
                        ++query_rollback_samples;
                    }
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
            if(tick==140) {
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
            const auto value=snapshot();const auto key=std::array<unsigned,6>{fps,divider,camera,target,item_kind,initial_state};
            if(playback==0)oracles[key].push_back(value);else {
                if(value!=oracles.at(key).at(tick-1)) {
                    std::cerr<<"Periodic producer mismatch fps="<<fps<<" divider="<<divider<<" camera="<<camera<<" target="<<target<<" initial_state="<<initial_state<<" item_kind="<<item_kind<<" tick="<<tick<<'\n';
                    for(unsigned i=0;i<std::min(value.words.size(),oracles.at(key).at(tick-1).words.size());++i)
                        if(value.words[i]!=oracles.at(key).at(tick-1).words[i])std::cerr<<" word="<<i<<" actual="<<std::hex<<value.words[i]<<" expected="<<oracles.at(key).at(tick-1).words[i]<<std::dec<<'\n';
                    std::abort();
                }
                ++endpoints;
            }
            if(playback==0&&MEM_W(0x94,int32_t(effect))==2)++settle_steps;
            if(playback==0&&MEM_W(0x94,int32_t(effect))==3)++scatter_steps;
            if(playback==0&&MEM_W(0x94,int32_t(effect))==7&&rf(rdram,ph+0x58)>1)++spin_states;
            if(playback==0&&MEM_W(0x94,int32_t(effect))==8&&MEM_H(0x34,int32_t(ph))==1)++arc_phases;
            if(playback==0&&initial_state==8&&MEM_W(0x94,int32_t(effect))==1)++arc_returns;
            if(!MEM_W(0x94,int32_t(effect)))retired=true;
        }
        if(!(retired&&allocations==before_heap&&resource_references==before_refs&&game_model_animation().model_count()==before_models)) {
            std::cerr<<"Periodic producer lifetime mismatch playback="<<playback<<" fps="<<fps<<" divider="<<divider<<" camera="<<camera<<" target="<<target<<" initial_state="<<initial_state<<" kind="<<item_kind
                <<" retired="<<retired<<" heap_equal="<<(allocations==before_heap)<<" refs_equal="<<(resource_references==before_refs)<<" models="<<game_model_animation().model_count()<<" expected_models="<<before_models<<'\n';
            for(unsigned slot=30;slot<230;++slot) {
                const auto a=0x80171CD0u+slot*0xCC;const auto state=MEM_W(0x94,int32_t(a));if(!state)continue;
                const auto physical=MEM_W(0x98,int32_t(a));
                std::cerr<<" live_slot="<<slot<<" kind="<<MEM_H(0x20,physical)<<" state="<<state<<" y="<<rf(rdram,a+0xA0)<<" vy="<<rf(rdram,uint32_t(physical)+0x14)
                    <<" latch="<<int(MEM_BU(0x28,physical))<<" count="<<MEM_H(0x34,physical)<<" phase="<<MEM_H(0x36,physical)<<'\n';
            }
            std::abort();
        }
        native_gameplay_preview_reset(rdram);preview_live=false;slot09_loaded=linked_construct_watch=linked_watch=false;
        for(auto m:owners){c.r4=int32_t(m);dora3_s0_8003B77C(rdram,&c);}
        if(playback==0)++source_cases;else ++native_cases;
    }
    assert(source_cases==432&&native_cases==432&&endpoints==432*256&&source_births==432&&intervals>10000);
    assert(settle_steps>216&&scatter_steps>216&&idle_yaw_samples>216&&collected_yaw_samples>216&&flash_height_samples>216);
#ifdef DORA3_PICKUP_SETTLE_NATIVE_CHECK
    assert(settle_height_samples>216&&settle_velocity_samples>216&&query_rollback_samples>216);
#ifdef DORA3_PICKUP_SCATTER_NATIVE_CHECK
    std::cout<<"PASS: 432 Original and432 mixed Native/Source initialized no-parent pickup2/3 Y/velocity/yaw/scatter motion with actual ordered Source gravity/probe/latch/state1 and full collection/six-flash retirement; Original heading/count/state/RNG commands retained; "<<intervals<<" Native calls, "<<endpoints<<" exact complete live actor/physics/model/heap/resource/command endpoints; "<<settle_height_samples<<'/'<<settle_velocity_samples<<" Native Y/velocity changes, "<<query_rollback_samples<<" collision-cache/terrain rollback checks; synthetic room/assets, explicit state/height/no-parent inputs; parent-follow,geometric wall/dynamic platform,real selectors,dynamic platform integration,renderer,pool exhaustion outside proof; no game/GPU.\n";
#else
    std::cout<<"PASS: 432 Original and432 mixed Native/Source initialized no-parent pickup2 Y/velocity with actual ordered Source gravity/probe/latch/state1 and full collection/six-flash retirement; Source scatter3 unchanged; "<<intervals<<" Native calls, "<<endpoints<<" exact complete live actor/physics/model/heap/resource/command endpoints; "<<settle_height_samples<<'/'<<settle_velocity_samples<<" Native Y/velocity changes, "<<query_rollback_samples<<" collision-cache/terrain rollback checks; synthetic room/assets, explicit state/height/no-parent inputs; parent-follow,scatter3 Native,real selectors,dynamic platform integration,renderer,pool exhaustion outside proof; no game/GPU.\n";
#endif
#else
    std::cout<<"PASS: 432 Original and432 mixed Native/Source Source-owned pickup2/3 schedules; actual no-parent settle2 initializer/gravity/probe/state1 and scatter3 RNG/speed/collision/height/state1, real Source mode3 contact140 and full collected/six-flash retirement; "<<intervals<<" Native intervals keep unclaimed2/3 actor/physics/model fields unchanged; "<<endpoints<<" exact complete live actor/physics/model/heap/resource/command frozen Original endpoints; actual settle/scatter steps "<<settle_steps<<'/'<<scatter_steps<<"; synthetic room/assets and explicit state/height/no-parent inputs; Native2/3 and parent-follow branch remain outside proof; no real level/script/renderer/pool exhaustion claim; no game/GPU.\n";
#endif
#ifdef DORA3_PICKUP_SCATTER_NATIVE_CHECK
    assert(scatter_yaw_samples>216&&scatter_position_samples>216&&scatter_speed_samples>216&&scatter_rollback_samples>216);
    std::cout<<"PASS: Native initialized standard scatter3 yaw/speed/XYZ/velocities; "<<scatter_yaw_samples<<'/'<<scatter_position_samples<<'/'<<scatter_speed_samples<<" intermediate changes, "<<scatter_rollback_samples<<" RNG/collision-cache/terrain rollback checks; all432 paired whole lifetime endpoints above retain actual Original headings/count/RNG/state1/collection/retirement; parent-follow/wall rebound guards/dynamic platform/real selectors/render unverified; no game/GPU.\n";
#endif
}
#endif
