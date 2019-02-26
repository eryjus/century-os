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
// -- This is the frame right below 4MB, which will be the next one allocated
//    -----------------------------------------------------------------------
frame_t pmmEarlyFrame __ldrdata = 0x000003ff;
frame_t pmmEarlyFrameSave __ldrdata = 0;


//
// -- This is the frame of the interrupt table, whatever that means for the arch
//    --------------------------------------------------------------------------
frame_t intTableAddr __ldrdata = 0;


//
// -- This is the frame that is used by the early stack
//    -------------------------------------------------
frame_t earlyStackFrame __ldrdata = 0;


//
// -- This is the frame for the MMU level 1 table and its frame count
//    ---------------------------------------------------------------
archsize_t mmuLvl1Table __ldrdata = 0;
size_t mmuLvl1Count __ldrdata = 0;


//
// -- This is for a call to set a block of memory
//    -------------------------------------------
kMemSetB_t lMemSetB __ldrdata;
