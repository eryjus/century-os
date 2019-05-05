//===================================================================================================================
//
//  SemIpcStat.cc -- Get the IPC status information for a semaphore and return it in the data structure provided
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
#include "user-group.h"
#include "semaphore.h"


//
// -- Get the current IPC Status information -- caller holds the lock on set
//    ----------------------------------------------------------------------
int __krntext SemIpcStat(SemaphoreSet_t *set, struct semid_ds *buf)
{
    if (!IS_MAPPED(buf, sizeof(struct semid_ds))) return -EFAULT;
    if (!HasReadPermission(set->permissions)) return -EACCES;

    buf->sem_perm = set->permissions;
    buf->sem_nsems = set->numSem;
    buf->sem_otime = set->semOtime;
    buf->sem_ctime = set->semCtime;

    return 0;
}


