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
//  This algorithm is inefficient since it must look through the 'small frame blocks' at the top of the stack
//  before it gets to the bigger 'large frame blocks' at the bottom of the stack.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-11  Initial   0.3.1   ADCL  Initial version
//  2020-Apr-12   #405    v0.6.1c  ADCL  Redesign the PMM to store the stack in the freed frames themselves
//
//===================================================================================================================


#include "types.h"
#include "spinlock.h"
#include "heap.h"
#include "pmm.h"


//
// -- Split the block as needed to pull out the proper alignment and size of frames
//    -----------------------------------------------------------------------------
EXTERN_C HIDDEN KERNEL
frame_t PmmSplitBlock(PmmFrameInfo_t *stack, frame_t frame, size_t blockSize, frame_t atFrame, size_t count)
{
    if (frame < atFrame) {
        // -- Create a new block with the leading frames
        PmmPush(stack, frame, atFrame - blockSize);

        // -- adjust the existing block
        frame = atFrame;
        blockSize -= (atFrame - blockSize);
    }


    // -- check for frames to remove at the end of this block; or free the block since it is not needed
    if (blockSize > count) {
        // -- adjust this block to remove what we want
        frame += count;
        blockSize -= count;

        // -- finally push this block back onto the stack
        PmmPush(stack, frame, blockSize);
    }


    // -- what is left at this point is `count` frames at `atFrame`; return this value
    return atFrame;
}


//
// -- This function is the working to find a frame that is properly aligned and allocate multiple contiguous frames
//    -------------------------------------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
frame_t PmmDoAllocAlignedFrames(Spinlock_t *lock, PmmFrameInfo_t *stack, const size_t count, const size_t bitAlignment)
{
    //
    // -- start by determining the bits we cannot have enabled when we evaluate a frame
    //    -----------------------------------------------------------------------------
    frame_t frameBits = ~(((frame_t)-1) << (bitAlignment<12?0:bitAlignment-12));
    frame_t rv = 0;

    if (!MmuIsMapped((archsize_t)stack)) return 0;


    archsize_t flags = SPINLOCK_BLOCK_NO_INT(*lock) {
        SPINLOCK_BLOCK(pmm.searchLock) {
            MmuMapToFrame((archsize_t)pmm.search, stack->frame, PG_WRT | PG_KRN);

            while(true) {
                frame_t end = pmm.search->frame + pmm.search->count - 1;
                frame_t next;

                // -- here we determine if the block is big enough
                if (((pmm.search->frame + frameBits) & ~frameBits) + count - 1 <= end) {
                    frame_t p = pmm.search->prev;
                    frame_t n = pmm.search->next;
                    frame_t f = pmm.search->frame;
                    size_t sz = pmm.search->count;

                    MmuUnmapPage((archsize_t)pmm.search);

                    if (n) {
                        MmuMapToFrame((archsize_t)pmm.search, n, PG_WRT | PG_KRN);
                        pmm.search->prev = p;
                        MmuUnmapPage((archsize_t)pmm.search);
                    }

                    if (p) {
                        MmuMapToFrame((archsize_t)pmm.search, p, PG_WRT | PG_KRN);
                        pmm.search->next = n;
                        MmuUnmapPage((archsize_t)pmm.search);
                    }

                    rv = PmmSplitBlock(stack, f, sz, (f + frameBits) & ~frameBits, count);
                    goto exit;
                }

                // -- move to the next node
                next = pmm.search->next;
                MmuUnmapPage((archsize_t)pmm.search);

                // -- here we check if we made it to the end of the stack
                if (next) MmuMapToFrame((archsize_t)pmm.search, next, PG_WRT | PG_KRN);
                else goto exit;
            }

exit:
            SPINLOCK_RLS(pmm.searchLock);
        }

        SPINLOCK_RLS_RESTORE_INT(*lock, flags);
    }

    return rv;
}

