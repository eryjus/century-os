//===================================================================================================================
//
//  MsqIpcStat.cc -- Get the IPC status information for a message queue and return it in the data structure provided
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
// -- Get the current IPC Status information -- caller holds the lock on queue
//    ------------------------------------------------------------------------
int __krntext MsqIpcStat(MessageQueue_t *queue, struct msqid_ds *msqid_ds)
{
    if (!IS_MAPPED(msqid_ds, sizeof(struct msqid_ds))) return -EFAULT;
    if (!HasReadPermission(queue->permissions)) return -EACCES;

    msqid_ds->msg_perm = queue->permissions;
    msqid_ds->msg_qnum = queue->msg_count;
    msqid_ds->msg_qbytes = queue->msg_qbytes;
    msqid_ds->msg_lspid = queue->msg_lspid;
    msqid_ds->msg_lrpid = queue->msg_lrpid;
    msqid_ds->msg_stime = queue->msg_stime;
    msqid_ds->msg_rtime = queue->msg_rtime;
    msqid_ds->msg_ctime = queue->msg_ctime;

    return 0;
}


