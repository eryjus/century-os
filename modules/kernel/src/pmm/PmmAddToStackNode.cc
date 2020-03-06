//===================================================================================================================
//
//  PmmAddToStackNode.cc -- Given the stack provided, see if the frame can be added to one of the blocks
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Note that this function will not be perfect.  If we allocate 20 contiguous frames and then release all the
//  odd numbered frames and then once that is done release the even numbered frames, the result will be that
//  we have 10 blocks each with 2 blocks in it.  There is nothing in this function to determine if the frame
//  actually will connect to individual blocks and combine then into one.  As a result, there is a risk of
//  running out of blocks that could service a larger request.
//
//  To combat this, I may have the butler process occasionally look through each stack for blocks that can be
//  combined, but I leave that for a later enhancement.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-12  Initial   0.3.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "spinlock.h"
#include "lists.h"
#include "heap.h"
#include "pmm.h"


//
// -- search through the stack and see if the frame can be added to an existing block; create a new one if not.
//    ---------------------------------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
bool _PmmAddToStackNode(StackHead_t *stack, frame_t frame, size_t count)
{
    bool rv = true;     // -- assume we will be able to add it to an existing block

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(stack->lock) {
        ListHead_t::List_t *wrk = stack->list.next;
        while (wrk != &stack->list) {
            PmmBlock_t *block = FIND_PARENT(wrk, PmmBlock_t, list);

            if (frame + count == block->frame) {
                block->frame -= count;
                block->count += count;
                stack->count += count;

                CLEAN_PMM_BLOCK(block);

                goto exit;
            } else if (frame == block->frame + block->count) {
                block->count += count;
                stack->count += count;

                CLEAN_PMM_BLOCK(block);

                goto exit;
            }

            wrk = wrk->next;
        }

        rv = false;     // -- we were not able to add it; leave it to the calling function

exit:
        SPINLOCK_RLS_RESTORE_INT(stack->lock, flags);
    }

    return rv;
}

