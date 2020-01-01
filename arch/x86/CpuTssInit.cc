//===================================================================================================================
//
//  CpuTssInit.cc -- Initialize the TSS in preparation for process switching between protection levels
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-11  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "cpu.h"
#include "printf.h"


//
// -- Initialize the TSS
//    ------------------
void CpuTssInit(void)
{
    kprintf("Initializing the TSS\n");

    kMemSetB((void *)TSS_ADDRESS, 0, 512);

    cpuStructs->tss.ss0 = 0x10;
    cpuStructs->tss.esp0 = ((archsize_t)tssStack + TSS_STACK_SIZE);
    cpuStructs->tss.ds = cpuStructs->tss.es = cpuStructs->tss.fs = cpuStructs->tss.gs = cpuStructs->tss.ss = 0x10;
    cpuStructs->tss.cs = 0x08;
}
