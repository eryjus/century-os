//===================================================================================================================
//
//  loader/src/frame-buffer/FrameBufferInit.cc -- Frame buffer initialization for the console
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-05-03  Initial   0.0.0   ADCL  Initial version
//  2018-06-13  Initial   0.1.0   ADCL  Copied this file from century (fb.c) to century-os
//
//===================================================================================================================


#include "types.h"
#include "hw-disc.h"
#include "fb.h"


//
// -- Initialize the additional frame buffer info
//    -------------------------------------------
void FrameBufferInit(void)
{
    SetFgColor(0xffff);
    SetBgColor(0x1234);
    FrameBufferClear();
}
