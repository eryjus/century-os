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
//  2020-Apr-12   #405    v0.6.1c  ADCL  Redesign the PMM to store the stack in the freed frames themselves
//
//===================================================================================================================


#include "types.h"
#include "spinlock.h"
#include "heap.h"
#include "pmm.h"


//
// -- Given the stack, remove a frame from the top of the stack (lock MUST be held to call)
//    -------------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
frame_t _PmmDoRemoveFrame(PmmFrameInfo_t *stack, bool scrub)
{
    frame_t rv = 0;         // assume we will not find anything

    if (MmuIsMapped((archsize_t)stack)) {
        rv = stack->frame + stack->count - 1;
        stack->count --;
        AtomicDec(&pmm.framesAvail);

        if (stack->count == 0) {
            PmmPop(stack);
        }
    } else {
        return 0;
    }


    // -- scrub the frame if requested
    if (scrub) PmmScrubFrame(rv);

    return rv;
}
