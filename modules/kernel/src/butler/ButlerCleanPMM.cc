//===================================================================================================================
//
//  ButlerCleanPmm.cc -- Clean up a PMM frame, sanitizing it
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
#include "pmm.h"
#include "butler.h"


//
// -- The Butler has been notified of a PMM frame to clean
//    ----------------------------------------------------
void ButlerCleanPmm(void)
{
    // -- With one message, we clean all we can; later messages will clean nothing
    while (!IsListEmpty(&pmm.scrubStack)) {
        PmmScrubBlock();
    }
}


