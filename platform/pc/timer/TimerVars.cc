//===================================================================================================================
//
//  TimerVars.cc -- These are the variables for the x86 Timer
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-24  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#include "pic.h"
#include "timer.h"


//
// -- This is the device description for the PIT
//    ------------------------------------------
__krndata TimerDevice_t timer8253Control = {
    .base = TIMER,
    .TimerCallBack = TimerCallBack,
    .TimerInit = _TimerInit,
    .TimerEoi = _TimerEoi,
    .TimerPlatformTick = _TimerPlatformTick,
    .TimerCurrentCount = _TimerCurrentCount,
};


//
// -- This is the timer controller we use for this runtime
//    ----------------------------------------------------
__krndata TimerDevice_t *timerControl = &timer8253Control;
