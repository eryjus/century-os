//===================================================================================================================
//
//  TimerVars.cc -- The globals for the PIT
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-28  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "timer.h"


//
// -- This is the control structure for determining exactly which functions are called for the PIT timer
//    --------------------------------------------------------------------------------------------------
TimerFunctions_t pitTimer = {
    TimerInit,
    TimerEoi,
};

