#include "recomp_context_copy.hpp"
extern "C" void reference_dora3_s3_core_801EBAD4(uint8_t*,recomp_context*);
#ifdef DORA3_PICKUP_PARENT_SOURCE_CHECK
static void check_pickup_parent_source(const std::vector<uint32_t>& image) {
    std::map<std::array<unsigned,9>,std::vector<FlashPoolSnapshot>> oracles;
    unsigned source_cases=0,native_cases=0,endpoints=0,intervals=0,source_births=0,spin_states=0,arc_phases=0,arc_returns=0;
    unsigned settle_steps=0,scatter_steps=0,attachment_changes=0,attachment_geometry_checks=0,duplicate_stamp_checks=0,rejected_attachment_calls=0,reentry_geometry_checks=0;
    unsigned scatter_yaw_samples=0,scatter_position_samples=0,scatter_speed_samples=0,scatter_rollback_samples=0;
    unsigned settle_height_samples=0,settle_velocity_samples=0,query_rollback_samples=0;
    unsigned idle_yaw_samples=0,collected_yaw_samples=0,flash_height_samples=0,periodic_height_samples=0,rise_samples=0,shrink_samples=0;
#ifdef DORA3_PICKUP_PARENT_GUARD_SMOKE
    constexpr unsigned paired_worlds=30;
    const std::vector<unsigned> cameras{12},dividers{2},rates{60},heights{80},kinds{118},profiles{0};
#else
    constexpr unsigned paired_worlds=12960;
    const std::vector<unsigned> cameras{12,2},dividers{2,3},rates{60,73,120,144,165,360},heights{80,250,400},kinds{118,123,127},profiles{0,5};
#endif
    for(unsigned camera:cameras)for(unsigned divider:dividers)for(unsigned fps:rates)for(unsigned target:heights)for(unsigned item_kind:kinds)for(unsigned initial_state:{2u})for(unsigned parent_profile:profiles)for(unsigned guard=0;guard<10;++guard)for(unsigned placement:{0u,1u,2u})for(unsigned playback:{0u,1u}) {
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
        c=caller;c.f_odd=&c.f0.u32h;c.r4=int32_t(0x801FC950u);
        c.r5=std::bit_cast<int32_t>(500.f);c.r6=std::bit_cast<int32_t>(float(target+140));c.r7=std::bit_cast<int32_t>(-300.f);
        MEM_W(0x10,int32_t(c.r29))=0;dora3_s3_801E6968(rdram,&c);
        const auto parent=uint32_t(c.r2);assert(parent==0x801734B8u);
        const auto parent_physical=uint32_t(MEM_W(0x98,int32_t(parent)));
        assert(MEM_H(0x20,int32_t(parent_physical))==130);
        MEM_H(0x44,int32_t(parent_physical))=int16_t(parent_profile);
        c=caller;c.f_odd=&c.f0.u32h;c.r4=int32_t(item_kind==118?0x801FC818u:item_kind==123?0x801FC894u:0x801FC904u);
        c.r5=std::bit_cast<int32_t>(500.f);c.r6=std::bit_cast<int32_t>(float(target));c.r7=std::bit_cast<int32_t>(-300.f);
        MEM_W(0x10,int32_t(c.r29))=0;dora3_s3_801E6968(rdram,&c);
        const auto effect=uint32_t(c.r2);assert(effect==0x801734B8u+0xCC);
        const auto ph=uint32_t(MEM_W(0x98,int32_t(effect))),model=uint32_t(MEM_W(0x74,int32_t(effect)));
        assert(MEM_H(0x20,int32_t(ph))==int16_t(item_kind)&&MEM_W(0x94,int32_t(effect))!=0);
        MEM_W(0x94,int32_t(effect))=int(initial_state);MEM_B(0x28,int32_t(ph))=0;
        MEM_H(0x7C,int32_t(ph))=30; // Real live Parent130 precedes child31 in the common pool.

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
        const auto update_address=0x80202064u+item_kind*84,original_update=uint32_t(MEM_W(0,int32_t(update_address)));
        const auto parent_update_address=0x80202064u+130u*84,original_parent_update=uint32_t(MEM_W(0,int32_t(parent_update_address)));
        const auto parent_model=uint32_t(MEM_W(0x74,int32_t(parent)));
        auto eligible_attachment=[&] {
            return MEM_W(0x94,int32_t(effect))==2&&MEM_BU(0x28,int32_t(ph))==1&&MEM_H(0x34,int32_t(ph))==0&&MEM_H(0x7C,int32_t(ph))==30&&
                MEM_H(0xD8,int32_t(ph))<=1&&!MEM_BU(0xC9,int32_t(ph))&&MEM_BU(0x28,int32_t(parent_physical))==1&&
                MEM_W(0x10,int32_t(ph))==0&&MEM_W(0x14,int32_t(ph))==0&&MEM_W(0x18,int32_t(ph))==0&&
                uint32_t(MEM_W(0,int32_t(update_address)))==original_update&&uint32_t(MEM_W(0,int32_t(parent_update_address)))==original_parent_update;
        };
        auto source_owned=[&] {
            std::vector<int32_t> words;
            for(unsigned slot=30;slot<230;++slot) {
                const auto actor=0x80171CD0u+slot*0xCC;const auto state=MEM_W(0x94,int32_t(actor));if(!state)continue;
                const auto physical=uint32_t(MEM_W(0x98,int32_t(actor))),m=uint32_t(MEM_W(0x74,int32_t(actor)));
                const auto kind=MEM_H(0x20,int32_t(physical));
                const bool idle=kind==int16_t(item_kind)&&state==1&&(uint32_t(MEM_W(0x10,int32_t(physical)))&0x7fffffffu)==0&&(uint32_t(MEM_W(0x14,int32_t(physical)))&0x7fffffffu)==0&&(uint32_t(MEM_W(0x18,int32_t(physical)))&0x7fffffffu)==0;
                const bool settle=kind==int16_t(item_kind)&&state==2&&MEM_BU(0x28,int32_t(physical))==1&&MEM_H(0x34,int32_t(physical))==1&&MEM_H(0x7C,int32_t(physical))==-1;
                const bool attached=actor==effect&&eligible_attachment();
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
#if defined(DORA3_PICKUP_SETTLE_NATIVE_CHECK) || defined(DORA3_PICKUP_PARENT_SOURCE_CHECK)
                        if(settle&&((a==actor&&off==0xA0)||(a==physical&&off==0x14)||(a==m&&off==0x64)||(a==physical&&off==0xC4)||(a==m&&(off==0x78||off==0x74))))value=0;
                        if(settle&&a==m&&off==0x7C)value&=~int32_t(0x02000000u);
                        if(settle&&a==physical&&off==0xC8)value&=~int32_t(0xFF000000u);
                        if(settle&&a==physical&&off==0x28)value&=~int32_t(0x0000FF00u);
#endif
#ifdef DORA3_PICKUP_PARENT_NATIVE_CHECK
                        if(attached&&((a==actor&&(off==0xC||off==0x10||off==0x9C||off==0xA0||off==0xA4))||(a==physical&&off==0xC4)||(a==m&&(off==0x64||off==0x78||off==0x74))))value=0;
                        if(attached&&a==m&&off==0x7C)value&=~int32_t(0x02000000u);
                        if(attached&&a==physical&&off==0xC8)value&=~int32_t(0xFF000000u);
                        if(attached&&a==physical&&off==0x28)value&=~int32_t(0x0000FF00u);
#endif
                        if(kind==131&&((a==actor&&off==0xA0)||(a==m&&off==0x64)))value=0;
                        if(kind==131&&a==m&&off==0x7C)value&=~int32_t(0x02000000u);
                        if(kind==131&&a==physical&&off==0x28)value&=~int32_t(0x0000FF00u);
                        words.push_back(value);
                    }
            }
            return words;
        };
        auto command=[&](unsigned tick) {
            if(tick==2){assert(MEM_W(0x94,int32_t(parent))!=0);wf(rdram,parent+0xC,15.f);wf(rdram,parent+0x10,359.f);}
            if(tick==5){assert(MEM_W(0x94,int32_t(parent))!=0);wf(rdram,parent+0xC,-20.f);wf(rdram,parent+0x10,40.f);}
            if(guard==0){if(tick==3)MEM_B(0xC9,int32_t(ph))=1;if(tick==6)MEM_B(0xC9,int32_t(ph))=0;}
            if(guard==1){if(tick==3)wf(rdram,ph+0x14,7.f);if(tick==6)wf(rdram,ph+0x14,0.f);}
            if(guard==2){if(tick==3)MEM_W(0,int32_t(update_address))=int32_t(0x801E5D84u);if(tick==6)MEM_W(0,int32_t(update_address))=int32_t(original_update);}
            if(guard==3){if(tick==3){wf(rdram,effect+0x9C,700.f);wf(rdram,effect+0xA0,450.f);wf(rdram,effect+0xA4,200.f);wf(rdram,effect+0xC,65.f);wf(rdram,effect+0x10,270.f);}}
            if(guard==4){if(tick==3)MEM_H(0xD8,int32_t(ph))=6;if(tick==6)MEM_H(0xD8,int32_t(ph))=1;}
            if(guard==5){
                if(tick==3){recomp_context seek;dora3::copy_recomp_context(seek,caller);seek.r4=int32_t(model);seek.r5=0;dora3_s0_80039564(rdram,&seek);}
                if(tick==4||tick==6){recomp_context pause;dora3::copy_recomp_context(pause,caller);pause.r4=int32_t(model);pause.r5=0;pause.r6=tick==4?0:1;dora3_s0_80039578(rdram,&pause);}
            }
            if(guard==6){
                if(tick==3)MEM_H(0x7C,int32_t(ph))=-1;
                if(tick==6){MEM_H(0x7C,int32_t(ph))=30;MEM_H(0x34,int32_t(ph))=0;wf(rdram,ph+0x10,0.f);wf(rdram,ph+0x14,0.f);wf(rdram,ph+0x18,0.f);}
            }
            if(guard==7){if(tick==3)MEM_B(0xC9,int32_t(parent_physical))=1;if(tick==6)MEM_B(0xC9,int32_t(parent_physical))=0;}
            if(guard==8){if(tick==3)MEM_W(0,int32_t(parent_update_address))=int32_t(0x801E5D84u);if(tick==6)MEM_W(0,int32_t(parent_update_address))=int32_t(original_parent_update);}
            if(guard==9){
                if(tick==3){recomp_context seek;dora3::copy_recomp_context(seek,caller);seek.r4=int32_t(parent_model);seek.r5=0;dora3_s0_80039564(rdram,&seek);}
                if(tick==4||tick==6){recomp_context pause;dora3::copy_recomp_context(pause,caller);pause.r4=int32_t(parent_model);pause.r5=0;pause.r6=tick==4?0:1;dora3_s0_80039578(rdram,&pause);}
            }
            if(tick==8){assert(MEM_W(0x94,int32_t(parent))!=0);MEM_H(0x7C,int32_t(ph))=-1;}
        };
        unsigned attached_endpoints=0;
        Nanoseconds last_native_stamp=0;
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
                    const auto old_velocity=MEM_W(0x14,int32_t(ph));
                    std::array<int32_t,15> candidates{};for(unsigned i=0;i<candidates.size();++i)candidates[i]=MEM_W(int32_t(4*i),int32_t(0x800DACB8u));
                    std::array<int32_t,52> terrain{};for(unsigned i=0;i<terrain.size();++i)terrain[i]=MEM_W(int32_t(4*i),int32_t(0x80280000u));
                    const auto old_scatter_x=MEM_W(0x9C,int32_t(effect)),old_scatter_z=MEM_W(0xA4,int32_t(effect)),old_scatter_speed=MEM_W(0,int32_t(ph));
                    const auto random_word=MEM_W(0,int32_t(0x801AA620));
                    assert(stamp>=last_native_stamp);const bool positive_dt=stamp>last_native_stamp;
                    std::array<int32_t,5> before_attachment{};
                    const std::array<uint32_t,5> attachment_addresses{effect+0xC,effect+0x10,effect+0x9C,effect+0xA0,effect+0xA4};
                    for(unsigned i=0;i<5;++i)before_attachment[i]=MEM_W(0,int32_t(attachment_addresses[i]));
                    const bool attachment_eligible=eligible_attachment();
                    native_gameplay_preview_update(rdram,&c,stamp);
#ifdef DORA3_PICKUP_PARENT_NATIVE_CHECK
                    if(positive_dt&&attachment_eligible&&tick<8) {
                        std::array<int32_t,5> actual{};for(unsigned i=0;i<5;++i)actual[i]=MEM_W(0,int32_t(attachment_addresses[i]));
                        if(actual!=before_attachment)++attachment_changes;
                        const auto stack=uint32_t(c.r29);std::array<uint8_t,352> saved_stack{};
                        std::memcpy(saved_stack.data(),rdram+((stack-320)&0xFFFFFF),saved_stack.size());
                        const auto old_current=MEM_W(0,int32_t(0x8021BDEC)),old_physical=MEM_W(0,int32_t(0x8021BCDC));
                        MEM_W(0,int32_t(0x8021BDEC))=int32_t(effect);MEM_W(0,int32_t(0x8021BCDC))=int32_t(ph);
                        recomp_context geometry=c;geometry.f_odd=&geometry.f0.u32h;
                        reference_dora3_s3_core_801EBAD4(rdram,&geometry);
                        for(unsigned i=0;i<5;++i)assert(actual[i]==MEM_W(0,int32_t(attachment_addresses[i])));
                        for(unsigned i=0;i<5;++i)MEM_W(0,int32_t(attachment_addresses[i]))=actual[i];
                        std::memcpy(rdram+((stack-320)&0xFFFFFF),saved_stack.data(),saved_stack.size());
                        MEM_W(0,int32_t(0x8021BDEC))=old_current;MEM_W(0,int32_t(0x8021BCDC))=old_physical;
                        ++attachment_geometry_checks;if(tick>=6)++reentry_geometry_checks;
                    }
#endif
#ifdef DORA3_PICKUP_SCATTER_NATIVE_CHECK
                    if(old_state==3) {
                        if(MEM_W(0x10,int32_t(effect))!=old_yaw)++scatter_yaw_samples;
                        if(MEM_W(0x9C,int32_t(effect))!=old_scatter_x||MEM_W(0xA4,int32_t(effect))!=old_scatter_z||MEM_W(0xA0,int32_t(effect))!=old_height)++scatter_position_samples;
                        if(MEM_W(0,int32_t(ph))!=old_scatter_speed)++scatter_speed_samples;
                        assert(random_word==MEM_W(0,int32_t(0x801AA620)));
                        // Live Parent130 commits its genuine ED7F4/E810C query at the Source boundary.
                        // Mid-period forecasts must roll back; full boundary results are compared
                        // with the frozen Original pool below, including all15 candidate words.
                        if(stamp<boundary)for(unsigned i=0;i<candidates.size();++i)assert(candidates[i]==MEM_W(int32_t(4*i),int32_t(0x800DACB8u)));
                        for(unsigned i=0;i<terrain.size();++i)assert(terrain[i]==MEM_W(int32_t(4*i),int32_t(0x80280000u)));
                        ++scatter_rollback_samples;
                    }
#endif
                    if(old_state==2) {
                        if(MEM_W(0xA0,int32_t(effect))!=old_height)++settle_height_samples;
                        if(MEM_W(0x14,int32_t(ph))!=old_velocity)++settle_velocity_samples;
                        // Live Parent130 commits its genuine ED7F4/E810C query at the Source boundary.
                        // Mid-period forecasts must roll back; full boundary results are compared
                        // with the frozen Original pool below, including all15 candidate words.
                        if(stamp<boundary)for(unsigned i=0;i<candidates.size();++i)assert(candidates[i]==MEM_W(int32_t(4*i),int32_t(0x800DACB8u)));
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
                    if(positive_dt&&old_state==2&&!attachment_eligible&&tick<8)++rejected_attachment_calls;
                    if(!positive_dt) {
                        for(unsigned i=0;i<5;++i)assert(before_attachment[i]==MEM_W(0,int32_t(attachment_addresses[i])));
                        ++duplicate_stamp_checks;
                    }
                    last_native_stamp=stamp;
                    assert(source_owned()==before&&std::memcmp(&context,&c,sizeof(c))==0);++intervals;
                };
                const auto start_units=uint64_t(second)*divider*(tick-1);
                const auto midpoint=Nanoseconds((start_units+units+119)/120);
                bool pending=placement==2;
                auto mid_command=[&] {
                    input_now=midpoint;city_source_stamp={Nanoseconds((start_units+units)/120),unsigned((start_units+units)%120)/2};command(tick);pending=false;
                };
                while(second*frame/fps<boundary) {
                    const auto stamp=second*frame/fps;
                    if(pending&&stamp>=midpoint){native_to(midpoint);mid_command();}
                    native_to(stamp);++frame;
                }
                if(pending){native_to(midpoint);mid_command();}
                native_to(boundary);
            }
            if(playback==0&&placement==2) {
                const auto start_units=uint64_t(second)*divider*(tick-1);input_now=Nanoseconds((start_units+units+119)/120);
                city_source_stamp={Nanoseconds((start_units+units)/120),unsigned((start_units+units)%120)/2};command(tick);
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
            if(playback==0)reference_dora3_s3_801E8BDC(rdram,&c);else dora3_s3_801E8BDC(rdram,&c);
            const auto value=snapshot();const auto key=std::array<unsigned,9>{fps,divider,camera,target,item_kind,initial_state,parent_profile,guard,placement};
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
            if(tick<8&&eligible_attachment()) {
                assert(MEM_W(0x94,int32_t(parent))!=0&&MEM_W(0x94,int32_t(effect))==2&&MEM_H(0x7C,int32_t(ph))==30&&MEM_H(0x34,int32_t(ph))==0);
                assert(MEM_W(0xC,int32_t(effect))==MEM_W(0xC,int32_t(parent))&&MEM_W(0x10,int32_t(effect))==MEM_W(0x10,int32_t(parent)));
                ++attached_endpoints;
            }

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
        assert(attached_endpoints>0);
        if(playback==1)oracles.erase(std::array<unsigned,9>{fps,divider,camera,target,item_kind,initial_state,parent_profile,guard,placement});
        assert(oracles.size()<=1);
        native_gameplay_preview_reset(rdram);preview_live=false;slot09_loaded=linked_construct_watch=linked_watch=false;
        for(auto m:owners){c.r4=int32_t(m);dora3_s0_8003B77C(rdram,&c);}
        if(playback==0)++source_cases;else ++native_cases;
    }
    assert(source_cases==paired_worlds&&native_cases==paired_worlds&&endpoints==paired_worlds*256&&source_births==paired_worlds&&intervals>paired_worlds*100&&oracles.empty());
    assert(settle_steps>paired_worlds&&idle_yaw_samples>paired_worlds&&collected_yaw_samples>paired_worlds&&flash_height_samples>paired_worlds);
    std::cout<<"PASS: "<<paired_worlds<<" Original and"<<paired_worlds<<" mixed Native/Source guarded actual Parent130/child118-123-127 complete lifetimes; Source parent index30/strideCC/base80171CD0 and actual pitch/yaw matrix transform of offset(0,-70,0), distinct live registered parent and child models, attached2 continuous ownership explicitly selected by test macro, all remaining raw fields unchanged on Native calls, parent0/5 actual effect profiles,10 child/parent command profiles before/after/inside Native, detach8, genuine no-parent settle reentry/contact140/collection/six-effect/parent retirement; "<<intervals<<" Native calls, "<<endpoints<<" exact actor/physics/model/heap/resource/command/RNG/collision-result frozen Original endpoints; query outcomes/parent controller unmodified; synthetic assets and explicit parent-index/angle/detach inputs, Native attached2 verified only when DORA3_PICKUP_PARENT_NATIVE_CHECK is set; player/other-parent integration not claimed; no game/GPU.\n";
#ifdef DORA3_PICKUP_PARENT_NATIVE_CHECK
    assert(attachment_changes>paired_worlds&&attachment_geometry_checks>paired_worlds&&duplicate_stamp_checks>paired_worlds&&rejected_attachment_calls>0&&reentry_geometry_checks>0);
    std::cout<<"Native attachment intermediate changes="<<attachment_changes<<" exact frozen Original current-parent matrix checks="<<attachment_geometry_checks<<" zero-dt no-pose-change checks="<<duplicate_stamp_checks<<" rejected attachment intervals="<<rejected_attachment_calls<<" reentry geometry checks="<<reentry_geometry_checks<<'\n';
#endif
}
#endif
