//===================================================================================================================
//
// PmmAllocFrame.cc -- Allocate a frame from the manager
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// Verify that the frame is free and then allocate it if it is available.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-10-30  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "pmm.h"

#include <errno.h>


//
// --  Allocate the specified Frame
//     ----------------------------
int PmmAllocFrame(frame_t frame)
{
    if (frame >= FRAME_COUNT) {
        return -ENOMEM;
    }

    if (PmmIsFrameAlloc(frame)) {
        return -EUNDEF;
    }

    pmmBitmap[frame >> 5] &= (~(1 << (frame & 0x1f)));
    return SUCCESS;
}
