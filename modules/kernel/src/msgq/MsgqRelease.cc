//===================================================================================================================
//
//  MsgqRelease.cc -- Remove the reference to this message queue
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This is going to be a little tricky since we first need to prove that this process contains a reference to the
//  Queue.
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
#include "lists.h"
#include "msgq.h"


//
// -- Release the reference to this message queue; marking for deletion when the reference count is 0
//    -----------------------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void MessageQueueRelease(MessageQueue_t *msgq)
{
    Reference_t *ref = NULL;

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(currentThread->references.lock) {
        ListHead_t::List_t *wrk = currentThread->references.list.next;
        while (wrk != &currentThread->references.list) {
            Reference_t *r = FIND_PARENT(wrk, Reference_t, procRefList);
            if (r->resAddr == msgq) {
                ListRemoveInit(&r->procRefList);
                currentThread->references.count --;
                ref = r;
                goto exit;
            }


            wrk = wrk->next;
        }

exit:
        SPINLOCK_RLS_RESTORE_INT(currentThread->references.lock, flags);
    }

    // -- if we did not find anything, we're done
    if (!ref) return;


    // -- now, we need to clean up the Message Queue
    flags = SPINLOCK_BLOCK_NO_INT(msgq->procList.lock) {
        ListRemoveInit(&ref->procRefList);
        msgq->procList.count --;
        SPINLOCK_RLS_RESTORE_INT(msgq->procList.lock, flags);
    }

    FREE(ref);
}
