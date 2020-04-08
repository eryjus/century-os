//===================================================================================================================
//
//  DebugTimer.cc -- Debug the timer across all cores
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
void DebugTimer(void)
{
    while (true) {
        if (kStrLen(debugCommand) == 0) DebugPrompt(debugState);
        DebuggerCommand_t cmd = DebugParse(debugState);

        switch(cmd) {
            case CMD_EXIT:
                debugState = DBG_HOME;
                return;

            case CMD_COUNTS:
                DebugTimerCounts();
                debugState = DBG_TIMER;
                break;

            case CMD_CONFIG:
//                DebugTimerConfig();
                debugState = DBG_TIMER;
                break;

            case CMD_ERROR:
            default:
                kprintf(ANSI_ATTR_BOLD ANSI_FG_RED
                        "Something went wrong (timer) -- a bug in the debugger is likely\n" ANSI_ATTR_NORMAL);
                continue;
        }
    }
}