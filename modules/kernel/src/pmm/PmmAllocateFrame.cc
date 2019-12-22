//===================================================================================================================
//
//  PmmAllocateFrame.cc -- Allocate a normal frame (not low memory required)
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
    if (!pmmInitialized) {
//        kprintf("Call to allocate a PMM frame before the PMM has been initialized!\n");
        return NextEarlyFrame();
    }

    frame_t rv = 0;         // assume we will not find anything


    //
    // -- check the normal stack for a frame to allocate
    //    ----------------------------------------------
    rv = _PmmDoRemoveFrame(&pmm.normalStack, false);
    if (rv != 0) return rv;


    //
    // -- check the scrub queue for a frame to allocate
    //    --------------------------------------------------------------------------------------------------
    rv = _PmmDoRemoveFrame(&pmm.scrubStack, true);
    if (rv != 0) {
        PmmScrubFrame(rv);          // -- it needs to be scrubbed
        return rv;
    }


    //
    // -- check the low stack for a frame to allocate
    //    -------------------------------------------
    return PmmAllocateLowFrame();
}

