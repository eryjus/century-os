//===================================================================================================================
//
//  SemaphoreVars.cc -- These are the variables for semaphore management
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-30  Initial   0.4.3   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "semaphore.h"


//
// -- these values control the semaphores
//    -----------------------------------
__krndata int semmsl = SEMMSL;
__krndata int semmns = SEMMNS;
__krndata int semopm = SEMOPM;
__krndata int semmni = SEMMNI;


//
// -- this is the global semaphore structure, encapsulating all system semaphores
//    ---------------------------------------------------------------------------
__krndata SemaphoreAll_t semaphoreAll = {0};