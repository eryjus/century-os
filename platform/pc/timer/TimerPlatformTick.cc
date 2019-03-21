//===================================================================================================================
//
//  TimerPlatformTick.cc -- This is the update that needs to take place with every tick of the timer.
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-19  Initial   0.3.2   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "timer.h"


//
// -- Do nothing on a timer tick
//    --------------------------
void __krntext _TimerPlatformTick(UNUSED(TimerDevice_t *dev))
{
    extern uint64_t ticksSinceBoot;

    ticksSinceBoot ++;
}
