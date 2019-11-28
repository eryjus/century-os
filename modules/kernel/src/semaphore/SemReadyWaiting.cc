//===================================================================================================================
//
//  SemaReadyWaiting.cc -- This function makes ready the processes that are waiting for a semaphore
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//
//  This function is like a scheduler function in that it may perform several operations and we want to delay
//  any actual scheduling change until we have completed all the work necessary.
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
//  2019-May-05  Initial   0.4.3   ADCL  Initial version
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
// -- Ready the processes waiting for a Semaphore
//    -------------------------------------------
void __krntext SemReadyWaiting(ListHead_t *list)
{
    ProcessLockAndPostpone();
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(list->lock) {
        while (!IsListEmpty(list)) {
            SemWaiting_t *wait = FIND_PARENT(list->list.next, SemWaiting_t, list);
            ListRemoveInit(&wait->list);
            if (wait->proc->status != PROC_READY && wait->proc->status != PROC_RUNNING) {
                if (wait->proc->stsQueue.next != &wait->proc->stsQueue) {
                    ListRemoveInit(&wait->proc->stsQueue);
                }

                ProcessReady(wait->proc);
            }

            FREE(wait);
        }

        SPINLOCK_RLS_RESTORE_INT(list->lock, flags);
    }
    ProcessUnlockAndSchedule();
}

