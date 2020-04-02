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
EXTERN_C EXPORT KERNEL
void ProcessTerminate(Process_t *proc)
{
    if (!assert(proc != NULL)) return;

    ProcessLockAndPostpone();

    kprintf("Terminating process at address %p on CPU%d\n", proc, thisCpu->cpuNum);
    kprintf(".. this process is %sRunning\n", proc == currentThread ? "" : "not ");

    if (proc == currentThread) {
        kprintf(".. ending the current process\n");
        assert(proc->stsQueue.next == &proc->stsQueue);
        Enqueue(&scheduler.listTerminated, &proc->stsQueue);
        ProcessDoBlock(PROC_TERM);
    } else {
        kprintf(".. termianting another process\n");
        ProcessListRemove(proc);
        Enqueue(&scheduler.listTerminated, &proc->stsQueue);
        proc->status = PROC_TERM;
    }

    ProcessUnlockAndSchedule();
}

