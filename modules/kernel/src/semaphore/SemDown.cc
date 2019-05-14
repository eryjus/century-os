//===================================================================================================================
//
//  SemDown.cc -- Decrease the value of the semaphore
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
// -- Handle a semaphore decrease
//    ---------------------------
int __krntext SemDown(struct sembuf *op, Semaphore_t *sem, struct ipc_perm *perm, int semid, key_t key, int oper)
{
    if (HasWritePermission(perm)) {
        int val = AtomicRead(&sem->semval);
//        kprintf(".... Down: val is %x\n", val);
        if (val >= -op->sem_op) {
//            kprintf("...... ok\n");
            if (oper == SEM_ITER_EXEC) {
                AtomicAdd(&sem->semval, op->sem_op);
                if ((op->sem_flg & SEM_UNDO) != 0) SemCreateUndo(semid, key, op->sem_num, -op->sem_op);
                if (val + op->sem_op == 0) SemReadyWaiting(&sem->waitZ);
                return 0;
            } else return 0;
        } else if (val < ABS(op->sem_op) && (op->sem_flg & IPC_NOWAIT) != 0) {
//            kprintf("...... no wait\n");
            return -EAGAIN;
        } else if (val < ABS(op->sem_op) && (op->sem_flg & IPC_NOWAIT) == 0) {
//            kprintf("...... wait\n");
            return 1;
        }
    } else return -EACCES;
    return -EUNDEF;     // should never get here; make the compiler happy
}


