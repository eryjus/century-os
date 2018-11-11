//===================================================================================================================
//
//  CpuTssInit.cc -- Initialize the TSS in preparation for process switching between protection levels
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-11  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "cpu.h"
#include "printf.h"
#include "tss.h"


//
// -- Initialize the TSS
//    ------------------
void CpuTssInit(void)
{
    kprintf("The TSS contains: %p:%p\n", *((uint32_t *)(&cpuStructs->gdt[9])), *(((uint32_t *)&cpuStructs->gdt[9]) + 1));
	kMemSetB((void *)512, 0, 512);
	cpuStructs->tss.ss0 = 0x10;
	cpuStructs->tss.esp0 = ((ptrsize_t)tssStack + TSS_STACK_SIZE);
	cpuStructs->tss.ds = cpuStructs->tss.es = cpuStructs->tss.fs = cpuStructs->tss.gs = cpuStructs->tss.ss = 0x10;
	cpuStructs->tss.cs = 0x08;

	Ltr(0x4b);
}
