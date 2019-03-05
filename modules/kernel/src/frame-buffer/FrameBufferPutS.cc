//===================================================================================================================
//
//  FrameBufferPutS.cc -- Write a string to the frame buffer
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Jun-13  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "hw-disc.h"
#include "fb.h"


//
// -- Write a string to the frame buffer
//    ----------------------------------
void FrameBufferPutS(const char *s)
{
    while (*s) {
        FrameBufferDrawChar(*s ++);
    }
}
