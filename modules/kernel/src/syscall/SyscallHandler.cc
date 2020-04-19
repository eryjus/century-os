//===================================================================================================================
//
//  SyscallHandler.cc -- Handler for system calls
//
//        Copyright (c)  2017-2020 -- Adam Clark
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
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "syscall.h"
#include "interrupt.h"

#include <errno.h>


//
// -- The ISR Handler Table
//    ---------------------
HIDDEN SYSCALL_DATA
SyscallFunc_t syscallHandlers[] = {
    SyscallExit,                        // Function 0: self-terminate the process (does not return)
};


//
// -- This is the ISR Handler routine
//    -------------------------------
EXTERN_C EXPORT SYSCALL
void SyscallHandler(isrRegs_t *regs)
{
    if ((uint32_t)SYSCALL_FUNC_NO(regs) >= sizeof(syscallHandlers) / sizeof(isrFunc_t)) {
        SYSCALL_RETURN(regs) = -ENOSYS;
        return;
    }

    // -- TODO: Here we need to map the kernel
    SyscallFunc_t handler = syscallHandlers[SYSCALL_FUNC_NO(regs)];
    SYSCALL_RETURN(regs) = handler(regs);
    // -- TODO: Before we exit, unmap the kernel
}

