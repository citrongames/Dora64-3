#ifdef DORA3_STORY_EFFECT_POOL_CHECK
static unsigned flash_pool_cases=0,flash_pool_intervals=0,flash_pool_samples=0,flash_pool_endpoints=0;
struct FlashPoolSnapshot {
    std::vector<int32_t> words;
    std::vector<CityCommand> commands;
    decltype(allocations) heap;
    decltype(resource_references) references;
    std::size_t models;
    bool operator==(const FlashPoolSnapshot&)const=default;
};
static std::map<std::array<unsigned,4>,std::vector<FlashPoolSnapshot>> flash_pool_oracles;
static FlashPoolSnapshot flash_pool_snapshot(uint8_t* rdram) {
    FlashPoolSnapshot value{{},city_commands,allocations,resource_references,game_model_animation().model_count()};
    for(unsigned slot=30;slot<230;++slot) {
        const auto actor=0x80171CD0u+slot*0xCC;
        value.words.push_back(MEM_W(0x94,int32_t(actor)));
        if(!MEM_W(0x94,int32_t(actor)))continue;
        const auto physical=uint32_t(MEM_W(0x98,int32_t(actor))),model=uint32_t(MEM_W(0x74,int32_t(actor)));
        assert(MEM_H(0x20,int32_t(physical))==118||MEM_H(0x20,int32_t(physical))==123||
            MEM_H(0x20,int32_t(physical))==127||MEM_H(0x20,int32_t(physical))==130);
        assert(model&&game_model_animation().lifetime_token(model));
        for(auto [address,size]:std::array<std::pair<uint32_t,unsigned>,3>{{{actor,0xCC},{physical,0xF4},{model,0x84}}})
            for(unsigned offset=0;offset<size;offset+=4)value.words.push_back(MEM_W(int32_t(offset),int32_t(address)));
    }
    return value;
}
static std::vector<int32_t> flash_pool_source_items(uint8_t* rdram) {
    std::vector<int32_t> result;
    for(unsigned slot=30;slot<230;++slot) {
        const auto actor=0x80171CD0u+slot*0xCC;
        if(!MEM_W(0x94,int32_t(actor)))continue;
        const auto physical=uint32_t(MEM_W(0x98,int32_t(actor))),model=uint32_t(MEM_W(0x74,int32_t(actor)));
        if(MEM_H(0x20,int32_t(physical))==130||(MEM_W(0x94,int32_t(actor))==10&&MEM_BU(0x28,int32_t(physical))==1))continue;
        for(auto [address,size]:std::array<std::pair<uint32_t,unsigned>,3>{{{actor,0xCC},{physical,0xF4},{model,0x84}}})
            for(unsigned offset=0;offset<size;offset+=4)result.push_back(MEM_W(int32_t(offset),int32_t(address)));
    }
    return result;
}
static std::vector<float> flash_pool_heights(uint8_t* rdram) {
    std::vector<float> value;
    for(unsigned slot=30;slot<230;++slot) {
        const auto actor=0x80171CD0u+slot*0xCC;
        if(!MEM_W(0x94,int32_t(actor)))continue;
        const auto physical=uint32_t(MEM_W(0x98,int32_t(actor)));
        if(MEM_H(0x20,int32_t(physical))==130)value.push_back(rf(rdram,actor+0xA0));
    }
    return value;
}
static void flash_pool_compare(uint8_t* rdram,unsigned fps,unsigned divider,unsigned camera,unsigned profile,unsigned tick) {
    const auto actual=flash_pool_snapshot(rdram);
    const auto& expected=flash_pool_oracles.at({fps,divider,camera,profile}).at(tick-1);
    if(actual!=expected) {
        std::cerr<<"Flash pool mismatch fps="<<fps<<" divider="<<divider<<" camera="<<camera<<" profile="<<profile<<" tick="<<tick
            <<" words="<<actual.words.size()<<'/'<<expected.words.size()<<" commands="<<actual.commands.size()<<'/'<<expected.commands.size()
            <<" heap="<<actual.heap.size()<<'/'<<expected.heap.size()<<" models="<<actual.models<<'/'<<expected.models<<'\n';
        for(unsigned i=0;i<std::min(actual.words.size(),expected.words.size());++i)
            if(actual.words[i]!=expected.words[i])std::cerr<<" word="<<i<<" actual="<<std::hex<<actual.words[i]<<" expected="<<expected.words[i]<<std::dec<<'\n';
        for(unsigned i=0;i<std::min(actual.commands.size(),expected.commands.size());++i)
            if(actual.commands[i]!=expected.commands[i]) {
                const auto& a=actual.commands[i];const auto& e=expected.commands[i];
                std::cerr<<" command="<<i<<" sites="<<std::hex<<a.site<<'/'<<e.site<<std::dec<<" stamp="<<a.stamp.nanoseconds<<'+'<<a.stamp.sixtieths<<'/'<<e.stamp.nanoseconds<<'+'<<e.stamp.sixtieths<<" args="<<std::hex<<a.args[0]<<','<<a.args[1]<<','<<a.args[2]<<'/'<<e.args[0]<<','<<e.args[1]<<','<<e.args[2]<<std::dec<<'\n';
            }
        std::abort();
    }
    ++flash_pool_endpoints;
}
#endif
