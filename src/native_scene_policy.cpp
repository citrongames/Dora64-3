#include "native_scene_policy.hpp"
#include "recomp.h"
#include "native_gameplay_preview.hpp"
#include "native_cutscene.hpp"
#include "native_ui.hpp"
#include "native_minigames.hpp"
namespace {
int32_t word(uint8_t* rdram,uint32_t address) {return MEM_W(0,int32_t(address));}
bool scene34_armed=false;
bool image_eligible(uint8_t* rdram) {
    if(!scene34_armed || word(rdram,0x801C68C4)!=0)return false;
    const auto transition=word(rdram,0x801C68CC);
    // White fade changes only the scene-selection request and its countdown.
    // Actual callback/resource replacement occurs on the update with count==1.
    const auto hold=word(rdram,0x801C68D0);
    const auto selected=uint32_t(word(rdram,0x801A950C));
    if(transition==0) {
        if(hold<=0)return false;
        // Only state1's simple next-scene request has been reviewed here.
        // State2 also resets progress: retain the previous drain-before-command.
        if(hold==1 && (selected>255 || MEM_BU(int32_t(selected),int32_t(0x80171C50))!=1))return false;
    } else if(transition<=1 || transition>16 || selected!=0x5A || MEM_BU(0x5A,int32_t(0x80171C50))!=1)return false;
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
const ScenePolicy cutscene20_policy{"scene1-scripted",cutscene20_eligible,3,native_cutscene_update};
const ScenePolicy cutscene30_policy{"scene1-scripted-30",cutscene30_eligible,2,native_cutscene_update};
bool gameplay20_eligible(uint8_t* ram) {return word(ram,0x801A39DC)==3&&native_gameplay_preview_eligible(ram);}
bool gameplay30_eligible(uint8_t* ram) {return word(ram,0x801A39DC)==2&&native_gameplay_preview_eligible(ram);}
const ScenePolicy gameplay20_policy{"scene1-ground-preview",gameplay20_eligible,3,native_gameplay_preview_update};
const ScenePolicy gameplay30_policy{"scene1-ground-preview-30",gameplay30_eligible,2,native_gameplay_preview_update};
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
void native_scene34_ready() {scene34_armed=true;}
void invalidate_native_scene_policy() {scene34_armed=false;}
