//===================================================================================================================
//
//  SemaphoreOperations.cc -- Complete the set of semaphore operations requested
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
//  2019-May-04  Initial   0.4.3   ADCL  Initial version
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
// --  Complete the semaphore operations
//     ---------------------------------
int __krntext SemaphoreOperations(int semid, struct sembuf *sops, size_t nsops)
{
    // -- first some sanity checks
    if ((int)nsops > semopm) return -E2BIG;
    if (semid < 0 || semid >= semmni) return -EINVAL;
    if (!IS_MAPPED(sops, sizeof(struct sembuf) * nsops)) return -EFAULT;

    SemaphoreSet_t *set;


    //
    // -- check that semid is actually valid
    //    ----------------------------------
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(semaphoreAll.globalLock) {
        set = semaphoreAll.semaphoreSets[semid];
        SPINLOCK_RLS_RESTORE_INT(semaphoreAll.globalLock, flags);
    }

    if (set == NULL) return -EINVAL;


    //
    // -- assume no pending error
    //    -----------------------
    scheduler.currentProcess->pendingErrno = 0;


    //
    // -- loop indefinitely until we can return
    //    -------------------------------------
    while (true) {
        //
        // -- was an error generated starting from some other process?
        //    --------------------------------------------------------
        if (scheduler.currentProcess->pendingErrno != 0) {
            int rv = scheduler.currentProcess->pendingErrno;
            scheduler.currentProcess->pendingErrno = 0;
            return rv;
        }


        flags = SPINLOCK_BLOCK_NO_INT(semaphoreAll.globalLock) {
            set = semaphoreAll.semaphoreSets[semid];


            //
            // -- Now check if the set was removed
            //    --------------------------------
            if (set == NULL) {
                SPINLOCK_RLS_RESTORE_INT(semaphoreAll.globalLock, flags);
                return -EIDRM;          // -- at this point, it existed when we started but does not anymore
            }

            SPINLOCK_BLOCK(set->lock) {
                SPINLOCK_RLS(semaphoreAll.globalLock);
                //
                // -- pre-check the ops (returns <0 on error; 0 on success; 1 to block)
                //    -----------------------------------------------------------------
                kprintf(".. Checking returns: ");
                int res = SemIterateOps(semid, set, sops, nsops, SEM_ITER_CHECK);

                if (res < 0) {
                    kprintf("error\n");
                    SPINLOCK_RLS_RESTORE_INT(set->lock, flags);
                    return res;
                } else if (res == 0) {
                    kprintf("exec\n");
                    ProcessEnterPostpone();                                      // -- do not reschedule with the lock held
                    res = SemIterateOps(semid, set, sops, nsops, SEM_ITER_EXEC);        // -- better be 0!!
                    SPINLOCK_RLS_RESTORE_INT(set->lock, flags);
                    ProcessExitPostpone();
                    return (res>0?-EUNDEF:res);
                } else {
                    kprintf("block\n");
                    SemIterateOps(semid, set, sops, nsops, SEM_ITER_BLOCK);
                    SPINLOCK_RLS_RESTORE_INT(set->lock, flags);
                    ProcessBlock(PROC_SEMW);
                }
            }
        }
    }
}

