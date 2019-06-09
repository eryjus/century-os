//===================================================================================================================
//
//  CpuNum.cc -- Get the current CPU Number from the Local APIC
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Jun-07  Initial   0.4.5   ADCL  Initial version
//
//===================================================================================================================


#include "cpu.h"
#include "pic.h"


//
// -- Get the CPU Number from the Local APIC
//    --------------------------------------
int __krntext CpuNum(void)
{
    return (MmioRead(LAPIC_MMIO + LAPIC_ID) >> 24) & 0xff;
}

