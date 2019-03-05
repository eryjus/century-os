//===================================================================================================================
//
//  PicInit.cc -- Initialize the rpi2b pic
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-24  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#include "cpu.h"
#include "interrupt.h"
#include "printf.h"
#include "timer.h"


//
// -- Initialize the pic
//    ------------------
void _PicInit(PicDevice_t *dev)
{
    if (!dev) return;

    PicBase_t base1 = dev->base1;
    PicBase_t base2 = dev->base2;

    // -- for good measure, disable the FIQ
    MmioWrite(base1 + INT_FIQCTL, 0x0);

    // -- Disable all IRQs -- write to clear, anything that is high will be pulled low
    MmioWrite(base1 + INT_IRQDIS0, 0xffffffff);
    MmioWrite(base1 + INT_IRQDIS1, 0xffffffff);
    MmioWrite(base1 + INT_IRQDIS2, 0xffffffff);

    // -- perform the per-core initialization
    int core = 0;               // TODO: will need to get the core dynamically
    MmioWrite(base2 + TIMER_INTERRUPT_CONTROL + (core * 4), 0x00000002);         // select as IRQ for core 0
    MmioWrite(base2 + TIMER_IRQ_SOURCE + (core * 4), 0x00000002);         // enable IRQs from the core for this CPU
    MmioWrite(base2 + TIMER_FIQ_SOURCE + (core * 4), 0x00000000);         // force disable FIQ for all sources
}
