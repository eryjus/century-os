//===================================================================================================================
//
//  SemGetAll.cc -- All all the current values of the semaphore into the structure provided
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
#include "heap.h"
#include "process.h"
#include "user-group.h"
#include "semaphore.h"


//
// -- Remove a semaphore set -- caller holds the lock on set
//    ------------------------------------------------------
int __krntext SemGetAll(SemaphoreSet_t *set, unsigned short *array)
{
    if (!IS_MAPPED(array, sizeof(unsigned short) * set->numSem)) return -EFAULT;
    if (!HasReadPermission(set->permissions)) return -EACCES;

    for (int i = 0; i < set->numSem; i ++) {
        array[i] = AtomicRead(&set->semSet[i].semval);
    }

    return 0;
}

