//===================================================================================================================
//
//  PicDetermineIrq.cc -- Get the current IRQ from the PIC
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


#include "printf.h"
#include "timer.h"
#include "pic.h"


int _PicDetermineIrq(PicDevice_t *dev)
{
    if (!dev) return -1;

    Bcm2835Pic_t *picData = (Bcm2835Pic_t *)dev->device.deviceData;

    int core = 0;
    archsize_t rv;


    //
    // -- start by checking the core's interrupts
    //    ---------------------------------------
    archsize_t irq = MmioRead(picData->timerLoc + TIMER_IRQ_SOURCE + (core * 4)) & 0xff;       // mask out the relevant ints
    rv = __builtin_ffs(irq);
    if (rv != 0) return 64 + (rv - 1);


    //
    // -- ok, not a core-specific interrupt, check ints 0-31
    //    --------------------------------------------------
    irq = MmioRead(picData->picLoc + INT_IRQPEND1);
    rv = __builtin_ffs(irq);
    if (rv != 0) return rv - 1;


    //
    // -- now, if we make it here, try ints 32-63
    //    ---------------------------------------
    irq = MmioRead(picData->picLoc + INT_IRQPEND0);
    rv = __builtin_ffs(irq);
    if (rv != 0) return 32 + (rv - 1);


    //
    // -- finally if we get here, it must be a spurious interrupt
    //    -------------------------------------------------------
    return (archsize_t)-1;
}
