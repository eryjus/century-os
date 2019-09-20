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
#include "heap.h"
#include "printf.h"
#include "timer.h"


//
// -- Initialize the pic
//    ------------------
void _PicInit(PicDevice_t *dev, const char *name)
{
    if (!dev) return;
    Bcm2835Pic_t *picData = (Bcm2835Pic_t *)dev->device.deviceData;
    PicBase_t base1 = picData->picLoc;
    PicBase_t base2 = picData->timerLoc;
    int core = CpuNum();

    if (core == 0) {
        picData->picLoc = PIC;
        picData->timerLoc = TIMER;

        // -- for good measure, disable the FIQ
        MmioWrite(base1 + INT_FIQCTL, 0x0);
        MmioWrite(base2 + TIMER_LOCAL_CONTROL, 0x00000000);         // ensure the local timer is disabled

        // -- Disable all IRQs -- write to clear, anything that is high will be pulled low
        MmioWrite(base1 + INT_IRQDIS0, 0xffffffff);
        MmioWrite(base1 + INT_IRQDIS1, 0xffffffff);
        MmioWrite(base1 + INT_IRQDIS2, 0xffffffff);
    }

    // -- perform the per-core initialization
    MmioWrite(base2 + TIMER_INTERRUPT_CONTROL + (core * 4), 0x00000002);    // select as IRQ for core
    MmioWrite(base2 + MAILBOX_INTERRUPT_CONTROL + (core * 4), 0x0000000f);    // Select IRQ handling for the IPI
    MmioWrite(base2 + TIMER_IRQ_SOURCE + (core * 4), 0x00000002);    // Select IRQ Source for the timer
}

