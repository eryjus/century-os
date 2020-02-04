//===================================================================================================================
//
//  CpuVars.cc -- CPU Astraction Variables
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Feb-02  Initial  v0.5.0f  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "stacks.h"
#include "mmu.h"
#include "pmm.h"
#include "cpu.h"


//
// -- This data will be uninitialized by the compiler
//    -----------------------------------------------
EXPORT KERNEL_BSS
Cpu_t cpus;

