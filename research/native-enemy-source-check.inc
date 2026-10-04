#ifdef DORA3_ENEMY_CHECK
struct EnemySnapshot {
    std::vector<int32_t> words;
    std::vector<std::array<int32_t,5>> commands;
    std::set<uint32_t> heap;
    std::array<unsigned,5> resources;
    size_t models;
    bool operator==(const EnemySnapshot&) const=default;
};
static EnemySnapshot enemy_snapshot(uint8_t* rdram) {
    EnemySnapshot value{{},enemy_fixture_commands,allocations,resource_references,game_model_animation().model_count()};
    for(unsigned slot=30;slot<230;++slot) {
        const uint32_t actor=0x80171CD0+slot*0xCC;const auto state=MEM_W(0x94,int32_t(actor));
        value.words.push_back(state);if(!state)continue;
        const auto ph=uint32_t(MEM_W(0x98,int32_t(actor))),model=uint32_t(MEM_W(0x74,int32_t(actor)));
        assert(ph>=0x802A0000&&ph<next_address&&model>=0x802A0000&&model<next_address);
        assert(game_model_animation().lifetime_token(model));
        for(auto [address,size]:std::array<std::pair<uint32_t,unsigned>,3>{{{actor,0xCC},{ph,0xF4},{model,0x84}}})
            for(unsigned off=0;off<size;off+=4)value.words.push_back(MEM_W(int32_t(off),int32_t(address)));
    }
    value.words.push_back(MEM_W(0,int32_t(0x801AA620)));
    for(unsigned off=0;off<0xC4;off+=4)value.words.push_back(MEM_W(int32_t(off),int32_t(0x801B3628)));
    value.words.push_back(MEM_W(0,int32_t(0x8021BDFC)));return value;
}
static void enemy_assert_equal(const EnemySnapshot& value,const EnemySnapshot& expected,unsigned kind,unsigned divider,unsigned profile,unsigned fps,unsigned tick) {
    if(value==expected)return;
    std::cerr<<"Enemy mismatch kind="<<kind<<" divider="<<divider<<" profile="<<profile<<" fps="<<fps<<" tick="<<tick
             <<" words="<<value.words.size()<<"/"<<expected.words.size()<<" commands="<<value.commands.size()<<"/"<<expected.commands.size()
             <<" models="<<value.models<<"/"<<expected.models<<'\n';
    for(unsigned i=0;i<std::min(value.words.size(),expected.words.size());++i)
        if(value.words[i]!=expected.words[i])std::cerr<<" word="<<i<<" actual="<<std::hex<<value.words[i]<<" expected="<<expected.words[i]<<std::dec<<'\n';
    std::abort();
}
static void check_enemy_sources(const std::vector<uint32_t>& image) {
    unsigned original_cases=0,native_cases=0,endpoints=0,intermediate=0,zero_dt=0,births=0,moving_samples=0,contacts=0;
    for(const auto& definition:enemy_fixture_definitions)for(unsigned divider:{2u,3u})for(unsigned profile:{0u,1u,2u}) {
        std::vector<EnemySnapshot> oracle;
        for(unsigned mode=0;mode<8;++mode) {
            const unsigned fps=mode<2?0:std::array<unsigned,6>{60,73,120,144,165,360}[mode-2];
            const bool native=mode>=2;
            auto memory=image;auto* rdram=reinterpret_cast<uint8_t*>(memory.data());seed(rdram,0,true);
            std::memset(rdram+(0x80171CD0&0xFFFFFF),0,230*0xCC);
            MEM_W(0x94,int32_t(player))=1;MEM_W(0,int32_t(0x8021D640))=int32_t(other);
            wf(rdram,player+0x9C,300);wf(rdram,player+0xA0,35);wf(rdram,player+0xA4,10);
            wf(rdram,physics+0x14,0);MEM_W(0x30,int32_t(control))=-1;wf(rdram,control+0x1C,1);
            MEM_H(0x16,int32_t(control))=40;MEM_H(0x18,int32_t(control))=0;
            MEM_W(0,int32_t(0x801B36E0))=0;MEM_B(0,int32_t(0x801B36EC))=0;
            for(unsigned i=0;i<8;++i) {
                const int32_t slot=int32_t(0x801A44F0+i*20);MEM_W(0,slot)=i<2?2:0;
                MEM_W(12,slot)=i;MEM_W(16,slot)=int32_t(i?0x801BB848:0x801BB730);
            }
            for(auto address:{0x8021D484u,0x8021D494u,0x80218AD8u,0x801A9C88u})MEM_W(0,int32_t(address))=0;
            MEM_W(0,int32_t(0x801AA648))=MEM_W(0,int32_t(0x801A9500))=1;
            MEM_B(0,int32_t(0x801A48B4))=1;MEM_H(0,int32_t(0x80171560))=0;
            MEM_B(2,int32_t(0x80171560))=MEM_B(3,int32_t(0x80171560))=MEM_B(4,int32_t(0x80171560))=0;
            recomp_context c{};c.r29=int32_t(0x80300000);c.f_odd=&c.f0.u32h;c.r4=1;
            game_model_animation().reset();allocations.clear();resource_references={};next_address=0x802A0000;
            models(rdram,c);wf(rdram,clip+0x448,60);wf(rdram,params+0x10,15);
            MEM_W(0,int32_t(0x801A39DC))=int(divider);MEM_B(0,int32_t(0x801B361A))=12;
            c.f12.fl=rf(rdram,player+0x9C);c.f14.fl=rf(rdram,player+0xA0);c.r6=MEM_W(0xA4,int32_t(player));
            dora3_s3_801BBDDC(rdram,&c);c.r4=1;const auto caller=c;
            native_gameplay_preview_reset(rdram);preview_live=false;room(rdram);
            for(unsigned n=2;n<4;++n)wf(rdram,0x80280000u+0x20+n*44+36,-5000.f);
            enemy_fixture_rom=definition.rom;enemy_fixture_reference=mode==0;
            enemy_fixture_active=true;enemy_fixture_overlay(memory,definition.rom);enemy_fixture_commands.clear();
            city_loaded=slot09_loaded=city_watch=patrol_watch=city_dialogue_watch=linked_watch=linked_construct_watch=false;
            MEM_W(0,int32_t(0x801AEC00+20))=int32_t(asset);MEM_W(0,int32_t(0x801AEC00+40))=int32_t(clip);
            MEM_W(0,int32_t(0x801AEC00+80))=int32_t(0x80102000u);
            for(unsigned slot=30;slot<230;++slot) {
                c.r4=int32_t(0x80171CD0u+slot*0xCC);dora3_scene_object_init(rdram,&c);MEM_H(0xA8,c.r4)=-1;
            }
            const auto before_heap=allocations;const auto before_refs=resource_references;const auto before_models=game_model_animation().model_count();
            for(unsigned i=0;i<16;++i)MEM_W(4*int32_t(i),int32_t(0x801B3628u))=0;
            MEM_H(0,int32_t(0x801B36B8))=0xA0;
            if(definition.kind>=387&&definition.kind<=390)MEM_H(0,int32_t(0x801B3748))=int16_t(definition.kind-387);
            MEM_H(0,int32_t(0x8021BE10))=31;MEM_H(2,int32_t(0x8021BE10))=-1;
            MEM_W(0,int32_t(0x8021BDF0))=0;
            for(unsigned i=0;i<4;++i)MEM_H(2*int32_t(i),int32_t(0x8021BE02))=int16_t(std::array<int,4>{0,80,0,359}[i]);
            input_now=0;
            if(native) {
                assert(native_gameplay_preview_eligible(rdram));preview_live=true;
                native_gameplay_preview_begin(rdram,0,divider);dora3_native_preview_hook(rdram,&c,0x801BB080);
            }
            c=caller;c.f_odd=&c.f0.u32h;
            (mode==0?definition.reference_create:definition.create)(rdram,&c);
            if(!MEM_W(0x94,int32_t(prop))) {
                // Projectile/effect descriptor entry points are intentionally
                // empty; their genuine parent uses this same Original constructor.
                assert(definition.script);c=caller;c.f_odd=&c.f0.u32h;c.r4=int32_t(definition.script);
                c.r5=std::bit_cast<int32_t>(0.f);c.r6=std::bit_cast<int32_t>(80.f);c.r7=std::bit_cast<int32_t>(0.f);
                MEM_W(0x10,c.r29)=std::bit_cast<int32_t>(359.f);actual_dora3_s3_801E6968(rdram,&c);
            }
            assert(MEM_W(0x94,int32_t(prop)));
            const auto ph=uint32_t(MEM_W(0x98,int32_t(prop))),model=uint32_t(MEM_W(0x74,int32_t(prop)));
            assert(MEM_H(0x20,int32_t(ph))==int16_t(definition.kind));
            if(native){assert(game_model_animation().native_birth(model));++births;}
            if(definition.kind==268||definition.kind==361){MEM_H(0x7C,int32_t(ph))=-1;MEM_H(0x36,int32_t(ph))=1;}
            if(native) {
                // BB080 revokes all domain permissions. The real Source
                // common-pool entry enables this domain again while owned
                // actors await Native time; do not fake the permission flag.
                c=caller;c.f_odd=&c.f0.u32h;dora3_s3_801E8BDC(rdram,&c);
            }
            auto value=enemy_snapshot(rdram);
            if(mode==0)oracle.push_back(value);else enemy_assert_equal(value,oracle[0],definition.kind,divider,profile,fps,0);
            unsigned frame=1;
            for(unsigned tick=1;tick<=96;++tick) {
                const auto units=uint64_t(second)*divider*tick;const auto boundary=Nanoseconds((units+59)/60);
                c=caller;c.f_odd=&c.f0.u32h;
                if(native) {
                    auto native_to=[&](Nanoseconds stamp) {
                        const auto saved=c;const auto position=MEM_W(0x9C,int32_t(prop)),yaw=MEM_W(0x10,int32_t(prop));input_now=stamp;
                        native_gameplay_preview_update(rdram,&c,stamp);assert(std::memcmp(&saved,&c,sizeof(c))==0);
                        if(MEM_W(0x94,int32_t(prop))&&(MEM_W(0x9C,int32_t(prop))!=position||MEM_W(0x10,int32_t(prop))!=yaw))++moving_samples;
                        const auto once=enemy_snapshot(rdram);native_gameplay_preview_update(rdram,&c,stamp);
                        assert(once==enemy_snapshot(rdram)&&std::memcmp(&saved,&c,sizeof(c))==0);++zero_dt;++intermediate;
                    };
                    while(second*frame/fps<boundary){native_to(second*frame/fps);++frame;}native_to(boundary);
                }
                input_now=boundary;c=caller;c.f_odd=&c.f0.u32h;
                if(mode==0)reference_dora3_s3_801E8BDC(rdram,&c);else dora3_s3_801E8BDC(rdram,&c);
                // Commands take effect between periods, equally in all replays.
                if(profile&&tick==18&&MEM_W(0x94,int32_t(prop))) {
                    MEM_W(0,int32_t(0x8021BDEC))=int32_t(prop);MEM_W(0,int32_t(0x8021BCDC))=int32_t(ph);
                    MEM_W(0,int32_t(0x8021BCE4))=profile==1?1:4;
                    const auto address=uint32_t(MEM_W(0,int32_t(0x80202068+definition.kind*84)));
                    auto* contact=enemy_fixture_lookup(definition.rom,address,mode==0);assert(contact);contact(rdram,&c);++contacts;
                }
                if(profile==2&&MEM_W(0x94,int32_t(prop))) {
                    if(tick==24)MEM_B(0xC9,int32_t(ph))=1;
                    if(tick==30)MEM_B(0xC9,int32_t(ph))=0;
                    if(tick==36){wf(rdram,prop+0x9C,rf(rdram,prop+0x9C)+13);wf(rdram,prop+0x10,359);}
                    if(tick==45&&(definition.kind==343||definition.kind==344))MEM_H(0x20,int32_t(ph))=int16_t(definition.kind==343?344:343);
                }
                value=enemy_snapshot(rdram);
                if(mode==0)oracle.push_back(value);else if(!native||divider==3||tick%3==0) {
                    // Source30 endpoints 1/2 have thirds of a nanosecond.
                    // Preview accepts integer wall timestamps: their ceiling
                    // already starts the following AI period. Compare every
                    // third endpoint at exact100ms, never mask guest counters
                    // or pretend that a ceiling is the rational Source end.
                    enemy_assert_equal(value,oracle.at(tick),definition.kind,divider,profile,fps,tick);++endpoints;
                }
            }
            // Genuine common retirement must return every owned resource.
            c=caller;c.f_odd=&c.f0.u32h;
            for(unsigned slot=30;slot<230;++slot) {
                const uint32_t actor=0x80171CD0+slot*0xCC;if(!MEM_W(0x94,int32_t(actor)))continue;
                c.r4=int32_t(actor);dora3_s3_801E6E6C(rdram,&c);
            }
            assert(allocations==before_heap&&resource_references==before_refs&&game_model_animation().model_count()==before_models);
            preview_live=false;native_gameplay_preview_reset(rdram);enemy_fixture_active=false;
            if(native)++native_cases;else ++original_cases;
        }
    }
    assert(original_cases==300&&native_cases==900&&births==900&&moving_samples>0);
    std::cout<<"PASS: enemy "<<original_cases<<" frozen/current Original and "<<native_cases<<" Native schedules; "<<endpoints
             <<" pool endpoints, "<<intermediate<<" intervals, "<<zero_dt<<" zero-dt checks, "<<contacts<<" genuine contacts; 25 kinds/4 ROM overlays, actual constructors/retirement on synthetic assets; no game/GPU.\n";
}
#endif
