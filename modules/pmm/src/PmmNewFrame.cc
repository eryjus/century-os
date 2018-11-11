//===================================================================================================================
//
//  PmmNewFrame.cc -- Allocate a new frame
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Look through the bitmap for a frame to be allocated
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-11  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "pmm.h"


//
// -- IsFrameAlloc() -- Returns if a frame is allocated
//    -------------------------------------------------
static inline bool IsFrameAlloc(frame_t frame)
{
    uint32_t chk = pmmBitmap[frame >> 5] & (1 << (frame & 0x1f));
    return chk == 0;
}


//
// -- Given an index and a bit number, calculate the frame number
//    -----------------------------------------------------------
static inline frame_t CalcFrameNumber(int index, int bit) { return (index << 5) + bit; }


//
// -- Find and allocate a frame
//    -------------------------
frame_t PmmNewFrame(void)
{
    static uint32_t lastLook = (BITMAP_SIZE / 2);            // let's start around the middle of the memory
    uint32_t lastStart = lastLook;

    for ( ; lastLook < BITMAP_SIZE; lastLook ++) {
        if (pmmBitmap[lastLook] != 0) {                 // if 0, then all frame allocated
            uint32_t temp = pmmBitmap[lastLook];

            for (int i = 0; i < 32; i ++) {
                if (temp & (1<<i)) {
                    frame_t rv = CalcFrameNumber(lastLook, i);
                    PmmAllocFrame(rv);
                    return rv;
                }
            }
        }
    }

    for (lastLook = 0 ; lastLook <= lastStart; lastLook ++) {
        if (pmmBitmap[lastLook] != 0) {                 // if 0, then all frame allocated
            uint32_t temp = pmmBitmap[lastLook];

            for (int i = 0; i < 32; i ++) {
                if (temp & (1<<i)) {
                    frame_t rv = CalcFrameNumber(lastLook, i);
                    PmmAllocFrame(rv);
                    return rv;
                }
            }
        }
    }

    return -1;
}