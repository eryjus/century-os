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
void __krntext ProcessReady(Process_t *proc)
{
    switch(proc->priority) {
    case PTY_OS:
        SPIN_BLOCK(scheduler.queueOS.lock) {
            Enqueue(&scheduler.queueOS, &proc->stsQueue);
            SpinlockUnlock(&scheduler.queueOS.lock);
        }

        break;

    case PTY_HIGH:
        SPIN_BLOCK(scheduler.queueHigh.lock) {
            Enqueue(&scheduler.queueHigh, &proc->stsQueue);
            SpinlockUnlock(&scheduler.queueHigh.lock);
        }

        break;

    default:
        // in this case, we have a priority that is not right; assume normal from now on
        proc->priority = PTY_NORM;
        // ...  fall through

    case PTY_NORM:
        SPIN_BLOCK(scheduler.queueNormal.lock) {
            Enqueue(&scheduler.queueNormal, &proc->stsQueue);
            SpinlockUnlock(&scheduler.queueNormal.lock);
        }

        break;

    case PTY_LOW:
        SPIN_BLOCK(scheduler.queueLow.lock) {
            Enqueue(&scheduler.queueLow, &proc->stsQueue);
            SpinlockUnlock(&scheduler.queueLow.lock);
        }

        break;
    }

    CLEAN_PROCESS(proc);
}

