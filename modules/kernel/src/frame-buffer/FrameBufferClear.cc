//===================================================================================================================
//
//  FrameBufferClear.cc -- Clear the frame buffer, setting the contents to the bgcolor
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
//  2018-Jun-13  Initial   0.1.0   ADCL  Copied this function from century to century-os
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "hw-disc.h"
#include "printf.h"
#include "fb.h"


//
// -- Clear the screen defined by the frame buffer
//    --------------------------------------------
void FrameBufferClear(void)
{
    //
    // -- calculate the number of 16-bit words to write (rows * cols)
    //    -----------------------------------------------------------
    size_t cnt = GetFrameBufferHeight() * GetFrameBufferWidth();
    uint16_t *b = (uint16_t *)GetFrameBufferAddr();

    kprintf("Attempting the clear the monitor screen at address %p\n", b);

    kMemSetW(b, GetBgColor(), cnt);

    SetRowPos(0);
    SetColPos(0);

    kprintf(".. Done!\n");
}
