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
    SPIN_BLOCK(schedulerLock) {
        schedulerLocksHeld --;
        SpinlockUnlock(&schedulerLock);
    }

    // -- interrupts are still disabled here
    if (schedulerLocksHeld == 0) {
        if (processChangePending != 0) {
            processChangePending = 0;           // need to clear this to actually perform a change
            ProcessSchedule();
        }

        EnableInterrupts();
    }
}

