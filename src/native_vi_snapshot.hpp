#pragma once
#include "ultramodern/renderer_context.hpp"
#include <stdexcept>
namespace dora3::native_time {
// Same register calculation as pinned events.cpp::update_vi, without swapping
// cur_state or advancing VI/audio. Preserve registers update_vi does not write.
inline ultramodern::renderer::ViRegs present_vi_snapshot(
    ultramodern::renderer::ViRegs regs,const OSViMode& mode,uint32_t state,uint32_t control,
    uint32_t framebuffer,unsigned field) {
    if (field>1) throw std::invalid_argument("Invalid native VI field");
    const auto& common=mode.comRegs;
    const auto& f=mode.fldRegs[field];
    framebuffer &= 0x1FFFFFFFu;
    regs.VI_ORIGIN_REG=framebuffer+f.origin;
    regs.VI_WIDTH_REG=common.width;
    regs.VI_TIMING_REG=common.burst;
    regs.VI_V_SYNC_REG=common.vSync;
    regs.VI_H_SYNC_REG=common.hSync;
    regs.VI_LEAP_REG=common.leap;
    regs.VI_H_START_REG=(state&0x20)?0:common.hStart;
    regs.VI_V_START_REG=f.vStart;
    regs.VI_V_BURST_REG=f.vBurst;
    regs.VI_INTR_REG=f.vIntr;
    regs.VI_X_SCALE_REG=common.xScale;
    regs.VI_Y_SCALE_REG=(state&0x40)?0:f.yScale;
    if (state&0x40) regs.VI_ORIGIN_REG=framebuffer;
    regs.VI_STATUS_REG=control;
    return regs;
}
}
