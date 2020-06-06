//===================================================================================================================
//
//  SyscallExit.cc -- Handler to self-terminate the calling process
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Note that at this moment, exit does not support passing the exit value on to the calling process
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-18  Initial  v0.7.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "process.h"
#include "syscall.h"

#include <errno.h>


//
// -- Handle self-terminating the calling process
//    -------------------------------------------
EXTERN_C EXPORT SYSCALL
int SyscallExit(isrRegs_t *reg)
{
    ProcessEnd();
    return -EUNDEF;
}

