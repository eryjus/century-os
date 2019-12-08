//===================================================================================================================
//
//  PmmScruber.cc -- Scrub the frames in the scrubStack
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
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


__CENTURY_FUNC__ void __krntext PmmScrubBlock(void)
{
    // -- quickly check if there is something to do; we will redo the check when we get the lock
    if (IsListEmpty(&pmm.scrubStack)) return;

    kprintf("Preparing to scrub a frame\n");
    PmmBlock_t *block = NULL;
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(pmm.scrubStack.lock) {
        // -- double check just in case something changed
        if (!IsListEmpty(&pmm.scrubStack)) {
            ListHead_t::List_t *list = pmm.scrubStack.list.next;
            ListRemoveInit(list);
            block = FIND_PARENT(list, PmmBlock_t, list);
            pmm.scrubStack.count -= block->count;
            CLEAN_PMM();
        }

        SPINLOCK_RLS_RESTORE_INT(pmm.scrubStack.lock, flags);

        if (block) CLEAN_PMM_BLOCK(block);
    }


    // -- if we found nothing to do, return
    if (block == NULL) return;


    kprintf("Found something to do for real; the block is at address %p\n", block);
    kprintf(".. The block starts at frame %x and has %x frames\n", block->frame, block->count);

    // -- here we scrub the frames in the block
    for (size_t i = 0; i < block->count; i ++) PmmScrubFrame(block->frame + i);


    // -- now, add it to the right stack
    StackHead_t *stack;
    if (block->frame < 0x100) stack = &pmm.lowStack;
    else stack = &pmm.normalStack;

    if (_PmmAddToStackNode(stack, block->frame, block->count)) FREE(block);
    else {
        kprintf("Pushing the block onto the stack\n");
        archsize_t flags = SPINLOCK_BLOCK_NO_INT(stack->lock) {
            stack->count += block->count;
            Push(stack, &block->list);
            CLEAN_PMM();
            SPINLOCK_RLS_RESTORE_INT(stack->lock, flags);
        }
    }

    CLEAN_PMM_BLOCK(block);
}