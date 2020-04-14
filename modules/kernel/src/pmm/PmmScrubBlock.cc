//===================================================================================================================
//
//  PmmScruber.cc -- Scrub the frames in the scrubStack
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-12  Initial   0.3.1   ADCL  Initial version
//  2020-Apr-12   #405    v0.6.1c  ADCL  Redesign the PMM to store the stack in the freed frames themselves
//
//===================================================================================================================


#include "types.h"
#include "spinlock.h"
#include "lists.h"
#include "heap.h"
#include "pmm.h"


//
// -- Clean a block of physical memory of all its data
//    ------------------------------------------------
EXTERN_C EXPORT KERNEL
void PmmScrubBlock(void)
{
    // -- quickly check if there is something to do; we will redo the check when we get the lock
    if (!MmuIsMapped((archsize_t)pmm.scrubStack)) return;

    frame_t frame = 0;
    size_t count = 0;

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(pmm.scrubLock) {
        // -- double check just in case something changed
        if (MmuIsMapped((archsize_t)pmm.scrubStack)) {
            frame = pmm.scrubStack->frame;
            count = pmm.scrubStack->count;

            AtomicSub(&pmm.framesAvail, count);

            PmmPop(pmm.scrubStack);
        }

        SPINLOCK_RLS_RESTORE_INT(pmm.scrubLock, flags);
    }


    // -- if we found nothing to do, return
    if (frame == 0) return;

    // -- here we scrub the frames in the block
    for (size_t i = 0; i < count; i ++) PmmScrubFrame(frame + i);


    // -- see if we can add it to an existing block
    if (frame < 0x100) {
        PmmAddToStackNode(&pmm.lowLock, pmm.lowStack, frame, count);
    } else {
        PmmAddToStackNode(&pmm.normLock, pmm.normStack, frame, count);
    }
}



