//===================================================================================================================
//
//  LApicEoi.cc -- Signal End of Interrupt to the Local APIC
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-26  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "interrupt.h"
#include "mmu.h"
#include "printf.h"
#include "cpu.h"
#include "pic.h"


//
// -- Signal EOI to the Local APIC
//    ----------------------------
void __krntext _LApicEoi(TimerDevice_t *dev)
{
    if (!dev) return;

    MmioWrite(dev->base + LAPIC_EOI, 0);        // all there needs to be is a write to the register
}
