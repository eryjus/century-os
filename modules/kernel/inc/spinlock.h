//===================================================================================================================
//
//  spinlock.h -- Structures for spinlock management
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
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__


#include "types.h"
#include "printf.h"


//
// -- This macro basically disappears because but helps to delineate the block that requires the lock
//    -----------------------------------------------------------------------------------------------
#define SPIN_BLOCK(lock)        SpinlockLock(&(lock));


//
// -- This marco only exists so I do not need to type an '&' with each unlock
//    -----------------------------------------------------------------------
#define SPIN_RLS(lock)          SpinlockUnlock(&(lock))


//
// -- This is the spinlock structure which notes who holds the lock
//    -------------------------------------------------------------
typedef struct Spinlock_t {
    archsize_t locked;
    PID_t lockHolder;
} Spinlock_t;


//
// -- This is an atomic function to lock a spinlock
//    ---------------------------------------------
extern "C" archsize_t SpinlockAtomicLock(Spinlock_t *lock, archsize_t expected, archsize_t newVal);


//
// -- This is an atomic function to unlock a spinlock
//    -----------------------------------------------
extern "C" void SpinlockClear(Spinlock_t *lock);


//
// -- This inline function will lock a spinlock, busy looping indefinitely until a lock is obtained
//    ---------------------------------------------------------------------------------------------
static inline void SpinlockLock(Spinlock_t *lock) {
    while (SpinlockAtomicLock(lock, 0, 1) != 0) {  }
    lock->lockHolder = currentPID;
}


//
// -- This inline function will unlock a spinlock, clearing the lock holder
//    ---------------------------------------------------------------------
static inline void SpinlockUnlock(Spinlock_t *lock) { SpinlockClear(lock); lock->lockHolder = 0; }


//
// -- This inline function returns the PID of the lock holder
//    -------------------------------------------------------
static inline PID_t SpinLockGetHolder(Spinlock_t *lock) { return lock->lockHolder; }


//
// -- This inline function will determine if a spinlock is locked
//    -----------------------------------------------------------
static inline bool SpinlockIsLocked(Spinlock_t *lock) { return lock->locked; }


//
// -- This is the spinlock for the temporary page for frame population ahead of putting it to a process
//    -------------------------------------------------------------------------------------------------
extern Spinlock_t lockTempPage;


#endif
