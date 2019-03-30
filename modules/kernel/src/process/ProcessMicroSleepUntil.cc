//===================================================================================================================
//
// ProcessMicroSleepUntil.cc -- Sleep until we get to the requested micros since boot
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-14  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "process.h"


//
// -- sleep until we get to the number of micros since boot
//    -----------------------------------------------------
void __krntext ProcessMicroSleepUntil(uint64_t when)
{
    ProcessEnterPostpone();

    if (when < TimerCurrentCount(&timerControl)) {
        ProcessExitPostpone();
        return;
    }

    currentProcess->wakeAtMicros = when;
    if (when < nextWake) nextWake = when;

    Enqueue(&sleepingTasks, &currentProcess->stsQueue);
    ProcessExitPostpone();
    ProcessBlock(PROC_DLYW);
}