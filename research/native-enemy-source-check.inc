#ifdef DORA3_ENEMY_CHECK
#include "native_enemy.hpp"
struct EnemySnapshot {
    std::vector<int32_t> words;
    std::vector<std::array<int32_t,5>> commands;
    std::set<uint32_t> heap;
    std::array<unsigned,5> resources;
    size_t models;
    bool operator==(const EnemySnapshot&) const=default;
};
static std::vector<uint32_t> enemy_snapshot_addresses;
static EnemySnapshot enemy_snapshot(uint8_t* rdram) {
    enemy_snapshot_addresses.clear();
    EnemySnapshot value{{},enemy_fixture_commands,allocations,resource_references,game_model_animation().model_count()};
    for(unsigned slot=30;slot<230;++slot) {
        const uint32_t actor=0x80171CD0+slot*0xCC;const auto state=MEM_W(0x94,int32_t(actor));
        value.words.push_back(state);enemy_snapshot_addresses.push_back(actor+0x94);if(!state)continue;
        const auto ph=uint32_t(MEM_W(0x98,int32_t(actor))),model=uint32_t(MEM_W(0x74,int32_t(actor)));
        assert(ph>=0x802A0000&&ph<next_address&&model>=0x802A0000&&model<next_address);
        assert(game_model_animation().lifetime_token(model));
        for(auto [address,size]:std::array<std::pair<uint32_t,unsigned>,3>{{{actor,0xCC},{ph,0xF4},{model,0x84}}})
            for(unsigned off=0;off<size;off+=4){value.words.push_back(MEM_W(int32_t(off),int32_t(address)));enemy_snapshot_addresses.push_back(address+off);}
    }
    value.words.push_back(MEM_W(0,int32_t(0x801AA620)));enemy_snapshot_addresses.push_back(0x801AA620);
    for(unsigned off=0;off<0xC4;off+=4){value.words.push_back(MEM_W(int32_t(off),int32_t(0x801B3628)));enemy_snapshot_addresses.push_back(0x801B3628+off);}
    value.words.push_back(MEM_W(0,int32_t(0x8021BDFC)));enemy_snapshot_addresses.push_back(0x8021BDFC);return value;
}
static void enemy_assert_equal(const EnemySnapshot& value,const EnemySnapshot& expected,unsigned kind,unsigned divider,unsigned profile,unsigned fps,unsigned tick) {
    if(value==expected)return;
    std::cerr<<"Enemy mismatch kind="<<kind<<" divider="<<divider<<" profile="<<profile<<" fps="<<fps<<" tick="<<tick
             <<" words="<<value.words.size()<<"/"<<expected.words.size()<<" commands="<<value.commands.size()<<"/"<<expected.commands.size()
             <<" models="<<value.models<<"/"<<expected.models<<'\n';
    for(unsigned i=0;i<std::min(value.words.size(),expected.words.size());++i)
        if(value.words[i]!=expected.words[i])std::cerr<<" word="<<i<<" address="<<std::hex<<enemy_snapshot_addresses[i]<<" actual="<<value.words[i]<<" expected="<<expected.words[i]<<std::dec<<'\n';
    for(unsigned i=0;i<std::min(value.commands.size(),expected.commands.size());++i)if(value.commands[i]!=expected.commands[i]) {
        std::cerr<<" command="<<i<<" actual=";for(auto x:value.commands[i])std::cerr<<std::hex<<uint32_t(x)<<',';
        std::cerr<<" expected=";for(auto x:expected.commands[i])std::cerr<<std::hex<<uint32_t(x)<<',';std::cerr<<std::dec<<'\n';
    }
    const auto snapshot_word=[&](uint32_t address) {
        const auto found=std::find(enemy_snapshot_addresses.begin(),enemy_snapshot_addresses.end(),address);
        assert(found!=enemy_snapshot_addresses.end());return value.words[size_t(found-enemy_snapshot_addresses.begin())];
    };
    for(unsigned slot=30;slot<230;++slot) {
        const uint32_t actor=0x80171CD0u+slot*0xCC;const auto state=snapshot_word(actor+0x94);
        if(!state)continue;
        const uint32_t physical=uint32_t(snapshot_word(actor+0x98));
        std::cerr<<" owner slot="<<slot<<" kind="<<(uint32_t(snapshot_word(physical+0x20))>>16)
                 <<" state="<<state<<" root="<<int16_t(uint32_t(snapshot_word(physical+0x7C))>>16)
                 <<" yaw="<<std::bit_cast<float>(snapshot_word(actor+0x10))<<'\n';
    }
    if(value.heap!=expected.heap)std::cerr<<" heap ownership differs\n";
    if(value.resources!=expected.resources) {std::cerr<<" resources actual=";for(auto x:value.resources)std::cerr<<x<<',';std::cerr<<" expected=";for(auto x:expected.resources)std::cerr<<x<<',';std::cerr<<'\n';}
    std::abort();
}
// Majin toggles bones9--12; Boss3 attaches at13/7 and Boss4 at6.
// All eleven indexed Source SDK calls have constant indices <=13.
// Construct complete synthetic assets for those families, using B39C's proven
// 48-byte pose records and relative child offsets. Keep the actual SDK
// constructor, traversal, visibility commands and retirement intact.
static uint32_t enemy_model_asset(uint8_t* rdram,unsigned kind) {
    const unsigned nodes=(kind==385||kind==386)?13:(kind>=428&&kind<=445)?14:0;
    if(!nodes)return asset;
    constexpr uint32_t skeleton=0x80260000;
    std::memset(rdram+(skeleton&0xFFFFFF),0,0xA00);
    std::memcpy(rdram+(skeleton&0xFFFFFF),rdram+(asset&0xFFFFFF),0x100);
    MEM_W(0x14,int32_t(skeleton))=0x100;MEM_H(0x2A,int32_t(skeleton))=int16_t(nodes);
    MEM_W(0x2C,int32_t(skeleton))=0x500;MEM_W(0x34,int32_t(skeleton))=0x800;
    MEM_W(0x800,int32_t(skeleton))=0x900;
    for(unsigned n=0;n<nodes;++n) {
        const uint32_t pose=skeleton+0x100+n*48;
        MEM_W(0,int32_t(pose))=n;MEM_W(0x2C,int32_t(pose))=n+1<nodes?1:0;
        for(unsigned off:{0x1Cu,0x20u,0x24u})wf(rdram,pose+off,1);
    }
    std::memcpy(rdram+((skeleton+0x900)&0xFFFFFF),rdram+((asset+0x300)&0xFFFFFF),0x10);
    return skeleton;
}
static void check_enemy_sources(const std::vector<uint32_t>& image) {
    const char* diagnostic_kind=std::getenv("DORA3_ENEMY_DIAGNOSTIC_KIND");
    const char* shard_count_text=std::getenv("DORA3_ENEMY_SHARD_COUNT");
    const char* shard_index_text=std::getenv("DORA3_ENEMY_SHARD_INDEX");
    const unsigned shard_count=shard_count_text?unsigned(std::strtoul(shard_count_text,nullptr,10)):1;
    const unsigned shard_index=shard_index_text?unsigned(std::strtoul(shard_index_text,nullptr,10)):0;
    assert(shard_count>=1&&shard_count<=8&&shard_index<shard_count);
    assert(!diagnostic_kind||!shard_count_text);
    std::set<unsigned> requested_kinds;
    if(const char* requested=std::getenv("DORA3_ENEMY_INCLUDE_KINDS")) {
        std::string values(requested);std::size_t at=0;
        while(at<values.size()) {const auto end=values.find(',',at);const auto item=values.substr(at,end-at);assert(!item.empty());requested_kinds.insert(unsigned(std::stoul(item)));if(end==std::string::npos)break;at=end+1;}
        assert(!requested_kinds.empty());
    }
    std::set<unsigned> selected_kinds;
    std::ofstream progress;
    if(const char* path=std::getenv("DORA3_ENEMY_PROGRESS_PATH"))progress.open(path);
    unsigned original_cases=0,native_cases=0,endpoints=0,intermediate=0,zero_dt=0,births=0,moving_samples=0,contacts=0;
    std::map<unsigned,std::set<int32_t>> reached;
    unsigned rabbit_landings=0,rabbit_substeps=0,rabbit_ground_hits=0,rabbit_stable_ground=0;
    // Evaluate the previously unexercised boss tail first, preserving every
    // complete case on a fresh image, all modes and exact comparisons.
    std::vector<EnemyFixtureDefinition> case_order(std::begin(enemy_fixture_definitions),std::end(enemy_fixture_definitions));
    std::stable_sort(case_order.begin(),case_order.end(),[](const auto& a,const auto& b){return (a.kind>=399)>(b.kind>=399);});
    for(const auto& definition:case_order)for(unsigned divider:{2u,3u})for(unsigned profile:{0u,1u,2u}) {
        if(diagnostic_kind&&definition.kind!=std::strtoul(diagnostic_kind,nullptr,10))continue;
        if(!requested_kinds.empty()&&!requested_kinds.count(definition.kind))continue;
        if(definition.kind%shard_count!=shard_index)continue;
        selected_kinds.insert(definition.kind);
        if(progress){progress<<"RUN kind="<<definition.kind<<" divider="<<divider<<" profile="<<profile<<std::endl;}
        std::vector<EnemySnapshot> oracle;
        for(unsigned mode=0;mode<8;++mode) {
            const unsigned fps=mode<2?0:std::array<unsigned,6>{60,73,120,144,165,360}[mode-2];
            const bool native=mode>=2;
            enemy_fixture_kind=definition.kind;enemy_fixture_mode=mode;enemy_fixture_profile=profile;enemy_fixture_tick=0;
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
            enemy_fixture_rom=definition.rom;enemy_fixture_parent_rom=0;enemy_fixture_reference=mode==0;
            enemy_fixture_active=true;enemy_fixture_overlay(memory,definition.rom);
            if(definition.constructor_rom!=definition.rom)enemy_fixture_overlay(memory,definition.constructor_rom);
            enemy_fixture_commands.clear();
            city_loaded=slot09_loaded=city_watch=patrol_watch=city_dialogue_watch=linked_watch=linked_construct_watch=false;
            MEM_W(0,int32_t(0x801AEC00+20))=int32_t(enemy_model_asset(rdram,definition.kind));MEM_W(0,int32_t(0x801AEC00+40))=int32_t(clip);
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
            uint32_t target_actor=prop;
            if(definition.parent_kind) {
                const auto root=std::find_if(std::begin(enemy_fixture_definitions),std::end(enemy_fixture_definitions),[&](const auto& d){return d.kind==definition.parent_kind;});
                assert(root!=std::end(enemy_fixture_definitions));enemy_fixture_parent_rom=root->rom;
                if(root->rom!=definition.rom&&root->rom!=0x70A50u)enemy_fixture_overlay(memory,root->rom);
                (mode==0?root->reference_create:root->create)(rdram,&c);
                if(definition.creates_actor) {
                    // An independently spawned variant retains its complete
                    // real constructor/post-parser setup. The cohort owner
                    // is present without inventing an E70E0 attachment.
                    c=caller;c.f_odd=&c.f0.u32h;
                    (mode==0?definition.reference_create:definition.create)(rdram,&c);
                }
            } else if(definition.create)(mode==0?definition.reference_create:definition.create)(rdram,&c);
            auto find_target=[&]() {
                for(unsigned slot=30;slot<230;++slot) {
                    const uint32_t actor=0x80171CD0+slot*0xCC,physical=uint32_t(MEM_W(0x98,int32_t(actor)));
                    if(MEM_W(0x94,int32_t(actor))&&physical&&MEM_H(0x20,int32_t(physical))==int16_t(definition.kind)){target_actor=actor;return true;}
                }
                return false;
            };
            bool found=find_target();
            if(!found&&definition.parent_kind&&MEM_W(0x94,int32_t(prop))==1) {
                // Some genuine constructors allocate the body alone; the
                // first Source FSM entry creates and configures its parts.
                // Bironn assigns each segment's count/index after E70E0.
                // Run that real initialization, with its actual globals,
                // before considering a separately produced outbound effect.
                MEM_W(0,int32_t(0x8021BDEC))=int32_t(prop);
                MEM_W(0,int32_t(0x8021BCDC))=MEM_W(0x98,int32_t(prop));
                MEM_W(0,int32_t(0x8021BDF0))=MEM_W(0x74,int32_t(prop));
                const auto address=uint32_t(MEM_W(0,int32_t(0x80202064+definition.parent_kind*84)));
                auto* initialize=enemy_fixture_lookup(enemy_fixture_parent_rom,address,mode==0);assert(initialize);
                c=caller;c.f_odd=&c.f0.u32h;initialize(rdram,&c);found=find_target();
            }
            if(!found) {
                // Run the genuine parser or linked producer, never synthesize
                // physical/model records or a fake allocation success.
                assert(definition.script);c=caller;c.f_odd=&c.f0.u32h;
                // TwinFaceFire is created independently by DE00:DF9C
                // (E6968), with copied launch properties via E63B4. Its
                // linked sibling uses E70E0 instead; attaching Fire there
                // invents a parent slot that its own retirement never clears.
                if(definition.parent_kind&&definition.linked_spawn&&MEM_W(0x94,int32_t(prop))) {
                    c.r4=int32_t(prop);c.r5=definition.link_index;c.r6=int32_t(definition.script);c.r7=std::bit_cast<int32_t>(0.f);
                    MEM_W(0x10,c.r29)=std::bit_cast<int32_t>(80.f);MEM_W(0x14,c.r29)=0;MEM_W(0x18,c.r29)=std::bit_cast<int32_t>(359.f);
                    dora3_s3_demo_801E70E0(rdram,&c);target_actor=uint32_t(c.r2);
                } else {
                    c.r4=int32_t(definition.script);c.r5=0;c.r6=std::bit_cast<int32_t>(80.f);c.r7=0;
                    MEM_W(0x10,c.r29)=std::bit_cast<int32_t>(359.f);actual_dora3_s3_801E6968(rdram,&c);target_actor=uint32_t(c.r2);
                    if(definition.kind==328) {
                        assert(target_actor);c.r4=int32_t(prop);c.r5=int32_t(target_actor);
                        dora3_s3_demo_801E63B4(rdram,&c);
                        c.r4=int32_t(target_actor);c.r5=2;c.r6=0;c.r7=1;dora3_object_axis_b_set(rdram,&c);
                    }
                }
            }
            assert(MEM_W(0x94,int32_t(target_actor)));
            const auto ph=uint32_t(MEM_W(0x98,int32_t(target_actor))),model=uint32_t(MEM_W(0x74,int32_t(target_actor)));
            assert(MEM_H(0x20,int32_t(ph))==int16_t(definition.kind));
            if(native){assert(native_enemy_owned(rdram,target_actor));assert(game_model_animation().native_birth(model));++births;}
            // Linked producers own attachment/release state and parent slots.
            // Preserve their actual initialization through the Source launch.
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
                enemy_fixture_tick=tick;
                const auto units=uint64_t(second)*divider*tick;const auto boundary=Nanoseconds((units+59)/60);
                c=caller;c.f_odd=&c.f0.u32h;
                if(native) {
                    auto native_to=[&](Nanoseconds stamp) {
                        const auto saved=c;const auto position=MEM_W(0x9C,int32_t(target_actor)),yaw=MEM_W(0x10,int32_t(target_actor));input_now=stamp;
                        const int32_t rabbit_state=MEM_W(0x94,int32_t(target_actor)),rabbit_phase=MEM_H(0x34,int32_t(ph));
                        const float rabbit_y=rf(rdram,target_actor+0xA0);
                        native_gameplay_preview_update(rdram,&c,stamp);assert(std::memcmp(&saved,&c,sizeof(c))==0);
                        if(definition.kind==267&&profile==0&&rabbit_state==2&&MEM_W(0x94,int32_t(target_actor))==2&&
                           (rabbit_phase==1||rabbit_phase==2)&&(MEM_H(0x34,int32_t(ph))==1||MEM_H(0x34,int32_t(ph))==2)) {
                            // FEAC phase1 descends; E810C floor result1 sets
                            // phase2. Require a monotonic fractional approach
                            // to the real Y=0 plane and no ground rebound while
                            // phase2 waits for the actual landing animation.
                            const float y=rf(rdram,target_actor+0xA0);
                            if(!(y>=0&&y<=rabbit_y+1e-4f)) {
                                std::cerr<<"Rabbit landing reversal fps="<<fps<<" divider="<<divider<<" tick="<<tick<<" y="<<rabbit_y<<"->"<<y<<'\n';std::abort();
                            }
                            if(rabbit_phase==1&&MEM_H(0x34,int32_t(ph))==2)++rabbit_landings;
                            if(rabbit_y>0&&y==0)++rabbit_ground_hits;
                            if(rabbit_y==0&&y==0)++rabbit_stable_ground;
                        }
                        if(MEM_W(0x94,int32_t(target_actor))&&(MEM_W(0x9C,int32_t(target_actor))!=position||MEM_W(0x10,int32_t(target_actor))!=yaw))++moving_samples;
                        const auto once=enemy_snapshot(rdram);native_gameplay_preview_update(rdram,&c,stamp);
                        assert(once==enemy_snapshot(rdram)&&std::memcmp(&saved,&c,sizeof(c))==0);++zero_dt;++intermediate;
                        if((definition.kind==266||definition.kind==267)&&MEM_W(0x94,int32_t(target_actor))) {
                            assert(std::isfinite(rf(rdram,target_actor+0xA0)));++rabbit_substeps;
                        }
                    };
                    while(second*frame/fps<boundary){native_to(second*frame/fps);++frame;}native_to(boundary);
                }
                input_now=boundary;c=caller;c.f_odd=&c.f0.u32h;
                if(mode==0)reference_dora3_s3_801E8BDC(rdram,&c);else dora3_s3_801E8BDC(rdram,&c);
                // Commands take effect between periods, equally in all replays.
                if(profile&&tick==18&&MEM_W(0x94,int32_t(target_actor))) {
                    MEM_W(0,int32_t(0x8021BDEC))=int32_t(target_actor);MEM_W(0,int32_t(0x8021BCDC))=int32_t(ph);
                    // Private contacts distinguish a whole-actor hit (-1)
                    // from a valid component index into the owner's slots.
                    MEM_W(0,int32_t(0x8021BDF8))=-1;
                    MEM_W(0,int32_t(0x8021BCE4))=profile==1?1:4;
                    const auto address=uint32_t(MEM_W(0,int32_t(0x80202068+definition.kind*84)));
                    auto* contact=enemy_fixture_lookup(definition.rom,address,mode==0);assert(contact);contact(rdram,&c);++contacts;
                }
                if(profile==2&&MEM_W(0x94,int32_t(target_actor))) {
                    if(tick==24)MEM_B(0xC9,int32_t(ph))=1;
                    if(tick==30)MEM_B(0xC9,int32_t(ph))=0;
                    if(tick==36){wf(rdram,target_actor+0x9C,rf(rdram,target_actor+0x9C)+13);wf(rdram,target_actor+0x10,359);}
                    if(tick==45&&(definition.kind==343||definition.kind==344))MEM_H(0x20,int32_t(ph))=int16_t(definition.kind==343?344:343);
                }
                if(MEM_W(0x94,int32_t(target_actor))) {
                    reached[definition.kind].insert(MEM_W(0x94,int32_t(target_actor)));

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
            // Soundorago's 800/1200-byte history belongs to its own death
            // callback, not the common actor destructor. After the compared
            // 96 ticks, request actual Source contact0 and let Source death
            // reach genuine geometry contact before generic fixture teardown.
            // This tail checks Source cleanup, not Native death animation.
            if(definition.kind>=300&&definition.kind<=307) {
                preview_live=false;native_gameplay_preview_reset(rdram);
                for(unsigned slot=30;slot<230;++slot) {
                    const uint32_t actor=0x80171CD0u+slot*0xCC,physical=uint32_t(MEM_W(0x98,int32_t(actor)));
                    if(!MEM_W(0x94,int32_t(actor))||!physical)continue;
                    const int kind=MEM_H(0x20,int32_t(physical));if(kind!=300&&kind!=305)continue;
                    MEM_W(0,int32_t(0x8021BDEC))=int32_t(actor);MEM_W(0,int32_t(0x8021BCDC))=int32_t(physical);
                    MEM_W(0,int32_t(0x8021BDF0))=MEM_W(0x74,int32_t(actor));MEM_W(0,int32_t(0x8021BCE4))=0;MEM_W(0,int32_t(0x8021BDF8))=-1;
                    const auto address=uint32_t(MEM_W(0,int32_t(0x80202068+kind*84)));
                    auto* contact=enemy_fixture_lookup(0x1561E0u,address,mode==0);assert(contact);
                    c=caller;c.f_odd=&c.f0.u32h;contact(rdram,&c);
                }
                auto root_alive=[&]() {
                    for(unsigned slot=30;slot<230;++slot) {
                        const uint32_t actor=0x80171CD0u+slot*0xCC,physical=uint32_t(MEM_W(0x98,int32_t(actor)));
                        if(!MEM_W(0x94,int32_t(actor))||!physical)continue;
                        const int kind=MEM_H(0x20,int32_t(physical));if(kind==300||kind==305)return true;
                    }
                    return false;
                };
                for(unsigned step=0;step<512&&root_alive();++step) {
                    c=caller;c.f_odd=&c.f0.u32h;
                    if(mode==0)reference_dora3_s3_801E8BDC(rdram,&c);else dora3_s3_801E8BDC(rdram,&c);
                }
                assert(!root_alive());
            }
            // Genuine common retirement must return every owned resource.
            c=caller;c.f_odd=&c.f0.u32h;
            for(unsigned slot=30;slot<230;++slot) {
                const uint32_t actor=0x80171CD0+slot*0xCC;if(!MEM_W(0x94,int32_t(actor)))continue;
                c.r4=int32_t(actor);dora3_s3_801E6E6C(rdram,&c);
            }
            if(allocations!=before_heap||resource_references!=before_refs||game_model_animation().model_count()!=before_models) {
                std::cerr<<"Enemy retirement mismatch kind="<<definition.kind<<" divider="<<divider<<" profile="<<profile<<" mode="<<mode<<" fps="<<fps<<" models="<<game_model_animation().model_count()<<" expected="<<before_models<<" allocations="<<allocations.size()<<" expected="<<before_heap.size()<<'\n';
                for(const auto address:allocations)if(!before_heap.contains(address))std::cerr<<"Retained heap "<<std::hex<<address<<std::dec<<'\n';
                for(unsigned i=0;i<resource_references.size();++i)if(resource_references[i]!=before_refs[i])std::cerr<<"Resource refs "<<i<<" value="<<resource_references[i]<<" expected="<<before_refs[i]<<'\n';
            }
            assert(allocations==before_heap&&resource_references==before_refs&&game_model_animation().model_count()==before_models);
            preview_live=false;native_gameplay_preview_reset(rdram);enemy_fixture_active=false;
            if(native)++native_cases;else ++original_cases;
        }
        if(progress){progress<<"PASS kind="<<definition.kind<<" divider="<<divider<<" profile="<<profile<<" states=";for(auto state:reached[definition.kind])progress<<state<<',';progress<<std::endl;}
    }
    if(selected_kinds.count(267))assert(rabbit_landings&&rabbit_ground_hits&&rabbit_stable_ground);
    if(diagnostic_kind){assert(native_cases&&original_cases);std::cout<<"DIAGNOSTIC ONLY: kind "<<diagnostic_kind<<" cases="<<original_cases+native_cases<<"; not the complete block proof. Rabbit landing transitions="<<rabbit_landings<<" ground hits="<<rabbit_ground_hits<<" stable ground="<<rabbit_stable_ground<<".\n";return;}
    if(shard_count_text) {
        // Every worker starts each unchanged kind/divider/profile/mode from
        // its own complete image. It reports evidence, never a whole-block
        // PASS; Python requires disjoint, exhaustive218-kind coverage.
        assert(!selected_kinds.empty()&&original_cases==12*selected_kinds.size()&&native_cases==36*selected_kinds.size());
        assert(births==native_cases&&moving_samples>0);
        std::cout<<"ENEMY SHARD RESULT {\"index\":"<<shard_index<<",\"count\":"<<shard_count
                 <<",\"original\":"<<original_cases<<",\"native\":"<<native_cases<<",\"births\":"<<births
                 <<",\"endpoints\":"<<endpoints<<",\"intermediate\":"<<intermediate<<",\"zero_dt\":"<<zero_dt
                 <<",\"moving\":"<<moving_samples<<",\"contacts\":"<<contacts
                 <<",\"rabbit_floor\":"<<rabbit_landings<<",\"rabbit_substeps\":"<<rabbit_substeps
                 <<",\"rabbit_ground_hits\":"<<rabbit_ground_hits<<",\"rabbit_stable_ground\":"<<rabbit_stable_ground<<",\"kinds\":[";
        bool first=true;for(unsigned kind:selected_kinds){if(!first)std::cout<<',';first=false;std::cout<<kind;}
        std::cout<<"],\"states\":{";first=true;
        for(const auto& [kind,states]:reached){if(!first)std::cout<<',';first=false;std::cout<<'"'<<kind<<"\":[";
            bool first_state=true;for(int state:states){if(!first_state)std::cout<<',';first_state=false;std::cout<<state;}std::cout<<']';}
        std::cout<<"}}\n";return;
    }
    assert(original_cases==2616&&native_cases==7848&&births==7848&&moving_samples>0);
    std::cout<<"PASS: enemy "<<original_cases<<" frozen/current Original and "<<native_cases<<" Native schedules; "<<endpoints
             <<" pool endpoints, "<<intermediate<<" intervals, "<<zero_dt<<" zero-dt checks, "<<contacts<<" genuine contacts; 218 kinds including all135 ordinary/all79 bosses and4 outbound owners, actual constructors/retirement on synthetic assets; no game/GPU.\n";
    std::cout<<"Enemy reached-state coverage:";
    for(const auto& [kind,states]:reached){std::cout<<' '<<kind<<'=';for(int state:states)std::cout<<state<<',';}
    std::cout<<" rabbit landing transitions="<<rabbit_landings<<" ground hits="<<rabbit_ground_hits<<" stable ground="<<rabbit_stable_ground<<" fractional samples="<<rabbit_substeps<<'\n';
}
#endif
