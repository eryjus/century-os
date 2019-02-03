//===================================================================================================================
//
//  SyscallReceiveMessage.cc -- SYSCALL for POSIX `msgrcv()`
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  SYSCALL to receive a message
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-02  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "process.h"
#include "ipc.h"
#include "interrupt.h"

#include <errno.h>


//
// -- SYSCALL to receive a message
//    ----------------------------
void SyscallReceiveMessage(isrRegs_t *regs)
{
    Message_t *msg = (Message_t *)SYSCALL_RCVMSG_PARM1(regs);

    if (msg == NULL) {
        SYSCALL_RETURN(regs) = -EINVAL;
        return;
    }

    SYSCALL_RETURN(regs) = MessageReceive(msg);
}
