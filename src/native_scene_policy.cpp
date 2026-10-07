#include "native_scene_policy.hpp"
#include "recomp.h"
#include "native_gameplay_preview.hpp"
#include "native_cutscene.hpp"
#include "native_ui.hpp"
#include "native_minigames.hpp"
extern "C" int dora3_overlay_loaded(uint32_t);
extern "C" uint64_t dora3_overlay_generation(uint32_t);
namespace {
int32_t word(uint8_t* rdram,uint32_t address) {return MEM_W(0,int32_t(address));}
bool scene34_armed=false;
bool image_eligible(uint8_t* rdram) {
    if(!scene34_armed || !dora3_overlay_loaded(0x635C0))return false;
    const auto transition=word(rdram,0x801C68CC),hold=word(rdram,0x801C68D0);
    const int direction=MEM_B(0,int32_t(0x801A99A0));
    const bool moving_fade=direction==-1||direction==1;
    if(transition<0||transition>16||hold<0)return false;
    // C0088 only decrements the deferred request. The selected destination and
    // its state do not affect this resident draw. Count1 still needs fractional
    // fade frames: actual 280DC drains the banks BEFORE installing the loader.
    // Likewise, Source progress reset at hold1 is not resource replacement.
    if(!moving_fade&&transition==0&&hold==0)return false;
    // C012C's controller-message branch is a static resident text panel. Only
    // its active fade needs extra frames; input/controller decisions stay Source.
    if(word(rdram,0x801C68C4)!=0&&!moving_fade)return false;
    if(word(rdram,0x801A39DC)!=3 || word(rdram,0x801AA648)==0 || word(rdram,0x801A9500)==0)return false;
    for(unsigned i=0;i<8;++i) {
        const uint32_t slot=0x801A44F0+i*20;
        if(i<2) {
            if(word(rdram,slot)!=2 || word(rdram,slot+12)!=int(i) || uint32_t(word(rdram,slot+16))!=(i?0x801C012Cu:0x801C0088u))return false;
        } else if(word(rdram,slot)!=0 && word(rdram,slot)!=5)return false;
    }
    // Cross-frame framebuffer copy reads the other bank. It needs an explicit
    // dependency model and is not authorized by this initial scene policy.
    return word(rdram,0x801A9C88)==0;
}
bool cutscene20_eligible(uint8_t* ram) {return word(ram,0x801A39DC)==3&&native_cutscene_eligible(ram);}
bool cutscene30_eligible(uint8_t* ram) {return word(ram,0x801A39DC)==2&&native_cutscene_eligible(ram);}
const ScenePolicy cutscene20_policy{"world-scripted-20",cutscene20_eligible,3,native_cutscene_update};
const ScenePolicy cutscene30_policy{"world-scripted-30",cutscene30_eligible,2,native_cutscene_update};
bool gameplay20_eligible(uint8_t* ram) {return dora3_overlay_loaded(0x70A50)&&word(ram,0x801A39DC)==3&&native_gameplay_preview_eligible(ram);}
bool gameplay30_eligible(uint8_t* ram) {return dora3_overlay_loaded(0x70A50)&&word(ram,0x801A39DC)==2&&native_gameplay_preview_eligible(ram);}
const ScenePolicy gameplay20_policy{"world-gameplay-20",gameplay20_eligible,3,native_gameplay_preview_update};
const ScenePolicy gameplay30_policy{"world-gameplay-30",gameplay30_eligible,2,native_gameplay_preview_update};
const ScenePolicy image_policy{"scene34-image",image_eligible,3};
bool ui20_eligible(uint8_t* ram){return word(ram,0x801A39DC)==3&&dora3::native_time::native_ui_menu_eligible(ram);}
bool ui30_eligible(uint8_t* ram){return word(ram,0x801A39DC)==2&&dora3::native_time::native_ui_menu_eligible(ram);}
const ScenePolicy ui20_policy{"reviewed-common-ui-20",ui20_eligible,3};
const ScenePolicy ui30_policy{"reviewed-common-ui-30",ui30_eligible,2};
bool minigame20_eligible(uint8_t* ram){return word(ram,0x801A39DC)==3&&native_minigame_eligible(ram);}
bool minigame30_eligible(uint8_t* ram){return word(ram,0x801A39DC)==2&&native_minigame_eligible(ram);}
const ScenePolicy minigame20_policy{"reviewed-minigames-20",minigame20_eligible,3,native_minigame_update};
const ScenePolicy minigame30_policy{"reviewed-minigames-30",minigame30_eligible,2,native_minigame_update};
}
const ScenePolicy* select_native_scene_policy(uint8_t* rdram) {
    // Additional reviewed scene families join this registry, without changing
    // the event loop, ownership protocol or settings implementation.
    if(image_policy.eligible(rdram))return &image_policy;
    if(minigame20_policy.eligible(rdram))return &minigame20_policy;
    if(minigame30_policy.eligible(rdram))return &minigame30_policy;
    if(cutscene20_policy.eligible(rdram))return &cutscene20_policy;
    if(cutscene30_policy.eligible(rdram))return &cutscene30_policy;
    if(gameplay20_policy.eligible(rdram))return &gameplay20_policy;
    if(gameplay30_policy.eligible(rdram))return &gameplay30_policy;
    // Common game UI shares the world policy. Acquiring a UI-only session in
    // gameplay/cutscenes would disable the previous renderer cadence while
    // leaving camera/world motion at20/30Hz. Standalone UI owns menus only.
    if(ui20_policy.eligible(rdram))return &ui20_policy;
    if(ui30_policy.eligible(rdram))return &ui30_policy;
    return nullptr;
}
SceneIdentity native_scene_identity(uint8_t* ram,const ScenePolicy& policy) {
    const uint32_t module=policy.update==native_minigame_update?native_minigame_module(ram):
        (policy.update==native_gameplay_preview_update||policy.update==native_cutscene_update||
          (uint32_t(word(ram,0x801A4500))==0x801BB730u&&uint32_t(word(ram,0x801A4514))==0x801BB848u)?0x70A50u:0x635C0u);
    return {uint32_t(word(ram,0x801A4500)),uint32_t(word(ram,0x801A4514)),module,
            policy.divider,dora3_overlay_generation(module)};
}
const char* native_scene_fallback_reason(uint8_t* ram) {
    const auto divider=word(ram,0x801A39DC);
    if(divider!=2&&divider!=3)return "unreviewed Source divider";
    if(word(ram,0x801A9C88))return "framebuffer copy/transition";
    if(!word(ram,0x801AA648)||!word(ram,0x801A9500))return "graphics disabled";
    for(unsigned i=2;i<8;++i)if(word(ram,0x801A44F0+i*20)!=0&&word(ram,0x801A44F0+i*20)!=5)
        return "additional Source process";
    const auto update=uint32_t(word(ram,0x801A4500)),draw=uint32_t(word(ram,0x801A4514));
    if(update==0x801BB730&&draw==0x801BB848) {
        if(!dora3_overlay_loaded(0x70A50))return "world module not loaded";
        return "world setup/pause/script/resource boundary";
    }
    if(dora3_overlay_loaded(0x59E80)) {
        if(update==0x801B4FE4&&draw==0x801B5064)return "auxiliary startup/device command screen: Source";
        if(update==0x801B59D8&&draw==0x801B5A70)return "auxiliary actor/camera control screen: Source";
        if(update==0x801B6FD0&&draw==0x801B70A8)return "auxiliary resource/model selector: Source";
    }
    if(dora3_overlay_loaded(0x70A50)&&update==0x801F9B58&&draw==0x801F9C88)
        return "auxiliary world actor/model control screen: Source";
    if(update==0x800433D8&&!draw)return "Source loader/scene replacement";
    return "Source initialization/transition or unreviewed callback pair";
}
void native_scene34_ready() {scene34_armed=true;}
void invalidate_native_scene_policy() {scene34_armed=false;}
