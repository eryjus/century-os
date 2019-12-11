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
// -- This is the frame of the interrupt table, whatever that means for the arch
//    --------------------------------------------------------------------------
EXPORT LOADER_BSS
frame_t intTableAddr = 0;


//
// -- This is the frame that is used by the early stack
//    -------------------------------------------------
EXPORT LOADER_BSS
frame_t earlyStackFrame = 0;


//
// -- This is the frame for the MMU level 1 table and its frame count
//    ---------------------------------------------------------------
EXPORT LOADER_BSS
archsize_t mmuLvl1Table = 0;

EXPORT LOADER_BSS
size_t mmuLvl1Count = 0;


//
// -- This is for a call to set a block of memory
//    -------------------------------------------
EXPORT LOADER_BSS
kMemSetB_t lMemSetB;
