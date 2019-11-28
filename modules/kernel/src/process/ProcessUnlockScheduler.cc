//===================================================================================================================
//
// ProcessUnlockScheduler.cc -- Unlock the scheduler after manipulation
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Nov-26  Initial   0.4.6a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "lists.h"
#include "timer.h"
#include "spinlock.h"
#include "process.h"


//
// -- Unlock the scheduler after changes
//    ----------------------------------
EXPORT KERNEL
void ProcessUnlockScheduler(void)
{
    if (AtomicDecAndTest0(&scheduler.schedulerLockCount)) {
        SPINLOCK_RLS_RESTORE_INT(schedulerLock, scheduler.flags);
    }
}


