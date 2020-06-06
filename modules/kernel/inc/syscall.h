//===================================================================================================================
//  syscall.h -- This is the kernel internal definitions for handling system calls
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-02  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#pragma once


#include "types.h"


//
// -- This is the prototype for all the system call functions
//    -------------------------------------------------------
typedef int(*SyscallFunc_t)(isrRegs_t *);


//
// -- Self-terminate the calling process
//    ----------------------------------
EXTERN_C EXPORT SYSCALL
int SyscallExit(isrRegs_t *regs);



