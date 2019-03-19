//===================================================================================================================
//
// ProcessVars.cc -- Global variables for process management
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-14  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "process.h"


//
// -- This is the current running process
//    -----------------------------------
__krndata Process_t *currentProcess = NULL;


//
// -- This is the next PID that will be allocated
//    -------------------------------------------
__krndata PID_t nextPID = 0;
