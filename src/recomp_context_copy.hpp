#pragma once
#include "recomp.h"
namespace dora3 {
// recomp_context contains a self-pointer. Plain assignment retains the source
// FPR storage, even though all scalar register fields have been copied.
// Use an output reference: returning by value could relocate that storage again.
inline void copy_recomp_context(recomp_context& destination,const recomp_context& source) {
    destination=source;
    destination.f_odd=destination.mips3_float_mode ? &destination.f1.u32l : &destination.f0.u32h;
}
}
