//===================================================================================================================
//
//  ArchCpuLateInit.cc -- Complete the final initialization for the CPU
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Jun-16  Initial   0.4.6   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "platform.h"
#include "serial.h"
#include "pic.h"
#include "cpu.h"


//
// -- Complete the final CPU initialization steps
EXTERN_C EXPORT LOADER
void ArchLateCpuInit(int c)
{
    ArchFpuInit();
    WriteTPIDRPRW((uint32_t)cpus.perCpuData[c].cpu);
}

