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
#include "cpu.h"
#include "hardware.h"
#include "interrupt.h"
#include "printf.h"
#include "timer.h"


//
// -- Set the timer to fire at the desires frequency
//    ----------------------------------------------
void _TimerInit(TimerDevice_t *dev, uint32_t frequency)
{
    if (!dev) return;

    IsrRegister(1, dev->TimerCallBack);
    kprintf("IsrHandler registered\n");

    if (READ_CNTFRQ() == 0) {
        kprintf("PANIC: Unable to determine the clock frequency (read as 0)\n");
        Halt();
    }


    //
    // -- So now, I should be able to calculate the desired interval.  This is done by taking the clock
    //    frequency and dividing it by the requested frequency.  i.e.: READ_CNTFRQ / frequency.
    //    ---------------------------------------------------------------------------------------------
    dev->reloadValue = READ_CNTFRQ() / frequency;

    PicInit(dev->pic);                              // now, init the pic first
    WRITE_CNTP_CTL(1);                  // -- enable the timer
    WRITE_CNTP_TVAL(dev->reloadValue);

    for (int i = 0; i < 20; i ++) {
        kprintf(".. The current timer value is %x\n", READ_CNTP_TVAL());
    }
}

