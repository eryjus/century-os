//===================================================================================================================
//
//  PmmPush.cc -- Push a new node of blocks onto a stack of frames
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Pushing a node is no longer a trivial task.  So, a function is added to complete this work and maintain
//  code readability.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-12   #405    v0.6.1c  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "spinlock.h"
#include "mmu.h"
#include "pmm.h"


//
// -- Push a new node onto the stack; stack must be locked to call this function
//    --------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void PmmPush(PmmFrameInfo_t *stack, frame_t frame, size_t count)
{
//    kprintf("Pushing a node onto the stack at %p (frame: %x; size: %d)\n", stack, frame, count);

    // -- decorate the frame with the proper info to push onto the stack
    SPINLOCK_BLOCK(pmm.insertLock) {
        MmuMapToFrame((archsize_t)pmm.insert, frame, PG_KRN | PG_WRT);

        pmm.insert->frame = frame;
        pmm.insert->count = count;
        pmm.insert->prev = 0;

        if (MmuIsMapped((archsize_t)stack)) {
            pmm.insert->next = stack->frame;
            stack->prev = frame;

            MmuUnmapPage((archsize_t)stack);
        } else {
            pmm.insert->next = 0;
        }

        MmuUnmapPage((archsize_t)pmm.insert);

        SPINLOCK_RLS(pmm.insertLock);
    }


    // -- finally, push the new node
    MmuMapToFrame((archsize_t)stack, frame, PG_WRT | PG_KRN);
}


