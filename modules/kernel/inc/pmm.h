//===================================================================================================================
//
//  pmm.h -- This is the kernel's interface into the PMM
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-11  Initial    0.1.0  ADCL  Initial version
//
//===================================================================================================================


#ifndef __PMM_H__
#define __PMM_H__

#include "types.h"
#include "pmm-msg.h"


//
// -- Go and ask the PMM for a new frame
//    ----------------------------------
frame_t PmmAllocFrame(void);


#endif
