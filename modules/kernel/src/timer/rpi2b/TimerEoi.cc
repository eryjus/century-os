//===================================================================================================================
//
//  TimerEoi.cc -- Issue an end of interrupt for the timer
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
#include "printf.h"
#include "timer.h"


//
// -- Issue an EOI for the timer
//    --------------------------
void TimerEoi(UNUSED(uint32_t irq))
{
    kprintf("Issue the proper EOI for rpi2b here\n");
    Halt();
}