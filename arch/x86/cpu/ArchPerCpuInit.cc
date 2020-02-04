//===================================================================================================================
//
//  ArchPerCpuInit.cc -- Initialize the arch-specific per cpu elementss
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
// -- Both the gs and the TSS need to be initialized for this CPU
//    -----------------------------------------------------------
EXTERN_C EXPORT LOADER
void ArchPerCpuInit(int i)
{
    cpus.perCpuData[i].gsSelector  = ((i * 3) + 9 + 0) << 3;
    cpus.perCpuData[i].tssSelector = ((i * 3) + 9 + 1) << 3;
    kprintf("!!>> [%d]: Setting the gs selector to %x and the tss selector to %x\n", i,
            cpus.perCpuData[i].gsSelector, cpus.perCpuData[i].tssSelector);
}

