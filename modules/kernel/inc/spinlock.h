//===================================================================================================================
//
//  spinlock.h -- Structures for spinlock management
//
//        Copyright (c)  2017-2020 -- Adam Clark
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


#pragma once


#include "types.h"
#include "cpu.h"


//
// -- This macro basically disappears because but helps to delineate the block that requires the lock
//
//    Block Usage:
//    ------------
//
//    Spinlock lock = {0};
//    SPINLOCK_BLOCK(lock) {
//        // Do some important stuff here...
//        SPINLOCK_RLS(lock);
//    }
//
//    Note that in this context, the trailing ';' is required.
//    -----------------------------------------------------------------------------------------------
#define SPINLOCK_BLOCK(lock)        SpinLock(&(lock));


//
// -- This marco only exists so I do not need to type an '&' with each unlock
//    -----------------------------------------------------------------------
#define SPINLOCK_RLS(lock)          SpinUnlock(&(lock))


//
// -- This macro exists to help with code readaibility -- get a lock and save interrupts
//
//    Block Usage:
//    ------------
//
//    Spinlock lock = {0};
//    archsize_t flags = SPINLOCK_BLOCK_NO_INT(lock) {
//        // Do some important stuff here...
//        SPINLOCK_RLS_RESTORE_INT(lock);
//    }
//
//    Note that in this context, the trailing ';' is required.
//    ----------------------------------------------------------------------------------
#define SPINLOCK_BLOCK_NO_INT(lock) ({                              \
            archsize_t flags = DisableInterrupts();                 \
            SpinLock(&(lock));                                      \
            flags;                                                  \
        });


//
// -- This macro exists to help with code readaibility -- restore interrupts and release lock
//    ---------------------------------------------------------------------------------------
#define SPINLOCK_RLS_RESTORE_INT(lock,f) do {                       \
            SpinUnlock(&(lock));                                    \
            RestoreInterrupts(f);                                   \
        } while (false)



//
// -- This is the spinlock structure which notes who holds the lock
//    -------------------------------------------------------------
typedef struct Spinlock_t {
    SMP_UNSTABLE int lock;
} Spinlock_t;


//
// -- This inline function will lock a spinlock, busy looping indefinitely until a lock is obtained
//    ---------------------------------------------------------------------------------------------
EXPORT INLINE
void SpinLock(Spinlock_t *lock) {
    int exp, des;
    do {
        exp = 0;
        des = 1;
    } while (!__atomic_compare_exchange(&(lock->lock), &exp, &des, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST));
}


//
// -- This inline function will unlock a spinlock, clearing the lock holder
//    ---------------------------------------------------------------------
EXPORT INLINE
void SpinUnlock(Spinlock_t *lock) {
    int zero = 0;
    __atomic_store(&(lock->lock), &zero, __ATOMIC_SEQ_CST);
}


//
// -- This inline function will determine if a spinlock is locked
//    -----------------------------------------------------------
EXPORT INLINE
bool SpinlockIsLocked(Spinlock_t *lock) {
    int l;
    __atomic_load(&(lock->lock), &l, __ATOMIC_SEQ_CST);
    return l == 1;
}


//
// -- This is the lock that controls access to the address space for initializing the table
//    -------------------------------------------------------------------------------------
EXTERN EXPORT KERNEL_DATA
Spinlock_t mmuTableInitLock;


//
// -- This is the lock that controls access to the address space for initializing the table
//    -------------------------------------------------------------------------------------
EXTERN EXPORT KERNEL_DATA
Spinlock_t mmuStackInitLock;


//
// -- This macro will clean (flush) the cache for a Spinlock, making changes visible to all
//    -------------------------------------------------------------------------------------
#define CLEAN_SPINLOCK(lock) CleanCache(lock, sizeof(Spinlock_t))


//
// -- This macro will invalidate the cache for a Spinlock, forcing it the be re-read from memory
//    ------------------------------------------------------------------------------------------
#define INVALIDATE_SPINLOCK(lock) InvalidateCache(lock, sizeof(Spinlock_t))


