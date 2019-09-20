//===================================================================================================================
//
// ProcessTerminate.cc -- End a task by placing it on the terminated queue
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
void __krntext ProcessTerminate(Process_t *proc)
{
    ProcessEnterPostpone();

    ProcessListRemove(proc);

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(scheduler.listTerminated.lock) {
        Enqueue(&scheduler.listTerminated, &proc->stsQueue);
        SPINLOCK_RLS_RESTORE_INT(scheduler.listTerminated.lock, flags);
    }

    if (proc == scheduler.currentProcess) ProcessBlock(PROC_TERM);
    else proc->status = PROC_TERM;

    CLEAN_PROCESS(proc);

    ProcessExitPostpone();
}

