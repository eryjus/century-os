//===================================================================================================================
//
//  DebugParse.cc -- Parse the buffer for a command, and return its token
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  So, a programmer's note: This function is incredibly inefficient!!  It's horrible.  A better solution would be
//  to use flex to tokenize an input stream.  However, for this to work, I would need to prepare several C-runtime
//  library functions to support the flex functions.  I do not yet have enough of the kernel available to support
//  that effort.  So, instead, this is currently a brute force method.
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
#include "debugger.h"


//
// -- Parse the entered command line for the next command
//    ---------------------------------------------------
EXTERN_C EXPORT KERNEL
DebuggerCommand_t DebugParse(DebuggerState_t state)
{
    DebuggerCommand_t rv = CMD_ERROR;

    // -- down-case the first word in the command string
    for (size_t i = 0; i < kStrLen(debugCommand); i ++ ) {
        if (debugCommand[i] >= 'A' and debugCommand[i] <= 'Z') {
            debugCommand[i] = debugCommand[i] - 'A' + 'a';
        }

        if (debugCommand[i] == ' ') {
            debugCommand[i] = '\0';
            break;
        }
    }

    // -- A few global commands
    if (kStrCmp(debugCommand, "help") == 0) { rv = CMD_HELP; goto exit; }
    if (kStrCmp(debugCommand, "?") == 0) { rv = CMD_HELP; goto exit; }
    if (kStrCmp(debugCommand, "exit") == 0) { rv = CMD_EXIT; goto exit; }
    if (kStrCmp(debugCommand, "x") == 0) { rv = CMD_EXIT; goto exit; }
    if (kStrCmp(debugCommand, "quit") == 0) { rv = CMD_EXIT; goto exit; }
    if (kStrCmp(debugCommand, "q") == 0) { rv = CMD_EXIT; goto exit; }


    // -- consider the current state
    switch (state) {
        case DBG_HOME:
            if (kStrCmp(debugCommand, "sched") == 0) { rv = CMD_SCHED; goto exit; }
            if (kStrCmp(debugCommand, "scheduler") == 0) { rv = CMD_SCHED; goto exit; }
            if (kStrCmp(debugCommand, "timer") == 0) { rv = CMD_TIMER; goto exit; }
            if (kStrCmp(debugCommand, "msgq") == 0) { rv = CMD_MSGQ; goto exit; }
            break;

        case DBG_SCHED:
            if (kStrCmp(debugCommand, "show") == 0) { rv = CMD_SHOW; goto exit; }
            if (kStrCmp(debugCommand, "stat") == 0) { rv = CMD_STAT; goto exit; }
            if (kStrCmp(debugCommand, "status") == 0) { rv = CMD_STAT; goto exit; }
            if (kStrCmp(debugCommand, "run") == 0) { rv = CMD_RUNNING; goto exit; }
            if (kStrCmp(debugCommand, "running") == 0) { rv = CMD_RUNNING; goto exit; }
            if (kStrCmp(debugCommand, "ready") == 0) { rv = CMD_READY; goto exit; }
            if (kStrCmp(debugCommand, "list") == 0) { rv = CMD_LIST; goto exit; }
            break;

        case DBG_TIMER:
            if (kStrCmp(debugCommand, "counts") == 0) { rv = CMD_COUNTS; goto exit; }
            if (kStrCmp(debugCommand, "count") == 0) { rv = CMD_COUNTS; goto exit; }
            if (kStrCmp(debugCommand, "cnt") == 0) { rv = CMD_COUNTS; goto exit; }
            if (kStrCmp(debugCommand, "configuration") == 0) { rv = CMD_CONFIG; goto exit; }
            if (kStrCmp(debugCommand, "config") == 0) { rv = CMD_CONFIG; goto exit; }
            if (kStrCmp(debugCommand, "conf") == 0) { rv = CMD_CONFIG; goto exit; }
            if (kStrCmp(debugCommand, "cfg") == 0) { rv = CMD_CONFIG; goto exit; }

        case DBG_MSGQ:
            if (kStrCmp(debugCommand, "show") == 0) { rv = CMD_SHOW; goto exit; }
            if (kStrCmp(debugCommand, "stat") == 0) { rv = CMD_STAT; goto exit; }
            if (kStrCmp(debugCommand, "status") == 0) { rv = CMD_STAT; goto exit; }
            break;

        default:
            kprintf(ANSI_FG_RED ANSI_ATTR_BOLD "\n\n!! Unimplemented state!!\n" ANSI_ATTR_NORMAL);
            break;
    }


exit:
    if (rv == CMD_ERROR) {
        kprintf("Invalid command.  Available commands are below the input line.  Use 'help' for a detailed description.\n");
        kMemSetB(debugCommand, 0, DEBUG_COMMAND_LEN);
        return CMD_ERROR;
    }

    // -- move any additional command up
    int len = kStrLen(debugCommand);
    kMemMove(debugCommand, debugCommand + len + 1, kStrLen(debugCommand + len + 1) + 1);
    kMemSetB(debugCommand + kStrLen(debugCommand), 0, DEBUG_COMMAND_LEN - kStrLen(debugCommand));

    return rv;
}

