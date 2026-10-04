#ifdef DORA3_PICKUP_SCATTER_GUARD_CHECK
#include "recomp_context_copy.hpp"
static void check_pickup_scatter_guards(const std::vector<uint32_t>& image) {
#ifdef DORA3_PICKUP_SIGNED_ZERO_SMOKE
    constexpr unsigned expected_cases=20;
#else
    constexpr unsigned expected_cases=4320;
#endif
    std::map<std::array<unsigned,8>,std::vector<FlashPoolSnapshot>> oracles;
    unsigned source_cases=0,native_cases=0,endpoints=0,intervals=0,source_births=0,spin_states=0,arc_phases=0,arc_returns=0;
    unsigned settle_steps=0,scatter_steps=0,signed_zero_idle_intervals=0;
    unsigned scatter_yaw_samples=0,scatter_position_samples=0,scatter_speed_samples=0,query_rollback_samples=0;
    unsigned source_contacts=0,source_boundary_random=0,native_odd_descendants=0,external_odd_rejections=0,external_odd_reentry=0;
    unsigned idle_yaw_samples=0,collected_yaw_samples=0,flash_height_samples=0,periodic_height_samples=0,rise_samples=0,shrink_samples=0;
    for(unsigned camera:{12u,2u})for(unsigned divider:{2u,3u})
    for(unsigned fps:{60u,73u,120u,144u,165u,360u})for(unsigned target:{80u,250u,400u})for(unsigned item_kind:{118u,123u,127u})for(unsigned initial_state:{3u})
    for(unsigned guard:{0u,1u,2u,3u,4u,5u,6u,7u,8u,9u})
    for(unsigned placement:{0u,1u})for(unsigned playback:{0u,1u}) {
#ifdef DORA3_PICKUP_SIGNED_ZERO_SMOKE
        if(divider!=2||fps!=60||camera!=12||target!=80||item_kind!=118)continue;
#endif
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
        MEM_H(0x7C,int32_t(ph))=-1; // Explicit unattached Source3 baseline.

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
        bool external_odd_speed=false;
        auto source_owned=[&] {
            std::vector<int32_t> words;
            for(unsigned slot=30;slot<230;++slot) {
                const auto actor=0x80171CD0u+slot*0xCC;const auto state=MEM_W(0x94,int32_t(actor));if(!state)continue;
                const auto physical=uint32_t(MEM_W(0x98,int32_t(actor))),m=uint32_t(MEM_W(0x74,int32_t(actor)));
                const auto kind=MEM_H(0x20,int32_t(physical));
                const bool idle=kind==int16_t(item_kind)&&state==1&&(uint32_t(MEM_W(0x10,int32_t(physical)))&0x7FFFFFFFu)==0&&(uint32_t(MEM_W(0x14,int32_t(physical)))&0x7FFFFFFFu)==0&&(uint32_t(MEM_W(0x18,int32_t(physical)))&0x7FFFFFFFu)==0;
                const auto velocity=rf(rdram,physical+0x14);
                const auto blink=MEM_H(0xD8,int32_t(physical));
                const auto update=item_kind==118?0x801ECC6Cu:item_kind==123?0x801ED020u:0x801ED244u;
                const auto contact=item_kind==118?0x801ECBE4u:item_kind==123?0x801ECFB4u:0x801ED228u;
                const auto speed=rf(rdram,physical);
                // Interpolation of even endpoints can itself produce an odd integer.
                // The test independently knows whether its explicit odd-speed
                // command is active; those intervals remain wholly Source-owned.
                const bool scatter=kind==int16_t(item_kind)&&state==3&&MEM_BU(0x28,int32_t(physical))==1&&MEM_H(0x34,int32_t(physical))>=0&&MEM_H(0x34,int32_t(physical))<=5&&
                    MEM_H(0x7C,int32_t(physical))==-1&&MEM_BU(0xC9,int32_t(physical))!=1&&(blink==0||blink==1)&&velocity==0&&
                    std::isfinite(speed)&&speed>=0&&speed<=20&&!external_odd_speed&&!game_model_animation().native_birth(m)&&
                    uint32_t(MEM_W(0,int32_t(0x80202064u+item_kind*84)))==update&&uint32_t(MEM_W(0,int32_t(0x80202068u+item_kind*84)))==contact;
                if(idle||(kind==int16_t(item_kind)&&state==10&&MEM_BU(0x28,int32_t(physical))==1)||(kind==130&&state==10))continue;
                for(auto [a,size]:std::array<std::pair<uint32_t,unsigned>,3>{{{actor,0xCC},{physical,0xF4},{m,0x84}}})
                    for(unsigned off=0;off<size;off+=4) {
                        auto value=MEM_W(int32_t(off),int32_t(a));
                        if(scatter&&((a==actor&&(off==0x10||off==0x9C||off==0xA0||off==0xA4))||(a==physical&&(off==0||off==0x10||off==0x14||off==0x18||off==0xC4))||(a==m&&(off==0x64||off==0x78||off==0x74))))value=0;
                        if(scatter&&a==m&&off==0x7C)value&=~int32_t(0x02000000u);
                        if(scatter&&a==physical&&off==0xC8)value&=~int32_t(0xFF000000u);
                        if(scatter&&a==physical&&off==0x28)value&=~int32_t(0x0000FF00u);

                        if(kind==131&&((a==actor&&off==0xA0)||(a==m&&off==0x64)))value=0;
                        if(kind==131&&a==m&&off==0x7C)value&=~int32_t(0x02000000u);
                        if(kind==131&&a==physical&&off==0x28)value&=~int32_t(0x0000FF00u);
                        words.push_back(value);
                    }
            }
            return words;
        };
        const auto update_address=0x80202064u+item_kind*84,original_update=uint32_t(MEM_W(0,int32_t(update_address)));
        const auto bound_address=0x80202074u+item_kind*84;
        const auto original_bound=MEM_W(0,int32_t(bound_address));
        auto command=[&](unsigned tick) {
            if(guard==0) {if(tick==2)MEM_B(0xC9,int32_t(ph))=1;if(tick==5)MEM_B(0xC9,int32_t(ph))=0;}
            if(guard==1) {if(tick==2)wf(rdram,ph+0x14,7.f);if(tick==4)wf(rdram,ph+0x14,0.f);}
            if(guard==2) {if(tick==2)MEM_W(0,int32_t(update_address))=int32_t(0x801E5D84u);if(tick==4)MEM_W(0,int32_t(update_address))=int32_t(original_update);}
            if(guard==3) {if(tick==2){wf(rdram,effect+0x9C,750.f);wf(rdram,effect+0xA0,450.f);wf(rdram,effect+0xA4,-500.f);}if(tick==5)wf(rdram,effect+0xA0,180.f);}
            if(guard==4) {if(tick==2){wf(rdram,ph,9.f);external_odd_speed=true;}if(tick==4){wf(rdram,ph,16.f);external_odd_speed=false;}}
            if(guard==5) {if(tick==2)MEM_H(0xD8,int32_t(ph))=6;if(tick==5)MEM_H(0xD8,int32_t(ph))=1;}
            if(guard==6) {if(tick==2){wf(rdram,ph+4,70.f);wf(rdram,effect+0x10,340.f);}if(tick==5){wf(rdram,ph+4,280.f);wf(rdram,effect+0x9C,900.f);}}
            if(guard==7) {
                if(tick==2){recomp_context seek;dora3::copy_recomp_context(seek,caller);seek.r4=int32_t(model);seek.r5=0;dora3_s0_80039564(rdram,&seek);}
                if(tick==3||tick==5){recomp_context pause;dora3::copy_recomp_context(pause,caller);pause.r4=int32_t(model);pause.r5=0;pause.r6=tick==3?0:1;dora3_s0_80039578(rdram,&pause);}
            }
            // Inject contact inputs, never the outcome of the Original reaction.
            if(guard==8) {if(tick==2){MEM_H(0xD4,int32_t(ph))=0;MEM_H(0xD6,int32_t(ph))=1;wf(rdram,ph+0xEC,120.f);}if(tick==6)MEM_H(0xD4,int32_t(ph))=-1;}
            // Actual E7A50 bound -> E8210/E8238 reflection -> Original RNG.
            // This is the spherical boundary branch, not a fabricated wall hit.
            if(guard==9) {if(tick==2)MEM_W(0,int32_t(bound_address))=1;if(tick==8)MEM_W(0,int32_t(bound_address))=original_bound;}
        };
        unsigned world_contacts=0,world_boundary_random=0,world_odd_rejections=0,world_odd_reentry=0;
        unsigned frame=1;bool retired=false;
        for(unsigned tick=1;tick<=256;++tick) {
            const auto units=uint64_t(second)*divider*tick;const auto boundary=Nanoseconds((units+59)/60);
            c=caller;c.f_odd=&c.f0.u32h;
            if(placement==0) {
                const auto start_units=uint64_t(second)*divider*(tick-1);input_now=Nanoseconds((start_units+59)/60);
                city_source_stamp={Nanoseconds(start_units/60),unsigned(start_units%60)};command(tick);
            }
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
                    const auto old_scatter_x=MEM_W(0x9C,int32_t(effect)),old_scatter_z=MEM_W(0xA4,int32_t(effect)),old_scatter_speed=MEM_W(0,int32_t(ph));
                    const auto random_word=MEM_W(0,int32_t(0x801AA620));

                    std::array<int32_t,15> candidates{};for(unsigned i=0;i<candidates.size();++i)candidates[i]=MEM_W(int32_t(4*i),int32_t(0x800DACB8u));
                    std::array<int32_t,52> terrain{};for(unsigned i=0;i<terrain.size();++i)terrain[i]=MEM_W(int32_t(4*i),int32_t(0x80280000u));
                    const auto idle_velocity_bits=uint32_t(MEM_W(0x10,int32_t(ph)))|uint32_t(MEM_W(0x14,int32_t(ph)))|uint32_t(MEM_W(0x18,int32_t(ph)));
                    if(old_state==1&&(idle_velocity_bits&0x7FFFFFFFu)==0&&idle_velocity_bits!=0)++signed_zero_idle_intervals;
                    native_gameplay_preview_update(rdram,&c,stamp);
                    if(old_state==3) {
                        const auto speed=std::bit_cast<float>(uint32_t(old_scatter_speed));
                        if(!external_odd_speed&&std::isfinite(speed)&&std::floor(speed)==speed&&std::fmod(speed,2.f)!=0)++native_odd_descendants;
                        if(external_odd_speed) {
                            assert(MEM_W(0,int32_t(ph))==old_scatter_speed&&MEM_W(0x9C,int32_t(effect))==old_scatter_x&&MEM_W(0xA0,int32_t(effect))==old_height&&MEM_W(0xA4,int32_t(effect))==old_scatter_z&&MEM_W(0x10,int32_t(effect))==old_yaw);
                            ++world_odd_rejections;
                        } else if(guard==4&&tick>=4&&MEM_W(0,int32_t(ph))!=old_scatter_speed)++world_odd_reentry;

                        if(MEM_W(0x10,int32_t(effect))!=old_yaw)++scatter_yaw_samples;
                        if(MEM_W(0x9C,int32_t(effect))!=old_scatter_x||MEM_W(0xA4,int32_t(effect))!=old_scatter_z||MEM_W(0xA0,int32_t(effect))!=old_height)++scatter_position_samples;
                        if(MEM_W(0,int32_t(ph))!=old_scatter_speed)++scatter_speed_samples;
                        assert(random_word==MEM_W(0,int32_t(0x801AA620)));
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
                    const auto after=source_owned();
                    if(after!=before||std::memcmp(&context,&c,sizeof(c))!=0) {
                        std::cerr<<"Scatter raw-field mismatch guard="<<guard<<" placement="<<placement<<" fps="<<fps<<" divider="<<divider<<" camera="<<camera<<" kind="<<item_kind<<" target="<<target<<" tick="<<tick<<" stamp="<<stamp<<" old_state="<<old_state<<" old_speed="<<std::bit_cast<float>(uint32_t(old_scatter_speed))<<" new_speed="<<rf(rdram,ph)<<" external_odd="<<external_odd_speed<<" context_equal="<<(std::memcmp(&context,&c,sizeof(c))==0)<<'\n';
                        for(unsigned i=0;i<std::min(before.size(),after.size());++i)if(before[i]!=after[i])std::cerr<<" raw_word="<<i<<" before="<<std::hex<<before[i]<<" after="<<after[i]<<std::dec<<'\n';
                        std::abort();
                    }
                    ++intervals;
                };
                while(second*frame/fps<boundary){native_to(second*frame/fps);++frame;}native_to(boundary);
            }
            input_now=boundary;city_source_stamp={Nanoseconds(units/60),unsigned(units%60)};
            if(placement==1)command(tick);
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
            const auto source_state=MEM_W(0x94,int32_t(effect)),source_rng=MEM_W(0,int32_t(0x801AA620));
            const auto source_count=MEM_H(0x34,int32_t(ph));
            if(playback==0)reference_dora3_s3_801E8BDC(rdram,&c);else dora3_s3_801E8BDC(rdram,&c);
            if(source_state==3&&guard==8&&MEM_H(0x34,int32_t(ph))>source_count)++world_contacts;
            if(source_state==3&&guard==9&&tick>=2&&tick<8&&MEM_W(0,int32_t(0x801AA620))!=source_rng) {
                assert((MEM_HU(0,int32_t(0x8021BDFC))&0x80)!=0);++world_boundary_random;
            }

            const auto value=snapshot();const auto key=std::array<unsigned,8>{fps,divider,camera,target,item_kind,initial_state,guard,placement};
            if(playback==0)oracles[key].push_back(value);else {
                if(value!=oracles.at(key).at(tick-1)) {
                    std::cerr<<"Periodic producer mismatch fps="<<fps<<" divider="<<divider<<" camera="<<camera<<" target="<<target<<" initial_state="<<initial_state<<" item_kind="<<item_kind<<" guard="<<guard<<" placement="<<placement<<" tick="<<tick<<'\n';
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
        if(playback==1&&guard==4){assert(world_odd_rejections>0&&world_odd_reentry>0);external_odd_rejections+=world_odd_rejections;external_odd_reentry+=world_odd_reentry;}
        if(guard==8)assert(world_contacts>0);
        if(guard==9)assert(world_boundary_random>0);
        if(playback==0){source_contacts+=world_contacts;source_boundary_random+=world_boundary_random;}
        else oracles.erase(std::array<unsigned,8>{fps,divider,camera,target,item_kind,initial_state,guard,placement});
        assert(oracles.size()<=1);
        native_gameplay_preview_reset(rdram);preview_live=false;slot09_loaded=linked_construct_watch=linked_watch=false;
        for(auto m:owners){c.r4=int32_t(m);dora3_s0_8003B77C(rdram,&c);}
        if(playback==0)++source_cases;else ++native_cases;
        if((source_cases+native_cases)%12==0)std::ofstream("build-local/native-pickup-scatter-guard-check/progress.txt")<<source_cases<<' '<<native_cases<<' '<<endpoints<<'\n';
    }
    assert(source_cases==expected_cases&&native_cases==expected_cases&&endpoints==expected_cases*256&&source_births==expected_cases&&intervals>10000&&oracles.empty());
    assert(scatter_steps>expected_cases&&idle_yaw_samples>expected_cases&&collected_yaw_samples>expected_cases&&flash_height_samples>expected_cases);
    assert(scatter_yaw_samples>expected_cases&&scatter_position_samples>expected_cases&&scatter_speed_samples>expected_cases&&query_rollback_samples>expected_cases);
    assert(source_contacts>=(expected_cases/10)&&source_boundary_random>=(expected_cases/10));
    assert(native_odd_descendants>(expected_cases/10)&&external_odd_rejections>=(expected_cases/10)&&external_odd_reentry>=(expected_cases/10));
    assert(signed_zero_idle_intervals>expected_cases);
    std::cout<<"PASS: "<<expected_cases<<" Original and"<<expected_cases<<" mixed Native/Source guarded initialized scatter3 whole lifetimes; 10 command profiles before/after Native; "<<intervals<<" Native calls, "<<endpoints<<" exact complete actor/physics/model/heap/resource/command/RNG/collision-result endpoints; "<<scatter_yaw_samples<<'/'<<scatter_position_samples<<'/'<<scatter_speed_samples<<" intermediate yaw/XYZ/speed changes, "<<query_rollback_samples<<" RNG/cache/terrain rollback checks; "<<native_odd_descendants<<" valid odd integer interpolation descendants, "<<external_odd_rejections<<" strict external odd-speed rejections/"<<external_odd_reentry<<" reentry samples; "<<source_contacts<<" actual contact reactions and "<<source_boundary_random<<" actual boundary reflection RNG commands; "<<signed_zero_idle_intervals<<" signed-zero idle intervals/"<<idle_yaw_samples<<" Native idle yaw changes; frozen scatter Oracle independent, state/heading/count/RNG remain Source; per-world reactions required, paired oracle released per case; synthetic room/assets/inputs, geometric wall/dynamic platform/real selectors/render/pool exhaustion unverified; no game/GPU.\n";
}
#endif
