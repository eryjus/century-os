//===================================================================================================================
//
//  LoaderVars.cc -- These are loader-specific variables
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  These types are architecture independent.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-10  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "serial.h"
#include "loader.h"


//
// -- This is the frame that is used by the early stack
//    -------------------------------------------------
EXPORT LOADER_BSS
frame_t earlyStackFrame = 0;


//
// -- This is for a call to set a block of memory
//    -------------------------------------------
EXPORT LOADER_BSS
kMemSetB_t lMemSetB;
