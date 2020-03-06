//===================================================================================================================
//
//  PicDetermineIrq.cc -- Get the current IRQ from the PIC
//
//        Copyright (c)  2017-2020 -- Adam Clark
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


#include "types.h"
#include "timer.h"
#include "printf.h"
#include "pic.h"


//
// -- From an interrupt, determine what is the IRQ to handle
//    ------------------------------------------------------
EXTERN_C EXPORT KERNEL
int _PicDetermineIrq(PicDevice_t *dev)
{
    if (!dev) return -1;

    Bcm2835Pic_t *picData = (Bcm2835Pic_t *)dev->device.deviceData;

    int core = thisCpu->cpuNum;
    archsize_t rv;


    //
    // -- start by checking the core's interrupts
    //    ---------------------------------------
    archsize_t irq = MmioRead(picData->timerLoc + TIMER_IRQ_SOURCE + (core * 4)) & 0xff; // mask out the relevant ints
    rv = __builtin_ffs(irq);
    if (rv != 0) return BCM2836_CORE_BASE + (rv - 1);


    //
    // -- ok, not a core-specific interrupt, check ints 0-31
    //    --------------------------------------------------
    irq = MmioRead(picData->picLoc + INT_IRQPEND1);
    rv = __builtin_ffs(irq);
    if (rv != 0) return BCM2835_GPU_BASE0 + (rv - 1);


    //
    // -- now, if we make it here, try ints 32-63
    //    ---------------------------------------
    irq = MmioRead(picData->picLoc + INT_IRQPEND2);
    rv = __builtin_ffs(irq);
    if (rv != 0) return BCM2835_GPU_BASE1 + (rv - 1);


    //
    // -- finally if we get here, it must be a spurious interrupt
    //    -------------------------------------------------------
    return (archsize_t)-1;
}
