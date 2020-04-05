//===================================================================================================================
//
//  DebugStart.cc -- This is the entry point for the kernel debugger
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-02  Initial  v0.6.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "serial.h"
#include "process.h"
#include "debugger.h"


//
// -- This is the main entry point for the kernel debugger
//    ----------------------------------------------------
EXTERN_C EXPORT KERNEL
void DebugStart(void)
{
    // -- we want the highest chance of getting CPU time!
    currentThread->priority = PTY_OS;
    debugState = DBG_HOME;

    kprintf(ANSI_CLEAR ANSI_SET_CURSOR(0,0) ANSI_FG_RED ANSI_ATTR_BOLD
            "Welcome to the Century-OS kernel debugger\n" ANSI_ATTR_NORMAL);

    while (true) {
        DebugPrompt(debugState);

        DebuggerCommand_t cmd = DebugParse(debugState);

        switch(cmd) {
            case CMD_EXIT:
                debugState = DBG_HOME;
                kMemSetB(debugCommand, 0, DEBUG_COMMAND_LEN);
                continue;

            case CMD_SCHED:
                debugState = DBG_SCHED;
                DebugScheduler();
                continue;

            case CMD_TIMER:
                debugState = DBG_TIMER;
                DebugTimer();
                continue;

            case CMD_ERROR:
            default:
                kprintf("\n\n" ANSI_ATTR_BOLD ANSI_FG_RED
                        "Something went wrong (main) -- a bug in the debugger is likely\n" ANSI_ATTR_NORMAL);
                continue;
        }

        if (cmd == CMD_ERROR) continue;
    }
}


