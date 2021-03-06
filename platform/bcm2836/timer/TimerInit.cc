//===================================================================================================================
//
//  TimerInit.cc -- Initialize the rpi2b timer
//
//        Copyright (c)  2017-2020 -- Adam Clark
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
#include "interrupt.h"
#include "printf.h"
#include "pic.h"
#include "timer.h"


//
// -- Set the timer to fire at the desires frequency
//    ----------------------------------------------
EXTERN_C EXPORT KERNEL
void _TimerInit(TimerDevice_t *dev, uint32_t frequency)
{
    if (!dev) return;

    if (thisCpu->cpuNum == 0) {
        IsrRegister(BCM2836_CORE_CNTPNSIRQ, dev->TimerCallBack);
        dev->factor = READ_CNTFRQ() / 1000000.0;
        kprintf("IsrHandler registered\n");
    }

    if (READ_CNTFRQ() == 0) {
        CpuPanicPushRegs("PANIC: Unable to determine the clock frequency (read as 0)\n");
    }


    //
    // -- So now, I should be able to calculate the desired interval.  This is done by taking the clock
    //    frequency and dividing it by the requested frequency.  i.e.: READ_CNTFRQ / frequency.
    //    ---------------------------------------------------------------------------------------------
    WRITE_CNTP_CVAL((uint64_t)-1);              // set the cval to its limit just to be in control
    dev->reloadValue = 1000000 / frequency;

    PicInit(dev->pic, "PIC");                   // now, init the pic first
    WRITE_CNTP_TVAL(dev->reloadValue);
    WRITE_CNTP_CTL(1);                          // enable the timer

    PicUnmaskIrq(dev->pic, BCM2836_CORE_CNTPNSIRQ);
    kprintf("Timer Initialized\n");
}

