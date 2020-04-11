//===================================================================================================================
//
//  MsgqSReceive.cc -- Receive a message from a queue, blocking is allowed
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
// -- Receive a message from a message queue, optionally blocking
//    -----------------------------------------------------------
EXTERN_C EXPORT KERNEL
bool MessageQueueReceive(MessageQueue_t *msgq, long *type, size_t sz, void *payload, bool block)
{
    bool wait = true;           // -- assume we are going to wait
    archsize_t flags;

    do {
        // -- sequence is important, get the lock and then check if we have something
        flags = SPINLOCK_BLOCK_NO_INT(msgq->queue.lock);

        if (IsListEmpty(&msgq->queue)) {
            SPINLOCK_RLS_RESTORE_INT(msgq->queue.lock, flags);

            if (block) {
                flags = SPINLOCK_BLOCK_NO_INT(msgq->waiting.lock) {
                    Enqueue(&msgq->waiting, &currentThread->stsQueue);
                    msgq->waiting.count ++;
                    SPINLOCK_RLS_RESTORE_INT(msgq->waiting.lock, flags);
                }

                ProcessBlock(PROC_MSGW);
            } else return false;
        } else {
            // -- we still hold the lock!!
            wait = false;
        }
    } while (wait);


    // -- at this point, we have the lock on the queue and we know we have something to pull
    Message_t *msg = FIND_PARENT(msgq->queue.list.next, Message_t, list);
    ListRemoveInit(&msg->list);
    msgq->queue.count --;

    // -- there is no need for the lock anymore
    SPINLOCK_RLS_RESTORE_INT(msgq->queue.lock, flags);

    *type = msg->payload.type;
    sz = (sz < msg->payloadSize?sz:msg->payloadSize);

    if (sz) kMemMove(payload, msg->payload.data, sz);
    FREE(msg);

    return true;
}

