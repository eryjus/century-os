//===================================================================================================================
//
// ProcessLockScheduler.cc -- Lock the scheduler for manipulation
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Nov-25  Initial   0.4.6a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "lists.h"
#include "timer.h"
#include "spinlock.h"
#include "process.h"


//
// -- Lock the scheduler in preparation for changes
//    ---------------------------------------------
EXPORT KERNEL
void ProcessLockScheduler(bool save)
{
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(schedulerLock);
    scheduler.lockCpu = thisCpu->cpuNum;

    if (AtomicRead(&scheduler.schedulerLockCount) == 0) {
//        kprintf("Scheduler locked on CPU%d\n", thisCpu->cpuNum);
        if (save) scheduler.flags = flags;
    }

    AtomicInc(&scheduler.schedulerLockCount);
}


