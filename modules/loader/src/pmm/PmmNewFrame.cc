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
//
//===================================================================================================================


#include "types.h"
#include "serial-loader.h"
#include "cpu.h"
#include "pmm.h"


#ifndef DEBUG_PMM
#   define DEBUG_PMM 0
#endif


//
// -- Allocate a frame for use (not going to do this too much here...)
//    ----------------------------------------------------------------
frame_t PmmNewFrame(size_t cnt)
{
    frame_t frame;

    if ((frame = GetModHightestFrame()) > 0x000fffff) frame = 0x000fffff;

#if DEBUG_PMM == 1
    SerialPutS("Checking from frame "); SerialPutHex(frame); SerialPutChar('\n');
#endif

    while (true && frame != GetPmmFrameCount()) {
        // -- check if we have enough frames from here
        for (size_t i = 0; i < cnt; i ++) {
            if (PmmIsFrameAlloc(frame + i)) break;
        }

        // -- we got here; we have enough frames
        if (!PmmIsFrameAlloc(frame)) {
            kMemSetB((void *)PmmFrameToLinear(frame), 0, 4096);
            PmmAllocFrameRange(frame, cnt);

            return frame;
        }

        frame ++;
    }

    SerialPutS("Out of memory allocating a new frame");
    Halt();

    return -1;
}

