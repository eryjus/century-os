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
#include "atomic.h"
#include "cpu.h"


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
    AtomicInt_t lock;
    struct Process_t *lockHolder;
} Spinlock_t;


//
// -- This is a count of the number of locks that are currently held and its lock
//    ---------------------------------------------------------------------------
extern int locksHeld;
extern Spinlock_t lockCounterLock;


//
// -- This inline function will lock a spinlock, busy looping indefinitely until a lock is obtained
//    ---------------------------------------------------------------------------------------------
__CENTURY_FUNC__ inline void SpinlockLock(Spinlock_t *lock) {
        while (AtomicSet(&lock->lock, 1) != 0) { }       // -- This is a busy wait
//        lock->lockHolder = scheduler.currentProcess;
}


//
// -- This inline function will unlock a spinlock, clearing the lock holder
//    ---------------------------------------------------------------------
__CENTURY_FUNC__ inline void SpinlockUnlock(Spinlock_t *lock) {
//    if (lock->lockHolder != scheduler.currentProcess) return;
    AtomicSet(&lock->lock, 0);
}


//
// -- This inline function returns the PID of the lock holder
//    -------------------------------------------------------
__CENTURY_FUNC__ inline struct Process_t *SpinLockGetHolder(Spinlock_t *lock) { return lock->lockHolder; }


//
// -- This inline function will determine if a spinlock is locked
//    -----------------------------------------------------------
__CENTURY_FUNC__ inline bool SpinlockIsLocked(Spinlock_t *lock) { return AtomicRead(&lock->lock) == 1; }


//
// -- This is the spinlock for the temporary page for frame population ahead of putting it to a process
//    -------------------------------------------------------------------------------------------------
extern Spinlock_t lockTempPage;


//
// -- This is the lock that controls access to the address space for initializing the table
//    -------------------------------------------------------------------------------------
extern Spinlock_t mmuTableInitLock;


//
// -- This is the lock that controls access to the address space for initializing the table
//    -------------------------------------------------------------------------------------
extern Spinlock_t mmuStackInitLock;


//
// -- This macro will clean (flush) the cache for a Spinlock, making changes visible to all
//    -------------------------------------------------------------------------------------
#define CLEAN_SPINLOCK(lock) CLEAN_CACHE(lock, sizeof(Spinlock_t))


//
// -- This macro will invalidate the cache for a Spinlock, forcing it the be re-read from memory
//    ------------------------------------------------------------------------------------------
#define INVALIDATE_SPINLOCK(lock) INVALIDATE_CACHE(lock, sizeof(Spinlock_t))


#endif
