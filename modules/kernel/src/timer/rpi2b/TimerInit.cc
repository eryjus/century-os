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
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "interrupt.h"
#include "printf.h"
#include "timer.h"


//
// -- This is the callback function that is triggered with each interrupt
//    -------------------------------------------------------------------
extern void TimerCallBack(isrRegs_t *reg);
extern "C" uint32_t ReadGenericTimerFreq(void);


//
// -- Set the timer to fire at the desires frequency
//    ----------------------------------------------
void TimerInit(uint32_t frequency)
{
	IsrRegister(11, TimerCallBack);
    kprintf("IsrHandler registered\n");

    // -- for good measure, disable the FIQ
    MmioWrite(INT_FIQCTL, 0x0);

    // -- Disable all IRQs -- write to clear, anything that is high will be pulled low
    MmioWrite(INT_IRQDIS0, 0xffffffff);
    MmioWrite(INT_IRQDIS1, 0xffffffff);
    MmioWrite(INT_IRQDIS2, 0xffffffff);

    // -- Now I should be able to set up the timer
    MmioWrite(TMR_BASE + 0x00, 0x00);               // this register is not emulated by qemu
    MmioWrite(TMR_BASE + 0x08, 0x80000000);         // this register is not emulated by qemu
    MmioWrite(TMR_BASE + 0x40, 0x00000002);         // select as IRQ for core 0
    MmioWrite(TMR_BASE + 0x60, 0x00000002);         // enable IRQs from the core for this CPU
    MmioWrite(TMR_BASE + 0x70, 0x00000000);         // force disable FIQ for all sources
    MmioWrite(TMR_BASE + 0x34, (1<<17) | (1<<28) | (1<<29));  // set up the counter for the timer and start it
    MmioWrite(TMR_BASE + 0x38, (1<<30) | (1<<31));  // clear and reload the timer
    MmioWrite(TMR_BASE + 0x24, 0x00000000);         // local timer goes to core 0 IRQ

    // -- Now, enable the ARM Timer interrupt only
    MmioWrite(INT_IRQENB0, 1);

	timerControl = &pitTimer;

    kprintf("Timer is initialized\n");
}

