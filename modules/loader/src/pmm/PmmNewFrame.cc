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
#   define DEBUG_PMM 1
#endif


//
// -- Allocate a frame for use (not going to do this too much here...)
//    ----------------------------------------------------------------
frame_t PmmNewFrame(void)
{
    frame_t frame;

    if ((frame = GetModHightestFrame()) > 0x000fffff) frame = 0x000fffff;

#if DEBUG_PMM == 1
    SerialPutS("Checking from frame "); SerialPutHex(frame); SerialPutChar('\n');
#endif

    while (true && frame != GetPmmFrameCount()) {
        if (!PmmIsFrameAlloc(frame)) {
            kMemSetB((void *)PmmFrameToLinear(frame), 0, 4096);
            PmmAllocFrame(frame);

            return frame;
        } else frame ++;
    }

    SerialPutS("Out of memory allocating a new frame");
    Halt();

    return -1;
}

