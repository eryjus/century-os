//===================================================================================================================
//
//  SpinlockLock.cc -- Lock a spinlock
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
#include "spinlock.h"


//
// -- This inline function will lock a spinlock, busy looping indefinitely until a lock is obtained
//    ---------------------------------------------------------------------------------------------
void __krntext SpinlockLock(Spinlock_t *lock)
{
    while (SpinlockAtomicLock(lock, 0, 1) != 0) {  }
    CLEAN_SPINLOCK(lock);

    //
    // -- Note the lock holder; may use later
    //    -----------------------------------
//    lock->lockHolder = currentProcess;
//    CLEAN_SPINLOCK(lock);
}


