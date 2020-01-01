//===================================================================================================================
//
// ProcessTerminate.cc -- End a task by placing it on the terminated queue
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-29  Initial   0.3.2   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "cpu.h"
#include "process.h"


//
// -- Terminate a task
//    ----------------
EXPORT KERNEL void ProcessTerminate(Process_t *proc)
{
    if (!assert(proc != NULL)) return;

    ProcessLockAndPostpone();
    ProcessListRemove(proc);
    Enqueue(&scheduler.listTerminated, &proc->stsQueue);

    if (proc == scheduler.currentProcess) ProcessDoBlock(PROC_TERM);
    else proc->status = PROC_TERM;

    ProcessUnlockAndSchedule();
}

