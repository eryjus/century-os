//===================================================================================================================
//
// ProcessUpdateTimeUsed.cc -- Update the time used for the current process before changing
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
// -- This is the last timer value that was updated
//    ---------------------------------------------
EXPORT KERNEL_DATA
uint64_t lastTimer = 0;


//
// -- This is the CPU idle time
//    -------------------------
EXPORT KERNEL_DATA
uint64_t cpuIdleTime = 0;


//
// -- Get the current timer value and update the time used of the current process
//    ---------------------------------------------------------------------------
EXPORT KERNEL
void ProcessUpdateTimeUsed(void)
{
    uint64_t now = TimerCurrentCount(timerControl);
    uint64_t elapsed = now - lastTimer;
    lastTimer = now;

    if (scheduler.currentProcess == NULL) {
        cpuIdleTime += elapsed;
    } else {
        scheduler.currentProcess->timeUsed += elapsed;
    }
}

