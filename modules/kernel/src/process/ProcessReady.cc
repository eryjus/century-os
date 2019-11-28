//===================================================================================================================
//
// ProcessReady.cc -- Ready a process by putting it on the proper ready queue
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-30  Initial   0.3.2   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "lists.h"
#include "process.h"


//
// -- Make a process ready to run
//    ---------------------------
EXPORT KERNEL
void ProcessDoReady(Process_t *proc)
{
    proc->status = PROC_READY;
    switch(proc->priority) {
    case PTY_OS:
        Enqueue(&scheduler.queueOS, &proc->stsQueue);
        break;

    case PTY_HIGH:
        Enqueue(&scheduler.queueHigh, &proc->stsQueue);
        break;

    default:
        // in this case, we have a priority that is not right; assume normal from now on
        proc->priority = PTY_NORM;
        // ...  fall through

    case PTY_NORM:
        Enqueue(&scheduler.queueNormal, &proc->stsQueue);
        break;

    case PTY_LOW:
        Enqueue(&scheduler.queueLow, &proc->stsQueue);
        break;
    }
}

