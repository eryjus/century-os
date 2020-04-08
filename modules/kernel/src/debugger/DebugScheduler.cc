//===================================================================================================================
//
//  DebugScheduler.cc -- Debug the scheduler
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  We have gotten to this point, we know we are debugging the scheduler.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-03  Initial  v0.6.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "serial.h"
#include "process.h"
#include "debugger.h"


//
// -- Control where we go to debug the scheduler
//    ------------------------------------------
EXTERN_C EXPORT KERNEL
void DebugScheduler(void)
{
    while (true) {
        if (kStrLen(debugCommand) == 0) DebugPrompt(debugState);
        DebuggerCommand_t cmd = DebugParse(debugState);

        switch(cmd) {
            case CMD_EXIT:
                debugState = DBG_HOME;
                return;

            case CMD_SHOW:
                DebugSchedulerShow();
                debugState = DBG_SCHED;
                break;

            case CMD_STAT:
                DebugSchedulerStat();
                debugState = DBG_SCHED;
                break;

            case CMD_RUNNING:
                DebugSchedulerRunning();
                debugState = DBG_SCHED;
                break;

            case CMD_READY:
                debugState = DBG_HOME;
                break;

            case CMD_LIST:
                debugState = DBG_HOME;
                return;

            case CMD_ERROR:
            default:
                kprintf(ANSI_ATTR_BOLD ANSI_FG_RED
                        "Something went wrong (scheduler) -- a bug in the debugger is likely\n" ANSI_ATTR_NORMAL);
                continue;
        }
    }
}



