//===================================================================================================================
//
//  ButlerCleanProcess.cc -- Clean up a terminated process, with all the required cleanup
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-14  Initial  v0.6.1d  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "pmm.h"
#include "process.h"
#include "heap.h"
#include "butler.h"


//
// -- The Butler has been notified of a Process to clean
//    --------------------------------------------------
void ButlerCleanProcess(void)
{
//    kprintf("Starting to clean a process up\n");
    Process_t *dlt = NULL;

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(schedulerLock) {
        if (!IsListEmpty(&scheduler.listTerminated)) {
            dlt = FIND_PARENT(scheduler.listTerminated.list.next, Process_t, stsQueue);
            ListRemoveInit(&dlt->stsQueue);
            ListRemoveInit(&dlt->globalList);
        }

        SPINLOCK_RLS_RESTORE_INT(schedulerLock, flags);
    }

    if (!dlt) return;


    // -- from here, there is a process to clean up; since it is not on any queue, we own the structure
    //    we are starting by cleaning up all references
    while (!IsListEmpty(&dlt->references)) {
        Reference_t *ref = FIND_PARENT(dlt->references.list.next, Reference_t, procRefList);
        ListRemoveInit(&ref->procRefList);

        // -- now, we can remove the reference from the resource once we know what it is
        switch (ref->type) {
        case REF_MSGQ: {
            MessageQueue_t *msgq = (MessageQueue_t *)ref->resAddr;
            archsize_t flags = SPINLOCK_BLOCK_NO_INT(msgq->procList.lock) {
                ListRemoveInit(&ref->resourceRefBy);
                msgq->procList.count --;
                SPINLOCK_RLS_RESTORE_INT(msgq->procList.lock, flags);
            }

            FREE(ref);

            break;
        }

        default:
            assert_msg(false, "Unsupported reference type to clean up");
            break;
        }
    }


    // -- resources are released; work on cleaning up virtual memory

    // -- TODO: Implement this


    // -- Clean up the stack
    if (dlt->topOfStack > STACK_BASE) {
        PmmReleaseFrame(MmuUnmapPage(dlt->topOfStack & ~(STACK_SIZE - 1)));
    }

    // -- Finally, free up the process memory
    FREE(dlt);
}
