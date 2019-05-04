//===================================================================================================================
//
//  SemaphoreGet.cc -- The implementation of the POSIX `semget` API
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Posix variance:
//  Posix specifies that a maximum SEMMNS total system-wide semaphores can be created across all set.  This value
//  is not implemented in the system at this time -- it exists but is not checked, effectively making
//  SEMMNS > SEMMSL * SEMMNI.
//
//  TODO: Implement the time fields.
//  TODO: Implement uid/gid.
//  TODO: Implement permissions.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-30  Initial   0.4.3   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "heap.h"
#include "cpu.h"
#include "errno.h"
#include "semaphore.h"


//
// -- create and initialize a new sem set
//    -----------------------------------
static SemaphoreSet_t *NewSemSet(key_t key, int nsems, int semflg)
{
    SemaphoreSet_t *rv = NEW(SemaphoreSet_t);
    if (rv == NULL) return NULL;
    kMemSetB(rv, 0, sizeof(SemaphoreSet_t));

    rv->numSem = nsems;
    rv->permissions.mode = (semflg & 0777);     // low 9 bits are typical linux xrwxrwxrw flags
    rv->semSet = (Semaphore_t *)HeapAlloc(nsems * sizeof(Semaphore_t), false);

    if (rv->semSet == NULL) {
        FREE(rv);
        return NULL;
    }

    for (int i = 0; i < nsems; i ++) {
        AtomicSet(&rv->semSet[i].semval, 0);
        rv->semSet[i].semPid = 0;
        rv->semSet[i].waitN = rv->semSet[i].waitZ = {0};
        ListInit(&rv->semSet[i].waitN.list);
        ListInit(&rv->semSet[i].waitZ.list);
    }

    return rv;
}


//
// -- Get a semaphore set
//    -------------------
int __krntext SemaphoreGet(key_t key, int nsems, int semflg)
{
    if (nsems < 0 || nsems > semmsl) return -EINVAL;

    SPIN_BLOCK(semaphoreAll.globalLock) {
        int empty = -1;
        int found = (key == IPC_PRIVATE ? -2 : -1);
        int i;
        int rv;

        // -- figure out if we have an empty slot and an existing key
        for (i = 0; i < semmni && empty == -1 && found == -1; i ++) {
            if (semaphoreAll.semaphoreSets[i] == NULL) {
                if (empty == -1) empty = i;
            } else if (semaphoreAll.semaphoreSets[i]->key == key) {
                if (found == -1) found = i;
            }
        }


        rv = found;


        //
        // -- Retrieving an existing semaphore
        //    --------------------------------
        if (semflg == 0 && key != IPC_PRIVATE && found != -1) goto exit;

        //
        // -- Is this a private Semaphore or are we creating one
        //    --------------------------------------------------
        if (key == IPC_PRIVATE || (found == -1 && (semflg & IPC_CREAT))) {
            if (empty == -1) {
                rv = -ENOSPC;
                goto exit;
            }

            if (nsems == 0) {       // cannot be 0 if we are creating...
                rv = -EINVAL;
                goto exit;
            }

            SemaphoreSet_t *set = NewSemSet(key, nsems, semflg);
            semaphoreAll.semaphoreSets[empty] = set;

            if (set == NULL) rv = -ENOMEM;
            else rv = empty;

            goto exit;
        }


        //
        // -- Check if exists and IPC_CREAT and IPC_EXCL are both specified
        //    -------------------------------------------------------------
        if ((semflg & (IPC_CREAT | IPC_EXCL)) && found != -1) {
            rv = -EEXIST;
            goto exit;
        }


        //
        // -- no sem but did not specify create
        //    ---------------------------------
        if (found == -1 && (semflg & IPC_CREAT) == 0) {
            rv = -ENOENT;
            goto exit;
        }


        //
        // -- check that the existing sem set has the right size
        //    --------------------------------------------------
        if (found != -1 && semaphoreAll.semaphoreSets[found]->numSem < nsems) {
            rv = -EINVAL;
            goto exit;
        }


        //
        // -- return the results
        //    ------------------
exit:
        SPIN_RLS(semaphoreAll.globalLock);
        return rv;
    }
}


