//===================================================================================================================
//
//  PmmVars.cc -- Global variables for the PMM
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-11  Initial   0.3.1   ADCL  Initial version
//
//===================================================================================================================


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
PmmManager_t pmm = {0};

