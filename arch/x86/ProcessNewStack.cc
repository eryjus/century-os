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
#include "process.h"


//
// -- build the stack needed to start a new process
//    ---------------------------------------------
frame_t __krntext ProcessNewStack(Process_t *proc, void (*startingAddr)(void))
{
    archsize_t *stack;
    frame_t rv = PmmAllocAlignedFrames(STACK_SIZE / FRAME_SIZE, 12);

    SPIN_BLOCK(mmuStackInitLock) {
        MmuMapToFrame(MMU_STACK_INIT_VADDR, rv, PG_KRN | PG_WRT);

        stack = (archsize_t *)(MMU_STACK_INIT_VADDR + STACK_SIZE);


//        *--stack = ProcessEnd;               // -- just in case, we will self-terminate
        *--stack = (archsize_t)startingAddr;   // -- this is the process starting point
        *--stack = (archsize_t)ProcessStart;   // -- initialize a new process
        *--stack = 0;                          // -- ebx
        *--stack = 0;                          // -- esi
        *--stack = 0;                          // -- edi
        *--stack = 0;                          // -- ebp


        MmuUnmapPage(MMU_STACK_INIT_VADDR);
        SpinlockUnlock(&mmuStackInitLock);
    }


    proc->topOfStack = ((archsize_t)stack - MMU_STACK_INIT_VADDR) + (STACK_LOCATION + STACK_SIZE * proc->pid);
    MmuMapToFrame((STACK_LOCATION + STACK_SIZE * proc->pid), rv, PG_KRN | PG_WRT);
//    kprintf("the new process stack is located at %p (frame %p)\n", (STACK_LOCATION + STACK_SIZE * proc->pid), rv);


    return rv;
}
