//===================================================================================================================
//
//  SpinlockUnlock.cc -- Unlock a spinlock
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-24  Initial   0.3.2   ADCL  Initial version; converted from an inline function
//
//===================================================================================================================


#include "types.h"
#include "process.h"
#include "spinlock.h"


//
// -- This inline function will unlock a spinlock, clearing the lock holder
//    ---------------------------------------------------------------------
void __krntext SpinlockUnlock(Spinlock_t *lock)
{
    SpinlockClear(lock);


    //
    // -- now, lock the counter lock so it can be incremented
    //    NOTE: This is the only lock that can be scheduled away from
    //    -----------------------------------------------------------
    while (SpinlockAtomicLock(&lockCounterLock, 0, 1) != 0) {  }
    locksHeld --;
    SpinlockClear(&lockCounterLock);

    if (locksHeld == 0) {
        if (processChangePending) {
            processChangePending = false;
            ProcessSchedule();
        }
    }
}



