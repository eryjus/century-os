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

    SPIN_BLOCK(semaphoreAll.globalLock) {
        SemaphoreSet_t *set = semaphoreAll.semaphoreSets[semid];
        if (set == NULL) {
            SPIN_RLS(semaphoreAll.globalLock);
            return -EINVAL;
        }

        switch(cmd) {
        case IPC_STAT:      // -- get the current semaphore settings
            SPIN_BLOCK(set->lock) {
                SPIN_RLS(semaphoreAll.globalLock);          // release the bigger lock when we have a smaller one
                rv = SemIpcStat(set, semun.buf);
                SPIN_RLS(set->lock);
            }

            return rv;

        case IPC_SET:       // -- set some new semaphore settings
            SPIN_BLOCK(set->lock) {
                SPIN_RLS(semaphoreAll.globalLock);          // release the bigger lock when we have a smaller one
                rv = SemIpcSet(set, semun.buf);
                SPIN_RLS(set->lock);
            }

            return rv;

        case IPC_RMID:      // -- remove the semaphore set
            SPIN_BLOCK(set->lock);      // use this to wait for all other locks to release; never released
            semaphoreAll.semaphoreSets[semid] = NULL;
            SPIN_RLS(semaphoreAll.globalLock);      // no one else can find this sem set now...
            return SemRemove(set, semid);

        case GETALL:        // get all the current counts -- atomically
            SPIN_BLOCK(set->lock) {
                SPIN_RLS(semaphoreAll.globalLock);          // release the bigger lock when we have a smaller one
                rv = SemGetAll(set, semun.array);
                SPIN_RLS(set->lock);
            }

            return rv;

        case GETNCNT:       // -- get the waitN count
            if (!HasReadPermission(set->permissions)) rv = -EACCES;
            else if (semnum < 0 || semnum > set->numSem - 1) rv = -EINVAL;
            else rv = set->semSet[semnum].waitN.count;   // -- no need to get lower-level locks; this is quick

            SPIN_RLS(semaphoreAll.globalLock);
            return rv;

        case GETPID:        // -- get the pid of the last process to update
            if (!HasReadPermission(set->permissions)) rv = -EACCES;
            else if (semnum < 0 || semnum > set->numSem - 1) rv = -EINVAL;
            else rv = set->semSet[semnum].semPid;

            SPIN_RLS(semaphoreAll.globalLock);
            return rv;

        case GETVAL:        // -- get the value of the nth semaphore
            if (!HasReadPermission(set->permissions)) rv = -EACCES;
            else if (semnum < 0 || semnum > set->numSem - 1) rv = -EINVAL;
            else rv = AtomicRead(&set->semSet[semnum].semval);

            SPIN_RLS(semaphoreAll.globalLock);
            return rv;

        case GETZCNT:       // -- get the waitZ count
            if (!HasReadPermission(set->permissions)) rv = -EACCES;
            else if (semnum < 0 || semnum > set->numSem - 1) rv = -EINVAL;
            else rv = set->semSet[semnum].waitZ.count;   // -- no need to get lower-level locks; this is quick
            SPIN_RLS(semaphoreAll.globalLock);
            return rv;

        case SETALL:
            {
                if (!HasReadPermission(0, 02)) {      // -- check write permissions
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EACCES;
                }

                Semaphore_t *sem = &set->semSet[semnum];
                int val = semun.val;

                ProcessEnterPostpone();                 // SemReadyWaiting might reschedule if we do not postpone
                SPIN_BLOCK(set->lock) {
                    SPIN_RLS(semaphoreAll.globalLock);
                    set->semCtime = 0;                  // TODO: set the time value

                    for (int i = 0; i < set->numSem; i ++) {
                        int prev;
                        prev = AtomicRead(&sem->semval);
                        AtomicSet(&sem->semval, val);

                        if (val == 0) SemReadyWaiting(&sem->waitZ);
                        else if (val > prev) SemReadyWaiting(&sem->waitN);

                        SemUndoReset(semid, set->key, -1);
                    }

                    SPIN_RLS(set->lock);
                }
                ProcessExitPostpone();
            }

            return 0;

        case SETVAL:
            {
                if (!HasReadPermission(0, 02)) {      // -- check write permissions
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EACCES;
                }

                if (semnum < 0 || semnum > set->numSem - 1) {
                    SPIN_RLS(semaphoreAll.globalLock);
                    return -EINVAL;
                }

                Semaphore_t *sem = &set->semSet[semnum];
                int val = semun.val;
                int prev;

                ProcessEnterPostpone();                 // SemReadyWaiting might reschedule with the lock held
                SPIN_BLOCK(set->lock) {
                    SPIN_RLS(semaphoreAll.globalLock);
                    set->semCtime = 0;                  // TODO: set the time value
                    prev = AtomicRead(&sem->semval);
                    AtomicSet(&sem->semval, val);


                    if (val == 0) SemReadyWaiting(&sem->waitZ);
                    else if (val > prev) SemReadyWaiting(&sem->waitN);

                    SemUndoReset(semid, set->key, semnum);
                    SPIN_RLS(set->lock);
                }
                ProcessExitPostpone();
            }

            return 0;

        default:
            SPIN_RLS(semaphoreAll.globalLock);
            return -EINVAL;
        }
    }

    return rv;         // -- should never get here, but just in case
}

