//===================================================================================================================
//
//  PmmVars.cc -- Global variables for the PMM
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-11  Initial   0.3.1   ADCL  Initial version
//  2020-Apr-12   #405    v0.6.1c  ADCL  Redesign the PMM to store the stack in the freed frames themselves
//
//===================================================================================================================


#include "mmu.h"
#include "pmm.h"


//
// -- Has the PMM been initialized properly for use?
//    ----------------------------------------------
EXPORT KERNEL_DATA
bool pmmInitialized = false;


//
// -- This is the structure for managing the PMM
//    ------------------------------------------
EXPORT KERNEL_DATA
Pmm_t pmm = {
    .framesAvail = {0},
    .lowLock = {0},
    .lowStack = (PmmFrameInfo_t *)MMU_PMM_LOW_TOS,
    .normLock = {0},
    .normStack = (PmmFrameInfo_t *)MMU_PMM_NORM_TOS,
    .scrubLock = {0},
    .scrubStack = (PmmFrameInfo_t *)MMU_PMM_SCRUB_TOS,
    .searchLock = {0},
    .search = (PmmFrameInfo_t *)MMU_PMM_SEARCH_TOS,
    .insertLock = {0},
    .insert = (PmmFrameInfo_t *)MMU_PMM_INSERT,
};

