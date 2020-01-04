//===================================================================================================================
//
//  FrameBufferDrawChar.cc -- Draw a character on to the screen
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  *** PROGRAMMING NOTE ***
//
//  This function has some important shortcomings, which are detailed here:
//  * Line wrapping is not implemented
//  * Screen scrolling is not implemented
//
//  Therefore, only one page of data can be printed currently
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-May-03  Initial   0.0.0   ADCL  Initial version
//  2018-Jun-13  Initial   0.1.0   ADCL  Copied this file from century to century-os
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "hw-disc.h"
#include "printf.h"
#include "fb.h"


//
// -- This is the internally linked system monospace font
//    ---------------------------------------------------
extern uint8_t systemFont[];


//
// -- Draw a character on the screen
//    ------------------------------
void FrameBufferDrawChar(char ch)
{
    if (GetRowPos() > 45) return;
    if (ch & 0x80) {
        if ((ch & 0xc0) == 0xc0) ch = '?';
        else return;
    }

    if (ch == '\n') {
        SetColPos(0);
        SetRowPos(GetRowPos() + 1);
        return;
    }

    if (ch == '\t') {
        SetColPos(GetColPos() + (8 - (GetColPos() % 8)));

        if (GetColPos() > GetFrameBufferWidth() / 8) {
            SetColPos(0);
            SetRowPos(GetRowPos() + 1);
        }

        return;
    }

    uint8_t *chImg = &systemFont[ch * 16];              // first the character image (16 rows per image)
    uint16_t *where = &((uint16_t *)GetFrameBufferAddr())[(GetRowPos() * GetFrameBufferWidth() * 16) + (GetColPos() * 8)];

    for (int i = 0; i < 16; i ++, where += GetFrameBufferWidth()) {
        uint8_t c = chImg[i];

        for (int j = 0; j < 8; j ++, c = c >> 1) {
            if (c & 0x01) where[j] = GetFgColor();
            else where[j] = GetBgColor();
        }
    }

    SetColPos(GetColPos() + 1);
}
