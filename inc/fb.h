//===================================================================================================================
//
//  inc/fb.h -- Framebuffer functions
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-06-13  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================

#ifndef __FB_H__
#define __FB_H__


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


#endif