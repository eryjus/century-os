//===================================================================================================================
//
//  SemRemove.cc -- Remove a semaphore set, waking up all the held processes that are blocked
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
#include "heap.h"
#include "process.h"
#include "user-group.h"
#include "semaphore.h"


//
// -- Remove a semaphore set -- caller holds the lock on set
//    ------------------------------------------------------
int __krntext SemRemove(SemaphoreSet_t *set, int semid)
{
    if (!IsCreatorOwner(set->owner, set->creator)) return -EPERM;

    for (int i = 0; i < set->numSem; i ++) {
        Semaphore_t *sem = &set->semSet[i];


        //
        // -- wake up all the processes waiting for the count to increase
        //    -----------------------------------------------------------
        while (!IsListEmpty(&sem->waitN)) {
            Process_t *proc = FIND_PARENT(sem->waitN.list.next, Process_t, stsQueue);
            ListRemoveInit(sem->waitN.list.next);
            proc->pendingErrno = -EIDRM;
            ProcessReady(proc);
        }


        //
        // -- wake up all the processes wayting for the count to reack zero
        //    -------------------------------------------------------------
        while (!IsListEmpty(&sem->waitZ)) {
            Process_t *proc = FIND_PARENT(sem->waitZ.list.next, Process_t, stsQueue);
            ListRemoveInit(sem->waitZ.list.next);
            proc->pendingErrno = -EIDRM;
            ProcessReady(proc);
        }
    }


    //
    // -- Now, remove all the undo entries from the undo list
    //    ---------------------------------------------------
    SPIN_BLOCK(semaphoreAll.undoList.lock) {
        ListHead_t::List_t *wrk = semaphoreAll.undoList.list.next;
        while (wrk != &semaphoreAll.undoList.list) {
            SemaphoreUndo_t *undo = FIND_PARENT(wrk, SemaphoreUndo_t, list);
            wrk = wrk->next;

            if (undo->semid == semid && undo->key == set->key) {
                ListRemoveInit(&undo->list);
                HeapFree(undo);
            }
        }

        SPIN_RLS(semaphoreAll.undoList.lock);
    }


    //
    // -- Finally, free the memory
    //    ------------------------
    FREE(set->semSet);
    FREE(set);

    return 0;
}



