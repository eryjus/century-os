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
//  2020-Apr-21  Initial  v0.7.0a  ADCL  Initial version
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
EXPORT KERNEL
frame_t ProcessNewUserStack(Process_t *proc, void (*startingAddr)(void))
{
    archsize_t *stack;
    frame_t rv = PmmAllocateFrame();
    assert(rv != 0);
    MmuMapToFrame(USER_STACK, rv, PG_WRT);

    stack = (archsize_t *)(USER_STACK + STACK_SIZE);


    //
    // -- This stack looks very different than the kernel stack.  I need to force an iret with a CPL change.
    //    This will need to be between ProcessStart (which has privileged instructions executed) and startingAddr
    //    (which will run in user space).  Therefore, I need to create a function to execute the iret.
    //    -------------------------------------------------------------------------------------------------------
//    *--stack = ProcessEnd;                        // -- just in case, we will self-terminate
    *--stack = 0x20|3;                              // -- ss: ring 3
    *--stack = (USER_STACK + STACK_SIZE);           // -- esp
    *--stack = (1<<9)|(1<<1)|(3<<12);               // -- flags (interrupts already enabled; keep them that way)
    *--stack = (archsize_t)0x18|3;                  // -- cs: ring 3
    *--stack = (archsize_t)startingAddr;            // -- this is the process starting point
    *--stack = (archsize_t)ProcessStartEpilogue;    // perform the iret to fulfill the privilege change (EIP)
    *--stack = (archsize_t)ProcessStart;            // -- initialize a new process
    *--stack = 0;                                   // -- ebx
    *--stack = 0;                                   // -- esi
    *--stack = 0;                                   // -- edi
    *--stack = 0;                                   // -- ebp

    kprintf("New user stack preparation complete; tos = %p!!\n", stack);

    proc->tosProcessSwap = (archsize_t)stack;

    return rv;
}
