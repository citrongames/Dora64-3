#pragma once
#include "port_touch_state.hpp"
#include "json/json.hpp"

namespace dora3::touch {
inline constexpr std::array<const char*, count> keys{
    "stick", "a", "b", "z", "start", "l", "r", "menu", "zoom_in", "zoom_out"
};
inline nlohmann::json encode_layout(const Layout& layout) {
    nlohmann::json j{{"version",1}, {"scale",layout.scale}, {"opacity",layout.opacity},
        {"dialogue_opacity",layout.dialogueOpacity}, {"camera_sensitivity",layout.sensitivity}};
    for(int i=0;i<count;++i) j["positions"][keys[i]]={layout.positions[i].x,layout.positions[i].y};
    return j;
}
inline Layout decode_layout(const nlohmann::json& j) {
    Layout layout;
    if(!j.is_object() || !j.contains("version") || j["version"]!=1) return layout;
    auto number=[](const nlohmann::json& v, float fallback, float low, float high) {
        if(!v.is_number()) return fallback;
        const float f=v.get<float>();
        return std::isfinite(f) ? std::clamp(f,low,high) : fallback;
    };
    auto setting=[&](const char* key, float& v, float low, float high) {
        if(j.contains(key)) v=number(j[key],v,low,high);
    };
    setting("scale",layout.scale,.7f,1.6f);
    setting("opacity",layout.opacity,.15f,.85f);
    setting("dialogue_opacity",layout.dialogueOpacity,.05f,.5f);
    setting("camera_sensitivity",layout.sensitivity,.25f,3.0f);
    if(j.contains("positions") && j["positions"].is_object()) {
        const auto& positions=j["positions"];
        for(int i=0;i<count;++i) if(positions.contains(keys[i])) {
            const auto& p=positions[keys[i]];
            if(p.is_array() && p.size()==2) {
                layout.positions[i].x=number(p[0],layout.positions[i].x,0,1);
                layout.positions[i].y=number(p[1],layout.positions[i].y,0,1);
            }
        }
    }
    return layout;
}
}
