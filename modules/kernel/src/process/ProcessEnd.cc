//===================================================================================================================
//
// ProcessEnd.cc -- End a the current process by placing it on the Term queue and blocking
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
// -- End current process
//    -------------------
EXTERN_C EXPORT KERNEL
void ProcessEnd(void)
{
    ProcessLockAndPostpone();

    Process_t *proc = currentThread;
    assert(proc->stsQueue.next == &proc->stsQueue);
    Enqueue(&scheduler.listTerminated, &proc->stsQueue);
    ProcessDoBlock(PROC_TERM);
    ProcessUnlockAndSchedule();
}

