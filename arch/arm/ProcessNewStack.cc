//===================================================================================================================
//
// ProcessNewStack.cc -- for a new process, create its stack so we can return from ProcessSwitch()
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-16  Initial   0.3.2   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "heap.h"
#include "mmu.h"
#include "pmm.h"
#include "stacks.h"
#include "process.h"


//
// -- build the stack needed to start a new process
//    ---------------------------------------------
EXPORT KERNEL
frame_t ProcessNewStack(Process_t *proc, void (*startingAddr)(void))
{
    archsize_t *stack;
    frame_t rv = PmmAllocAlignedFrames(STACK_SIZE / FRAME_SIZE, 12);

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(mmuStackInitLock) {
        MmuMapToFrame(MMU_STACK_INIT_VADDR, rv, PG_KRN | PG_WRT);

        stack = (archsize_t *)(MMU_STACK_INIT_VADDR + STACK_SIZE);

//        *--stack = ProcessEnd;               // -- just in case, we will self-terminate

        //
        // -- OK, these 2 are backwards.  See arm `ProcessSwitch()` for more info.  I need to restore `startingAddr()`
        //    into `lr` before calling `ProcessStart()` so that I get returns in the right order.
        //    --------------------------------------------------------------------------------------------------------
        *--stack = (archsize_t)ProcessStart;   // -- initialize a new process
        *--stack = (archsize_t)startingAddr;   // -- this is the process starting point
        *--stack = 0;                          // -- ip
        *--stack = 0;                          // -- r11
        *--stack = 0;                          // -- r10
        *--stack = 0;                          // -- r9
        *--stack = 0;                          // -- r8
        *--stack = 0;                          // -- r7
        *--stack = 0;                          // -- r6
        *--stack = 0;                          // -- r5
        *--stack = 0;                          // -- r4


        MmuUnmapPage(MMU_STACK_INIT_VADDR);
        SPINLOCK_RLS_RESTORE_INT(mmuStackInitLock, flags);
    }

    archsize_t stackLoc = StackFind();    // get a new stack
    assert(stackLoc != 0);
    proc->topOfStack = ((archsize_t)stack - MMU_STACK_INIT_VADDR) + stackLoc;
    MmuMapToFrame(stackLoc, rv, PG_KRN | PG_WRT);
    kprintf("the new process stack is located at %p (frame %p)\n", stackLoc, rv);


    return rv;
}

