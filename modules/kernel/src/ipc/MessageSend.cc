//===================================================================================================================
//
// MessageSend.cc -- Send a message to a process by PID
//
// Send a message to a PID, copying the data and payload into kernel heap
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
#include "errors.h"
#include "ipc.h"


//
// -- Send a message to a PID
//    -----------------------
Errors_t MessageSend(PID_t pid, Message_t *m)
{
    if (!m) {
        ERROR_00000002("MessageSend()");
        return ERR_00000002;
    }

    Process_t *proc;
    Message_t *msg = NEW(Message_t);
    void *payload = (m->payloadSize?HeapAlloc(m->payloadSize, false):NULL);

    if (!msg || !payload) {
        if (msg) HeapFree(msg);
        if (payload) HeapFree(payload);

        ERROR_90000001("MessageSend()");
        return ERR_90000001;
    }

    ListInit(&msg->list);
    msg->msg = m->msg;
    msg->parm1 = m->parm1;
    msg->parm2 = m->parm2;
    msg->payloadSize = m->payloadSize;
    if (m->payloadSize) kMemMove(payload, m->dataPayload, m->payloadSize);
    msg->dataPayload = payload;

    SANITY_CHECK_PID(pid, proc);
    SPIN_BLOCK(proc->lock) {
        ListAddTail(&proc->messages, &msg->list);
        SpinlockUnlock(&proc->lock);
    }

    if (proc->status == PROC_MSGW) ProcessReady(pid);

    return SUCCESS;
}
