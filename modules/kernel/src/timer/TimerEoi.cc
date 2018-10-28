//===================================================================================================================
//
// TimerEoi.cc -- Issue an end of interrupt for the timer
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-28  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "timer.h"


//
// -- Issue an EOI for the timer
//    --------------------------
void TimerEoi(UNUSED(uint32_t irq))
{
    outb(0x20, 0x20);
}
