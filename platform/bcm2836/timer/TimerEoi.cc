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
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "hardware.h"
#include "printf.h"
#include "timer.h"


//
// -- Issue an EOI for the timer
//    --------------------------
void _TimerEoi(TimerDevice_t *dev)
{
    if (!dev) return;

    archsize_t base = dev->base;

    uint32_t reload = MmioRead(base + TIMER_LOCAL_CONTROL) & 0x0fffffff;
    MmioWrite(base + TIMER_LOCAL_CONTROL, reload); // disable the timer
    MmioWrite(base + TIMER_WRITE_FLAGS, (1<<30) | (1<<31));  // clear and reload the timer
    MmioWrite(base + TIMER_LOCAL_CONTROL, reload | (1<<28) | (1<<29)); // re-enable the timer
}

