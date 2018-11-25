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


//
// -- Set the timer to fire at the desires frequency
//    ----------------------------------------------
void TimerInit(uint32_t frequency)
{
	kprintf("Initialize the timer for rpi2b here\n");
	Halt();
}

