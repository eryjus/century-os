//===================================================================================================================
//
// ProcessUpdateTimeUsed.cc -- Update the time used for the current process before changing
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-18  Initial   0.3.2   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "lists.h"
#include "timer.h"
#include "process.h"


//
// -- Get the current timer value and update the time used of the current process
//    ---------------------------------------------------------------------------
EXPORT KERNEL
void ProcessUpdateTimeUsed(void)
{
    uint64_t now = TimerCurrentCount(timerControl);
    uint64_t elapsed = now - thisCpu->lastTimer;
    thisCpu->lastTimer = now;

    if (currentThread == NULL) {
        thisCpu->cpuIdleTime += elapsed;
    } else {
        currentThread->timeUsed += elapsed;
    }
}

