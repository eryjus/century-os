//===================================================================================================================
// SyscallSendMessage.cc -- SYSCALL for POSIX `msgsnd()`
//
// SYSCALL to send a message
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

#include <errno.h>


//
// -- SYSCALL to send a message
//    ----------------------------
void SyscallSendMessage(isrRegs_t *regs)
{
    PID_t pid = regs->edx;
    Message_t *msg = (Message_t *)regs->edi;
    Process_t *proc = ProcessGetStruct(pid);

    if (msg == NULL) {
        regs->eax = -EINVAL;
        return;
    }

    if (proc == NULL) {
        regs->eax = -EINVAL;
        return;
    }

    regs->eax = MessageSend(regs->edx, (Message_t *)regs->edi);
}