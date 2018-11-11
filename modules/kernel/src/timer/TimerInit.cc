//===================================================================================================================
//
//  TimerInit.cc -- Initialize the Programmable Interrupt Timer (PIT)
//
//        Copyright (c)  2017-2018 -- Adam Clark
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
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "idt.h"
#include "timer.h"


//
// -- This is the callback function that is triggered with each interrupt
//    -------------------------------------------------------------------
extern void TimerCallBack(isrRegs_t *reg);


//
// -- Set the timer to fire at the desires frequency
//    ----------------------------------------------
void TimerInit(uint32_t frequency)
{
	regval_t flags = DisableInterrupts();

    // -- Remap the irq table, even though we may not be using it.
	outb(0x21, 0xff);			// Disable all IRQs
	outb(0xA1, 0xff);			// Disable all IRQs
	outb(0x20, 0x11);
	outb(0xA0, 0x11);
	outb(0x21, 0x20);
	outb(0xA1, 0x28);
	outb(0x21, 0x04);
	outb(0xA1, 0x02);
	outb(0x21, 0x01);
	outb(0xA1, 0x01);

	uint32_t divisor = 1193180 / frequency;
	uint8_t l = (uint8_t)(divisor & 0xff);
	uint8_t h = (uint8_t)((divisor >> 8) & 0xff);

	IsrRegister(32, TimerCallBack);

	outb(0x43, 0x36);
	outb(0x40, l);
	outb(0x40, h);

	timerControl = &pitTimer;

	outb(0x21, 0x0);			// Enable all IRQs
	outb(0xA1, 0x0);			// Enable all IRQs

	RestoreInterrupts(flags);
}

