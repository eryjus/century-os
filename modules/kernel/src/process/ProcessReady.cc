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
    archsize_t flags;

    switch(proc->priority) {
    case PTY_OS:
        flags = SPINLOCK_BLOCK_NO_INT(scheduler.queueOS.lock) {
            Enqueue(&scheduler.queueOS, &proc->stsQueue);
            SPINLOCK_RLS_RESTORE_INT(scheduler.queueOS.lock, flags);
        }

        break;

    case PTY_HIGH:
        flags = SPINLOCK_BLOCK_NO_INT(scheduler.queueHigh.lock) {
            Enqueue(&scheduler.queueHigh, &proc->stsQueue);
            SPINLOCK_RLS_RESTORE_INT(scheduler.queueHigh.lock, flags);
        }

        break;

    default:
        // in this case, we have a priority that is not right; assume normal from now on
        proc->priority = PTY_NORM;
        // ...  fall through

    case PTY_NORM:
        flags = SPINLOCK_BLOCK_NO_INT(scheduler.queueNormal.lock) {
            Enqueue(&scheduler.queueNormal, &proc->stsQueue);
            SPINLOCK_RLS_RESTORE_INT(scheduler.queueNormal.lock, flags);
        }

        break;

    case PTY_LOW:
        flags = SPINLOCK_BLOCK_NO_INT(scheduler.queueLow.lock) {
            Enqueue(&scheduler.queueLow, &proc->stsQueue);
            SPINLOCK_RLS_RESTORE_INT(scheduler.queueLow.lock, flags);
        }

        break;
    }

    CLEAN_PROCESS(proc);
}

