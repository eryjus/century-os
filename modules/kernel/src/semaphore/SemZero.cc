//===================================================================================================================
//
//  SemZero.cc -- Handle the situation where the operation on a semaphore is 0.
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-May-10  Initial   0.4.3   ADCL  Initial version
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
// -- Handle the operation 0 on a semaphore
//    -------------------------------------
int __krntext SemZero(struct sembuf *op, Semaphore_t *sem, struct ipc_perm *perm, int oper)
{
    if (HasReadPermission(perm)) {
        int val = AtomicRead(&sem->semval);

        if ((val != 0) && ((op->sem_flg & IPC_NOWAIT) != 0)) {
            return -EAGAIN;
        } else if ((val != 0) && ((op->sem_flg & IPC_NOWAIT) == 0)) {
            if (oper == SEM_ITER_EXEC) return -EAGAIN;
            else return 1;
        } else {    // val == 0
            return 0;
        }
    } else return -EACCES;
}


