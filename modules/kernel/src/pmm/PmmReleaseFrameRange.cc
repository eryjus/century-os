//===================================================================================================================
//
//  PmmReleaseFrameRange.cc -- Release a frame and place it in the scrub queue
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
// -- Add the frames to the scrub queue
//    ---------------------------------
__CENTURY_FUNC__ void __krntext PmmReleaseFrameRange(const frame_t frame, const size_t count)
{
    PmmBlock_t *block = NEW(PmmBlock_t);        // this may deadlock and will be addressed in PmmAllocateFrame()

    if (!block) {
        kprintf("PANIC: unable to allocate memory for freeing a frame\n");
        Halt();
    }

    ListInit(&block->list);
    block->frame = frame;
    block->count = count;

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(pmm.scrubStack.lock) {
        Push(&pmm.scrubStack, &block->list);
        pmm.scrubStack.count += block->count;

        CLEAN_PMM();

        SPINLOCK_RLS_RESTORE_INT(pmm.scrubStack.lock, flags);
    }

    CLEAN_PMM_BLOCK(block);
}
