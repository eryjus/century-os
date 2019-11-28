//===================================================================================================================
//
//  SemaphoreControl.cc -- The implementation of the POSIX `semctl` API -- control semaphores
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Programmer's note:
//  While I use hex numbers quite a bit to express bit flags, this file contains several octal numbers.  Be aware
//  of this fact as you are reading this source.
//
//  POSIX variances:
//  A) Permissions are not yet implemented.  Therefore, all processes have all permissions.
//  B) Times are not yet maintained and will all be reported as 0.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-May-01  Initial   0.4.3   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "errno.h"
#include "mmu.h"
#include "user-group.h"
#include "lists.h"
#include "process.h"
#include "heap.h"
#include "spinlock.h"
#include "semaphore.h"


//
// -- Semaphore Control API
//    ---------------------
int __krntext SemaphoreControl(int semid, int semnum, int cmd, union semun semun)
{
    int rv = -EUNDEF;       // -- should never return, but just in case

    if (semid < 0 || semid >= semmni) return -EINVAL;

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(semaphoreAll.globalLock) {
        SemaphoreSet_t *set = semaphoreAll.semaphoreSets[semid];
        if (set == NULL) {
            SPINLOCK_RLS_RESTORE_INT(semaphoreAll.globalLock, flags);
            return -EINVAL;
        }

        switch(cmd) {
        case IPC_STAT:      // -- get the current semaphore settings
            SPINLOCK_BLOCK(set->lock) {
                SPINLOCK_RLS(semaphoreAll.globalLock);          // release the bigger lock when we have a smaller one
                rv = SemIpcStat(set, semun.buf);
                SPINLOCK_RLS_RESTORE_INT(set->lock, flags);
            }

            return rv;

        case IPC_SET:       // -- set some new semaphore settings
            SPINLOCK_BLOCK(set->lock) {
                SPINLOCK_RLS(semaphoreAll.globalLock);          // release the bigger lock when we have a smaller one
                rv = SemIpcSet(set, semun.buf);
                SPINLOCK_RLS_RESTORE_INT(set->lock, flags);
            }

            return rv;

        case IPC_RMID:      // -- remove the semaphore set
            SPINLOCK_BLOCK(set->lock);      // use this to wait for all other locks to release; never released
            semaphoreAll.semaphoreSets[semid] = NULL;
            SPINLOCK_RLS_RESTORE_INT(semaphoreAll.globalLock, flags);      // no one else can find this sem set now...
            return SemRemove(set, semid);

        case GETALL:        // get all the current counts -- atomically
            SPINLOCK_BLOCK(set->lock) {
                SPINLOCK_RLS(semaphoreAll.globalLock);          // release the bigger lock when we have a smaller one
                rv = SemGetAll(set, semun.array);
                SPINLOCK_RLS_RESTORE_INT(set->lock, flags);
            }

            return rv;

        case GETNCNT:       // -- get the waitN count
            if (!HasReadPermission(set->permissions)) rv = -EACCES;
            else if (semnum < 0 || semnum > set->numSem - 1) rv = -EINVAL;
            else rv = set->semSet[semnum].waitN.count;   // -- no need to get lower-level locks; this is quick

            SPINLOCK_RLS_RESTORE_INT(semaphoreAll.globalLock, flags);
            return rv;

        case GETPID:        // -- get the pid of the last process to update
            if (!HasReadPermission(set->permissions)) rv = -EACCES;
            else if (semnum < 0 || semnum > set->numSem - 1) rv = -EINVAL;
            else rv = set->semSet[semnum].semPid;

            SPINLOCK_RLS_RESTORE_INT(semaphoreAll.globalLock, flags);
            return rv;

        case GETVAL:        // -- get the value of the nth semaphore
            if (!HasReadPermission(set->permissions)) rv = -EACCES;
            else if (semnum < 0 || semnum > set->numSem - 1) rv = -EINVAL;
            else rv = AtomicRead(&set->semSet[semnum].semval);

            SPINLOCK_RLS_RESTORE_INT(semaphoreAll.globalLock, flags);
            return rv;

        case GETZCNT:       // -- get the waitZ count
            if (!HasReadPermission(set->permissions)) rv = -EACCES;
            else if (semnum < 0 || semnum > set->numSem - 1) rv = -EINVAL;
            else rv = set->semSet[semnum].waitZ.count;   // -- no need to get lower-level locks; this is quick
            SPINLOCK_RLS_RESTORE_INT(semaphoreAll.globalLock, flags);
            return rv;

        case SETALL:
            {
                if (!HasReadPermission(0, 02)) {      // -- check write permissions
                    SPINLOCK_RLS_RESTORE_INT(semaphoreAll.globalLock, flags);
                    return -EACCES;
                }

                Semaphore_t *sem = &set->semSet[semnum];
                int val = semun.val;

                ProcessLockAndPostpone();                 // SemReadyWaiting might reschedule if we do not postpone
                SPINLOCK_BLOCK(set->lock) {
                    SPINLOCK_RLS(semaphoreAll.globalLock);
                    set->semCtime = 0;                  // TODO: set the time value

                    for (int i = 0; i < set->numSem; i ++) {
                        int prev;
                        prev = AtomicRead(&sem->semval);
                        AtomicSet(&sem->semval, val);

                        if (val == 0) SemReadyWaiting(&sem->waitZ);
                        else if (val > prev) SemReadyWaiting(&sem->waitN);

                        SemUndoReset(semid, set->key, -1);
                    }

                    SPINLOCK_RLS_RESTORE_INT(set->lock, flags);
                }
                ProcessUnlockAndSchedule();
            }

            return 0;

        case SETVAL:
            {
                if (!HasReadPermission(0, 02)) {      // -- check write permissions
                    SPINLOCK_RLS_RESTORE_INT(semaphoreAll.globalLock, flags);
                    return -EACCES;
                }

                if (semnum < 0 || semnum > set->numSem - 1) {
                    SPINLOCK_RLS_RESTORE_INT(semaphoreAll.globalLock, flags);
                    return -EINVAL;
                }

                Semaphore_t *sem = &set->semSet[semnum];
                int val = semun.val;
                int prev;

                ProcessLockAndPostpone();                 // SemReadyWaiting might reschedule with the lock held
                SPINLOCK_BLOCK(set->lock) {
                    SPINLOCK_RLS(semaphoreAll.globalLock);
                    set->semCtime = 0;                  // TODO: set the time value
                    prev = AtomicRead(&sem->semval);
                    AtomicSet(&sem->semval, val);


                    if (val == 0) SemReadyWaiting(&sem->waitZ);
                    else if (val > prev) SemReadyWaiting(&sem->waitN);

                    SemUndoReset(semid, set->key, semnum);
                    SPINLOCK_RLS_RESTORE_INT(set->lock, flags);
                }
                ProcessUnlockAndSchedule();
            }

            return 0;

        default:
            SPINLOCK_RLS_RESTORE_INT(semaphoreAll.globalLock, flags);
            return -EINVAL;
        }
    }

    return rv;         // -- should never get here, but just in case
}

