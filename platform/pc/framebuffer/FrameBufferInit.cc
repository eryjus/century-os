//===================================================================================================================
//
//  FrameBufferInit.cc -- Frame buffer initialization for the console
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-May-03  Initial   0.0.0   ADCL  Initial version
//  2018-Jun-13  Initial   0.1.0   ADCL  Copied this file from century (fb.c) to century-os
//  2019-Feb-15  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "hw-disc.h"
#include "mmu.h"
#include "printf.h"
#include "fb.h"


//
// -- Initialize the additional frame buffer info
//    -------------------------------------------
EXTERN_C EXPORT LOADER
void FrameBufferInit(void)
{
    kprintf(".. Framebuffer located at: %p\n", GetFrameBufferAddr());
    kprintf(".. Framebuffer size: %p\n", GetFrameBufferPitch() * GetFrameBufferHeight());


    //
    // -- Map the frame buffer to its final location in virtual memory
    //    ------------------------------------------------------------
    kprintf("Mapping the Frame Buffer\n");
    for (archsize_t fbVirt = MMU_FRAMEBUFFER, fbFrame = ((archsize_t)GetFrameBufferAddr()) >> 12,
                    fbEnd = fbVirt + (GetFrameBufferPitch() * GetFrameBufferHeight());
            fbVirt < fbEnd; fbVirt += PAGE_SIZE, fbFrame ++) {
        MmuMapToFrame(fbVirt, fbFrame, PG_KRN | PG_WRT | PG_DEVICE);
    }

    // -- goose the config to the correct fb address
    SetFrameBufferAddr((uint16_t *)MMU_FRAMEBUFFER);
    SetFgColor(0xffff);
    SetBgColor(0x1234);
    FrameBufferClear();
}
