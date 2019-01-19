//===================================================================================================================
//
//  TimerEoi.cc -- Issue an end of interrupt for the timer
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
#include "printf.h"
#include "hw-kernel.h"
#include "timer.h"


//
// -- Issue an EOI for the timer
//    --------------------------
void TimerEoi(UNUSED(uint32_t irq))
{
    uint32_t reload = MmioRead(TMR_BASE + 0x34) & 0x0fffffff;
    MmioWrite(TMR_BASE + 0x34, reload); // disable the timer
    MmioWrite(TMR_BASE + 0x38, (1<<30) | (1<<31));  // clear and reload the timer
    MmioWrite(TMR_BASE + 0x34, reload | (1<<28) | (1<<29)); // re-enable the timer
}

