#ifdef DORA3_PERIODIC_PRODUCER_GUARD_CHECK
static void check_periodic_producer_guards(const std::vector<uint32_t>& image) {
    std::map<std::array<unsigned,7>,std::vector<FlashPoolSnapshot>> oracles;
    unsigned source_cases=0,native_cases=0,endpoints=0,intervals=0,source_births=0;
    unsigned idle_yaw_samples=0,collected_yaw_samples=0,flash_height_samples=0,periodic_height_samples=0,rise_samples=0,shrink_samples=0;
    for(unsigned playback:{0u,1u})for(unsigned camera:{12u,2u})for(unsigned divider:{2u,3u})
    for(unsigned fps:{60u,73u,120u,144u,165u,360u})for(unsigned height:{80u,250u,400u})for(unsigned item_kind:{118u,123u,127u})
    for(unsigned guard=0;guard<8;++guard)for(unsigned placement=0;placement<2;++placement) {
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
        c.r5=std::bit_cast<int32_t>(500.f);c.r6=std::bit_cast<int32_t>(float(height));c.r7=std::bit_cast<int32_t>(-300.f);
        MEM_W(0x10,int32_t(c.r29))=0;dora3_s3_801E6968(rdram,&c);
        const auto effect=uint32_t(c.r2);assert(effect==0x801734B8u);
        const auto ph=uint32_t(MEM_W(0x98,int32_t(effect))),model=uint32_t(MEM_W(0x74,int32_t(effect)));
        assert(MEM_H(0x20,int32_t(ph))==int16_t(item_kind)&&MEM_W(0x94,int32_t(effect))!=0);
        MEM_W(0x94,int32_t(effect))=5;MEM_B(0x28,int32_t(ph))=0;
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
                if(kind==int16_t(item_kind)&&(state==5||state==6)) {
                    words.push_back(state);words.push_back(MEM_BU(0x28,int32_t(physical)));
                    words.push_back(MEM_H(0x34,int32_t(physical)));words.push_back(MEM_H(0x36,int32_t(physical)));
                    words.push_back(MEM_W(0x14,int32_t(physical)));continue;
                }
                if(idle||(kind==int16_t(item_kind)&&state==10&&MEM_BU(0x28,int32_t(physical))==1)||(kind==130&&state==10))continue;
                for(auto [a,size]:std::array<std::pair<uint32_t,unsigned>,3>{{{actor,0xCC},{physical,0xF4},{m,0x84}}})
                    for(unsigned off=0;off<size;off+=4) {
                        auto value=MEM_W(int32_t(off),int32_t(a));
                        if(kind==131&&((a==actor&&off==0xA0)||(a==m&&off==0x64)))value=0;
                        if(kind==131&&a==m&&off==0x7C)value&=~int32_t(0x02000000u);
                        if(kind==131&&a==physical&&off==0x28)value&=~int32_t(0x0000FF00u);
                        words.push_back(value);
                    }
            }
            return words;
        };
        auto command=[&](unsigned tick) {
            const auto update=0x80202064u+item_kind*84u;
            const auto callback=item_kind==118?0x801ECC6Cu:item_kind==123?0x801ED020u:0x801ED244u;
            if(guard==0) {
                if(tick==5||tick==45)MEM_B(0xC9,int32_t(ph))=1;
                if(tick==10||tick==50)MEM_B(0xC9,int32_t(ph))=0;
            }
            if(guard==1) {
                if(tick==5||tick==45)wf(rdram,ph+0x10,11);
                if(tick==10||tick==50)wf(rdram,ph+0x10,0);
            }
            if(guard==2) {
                if(tick==5||tick==45)MEM_W(0,int32_t(update))=int32_t(0x801E5D84u);
                if(tick==10||tick==50)MEM_W(0,int32_t(update))=int32_t(callback);
            }
            if(guard==3) {
                if(tick==5)wf(rdram,effect+0xA0,243.f);
                if(tick==45)for(auto address:{effect+0x18,effect+0x1C,effect+0x20,ph+0x30})wf(rdram,address,3.f);
            }
            if(guard==4) {
                if(tick==5||tick==45)MEM_H(0xD8,int32_t(ph))=6;
                if(tick==10||tick==50)MEM_H(0xD8,int32_t(ph))=1;
            }
            if(guard==5&&(tick==5||tick==45)) {
                auto cmd=caller;cmd.f_odd=&cmd.f0.u32h;cmd.r4=int32_t(model);cmd.r5=0;
                dora3_s0_80039564(rdram,&cmd);
            }
            if(guard==6) {
                if(tick==5||tick==45){wf(rdram,ph+0xC4,.25f);MEM_B(0xC8,int32_t(ph))=2;}
                if(tick==8||tick==48){wf(rdram,ph+0xC4,.875f);MEM_B(0xC8,int32_t(ph))=1;}
            }
            if(guard==7&&(tick==11||tick==16||tick==51||tick==56)) {
                auto cmd=caller;cmd.f_odd=&cmd.f0.u32h;cmd.r4=int32_t(model);cmd.r5=(tick==11||tick==51)?1:0;
                dora3_s0_80039578(rdram,&cmd);
            }
        };
        unsigned frame=1;bool retired=false;
        for(unsigned tick=1;tick<=192;++tick) {
            const auto units=uint64_t(second)*divider*tick;const auto boundary=Nanoseconds((units+59)/60);
            c=caller;c.f_odd=&c.f0.u32h;
            // Explicit external state command at the previous Source boundary;
            // both playbacks receive it before this period's Native intervals.
            if(tick==40){assert(MEM_W(0x94,int32_t(effect))==5);MEM_W(0x94,int32_t(effect))=6;MEM_B(0x28,int32_t(ph))=0;}
            if(placement==0)command(tick);
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
                    native_gameplay_preview_update(rdram,&c,stamp);
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
            if(tick==100) {
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
            if(placement==1)command(tick);
            if(playback==0)reference_dora3_s3_801E8BDC(rdram,&c);else dora3_s3_801E8BDC(rdram,&c);
            const auto value=snapshot();const auto key=std::array<unsigned,7>{fps,divider,camera,height,item_kind,guard,placement};
            if(playback==0)oracles[key].push_back(value);else {
                if(value!=oracles.at(key).at(tick-1)) {
                    std::cerr<<"Periodic producer mismatch fps="<<fps<<" divider="<<divider<<" camera="<<camera<<" height="<<height<<" item_kind="<<item_kind<<" guard="<<guard<<" placement="<<placement<<" tick="<<tick<<'\n';
                    for(unsigned i=0;i<std::min(value.words.size(),oracles.at(key).at(tick-1).words.size());++i)
                        if(value.words[i]!=oracles.at(key).at(tick-1).words[i])std::cerr<<" word="<<i<<" actual="<<std::hex<<value.words[i]<<" expected="<<oracles.at(key).at(tick-1).words[i]<<std::dec<<'\n';
                    std::abort();
                }
                ++endpoints;
            }
            if(!MEM_W(0x94,int32_t(effect)))retired=true;
        }
        if(!(retired&&allocations==before_heap&&resource_references==before_refs&&game_model_animation().model_count()==before_models)) {
            std::cerr<<"Periodic producer lifetime mismatch playback="<<playback<<" fps="<<fps<<" divider="<<divider<<" camera="<<camera<<" height="<<height<<" kind="<<item_kind
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
    assert(source_cases==3456&&native_cases==3456&&endpoints==3456*192&&source_births==3456&&intervals>10000);
    assert(idle_yaw_samples>216&&collected_yaw_samples>216&&flash_height_samples>216&&periodic_height_samples>216&&rise_samples>72&&shrink_samples>216);
    std::cout<<"PASS: 3456 Original and3456 mixed Native/Source guarded pickup5/6 rise/periodic131 producer, explicit state6 shrink/signal, Native stationary state1 and actual Source mode3 collection/full six-flash130 lifetime schedules; "<<intervals
        <<" real Native intervals; intermediate idle/collected yaw, flash/periodic131 heights "<<idle_yaw_samples<<'/'<<collected_yaw_samples<<'/'<<flash_height_samples<<'/'<<periodic_height_samples<<"; Native rise/shrink samples "<<rise_samples<<'/'<<shrink_samples<<"; "<<endpoints
        <<" exact live actor/physics/model/heap/resource/command frozen Original endpoints; Source state5/6 and131 private command fields freeze between boundaries, genuine constructors/destructors and complete resource lifetime; Native initialized131 drift and pickup5/6 Y/scale with Source ordered private commands and retirement; 8 guards before/after Native: actor pause, moving rejection, callback replacement, absoluteY/uniform scale, actual blink D8=6/reentry1, seek0, opacity and model pause/resume at clip-edge periods; no game/GPU.\n";
}
#endif
