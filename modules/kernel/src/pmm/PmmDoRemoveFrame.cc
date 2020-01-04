//===================================================================================================================
//
//  PmmDoRemoveFrame.cc -- This is a worker function to remove a frame from a given list and optionally clear it
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-11  Initial   0.3.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "spinlock.h"
#include "heap.h"
#include "pmm.h"


//
// -- Given the stack, remove a frame from the top of the stack
//
//    TODO: This may result in a deadlock if called from the heap; this needs to be fixed with a timeout
//    See: http://eryjus.ddns.net:3000/issues/405 for more details.
//    --------------------------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
frame_t _PmmDoRemoveFrame(StackHead_t *stack, bool scrub)
{
    frame_t rv = 0;         // assume we will not find anything
    PmmBlock_t *block;

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(stack->lock) {
        if (!IsListEmpty(stack)) {
            block = FIND_PARENT(stack->list.next, PmmBlock_t, list);
            rv = block->frame;
            block->frame ++;
            block->count --;
            stack->count --;

            // -- if we have emptied the block, free the structure
            if (block->count == 0) {
                ListRemoveInit(&block->list);
                FREE(block);
            }

            CLEAN_PMM_BLOCK(block);

            // -- scrub the frame if requested
            if (scrub) PmmScrubFrame(rv);
        }

        SPINLOCK_RLS_RESTORE_INT(stack->lock, flags);
    }

    CLEAN_PMM();
    return rv;
}
