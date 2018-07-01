//===================================================================================================================
//
// loader/src/pmm/PmmNewFrame.cc -- Allocate a new frame from the system and initialize it to zeros
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-06-30  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "serial.h"
#include "cpu.h"
#include "pmm.h"


//
// -- Allocate a frame for use (not going to do this too much here...)
//    ----------------------------------------------------------------
frame_t PmmNewFrame(void)
{
    frame_t frame;

    if ((frame = GetModHightestFrame()) > 0x000fffff) frame = 0x000fffff;

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

