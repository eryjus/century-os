//===================================================================================================================
//
//  MessageControl.cc -- The implementation of the POSIX `msgctl` API -- control message queue
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
#include "lists.h"
#include "process.h"
#include "heap.h"
#include "spinlock.h"
#include "message.h"


//
// -- Message Control API
//    -------------------
int __krntext MessageControl(int msqid, int cmd, struct msqid_ds *msqid_ds)
{
    int rv = -EUNDEF;       // -- should never return, but just in case

    if (msqid < 0 || msqid >= msgmni) return -EINVAL;

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(messageAll.globalLock) {
        MessageQueue_t *queue = messageAll.queues[msqid];
        if (queue == NULL) {
            SPINLOCK_RLS_RESTORE_INT(messageAll.globalLock, flags);
            return -EINVAL;
        }

        switch(cmd) {
        case IPC_STAT:      // -- get the current message queue settings
            SPINLOCK_BLOCK(queue->lock) {
                SPINLOCK_RLS(messageAll.globalLock);          // release the bigger lock when we have a smaller one
                rv = MsqIpcStat(queue, msqid_ds);
                SPINLOCK_RLS_RESTORE_INT(queue->lock, flags);
            }

            return rv;

        case IPC_SET:       // -- set some new message queue settings
            SPINLOCK_BLOCK(queue->lock) {
                SPINLOCK_RLS(messageAll.globalLock);          // release the bigger lock when we have a smaller one
                rv = MsqIpcSet(queue, msqid_ds);
                SPINLOCK_RLS_RESTORE_INT(queue->lock, flags);
            }

            return rv;

        case IPC_RMID:      // -- remove the message queue
            SPINLOCK_BLOCK(queue->lock);      // use this to wait for all other locks to release
            messageAll.queues[msqid] = NULL;
            SPINLOCK_RLS(queue->lock);
            SPINLOCK_RLS_RESTORE_INT(messageAll.globalLock, flags);      // no one else can find this sem set now...
            return MsqRemove(queue, msqid);

        default:
            SPINLOCK_RLS_RESTORE_INT(messageAll.globalLock, flags);
            return -EINVAL;
        }
    }

    return rv;         // -- should never get here, but just in case
}

