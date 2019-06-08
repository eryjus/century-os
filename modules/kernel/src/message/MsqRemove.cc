//===================================================================================================================
//
//  MsqRemove.cc -- Remove a message queue, waking up all the held processes that are blocked
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
#include "heap.h"
#include "process.h"
#include "user-group.h"
#include "message.h"


//
// -- Remove a message queue -- caller holds the lock on queue
//    --------------------------------------------------------
int __krntext MsqRemove(MessageQueue_t *queue, int msqid)
{
    if (!IsCreatorOwner(queue->owner, queue->creator)) return -EPERM;

    //
    // -- wake up all the processes waiting to send
    //    -----------------------------------------
    while (!IsListEmpty(&queue->sendList)) {
        MsgWaiting_t *waiting = FIND_PARENT(queue->sendList.list.next, MsgWaiting_t, list);
        Process_t *proc = waiting->proc;
        ListRemoveInit(&waiting->list);
        FREE(waiting);
        proc->pendingErrno = -EIDRM;
        ProcessReady(proc);
    }


    //
    // -- wake up all the processes waiting to receive
    //    --------------------------------------------
    while (!IsListEmpty(&queue->recvList)) {
        MsgWaiting_t *waiting = FIND_PARENT(queue->recvList.list.next, MsgWaiting_t, list);
        Process_t *proc = waiting->proc;
        ListRemoveInit(&waiting->list);
        FREE(waiting);
        proc->pendingErrno = -EIDRM;
        ProcessReady(proc);
    }


    //
    // -- Finally, free the memory
    //    ------------------------
    FREE(queue);

    return 0;
}



