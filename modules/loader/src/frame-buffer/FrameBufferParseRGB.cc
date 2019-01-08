//===================================================================================================================
//
//  FrameBufferParseRGB.cc -- Frame buffer initialization for the console
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-May-03  Initial   0.0.0   ADCL  Initial version
//  2018-Jun-13  Initial   0.1.0   ADCL  Copied this file from century (fb.c) to century-os
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "fb.h"


//
// -- an internal worker function that will parse a hex digit to a value
//    ------------------------------------------------------------------
static inline uint16_t ParseHex(char c)
{
    if (c >= '0' && c <= '9') return (c - '0') & 0x0f;
    else if (c >= 'a' && c <= 'f') return (c - 'a' + 10) & 0x0f;
    else if (c >= 'A' && c <= 'F') return (c - 'A' + 10) & 0x0f;
    else return 0;
}



//
// -- set the color code (uint16_t) to be the result of parsing the string ("#FFFFFF")
//    --------------------------------------------------------------------------------
uint16_t FrameBufferParseRGB(const char *c)
{
    if (!c || kStrLen(c) != 7 || *c != '#') return 0;

    int r = (ParseHex(c[1]) << 4) | ParseHex(c[2]);
    int g = (ParseHex(c[3]) << 4) | ParseHex(c[4]);
    int b = (ParseHex(c[5]) << 4) | ParseHex(c[6]);

    r = (r & 0xff) >> 3;        // 5 bits
    g = (g & 0xff) >> 2;        // 6 bits
    b = (b & 0xff) >> 3;        // 5 bits

    return (r << 11) | (g << 5) | b;
}
