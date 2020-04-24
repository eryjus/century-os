//===================================================================================================================
//
// ProcessNewUserStack.cc -- for a new user process, create its stack so we can return from ProcessSwitch()
//
//        Copyright (c)  2017-2020 -- Adam Clark
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
// -- build the stack needed to start a new user process
//    --------------------------------------------------
EXTERN_C EXPORT KERNEL
frame_t ProcessNewUserStack(Process_t *proc, void (*startingAddr)(void))
{
    archsize_t *stack;
    frame_t rv = PmmAllocAlignedFrames(STACK_SIZE / FRAME_SIZE, 12);
    MmuMapToFrame(USER_STACK, rv, PG_WRT);

    stack = (archsize_t *)(USER_STACK + STACK_SIZE);

    //
    // -- OK, these 2 are backwards.  See arm `ProcessSwitch()` for more info.  I need to restore `startingAddr()`
    //    into `lr` before calling `ProcessStart()` so that I get returns in the right order.
    //    --------------------------------------------------------------------------------------------------------
    *--stack = (archsize_t)0x10;                    // -- cpsr
    *--stack = (archsize_t)startingAddr;            // -- final IP
    *--stack = (archsize_t)ProcessStart;            // -- initialize a new process (pops into pc)
    *--stack = (archsize_t)ProcessStartEpilogue;    // -- this is the process starting point (pops into lr)
    *--stack = 0;                                   // -- ip
    *--stack = 0;                                   // -- r11
    *--stack = 0;                                   // -- r10
    *--stack = 0;                                   // -- r9
    *--stack = 0;                                   // -- r8
    *--stack = 0;                                   // -- r7
    *--stack = 0;                                   // -- r6
    *--stack = 0;                                   // -- r5
    *--stack = 0;                                   // -- r4



    proc->tosProcessSwap = (archsize_t)stack;

    return rv;
}

