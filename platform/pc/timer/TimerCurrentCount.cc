//===================================================================================================================
//
//  TimerCurrentCount.cc -- Get the current count from the timer
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-19  Initial   0.3.2   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "timer.h"


//
// -- This is the number of ticks since boot
//    --------------------------------------
EXPORT KERNEL_DATA
uint64_t microsSinceBoot = 0;


//
// -- Get the number of ticks since boot
//    ----------------------------------
EXPORT KERNEL
uint64_t _TimerCurrentCount(TimerDevice_t *dev)
{
    return microsSinceBoot;
}

