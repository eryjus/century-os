//===================================================================================================================
//
//  SemUndoReset.cc -- Resets the undo entry for a semaphore (or semaphore set) to 0
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
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
#include "lists.h"
#include "spinlock.h"
#include "semaphore.h"


//
// -- Reset the undo entries for a semaphore or set to 0 (when semnum < 0; operates on whole set)
//    -------------------------------------------------------------------------------------------
void __krntext SemUndoReset(int semid, key_t key, int semnum)
{
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(semaphoreAll.undoList.lock) {
        ListHead_t::List_t *wrk = semaphoreAll.undoList.list.next;

        while (wrk != &semaphoreAll.undoList.list) {
            SemaphoreUndo_t *undo = FIND_PARENT(wrk, SemaphoreUndo_t, list);
            wrk = wrk->next;

            if (undo->semid == semid && undo->key == key && (semnum < 0 || undo->semnum == semnum)) {
                undo->semadj = 0;
            }
        }

        SPINLOCK_RLS_RESTORE_INT(semaphoreAll.undoList.lock, flags);
    }
}


