//===================================================================================================================
//
//  PmmAddToStackNode.cc -- Given the stack provided, see if the frame can be added to one of the blocks
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The problem with this function is that a newly freed frame or block of frames may be adjascent to up to 2
//  other frame blocks that are already in the stack.  To deal with this, the best thing to do would be to traverse
//  the stack and check for both conditions.  However, the challenge with that is twofold:
//  1) the implementation requires the TLB to be flushed on all CPUs for each mapping change, which is expensive
//  2) this function is called from the butler, which is running at a low priority and will have a lock for an
//     extended period of time.
//
//  The previous implementation struggled with the same problem, and I had decided to leave this work to the butler
//  to clean up.  It the moment, I will do the same thing.
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
// -- Search through the stack and see if the frame can be added to an existing block; create a new one if not.
//    ---------------------------------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void PmmAddToStackNode(Spinlock_t *lock, PmmFrameInfo_t *stack, frame_t frame, size_t count)
{
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(*lock) {
        PmmPush(stack, frame, count);
        SPINLOCK_RLS_RESTORE_INT(*lock, flags);
    }
}

