//===================================================================================================================
//
//  SemaCreateUndo.cc -- This function finds and updates or creates an undo entry for a Semaphore
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
void __krntext SemCreateUndo(int semid, key_t key, int semnum, int semadj)
{
    PID_t pid = scheduler.currentProcess->pid;
    SemaphoreUndo_t *undo;

    SPIN_BLOCK(semaphoreAll.undoList.lock) {
        ListHead_t::List_t *wrk = semaphoreAll.undoList.list.next;
        while (wrk != &semaphoreAll.undoList.list) {
            undo = FIND_PARENT(wrk, SemaphoreUndo_t, list);

            if (undo->semid == semid && undo->key == key && undo->pid == pid && undo->semnum == semnum) {
                undo->semadj += semadj;
                SPIN_RLS(semaphoreAll.undoList.lock);
                return;
            }

            wrk = wrk->next;
        }

        // -- ok, create a new one
        undo = NEW(SemaphoreUndo_t);

        ListInit(&undo->list);
        undo->semid = semid;
        undo->key = key;
        undo->pid = pid;
        undo->semnum = semnum;
        undo->semadj = semadj;

        ListAdd(&semaphoreAll.undoList, &undo->list);

        SPIN_RLS(semaphoreAll.undoList.lock);
    }
}
