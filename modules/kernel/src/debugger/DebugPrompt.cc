//===================================================================================================================
//
//  DebugPrompt.cc -- Prompt for a command and get the input into a global buffer
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
#include "debugger.h"



//
// -- Given the current command branch, prompt for and get the next command
//    ---------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void DebugPrompt(DebuggerState_t state)
{
    kprintf("\n (allowed: %s)\r" ANSI_CURSOR_UP(1), dbgPrompts[state].allowed);
    kprintf("%s :> ", dbgPrompts[state].branch);
    kMemSetB(debugCommand, 0, DEBUG_COMMAND_LEN);
    int pos = 0;

    while (true) {
        // -- we always need room for at least one more character
        if (pos == DEBUG_COMMAND_LEN - 1) {
            pos = DEBUG_COMMAND_LEN - 2;
        }

        while (!SerialHasChar(&debugSerial)) {  }
        char ch = (char)SerialGetChar(&debugSerial);

        // -- an enter completes the command input
        if (ch == 13 || ch == 10) {
            if (*debugCommand == 0) continue;       // make sure we do not have an empty buffer
            kprintf("\n" ANSI_ERASE_LINE);
            return;
        } else if (ch < ' ') continue;              // other special characters ignored
        else if (ch == 127) {
            if (--pos < 0) {
                pos = 0;
                continue;
            }
            kprintf("\b \b");
            debugCommand[pos] = '\0';
            continue;
        }

        debugCommand[pos ++] = ch;
        kprintf("%c", ch);
    }
}

