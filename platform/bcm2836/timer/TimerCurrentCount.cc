//===================================================================================================================
//
//  TimerCurrentCount.cc -- Get the current count from the timer
//
//        Copyright (c)  2017-2019 -- Adam Clark
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


uint64_t _TimerCurrentCount(TimerDevice_t *dev)
{
    return READ_CNTPCT();
}

