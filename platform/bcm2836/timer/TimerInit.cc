//===================================================================================================================
//
//  TimerInit.cc -- Initialize the rpi2b timer
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-21  Initial   0.2.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "hardware.h"
#include "interrupt.h"
#include "printf.h"
#include "timer.h"


//
// -- Set the timer to fire at the desires frequency
//    ----------------------------------------------
void _TimerInit(TimerDevice_t *dev, UNUSED(uint32_t frequency))
{
    if (!dev) return;

	IsrRegister(11, dev->TimerCallBack);
    kprintf("IsrHandler registered\n");

    archsize_t base = dev->base;

    // -- Now I should be able to set up the timer
    MmioWrite(base + TIMER_CONTROL, 0x00);               // this register is not emulated by qemu
    MmioWrite(base + TIMER_PRESCALAR, 0x80000000);         // this register is not emulated by qemu

    PicInit(dev->pic);                              // now, init the pic first

    MmioWrite(base + TIMER_LOCAL_CONTROL, (1<<19) | (1<<28) | (1<<29));  // set up the counter for the timer and start it
    MmioWrite(base + TIMER_WRITE_FLAGS, (1<<30) | (1<<31));  // clear and reload the timer
    MmioWrite(base + TIMER_LOCAL_INT_ROUTING, 0x00000000);         // local timer goes to core 0 IRQ

    // -- Now, enable the ARM Timer interrupt only
    PicEnableIrq(dev->pic, IRQ_ARM_TIMER);

    kprintf("Timer is initialized\n");
}

