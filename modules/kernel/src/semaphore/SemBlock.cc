//===================================================================================================================
//
//  SemaBlock.cc -- Block the current process for the oerations provided
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//
//  This functions will not actually block the process, but will add the current process to the queue of processes
//  tha are blocked.
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
// -- Set the current process to be blocked on the semaphores
//    -------------------------------------------------------
void __krntext SemBlock(Semaphore_t *sem, bool zeroWait)
{
    SemWaiting_t *waiting = NEW(SemWaiting_t);
    ListInit(&waiting->list);
    waiting->proc = scheduler.currentProcess;

    if (zeroWait) {
        archsize_t flags = SPINLOCK_BLOCK_NO_INT(sem->waitZ.lock) {
            ListAdd(&sem->waitZ, &waiting->list);
            SPINLOCK_RLS_RESTORE_INT(sem->waitZ.lock, flags);
        }
    } else {
        archsize_t flags = SPINLOCK_BLOCK_NO_INT(sem->waitN.lock) {
            ListAdd(&sem->waitN, &waiting->list);
            SPINLOCK_RLS_RESTORE_INT(sem->waitN.lock, flags);
        }
    }
}

