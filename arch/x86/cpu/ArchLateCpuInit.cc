//===================================================================================================================
//
//  ArchLateCpuInit.cc -- Polish off the CPU structures for the x86 arch
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Feb-01  Initial  v0.5.0f  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "cpu.h"


//
// -- Complete the CPU initialization
//    -------------------------------
EXTERN_C EXPORT LOADER
void ArchLateCpuInit(int c)
{
    kprintf("Finalizing CPU initialization\n");
    ArchGsLoad(cpus.perCpuData[c].gsSelector);
    ArchTssLoad(cpus.perCpuData[c].tssSelector);
}



