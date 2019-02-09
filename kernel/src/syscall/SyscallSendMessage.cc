//===================================================================================================================
//
//  SyscallSendMessage.cc -- SYSCALL for POSIX `msgsnd()`
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  SYSCALL to send a message
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-02  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "process.h"
#include "ipc.h"
#include "interrupt.h"

#include <errno.h>


//
// -- SYSCALL to send a message
//    ----------------------------
void SyscallSendMessage(isrRegs_t *regs)
{
    PID_t pid = SYSCALL_SNDMSG_PARM1(regs);
    Message_t *msg = (Message_t *)SYSCALL_SNDMSG_PARM2(regs);
    Process_t *proc = ProcessGetStruct(pid);

    if (msg == NULL) {
        SYSCALL_RETURN(regs) = -EINVAL;
        return;
    }

    if (proc == NULL) {
        SYSCALL_RETURN(regs) = -EINVAL;
        return;
    }

    SYSCALL_RETURN(regs) = MessageSend(pid, (Message_t *)msg);
}
