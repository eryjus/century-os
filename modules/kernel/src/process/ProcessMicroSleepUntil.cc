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
    if (when < TimerCurrentCount(timerControl)) return;

//    ProcessEnterPostpone();

    scheduler.currentProcess->wakeAtMicros = when;
    if (when < scheduler.nextWake) scheduler.nextWake = when;

//    archsize_t flags = SPINLOCK_BLOCK_NO_INT(scheduler.listSleeping.lock) {
//        Enqueue(&scheduler.listSleeping, &scheduler.currentProcess->stsQueue);
//        SPINLOCK_RLS_RESTORE_INT(scheduler.listSleeping.lock, flags);
//    }

    CLEAN_PROCESS(scheduler.currentProcess);

    ProcessBlock(PROC_DLYW);
//    ProcessExitPostpone();
}