#pragma once
#include "graphics_profile.hpp"
#include <array>
#include <cstdint>
#include <cstring>
#include <memory>

// Included after RT64's pinned TMEMHasher and LoadTile definitions.
// Memoizes the pure hash function only, never a GPU texture or RDRAM address.
namespace RT64::dora3_detail {
struct TMEMHashEntry {
    std::array<uint32_t,9> key{};
    uint64_t fingerprint=0, hash=0, used=0;
    std::array<uint8_t,4096> bytes{};
};
struct TMEMHashCache {
    static constexpr size_t ways=4, buckets=512;
    std::array<TMEMHashEntry,ways*buckets> entries{};
    uint64_t clock=0;
};
inline uint64_t cachedTMEMHash(const uint8_t* tmem,const LoadTile& tile,
                              uint16_t width,uint16_t height,uint32_t tlut,uint32_t version) {
    // V5's palette index scan is the expensive operation. Keep the ordinary
    // hash for non-paletted textures and any future algorithm version.
    if(!tlut || version!=5) {
        dora3::graphics_profile::add(dora3::graphics_profile::Counter::HashMemoBypass);
        return TMEMHasher::hash(tmem,tile,width,height,tlut,version);
    }

    // These are ALL fields read by the pinned v5 implementation, including
    // needsToHashRowsIndividually. The key contains no padding or pointers.
    const std::array<uint32_t,9> key{
        width,height,tlut,version,tile.line,tile.siz,tile.fmt,tile.tmem,tile.palette};
    const auto seed=XXH3_64bits(key.data(),sizeof(key));
    const auto fingerprint=XXH3_64bits_withSeed(tmem,4096,seed);
    // One bounded ~8.2MiB cache per producer thread, allocated on first use.
    // Numeric hashes remain valid across scene changes, reset and GPU eviction.
    static thread_local auto cache=std::make_unique<TMEMHashCache>();
    auto* bucket=&cache->entries[(fingerprint&(TMEMHashCache::buckets-1))*TMEMHashCache::ways];
    auto* oldest=&bucket[0];
    for(size_t i=0;i<TMEMHashCache::ways;++i) {
        auto& entry=bucket[i];
        if(entry.used && entry.fingerprint==fingerprint && entry.key==key &&
           std::memcmp(entry.bytes.data(),tmem,entry.bytes.size())==0) {
            dora3::graphics_profile::add(dora3::graphics_profile::Counter::HashMemoHit);
            entry.used=++cache->clock;
            return entry.hash;
        }
        if(entry.used<oldest->used)oldest=&entry;
    }
    // A fingerprint collision can only cause a miss: exact bytes/parameters
    // above are mandatory. Dynamic palettes and TMEM writes are never skipped.
    dora3::graphics_profile::add(dora3::graphics_profile::Counter::HashMemoMiss);
    const auto hash=TMEMHasher::hash(tmem,tile,width,height,tlut,version);
    oldest->key=key;oldest->fingerprint=fingerprint;oldest->hash=hash;
    std::memcpy(oldest->bytes.data(),tmem,oldest->bytes.size());
    oldest->used=++cache->clock;
    return hash;
}
}
