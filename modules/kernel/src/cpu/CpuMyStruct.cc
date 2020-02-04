//===================================================================================================================
//
//  CpuMyStruct.cc -- For the APs, initialize the specific elements in the cpus array
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Feb-03  Initial  v0.5.0f  ADCL  Initial version
//
//===================================================================================================================



#include "cpu.h"
#include "printf.h"
#include "pic.h"


//
// -- Complete the cpu structure initialization for this core
//    -------------------------------------------------------
EXTERN_C EXPORT LOADER
archsize_t CpuMyStruct(void)
{
    int idx = cpus.cpuStarting;
    volatile ArchCpu_t *rv = &cpus.perCpuData[idx];
    rv->location = GetLocation();
    cpus.cpusRunning ++;

    return (archsize_t)rv;
}

