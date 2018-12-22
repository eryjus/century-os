//===================================================================================================================
//
//  TimerInit.cc -- Initialize the rpi2b timer
//
//        Copyright (c)  2017-2018 -- Adam Clark
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


#define TMR_BASE (HW_BASE + 0x01000000)

//
// -- Set the timer to fire at the desires frequency
//    ----------------------------------------------
void TimerInit(uint32_t frequency)
{
    uint32_t scalar = 38400000 / frequency;

    kprintf("Setting the scalar value to %x for frequency %x\n", scalar, frequency);

	IsrRegister(53, TimerCallBack);
    kprintf("IsrHandler registered\n");

    // -- Neither of these 2 registers are emulated in qemu, so there is no point is checking the work
    MmioWrite(TMR_BASE + 0x00, 0x00);
    MmioWrite(TMR_BASE + 0x08, 0x80000000);


    // -- Set the GPU routing flags -- results in no change
//    kprintf("Set the GPU Routing from %p\n", MmioRead(TMR_BASE + 0x0c));
//    MmioWrite(TMR_BASE + 0x0c, 0x00);

    // -- Here we set the routing for the timer
    kprintf("Set the timer routing from %p\n", MmioRead(TMR_BASE + 0x40));
    MmioWrite(TMR_BASE + 0x40, 0x00000020);
    kprintf("... checking the update: %p\n", MmioRead(TMR_BASE + 0x40));

    // -- Additional routing for the timer -- results on no change
//    kprintf("Route the local timer to core 0/IRQ changed from %p\n", MmioRead(TMR_BASE + 0x24));
//    MmioWrite(TMR_BASE + 0x24, 0x00);

    // -- Set the core interrupt sources
//    kprintf("Set the core 0 interrupt sources from %p\n", MmioRead(TMR_BASE + 0x60));
//    MmioWrite(TMR_BASE + 0x60, 0x00000020);
//    kprintf("... checking the update: %p\n", MmioRead(TMR_BASE + 0x60));

    // -- Enable the timer and interrupt (reload value of 0x100)
//    kprintf("Enable the timer/interrupt from %p\n", MmioRead(TMR_BASE + 0x34));
//    MmioWrite(TMR_BASE + 0x34, 0x30000100);
//    kprintf("... checking the update: %p\n", MmioRead(TMR_BASE + 0x34));

    // -- Reload and reset timer
//    kprintf("Reload and reset timer from %p\n", MmioRead(TMR_BASE + 0x38));
//    MmioWrite(TMR_BASE + 0x38, 0xc0000000);


//    kprintf("The core timer is currently %p : %p\n", MmioRead(TMR_BASE + 0x20), MmioRead(TMR_BASE + 0x1c));

#if 0
    // -- Set the core timer prescalar value as a test
    MmioWrite(TMR_BASE + 0x04, 1);

    // -- Program the local timer -- set up the interrupt and pre-scalar adjust
    uint32_t value = 0;
    value = (1 << 29) | (1 << 28) | (scalar & 0x0fffffff);
    MmioWrite(TMR_BASE + 0x34, value);

    // -- Direct the Timer IRQ to core 0
    MmioWrite(TMR_BASE + 0x24, 0x00);

    // -- Set up the timer interrupt source -- not sure if this will help
    MmioWrite(TMR_BASE + 0x40, 0x0f);

    // -- Enable IRQ source to core 0 for timer
    MmioWrite(TMR_BASE + 0x60, 1 << 11);

    // -- disable everything and enable only IRQ 53
    MmioWrite(INT_IRQDIS0, 0xffffffff);
    MmioWrite(INT_IRQDIS1, 0xffffffff);
    MmioWrite(INT_IRQDIS2, 0xffffffff);
    MmioWrite(INT_IRQENB2, 1 << (53 - 32));

//    MmioWrite(INT_IRQENB0, 0xffffffff);
//    MmioWrite(INT_IRQENB1, 0xffffffff);
//    MmioWrite(INT_IRQENB2, 0xfdffffff);

    // -- Clear the interrupt flag and reset the counter
    MmioWrite(TMR_BASE + 0x38, 0xc0000000);
#endif
    EnableInterrupts();
    kprintf("Timer is initialized\n");
}

