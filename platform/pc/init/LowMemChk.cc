//===================================================================================================================
//
//  LowMemCheck.cc -- Determine if this low memory is used or available
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-10  Initial  v0.6.1b  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "hw-disc.h"



//
// -- all low memory is available on rpi2b
//    ------------------------------------
EXTERN_C EXPORT LOADER
bool LowMemCheck(frame_t frame)
{
    // -- the NULL frame is abandonned
    if (frame == 0) return false;

    // -- The GDT is not available
    if (frame == 0x10) return false;

    // -- The IDT is not available
    if (frame ==  0x09) return false;

    // -- The Trampoline is not available
    if (frame == 0x08) return false;

    // -- The EBDA is not available
    if (frame > GetEbda() >> 12) return false;

    // -- everything else is available
    return true;
}

