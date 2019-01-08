//===================================================================================================================
//
//  PmmFreeFrame.cc -- Free a frame back to the manager
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Update the bitmap that the frame is available to be used somewhere else.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-30  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "pmm.h"

#include <errno.h>


//
// --  Free the specified Frame
//     ------------------------
int PmmFreeFrame(frame_t frame)
{
    if (frame >= FRAME_COUNT) {
        return -ENOMEM;
    }

    pmmBitmap[frame >> 5] |= (1 << (frame & 0x1f));
    return SUCCESS;
}
