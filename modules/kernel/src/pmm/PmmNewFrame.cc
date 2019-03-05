//===================================================================================================================
//
//  PmmNewFrame.cc -- Allocate a new frame from the system and initialize it to zeros
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Jun-30  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-14  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "cpu.h"
#include "pmm.h"


#ifndef DEBUG_PMM
#   define DEBUG_PMM 0
#endif


//
// -- This variable marks the last place we looked for a frame; we will start from this location
//    ------------------------------------------------------------------------------------------
frame_t pmmLastLook;
frame_t pmmLimit;


//
// -- Allocate a frame for use (not going to do this too much here...)
//    ----------------------------------------------------------------
frame_t PmmNewFrame(size_t cnt)
{
    frame_t currFrame = pmmLastLook;

#if DEBUG_PMM == 1
    kprintf("Checking from frame %p\n", currMap);
#endif

    while (currFrame <= pmmLimit) {
        // -- check if we have enough frames from here
        for (size_t i = 0; i < cnt; i ++) {
            if (PmmIsFrameAlloc(currFrame + i)) break;
        }

        // -- we got here; we have enough frames
        if (!PmmIsFrameAlloc(currFrame)) {
            PmmAllocFrameRange(currFrame, cnt);
            pmmLastLook = currFrame;

            return currFrame;
        }

        currFrame ++;
    }

    currFrame = 1;     // -- we will never allocate frame 0

    while (currFrame <= pmmLastLook) {
        // -- check if we have enough frames from here
        for (size_t i = 0; i < cnt; i ++) {
            if (PmmIsFrameAlloc(currFrame + i)) break;
        }

        // -- we got here; we have enough frames
        if (!PmmIsFrameAlloc(currFrame)) {
            PmmAllocFrameRange(currFrame, cnt);
            pmmLastLook = currFrame;

            return currFrame;
        }

        currFrame ++;
    }

    kprintf("Out of memory allocating a new frame");
    Halt();

    return -1;
}

