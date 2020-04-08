//===================================================================================================================
//
//  IpiHandleDebugger.cc -- Stop the cores and handle debugger requests as required
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-03  Initial  v0.6.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "pic.h"
#include "timer.h"
#include "debugger.h"


//
// -- Stop a core and wait for permission to continue
//    -----------------------------------------------
EXTERN_C EXPORT KERNEL
void IpiHandleDebugger(isrRegs_t *regs)
{
    AtomicInc(&debugCommunication.coresEngaged);

    switch(debugCommunication.command) {
        case DIPI_ENGAGE:
            // -- no action required
            break;

        case DIPI_TIMER:
            // -- get the current timer from each core and report the results
            debugCommunication.timerValue[thisCpu->cpuNum] = TimerCurrentCount(timerControl);
            AtomicInc(&debugCommunication.coresResponded);
            break;

        default:
            kprintf("\n\nCPU%d: Unimplemented Debugger command %d\n", debugCommunication.command);
            break;
    }

    while (AtomicRead(&debugCommunication.coresEngaged) != 0) {}
    PicEoi(picControl, (Irq_t)0);
}





