//===================================================================================================================
//
//  PmmAllocateFrame.cc -- Allocate a normal frame (not low memory required)
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  So the algorithm here is relatively simple.  Under most circumstances, we will allocate from the
//  normal stack of free frames.  However, if that is empty, then we will check the scrub queue for
//  something that we can clean up ourselves.  Finally if that is empty, we will go to the low
//  memory stack.  Finally, if there is nothing there, we will report we are out of memory.
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
// -- Allocate a frame and return it
//    ------------------------------
EXTERN_C EXPORT KERNEL
frame_t PmmAllocateFrame(void)
{
    if (unlikely(!pmmInitialized)) {
//        kprintf("The PMM is not yet initialized; returning an early frame\n");
        return (NextEarlyFrame() >> 12);
    }

//    kprintf("Allcoating a PMM frame\n");

    frame_t rv = 0;         // assume we will not find anything
    archsize_t flags;


    //
    // -- check the normal stack for a frame to allocate
    //    ----------------------------------------------
    flags = SPINLOCK_BLOCK_NO_INT(pmm.normLock) {
        rv = _PmmDoRemoveFrame(pmm.normStack, false);
        SPINLOCK_RLS_RESTORE_INT(pmm.normLock, flags);
    }
    if (rv != 0) return rv;


    //
    // -- check the scrub queue for a frame to allocate
    //    --------------------------------------------------------------------------------------------------
    flags = SPINLOCK_BLOCK_NO_INT(pmm.scrubLock) {
        rv = _PmmDoRemoveFrame(pmm.scrubStack, true);       // -- scrub the frame when it is removed
        SPINLOCK_RLS_RESTORE_INT(pmm.scrubLock, flags);
    }

    if (rv != 0) return rv;


    //
    // -- check the low stack for a frame to allocate
    //    -------------------------------------------
    return PmmAllocateLowFrame();
}

