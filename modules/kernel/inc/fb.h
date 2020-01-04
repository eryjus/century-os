//===================================================================================================================
//
//  fb.h -- Framebuffer functions
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Jun-13  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================

#ifndef __FB_H__
#define __FB_H__


#include "types.h"


//
// -- Initialize the frame buffer
//    ---------------------------
void FrameBufferInit(void);


//
// -- Clear the frame buffer
//    ----------------------
void FrameBufferClear(void);


//
// -- Parse an RGB color in the form '#ffffff' into an RGB color
//    ----------------------------------------------------------
uint16_t FrameBufferParseRGB(const char *c);


//
// -- Draw a character on the screen
//    ------------------------------
void FrameBufferDrawChar(char ch);


//
// -- Write a screen on the screen
//    ----------------------------
void FrameBufferPutS(const char *s);


//
// -- Output a hex string to the screen
//    ---------------------------------
#define FrameBufferPutHex(val)                                      \
    do {                                                            \
        FrameBufferDrawChar('0');                                   \
        FrameBufferDrawChar('x');                                   \
        for (int i = 28; i >= 0; i -= 4) {                          \
            char c = (((val) >> i) & 0x0f);                         \
                                                                    \
            if (c > 9) FrameBufferDrawChar(c - 10 + 'a');           \
            else FrameBufferDrawChar(c + '0');                      \
        }                                                           \
    } while (0)



#endif