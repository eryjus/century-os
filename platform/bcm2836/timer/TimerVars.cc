//===================================================================================================================
//
//  TimerVars.cc -- These are the variables for the bcm2835 Timer
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
#include "mmu.h"
#include "timer.h"


//
// -- This is the device description that is used to output data to the serial port during loader initialization
//    ----------------------------------------------------------------------------------------------------------
__krndata TimerDevice_t _timerControl = {
    .base = TIMER,
    .pic = &picBcm2835,
    .TimerCallBack = TimerCallBack,
    .TimerInit = _TimerInit,
    .TimerEoi = _TimerEoi,
    .TimerPlatformTick = _TimerPlatformTick,
    .TimerCurrentCount = _TimerCurrentCount,
};


//
// -- This is the pointer to the structure we will really use
//    -------------------------------------------------------
__krndata TimerDevice_t *timerControl = &_timerControl;
