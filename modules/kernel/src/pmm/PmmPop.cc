//===================================================================================================================
//
//  PmmPop.cc -- Pop a node of blocks off a stack of frames
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Popping a node is no longer a trivial task.  So, a function is added to complete this work and maintain
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
// -- Pop a node off the stack; stack must be locked to call this function
//    --------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void PmmPop(PmmFrameInfo_t *stack)
{
//    kprintf("Popping a node off the stack at %p\n", stack);

    if (!MmuIsMapped((archsize_t)stack)) return;

    frame_t nx = stack->next;

    // -- clear out the data elements! -- prev is already 0
    stack->count = 0;
    stack->frame = 0;
    stack->next = 0;

    MmuUnmapPage((archsize_t)stack);

    if (nx) {
        MmuMapToFrame((archsize_t)stack, nx, PG_KRN | PG_WRT);
        stack->prev = 0;
    }
}


