//===================================================================================================================
//
//  TimerPick.cc -- Make a decision on which Timer will be used
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-26  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "hw-disc.h"
#include "printf.h"
#include "platform.h"
#include "timer.h"


//
// -- pick the timer device we will use
//    ---------------------------------
__krntext TimerDevice_t *TimerPick(void)
{
    kprintf("Picking a timer to use...\n");

    if (GetLocalApicCount() > 0) timerControl = &lapicTimerControl;
    else timerControl = &timer8253Control;

    // -- initialized in kInit();

    return timerControl;
}

