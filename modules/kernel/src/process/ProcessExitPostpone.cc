//===================================================================================================================
//
// ProcessExitPostpone.cc -- Exit a postponed schedule block and take care of any pending schedule changes
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
#include "spinlock.h"
#include "process.h"


//
// -- decrease the lock count on the scheduler
//    ----------------------------------------
void __krntext ProcessExitPostpone(void)
{
    SPIN_BLOCK(scheduler.schedulerLock) {
        scheduler.schedulerLocksHeld --;
        SpinlockUnlock(&scheduler.schedulerLock);
    }

    // -- interrupts are still disabled here
    if (scheduler.schedulerLocksHeld == 0) {
        if (scheduler.processChangePending != 0) {
            scheduler.processChangePending = 0;           // need to clear this to actually perform a change
            ProcessSchedule();
        }

        CLEAN_SCHEDULER();
        EnableInterrupts();
    }
}

