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
    SPIN_BLOCK(semaphoreAll.globalLock) {
        set = semaphoreAll.semaphoreSets[semid];
        SPIN_RLS(semaphoreAll.globalLock);
    }

    if (set == NULL) return -EINVAL;


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


        SPIN_BLOCK(semaphoreAll.globalLock) {
            set = semaphoreAll.semaphoreSets[semid];


            //
            // -- Now check if the set was removed
            //    --------------------------------
            if (set == NULL) {
                SPIN_RLS(semaphoreAll.globalLock);
                return -EIDRM;          // -- at this point, it existed when we started but does not anymore
            }

            SPIN_BLOCK(set->lock) {
                SPIN_RLS(semaphoreAll.globalLock);
                //
                // -- pre-check the ops (returns <0 on error; 0 on success; 1 to block)
                //    -----------------------------------------------------------------
                int res = SemIterateOps(semid, set, sops, nsops, SEM_ITER_CHECK);

                if (res < 0) {
                    SPIN_RLS(set->lock);
                    return res;
                } else if (res == 0) {
                    ProcessEnterPostpone();                                      // -- do not reschedule with the lock held
                    res = SemIterateOps(semid, set, sops, nsops, SEM_ITER_EXEC);        // -- better be 0!!
                    SPIN_RLS(set->lock);
                    ProcessExitPostpone();
                    return (res>0?-EUNDEF:res);
                } else {
                    SemIterateOps(semid, set, sops, nsops, SEM_ITER_BLOCK);
                    SPIN_RLS(set->lock);
                    ProcessBlock(PROC_SEMW);
                }
            }
        }
    }
}

