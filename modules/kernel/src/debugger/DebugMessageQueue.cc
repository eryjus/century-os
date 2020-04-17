//===================================================================================================================
//
//  DebugMessageQueue.cc -- Debug the message queues
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-10  Initial  v0.6.1a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "pic.h"
#include "debugger.h"


//
// -- Debug the message queues
//    ------------------------
EXTERN_C EXPORT KERNEL
void DebugMsgq(void)
{
    while (true) {
        if (kStrLen(debugCommand) == 0) DebugPrompt(debugState);
        DebuggerCommand_t cmd = DebugParse(debugState);

        switch(cmd) {
            case CMD_EXIT:
                debugState = DBG_HOME;
                return;

            case CMD_STAT:
                DebugMsgqStatus();
                debugState = DBG_MSGQ;
                break;

            case CMD_SHOW:
                debugState = DBG_MSGQ;
                break;

            case CMD_ERROR:
            default:
                kprintf(ANSI_ATTR_BOLD ANSI_FG_RED
                        "Something went wrong (msgq) -- a bug in the debugger is likely\n" ANSI_ATTR_NORMAL);
                continue;
        }
    }
}


