//===================================================================================================================
//  syscall.h -- This is the kernel internal definitions for handling system calls
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-02  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __SYSCALL_H__
#define __SYSCALL_H__


#include "types.h"


//
// -- System function 1: Receive a message
//    ------------------------------------
void SyscallReceiveMessage(isrRegs_t *regs);


//
// -- System function 2: Send a message
//    ------------------------------------
void SyscallSendMessage(isrRegs_t *regs);


#endif
