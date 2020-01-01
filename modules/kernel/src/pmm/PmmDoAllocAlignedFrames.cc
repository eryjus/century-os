//===================================================================================================================
//
//  PmmAllocAlignedFrames.cc -- Allocate a number of frames (which could be 1 frame) and align the resulting block
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This is going to be messy.  First, we cannot lock the table while we search since the time to search is going
//  to take so long.  We would block nearly everything for an extended period of time during a complicated
//  algorithm.  So, we are going to perform some of this work before we get the lock.  To reduce the amount of
//  opportunity for conflict (although it will not eliminate it), we will work from the bottom of the stack up.
//
//  Once we find a candidate that should fit our needs, we will get the lock, double check our evaluation, and
//  remove it from the stack.
//
//  From there, it will be one of 4 scenarios:
//  1. the block is perfectly aligned and perfectly sized -- well very low proably
//  2. the block is perfectly aligned but has extra frames after it
//  3. the block is sized and aligned such that the last frames of the block are aligned and sized just right for
//     our needs
//  4. the block is quite large and has extra frames both before and after it
//
//  If this looks like the heap block splitting scenarios, it is the same.
//
//  Once we have removed it from the stack, we will trim off (as appropriate) both the starting frames and the
//  trailing frames, adding the extra frames back onto the top of the stack.
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
// -- Split the block as needed to pull out the proper alignment and size of frames
//    -----------------------------------------------------------------------------
static frame_t __krntext PmmSplitBlock(StackHead_t *stack, PmmBlock_t *block, frame_t atFrame, size_t count)
{
    if (block->frame < atFrame) {
        // -- Create a new block with the leading frames
        PmmBlock_t *blk = NEW(PmmBlock_t);
        ListInit(&blk->list);
        blk->frame = block->frame;
        blk->count = atFrame - block->frame;

        // -- adjust the existing block
        block->frame = atFrame;
        block->count -= blk->count;

        // -- finally push this block back onto the stack
        archsize_t flags = SPINLOCK_BLOCK_NO_INT(stack->lock) {
            Push(stack, &blk->list);
            stack->count += blk->count;
            SPINLOCK_RLS_RESTORE_INT(stack->lock, flags);
        }
    }


    // -- check for frames to remove at the end of this block; or free the block since it is not needed
    if (block->count > count) {
        // -- adjust this block to remove what we want
        block->frame += count;
        block->count -= count;

        // -- finally push this block back onto the stack
        archsize_t flags = SPINLOCK_BLOCK_NO_INT(stack->lock) {
            Push(stack, &block->list);
            stack->count += block->count;
            SPINLOCK_RLS_RESTORE_INT(stack->lock, flags);
        }
    } else FREE(block);


    // -- what is left at this point is `count` frames at `atFrame`; return this value
    return atFrame;
}


//
// -- This function is the working to find a frame that is properly aligned and allocate multiple contiguous frames
//    -------------------------------------------------------------------------------------------------------------
__CENTURY_FUNC__ frame_t __krntext _PmmDoAllocAlignedFrames(StackHead_t *stack, const size_t count, const size_t bitAlignment)
{
    //
    // -- start by determining the bits we cannot have enabled when we evaluate a frame
    //    -----------------------------------------------------------------------------
    frame_t frameBits = ~(((frame_t)-1) << (bitAlignment<12?0:bitAlignment-12));

    //
    // -- Now, starting from the bottom of the stack, we look for a block big enough to suit our needs
    //    --------------------------------------------------------------------------------------------
    ListHead_t::List_t *wrk = stack->list.prev;
    while (wrk != &stack->list) {
        PmmBlock_t *block = FIND_PARENT(wrk, PmmBlock_t, list);
        frame_t end = block->frame + block->count - 1;

        // -- here we determine if the block is big enough
        if (((block->frame + frameBits) & ~frameBits) + count - 1 <= end) {
            //
            // -- we now have a candidate.  At this point we need to check our work by acquiring the spinlock
            //    and double checking our work under lock conditions.
            //    -------------------------------------------------------------------------------------------
            archsize_t flags = SPINLOCK_BLOCK_NO_INT(stack->lock) {
                block = FIND_PARENT(wrk, PmmBlock_t, list);
                end = block->frame + block->count - 1;

                if (((block->frame + frameBits) & ~frameBits) + count - 1 <= end) {
                    // -- OK, we do have a good block, remove it so we can work with it
                    ListRemoveInit(wrk);
                    stack->count -= block->count;
                    CLEAN_PMM_BLOCK(block);
                    CLEAN_PMM();
                    SPINLOCK_RLS_RESTORE_INT(stack->lock, flags);
                    return PmmSplitBlock(stack, block, (block->frame + frameBits) & ~frameBits, count);
                }

                // -- it did not work out; release the lock and loop again (do we start over?)
                SPINLOCK_RLS_RESTORE_INT(stack->lock, flags);
            }
        }

        wrk = wrk->prev;
    }

    CLEAN_PMM();

    return 0;
}

