//===================================================================================================================
//
//  SyscallHandler.cc -- Handler for system calls
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Handle system calls
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-01  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "syscall.h"
#include "interrupt.h"

#include <errno.h>


//
// -- This is a static function to make sure there is always a function to call
//    -------------------------------------------------------------------------
static void SyscallNullHandler(isrRegs_t *regs)
{
    regs->eax = -ENOSYS;
}


//
// -- The ISR Handler Table
//    ---------------------
static isrFunc_t syscallHandlers[] = {
    SyscallNullHandler,                 // Function 0; trivial call
    SyscallReceiveMessage,              // Function 1: receive a message
    SyscallSendMessage,                 // Function 2: send a message
};


//
// -- This is the ISR Handler routine
//    -------------------------------
void SyscallHandler(isrRegs_t *regs)
{
    if ((uint32_t)regs->eax >= sizeof(syscallHandlers) / sizeof(isrFunc_t)) {
        SyscallNullHandler(regs);
        return;
    }

    isrFunc_t handler = syscallHandlers[regs->eax];
    handler(regs);
}
