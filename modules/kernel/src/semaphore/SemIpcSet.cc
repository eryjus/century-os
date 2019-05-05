//===================================================================================================================
//
//  SemIpcSet.cc -- Set the IPC status information for the semaphore
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
// -- Set the IPC Status information -- caller holds the lock on set
//    --------------------------------------------------------------
int __krntext SemIpcSet(SemaphoreSet_t *set, struct semid_ds *buf)
{
    if (!IS_MAPPED(buf, sizeof(struct semid_ds))) return -EFAULT;
    if (!IsCreatorOwner(set->owner, set->creator)) return -EPERM;

    set->permissions.uid = buf->sem_perm.uid;
    set->permissions.gid = buf->sem_perm.gid;
    set->permissions.mode &= ~0777;     // clear previous permissions
    set->permissions.mode |= (buf->sem_perm.mode & 0777);
    set->semCtime = 0;                  // TODO: set this to the current time

    return 0;
}


