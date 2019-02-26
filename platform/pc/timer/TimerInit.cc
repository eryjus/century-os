//===================================================================================================================
//
//  TimerInit.cc -- Initialize the Programmable Interrupt Timer (PIT)
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Note that this is only included for legacy reasons and Century-OS will prefer the Local APIC for the timer.
//  For the moment, it is also used for initialization.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Sep-16                          Leveraged from Royalty
//  2012-Sep-23                          set new SpuriousIRQ handler
//  2013-Sep-03    #73                   Encapsulate Process Structure
//  2013-Sep-12   #101                   Resolve issues splint exposes
//  2018-Oct-28  Initial   0.1.0   ADCL  Copied this function from Century32 to Centrury-OS
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "hardware.h"
#include "cpu.h"
#include "interrupt.h"
#include "timer.h"


//
// -- This is the callback function that is triggered with each interrupt
//    -------------------------------------------------------------------
extern void TimerCallBack(isrRegs_t *reg);


//
// -- Set the timer to fire at the desires frequency
//    ----------------------------------------------
void _TimerInit(TimerDevice_t *dev, uint32_t frequency)
{
    if (!dev) return;

    PicInit(dev->pic);

    uint16_t port = dev->base;

	uint32_t divisor = 1193180 / frequency;
	uint8_t l = (uint8_t)(divisor & 0xff);
	uint8_t h = (uint8_t)((divisor >> 8) & 0xff);

	IsrRegister(32, dev->TimerCallBack);

	outb(port + TIMER_COMMAND, 0x36);
	outb(port + TIMER_CHAN_0, l);
	outb(port + TIMER_CHAN_0, h);

    PicEnableIrq(dev->pic, 0x01);
}

