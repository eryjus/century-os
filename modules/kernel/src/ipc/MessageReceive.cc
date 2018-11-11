//===================================================================================================================
//
//  MessageReceive.cc -- Receive a message, blocking if there is not one to received
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-27  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "spinlock.h"
#include "heap.h"
#include "ipc.h"

#include <errno.h>


//
// -- Receive a message
//    -----------------
int MessageReceive(Message_t *m)
{
    if (!m) {
        return -EUNDEF;
    }

    Process_t *proc = procs[currentPID];
    Message_t *msg;
    void *prevPayload = NULL;

    while (!MessageWaiting()) {
        ProcessWait(PROC_MSGW);
    }

    SANITY_CHECK_PID(currentPID, proc);
    SPIN_BLOCK(proc->lock) {
        msg = FIND_PARENT(proc->messages.list.next, Message_t, list);
        ListRemoveInit(&msg->list);
        if (proc->prevPayload) prevPayload = proc->prevPayload;
        proc->prevPayload = msg->dataPayload;
        SpinlockUnlock(&proc->lock);
    }

    // -- free a previous payload
    if (prevPayload) HeapFree(prevPayload);

    m->msg = msg->msg;
    m->parm1 = msg->parm1;
    m->parm2 = msg->parm2;

    if (m->dataPayload) {
        kMemMove(m->dataPayload, msg->dataPayload, (m->payloadSize>msg->payloadSize?m->payloadSize:msg->payloadSize));
    }

    m->payloadSize = msg->payloadSize;
    HeapFree(msg);

    return SUCCESS;
}
