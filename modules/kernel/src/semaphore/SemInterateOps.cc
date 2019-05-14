//===================================================================================================================
//
//  SemaphoreIterateOps.cc -- Iterate through all the operations, checking or executing each one as requested
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//
//  This functions will iterate all the operations to determine what the end result will be.  This function
//  will return one of 3 values: 1) if there is an error, < 0; 2) if everything is good and we can execute
//  the operations immediately, a 0; and 3) if things are good enough to not have an error but we need to
//  block and try again, a 1.
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
// -- Iterate all the operations
//    --------------------------
int __krntext SemIterateOps(int semid, SemaphoreSet_t *set, struct sembuf *sops, size_t nsops, int oper)
{
    for (size_t i = 0; i < nsops; i ++) {
        struct sembuf *op = &sops[i];

        if (oper == SEM_ITER_BLOCK) {
            SemBlock(&set->semSet[op->sem_num], op->sem_op == 0);
        } else if (op->sem_op < 0 && HasWritePermission(set->permissions)) {
            return SemDown(op, &set->semSet[op->sem_num], &set->permissions, semid, set->key, oper);
        } else if (op->sem_op > 0) {
            return SemUp(op, &set->semSet[op->sem_num], &set->permissions, semid, set->key, oper);
        } else {    // sem_op == 0
            return SemZero(op, &set->semSet[op->sem_num], &set->permissions, oper);
        }
    }


    //
    // -- If we get here, all checks pass and we can execute (or we did execute or block)
    //    -------------------------------------------------------------------------------
    return 0;
}


