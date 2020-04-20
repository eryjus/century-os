//===================================================================================================================
//
//  ButlerMemCheck.cc -- Check the memory frame to see if it can be freed
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Up to 4MB, check the memory to see if it can be freed.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-11  Initial  v0.6.1b  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "pmm.h"
#include "hw-disc.h"
#include "butler.h"


//
// -- Check the memory to see if it is eligible to be freed
//    -----------------------------------------------------
EXTERN_C EXPORT LOADER
bool ButlerMemCheck(frame_t frame)
{
    archsize_t addr = frame << 12;
    archsize_t krnBeg = 0x100000;

    if (frame < 0x100) return LowMemCheck(frame);
    if (addr >= krnBeg && frame < localHwDisc->modHighestFrame) return false;

    return true;
}




