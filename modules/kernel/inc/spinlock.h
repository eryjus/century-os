//===================================================================================================================
//
// spinlock.h -- Structures for spinlock management
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-14  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __SPINLOCK_H__
#define __SPINLOCK_H__


#include "types.h"


//
// -- This macro basically disappears because but helps to delineate the block that requires the lock
//    -----------------------------------------------------------------------------------------------
#define SPIN_BLOCK(lock)        SpinlockLock(&(lock));


//
// -- This is the spinlock structure which notes who holds the lock
//    -------------------------------------------------------------
typedef struct Spinlock_t {
    int32_t locked;
    PID_t lockHolder;
} Spinlock_t;


//
// -- This is an atomic function to lock a spinlock
//    ---------------------------------------------
extern "C" int32_t SpinlockCmpXchg(Spinlock_t *lock, int32_t expected, int32_t newVal);


//
// -- This is an atomic function to unlock a spinlock
//    -----------------------------------------------
extern "C" void SpinlockClear(Spinlock_t *lock);


//
// -- This inline function will lock a spinlock, busy looping indefinitely until a lock is obtained
//    ---------------------------------------------------------------------------------------------
static inline void SpinlockLock(Spinlock_t *lock) {
    while (SpinlockCmpXchg(lock, 0, 1) != 0) {};
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


#endif
