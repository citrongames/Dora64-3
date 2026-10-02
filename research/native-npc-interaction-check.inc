unsigned check_npc_action_contacts(const std::vector<uint32_t>& image) {
    // Run after the earlier dispatcher assertions, with independent RDRAM
    // and model lifetimes. The Original selector/dispatcher is the oracle.
    script_objects=world_init_watch=linked_construct_watch=linked_watch=false;
    city_dialogue_watch=patrol_watch=patrol_original_reference=false;
    death_watch=respawn_watch=door_watch=weapon_watch=repeat_watch=false;
    native_damage_rumble_watch=flight_damage_watch=special_command_watch=false;
    initial_geometry_watch=capture_birth=false;binding=nullptr;
    unsigned cases=0;
    for(unsigned divider:{3u,2u})for(unsigned fps:{60u,73u,120u,144u,165u,360u})for(unsigned kind:{13u,14u,15u,16u,17u,18u,20u,21u,22u,75u,76u,77u})for(unsigned scenario=0;scenario<12;++scenario)for(bool native:{false,true}) {
        if(scenario==11&&kind!=18)continue;
        auto memory=image;auto* rdram=reinterpret_cast<uint8_t*>(memory.data());seed(rdram,0,true);
        std::memset(rdram+(0x80171CD0&0xFFFFFF),0,230*0xCC);
        recomp_context c{};c.f_odd=&c.f0.u32h;c.r29=int32_t(0x80300000);
        native_gameplay_preview_reset(rdram);preview_live=false;input_now=0;
        for(unsigned i=0;i<8;++i) {
            const auto slot=int32_t(0x801A44F0+i*20);MEM_W(0,slot)=i<2?2:0;MEM_W(12,slot)=i;
            MEM_W(16,slot)=int32_t(i?0x801BB848:0x801BB730);
        }
        for(auto p:{0x8021D484u,0x8021D494u,0x80218AD8u,0x801A9C88u})MEM_W(0,int32_t(p))=0;
        MEM_W(0,int32_t(0x801AA648))=MEM_W(0,int32_t(0x801A9500))=1;
        MEM_W(0x94,int32_t(player))=1;MEM_W(0,int32_t(0x8021D640))=int32_t(other);
        wf(rdram,player+0x9C,0);wf(rdram,player+0xA0,0);wf(rdram,player+0xA4,0);
        wf(rdram,physics,0);wf(rdram,physics+0x14,0);MEM_W(0x30,int32_t(control))=-1;
        wf(rdram,control+0x1C,1);MEM_H(0x16,int32_t(control))=40;
        MEM_H(0xD8,int32_t(physics))=1;MEM_H(0xD4,int32_t(physics))=-1;
        MEM_W(0,int32_t(0x801B36E0))=1;MEM_B(0,int32_t(0x801B36EC))=0;
        MEM_B(0,int32_t(0x8021710D))=0;MEM_B(0,int32_t(0x8021710C))=0;
        MEM_H(0,int32_t(0x80171560))=0;MEM_B(2,int32_t(0x80171560))=0;MEM_B(3,int32_t(0x80171560))=0;
        MEM_B(0,int32_t(0x801A48B4))=1;MEM_B(4,int32_t(0x80171560))=0;
        dora3_controller_accumulate(rdram,&c);dora3_controller_latch(rdram,&c);
        game_model_animation().reset();allocations.clear();next_address=0x802A0000;
        const auto player_owners=models(rdram,c);
        std::vector<uint32_t> owners(player_owners.begin(),player_owners.end());
        const uint32_t npc=0x80171CD0u+(scenario==7?229u:scenario==9?31u:30u)*0xCC;
        constexpr uint32_t npc_physics=0x80256000,collider=asset+0x900,index_list=asset+0x880;
        MEM_W(0x94,int32_t(npc))=1;MEM_W(0x98,int32_t(npc))=int32_t(npc_physics);
        MEM_H(0x20,int32_t(npc_physics))=int16_t(kind);MEM_H(0xD8,int32_t(npc_physics))=1;
        MEM_H(0xD4,int32_t(npc_physics))=-1;MEM_H(0x7C,int32_t(npc_physics))=-1;
        for(unsigned off=0x80;off<=0x9E;off+=2)MEM_H(int32_t(off),int32_t(npc_physics))=-1;
        // The actual kind77 contact producer binds an actor clip immediately.
        // Supply a genuinely constructed model and half-index clip list.
        c.r4=int32_t(asset);dora3_s0_8003B560(rdram,&c);
        const auto npc_model=uint32_t(c.r2);owners.push_back(npc_model);
        c.r4=int32_t(npc_model);c.r5=int32_t(clip);dora3_s0_8003B6BC(rdram,&c);
        MEM_W(0x74,int32_t(npc))=int32_t(npc_model);
        MEM_W(0xB4,int32_t(npc))=int32_t(asset+0x800);
        for(unsigned off=0;off<6;off+=2)MEM_H(int32_t(off),int32_t(asset+0x800))=0;
        MEM_H(0,int32_t(index_list))=100;MEM_W(0,int32_t(0x801AEC00+100*20))=int32_t(collider-4);
        MEM_H(8,int32_t(collider))=200;MEM_H(0x12,int32_t(collider))=1;
        MEM_H(0x12+8,int32_t(collider))=50;MEM_H(0x12+0xA,int32_t(collider))=-1;
        MEM_H(0x24,int32_t(collider))=-1;
        for(auto actor:{player,npc}) {
            const auto physical=uint32_t(MEM_W(0x98,int32_t(actor)));
            MEM_H(0x22,int32_t(physical))=0;MEM_B(0x6A,int32_t(actor))=1;
            MEM_W(0xBC,int32_t(actor))=int32_t(index_list);
            for(unsigned off:{0x18u,0x1Cu,0x20u})wf(rdram,actor+off,1);
            for(unsigned off=0;off<64;off+=4)wf(rdram,actor+0x24+off,off%20==0?1.f:0.f);
        }
        city_loaded=city_watch=dialogue_actor_loaded=true;dialogue_actor_watch=false;
        npc_action_watch=true;npc_action_actor=npc;npc_action_sounds=0;city_identity_verified=true;city_commands.clear();
        assert(MEM_BU(0,int32_t(0x80202091+kind*84))==1);
        const bool accepted=scenario==0||scenario==5||scenario==7||scenario==8||scenario==10||scenario==11||(scenario==4&&kind>=75);
        const auto dialogue_state=kind==18||kind>=75?2:3;
        if(scenario==1) {wf(rdram,npc+0x9C,1000);MEM_H(0xD4,int32_t(physics))=30;} // Stale Source contact.
        if(scenario==2)MEM_W(0x94,int32_t(npc))=0;
        if(scenario==3)MEM_B(0xC8,int32_t(npc_physics))=2;
        if(scenario==4)MEM_W(0x94,int32_t(npc))=100; // Original producer rejects this state.
        if(scenario==5)MEM_H(0xD4,int32_t(physics))=200; // New edge must discover the current collider.
        if(scenario==6)MEM_H(0xD8,int32_t(physics))=20; // Original damage cooldown forbids talk.
        constexpr uint32_t secondary=0x80171CD0u+31u*0xCC,secondary_physics=0x80257000;
        if(scenario>=8) {
            const auto extra=scenario==9?0x801734B8u:secondary;
            std::memcpy(rdram+(extra&0xFFFFFF),rdram+(npc&0xFFFFFF),0xCC);
            std::memcpy(rdram+(secondary_physics&0xFFFFFF),rdram+(npc_physics&0xFFFFFF),0xF4);
            MEM_W(0x98,int32_t(extra))=int32_t(secondary_physics);
            MEM_H(0x20,int32_t(secondary_physics))=scenario==8?18:scenario==11?19:0;
            if(scenario==11) {
                wf(rdram,npc+0x9C,1000);
                MEM_H(0x7C,int32_t(secondary_physics))=30;
                MEM_H(0x80,int32_t(npc_physics))=31;
            }
        }
        const Nanoseconds press=scenario%2?73000000:second/60;
        const auto selected_b_before=MEM_H(0,int32_t(0x801AC69C));
        if(native) {
            assert(native_gameplay_preview_eligible(rdram));
            preview_live=true;native_gameplay_preview_begin(rdram,0,divider);
            dora3_native_preview_hook(rdram,&c,0x801BB080); // Source command barrier, arm only the actual player.
            c.r4=1;dora3_s3_801C8B28(rdram,&c);
        }
        input_now=press;MEM_H(0,int32_t(0x80171560))=int16_t(0x4000);
        dora3_controller_accumulate(rdram,&c);
        if(native) {
            const Nanoseconds end=accepted?500000000:press+second/30;
            const auto input_context=c;
            for(unsigned frame=1;second*frame/fps<=end;++frame) {
                native_gameplay_preview_update(rdram,&c,second*frame/fps);
                assert(std::memcmp(&c,&input_context,sizeof(c))==0);
            }
            native_gameplay_preview_update(rdram,&c,end);
            assert(std::memcmp(&c,&input_context,sizeof(c))==0);
            assert(MEM_H(0,int32_t(0x801AC69C))==selected_b_before);
        } else {
            dora3_controller_latch(rdram,&c);c.r4=0;dora3_select_controller(rdram,&c);
            MEM_H(0xD4,int32_t(physics))=-1;
            dora3_s3_801DCB78(rdram,&c);dora3_s3_801DD0DC(rdram,&c);
            if(accepted||scenario==4)assert(MEM_H(0xD4,int32_t(physics))==int(scenario==7?229:scenario==11?31:30));
            else assert(MEM_H(0xD4,int32_t(physics))==(scenario==9?30:-1));
            dora3_s3_801DD8FC(rdram,&c);c.r4=1;dora3_s3_801C8B28(rdram,&c);
        }
        const auto actual_npc=MEM_W(0x94,int32_t(npc)),actual_player=MEM_W(0x94,int32_t(player));
        if((accepted&&(actual_npc!=dialogue_state||MEM_BU(0,int32_t(0x8021710D))!=1||(actual_player!=65&&actual_player!=66))) ||
           (!accepted&&(actual_npc!=int(scenario==2?0:scenario==4?100:1)||MEM_BU(0,int32_t(0x8021710D))!=0||(scenario!=9&&actual_player!=28&&actual_player!=29)))) {
            std::cerr<<"NPC B regression native="<<native<<" fps="<<fps<<" original_hz="<<60/divider
                     <<" kind="<<kind<<" scenario="<<scenario<<" npc_state="<<MEM_W(0x94,int32_t(npc))<<" player_state="<<MEM_W(0x94,int32_t(player))
                     <<" dialogue_busy="<<unsigned(MEM_BU(0,int32_t(0x8021710D)))<<'\n';return 0;
        }
        unsigned initializers=0;
        for(const auto& command:city_commands)if(command.site==0x801B91D4)++initializers;
        assert(npc_action_sounds==((accepted&&kind==77)?1u:0u));
        assert(initializers==(accepted?1u:0u)); // Held B must not replay the dialogue command.
        if(accepted)assert(uint32_t(MEM_W(0,int32_t(0x8021BDEC)))==npc);
        if(scenario==8)assert(MEM_W(0x94,int32_t(secondary))==1);
        native_gameplay_preview_reset(rdram);preview_live=false;game_model_animation().reset();
        for(auto model:owners){c.r4=int32_t(model);dora3_s0_8003B77C(rdram,&c);}
        assert(allocations.empty()&&game_model_animation().model_count()==0);
        npc_action_watch=false;city_watch=city_loaded=dialogue_actor_loaded=false;
        ++cases;
    }
    std::cout<<"PASS: "<<cases<<" real common NPC collision/B priority, twelve actual NPC kinds in city and two other overlays, first-contact priority/non-talk obstacles/linked parent, stale/fresh contact, far/inactive/hidden/state/cooldown rejection, final pool slot and held-B cases; Original20/30 and Native60/73/120/144/165/360; ASan/UBSan/LSan; no game/GPU.\n";
    return cases;
}
