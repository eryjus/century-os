//===================================================================================================================
//
// ProcessUnlockAndSchedule.cc -- Exit a postponed schedule block and take care of any pending schedule changes
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
#include "spinlock.h"
#include "process.h"


//
// -- decrease the lock count on the scheduler
//    ----------------------------------------
EXPORT KERNEL
void ProcessUnlockAndSchedule(void)
{
    assert_msg(AtomicRead(&scheduler.postponeCount) > 0, "postponeCount out if sync");

    if (AtomicDecAndTest0(&scheduler.postponeCount) == true) {
        if (scheduler.processChangePending != false) {
            scheduler.processChangePending = false;           // need to clear this to actually perform a change
            ProcessSchedule();
        }
    }

    ProcessUnlockScheduler();
}

