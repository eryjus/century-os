//===================================================================================================================
//
//  TimerEoi.cc -- Issue an end of interrupt for the timer
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-21  Initial   0.2.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "hardware.h"
#include "printf.h"
#include "timer.h"


//
// -- Issue an EOI for the timer
//    --------------------------
void _TimerEoi(TimerDevice_t *dev)
{
    if (!dev) return;
    WRITE_CNTP_TVAL(dev->reloadValue);
    WRITE_CNTP_CTL(1);                              // -- enable the timer
}

