//===================================================================================================================
//
//  MsgqCreate.cc -- Find and create a new message queue, returning its pointer
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
#include "cpu.h"
#include "spinlock.h"
#include "msgq.h"


//
// -- Create a message queue
//    ----------------------
EXTERN_C EXPORT KERNEL
MessageQueue_t *MessageQueueCreate(void)
{
    archsize_t flags;

    // -- First create the message Queue
    MessageQueue_t *rv = NEW(MessageQueue_t);
    assert(rv != NULL);

    AtomicSet(&rv->status, MSGQ_INITIALIZING);
    ListInit(&rv->list);
    ListInit(&rv->queue.list);
    rv->queue.count = 0;
    rv->queue.lock = {0};
    ListInit(&rv->procList.list);
    rv->procList.count = 0;
    rv->procList.lock = {0};
    ListInit(&rv->waiting.list);
    rv->waiting.count = 0;
    rv->waiting.lock = {0};

    // -- With that done, add the reference to the queue and to the Process
    Reference_t *ref = NEW(Reference_t);
    assert(ref != NULL);
    ref->type = REF_MSGQ;
    ListInit(&ref->procRefList);
    ListInit(&ref->resourceRefBy);
    ref->process = currentThread;
    ref->resAddr = rv;


    flags = SPINLOCK_BLOCK_NO_INT(currentThread->references.lock) {
        ListAddTail(&currentThread->references, &ref->procRefList);
        SPINLOCK_RLS_RESTORE_INT(currentThread->references.lock, flags);
    }

    flags = SPINLOCK_BLOCK_NO_INT(rv->procList.lock) {
        ListAddTail(&rv->procList, &ref->resourceRefBy);
        SPINLOCK_RLS_RESTORE_INT(rv->procList.lock, flags);
    }

    // -- finally, we can add it to the msgq list and return its value
    flags = SPINLOCK_BLOCK_NO_INT(msgqList.lock) {
        ListAddTail(&msgqList, &rv->list);
        msgqList.count ++;
        AtomicSet(&rv->status, MSGQ_ALLOCATED);
        SPINLOCK_RLS_RESTORE_INT(msgqList.lock, flags);
    }


    return rv;
}


