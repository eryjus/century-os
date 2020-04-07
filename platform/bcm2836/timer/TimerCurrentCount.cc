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
//  2020-Apr-06  Initial  v0.6.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "timer.h"


//
// -- Get the number of ticks since boot
//    ----------------------------------
EXPORT KERNEL
uint64_t _TimerCurrentCount(TimerDevice_t *dev)
{
    return READ_CNTPCT();
}

