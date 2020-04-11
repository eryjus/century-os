//===================================================================================================================
//
//  MsgqSend.cc -- Send a message to a queue and wake up all waiting processes
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  --------------------------------------------------------------------------
//  2020-Apr-09  Initial  v0.6.1a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "heap.h"
#include "process.h"
#include "msgq.h"


//
// -- Send a message to a message queue (all pre-checks completed)
//    ------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void MessageQueueSend(MessageQueue_t *msgq, long type, size_t sz, void *payload)
{
    // -- construct the message
    size_t size = sz + sizeof(Message_t);                // -- adjust for the overhead
    Message_t *msg = (Message_t *)HeapAlloc(size, false);

    ListInit(&msg->list);
    msg->payloadSize = sz + sizeof(long);
    msg->payload.type = type;
    if (sz) kMemMove(msg->payload.data, payload, sz);

    // -- queue the message
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(msgq->queue.lock) {
        Enqueue(&msgq->queue, &msg->list);
        msgq->queue.count ++;

        SPINLOCK_RLS_RESTORE_INT(msgq->queue.lock, flags);
    }


    // -- release anything waiting for something in the queue and let the scheduler sort it all out
    ProcessLockAndPostpone();

    flags = SPINLOCK_BLOCK_NO_INT(msgq->waiting.lock) {

        while (IsListEmpty(&msgq->waiting) == false) {
            Process_t *proc = FIND_PARENT(msgq->waiting.list.next, Process_t, stsQueue);
            ListRemoveInit(&proc->stsQueue);
            msgq->waiting.count --;
            ProcessDoReady(proc);
        }

        SPINLOCK_RLS_RESTORE_INT(msgq->waiting.lock, flags);
    }

    ProcessUnlockAndSchedule();
}
