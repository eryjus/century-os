//===================================================================================================================
//
// ProcessUnblock.cc -- Unblock a process
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-22  Initial   0.3.2   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "lists.h"
#include "process.h"


//
// -- Block the current process
//    -------------------------
EXPORT KERNEL
void ProcessDoUnblock(Process_t *proc)
{
    proc->status = PROC_READY;
    ProcessDoReady(proc);
}

