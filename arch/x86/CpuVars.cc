//===================================================================================================================
//
//  CpuVars.cc -- Various variables used by the i686 CPU
//
//        Copyright (c)  2017-2019 -- Adam Clark
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


#include "mmu.h"
#include "cpu.h"


//
// -- Point to the CPU structures in virtual memory
//    ---------------------------------------------
Frame0_t *cpuStructs = (Frame0_t *)GDT_ADDRESS;


//
// -- This is a small stack for the TSS
//    ---------------------------------
byte_t tssStack[TSS_STACK_SIZE];
