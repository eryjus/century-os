//===================================================================================================================
//
// ProcessMicroSleepUntil.cc -- Sleep until we get to the requested micros since boot
//
//        Copyright (c)  2017-2020 -- Adam Clark
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
EXPORT KERNEL
void ProcessDoMicroSleepUntil(uint64_t when)
{
    assert_msg(AtomicRead(&scheduler.schedulerLockCount) > 0,
            "Calling `ProcessDoMicroSleepUntil()` without the proper lock");
    assert_msg(currentThread != NULL, "scheduler.currentProcess is NULL");

    if (when <= TimerCurrentCount(timerControl)) return;

    currentThread->wakeAtMicros = when;
    if (when < scheduler.nextWake) scheduler.nextWake = when;

    Enqueue(&scheduler.listSleeping, &currentThread->stsQueue);

    ProcessDoBlock(PROC_DLYW);
}