//===================================================================================================================
//
//  MsqIpcSet.cc -- Set the IPC status information for the message queue
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
//  2019-May-16  Initial   0.4.4   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "errno.h"
#include "mmu.h"
#include "user-group.h"
#include "message.h"


//
// -- Set the IPC Status information -- caller holds the lock on queue
//    ----------------------------------------------------------------
int __krntext MsqIpcSet(MessageQueue_t *queue, struct msqid_ds *msqid_ds)
{
    if (!IS_MAPPED(msqid_ds, sizeof(struct msqid_ds))) return -EFAULT;
    if (!IsCreatorOwner(queue->owner, queue->creator)) return -EPERM;

    queue->permissions.uid = msqid_ds->msg_perm.uid;
    queue->permissions.gid = msqid_ds->msg_perm.gid;
    queue->permissions.mode &= ~0777;     // clear previous permissions
    queue->permissions.mode |= (msqid_ds->msg_perm.mode & 0777);
    queue->msg_qbytes = msqid_ds->msg_qbytes;
    queue->msg_ctime = 0;                  // TODO: set this to the current time

    return 0;
}


