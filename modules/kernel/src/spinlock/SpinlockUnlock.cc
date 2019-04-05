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
    CLEAN_SPINLOCK(lock);
}



