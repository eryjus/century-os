//===================================================================================================================
//
//  loader/src/string/memmove.cc -- copy a block of memory from one address to another
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-04-26  Initial   0.0.0   ADCL  Initial version
//  2018-06-02  Initial   0.1.0   ADCL  Copied this file (string.c) from century and whittled down to memmove().
//
//===================================================================================================================


#include <stdint.h>
#include <stddef.h>


extern "C" void memmove(uint8_t *d, const uint8_t *s, size_t cnt);


//
// -- memmove() -- copy bytes from one memory buffer to another
//    ---------------------------------------------------------------------------------------------------------------
void memmove(uint8_t *d, const uint8_t *s, size_t cnt)
{
    for (unsigned int i = 0; i < cnt; i ++) d[i] = s[i];
}
