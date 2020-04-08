//===================================================================================================================
//
//  DebugTimerCounts.cc -- Debug the timer counts across all cores
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-05  Initial  v0.6.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "serial.h"
#include "process.h"
#include "debugger.h"


//
// -- Debug the timer over all CPUs
//    -----------------------------
EXTERN_C EXPORT KERNEL
void DebugTimerCounts(void)
{
    while (true) {
        AtomicSet(&debugCommunication.coresResponded, 1);
        DebuggerEngage(DIPI_TIMER);
        debugCommunication.timerValue[thisCpu->cpuNum] = TimerCurrentCount(timerControl);

        while (AtomicRead(&debugCommunication.coresResponded) != cpus.cpusRunning) {}

        DebuggerRelease();

        // -- now we have the values -- dump them
        kprintf(ANSI_CLEAR ANSI_SET_CURSOR(0,0) ANSI_FG_BLUE ANSI_ATTR_BOLD
                "Current Timer Counts (press <Enter> to exit)\n" ANSI_ATTR_NORMAL);
        for (int i = 0; i < cpus.cpusRunning; i ++) {
            kprintf("CPU%d | %d\n", i, (uint32_t)debugCommunication.timerValue[i]);
        }

        while (SerialHasChar(&debugSerial)) {
            uint8_t ch = SerialGetChar(&debugSerial);
            if (ch == 10 || ch == 13) {
                return;
            }
        }

        ProcessSleep(1);
    }
}



