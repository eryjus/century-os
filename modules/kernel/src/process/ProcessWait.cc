//===================================================================================================================
//
// ProcessWait.cc -- Put the process on the wait list and set the current status accordingly.
//
// This function will will never be called by a user process directly.  But with that said, it will also never be
// called by any other process.  In other words, it will only be called indirectly by the currently running process.
//
// This makes the checking easier to accomplish.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-26  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "errors.h"
#include "lists.h"
#include "spinlock.h"
#include "process.h"


//
// -- Put the process on the wait queue and set its status
//    ----------------------------------------------------
Errors_t ProcessRelease(ProcStatus_t newStat)
{
    Process_t *proc;

    SANITY_CHECK_PID(currentPID, proc);
    SPIN_BLOCK(proc->lock) {
        if (!proc->isHeld) {
            ERROR_80000007(currentPID);
            SpinlockUnlock(&proc->lock);
            return ERR_80000007;
        }

        SPIN_BLOCK(readyQueueLock) {
            ListDelInit(&proc->stsQueue);
            SpinlockUnlock(&readyQueueLock);
        }

        SPIN_BLOCK(waitListLock) {
            ListAddTail(&procWaitList, &proc->stsQueue);
            SpinlockUnlock(&waitListLock);
        }

        proc->status = newStat;
        SpinlockUnlock(&proc->lock);
    }

    ProcessReschedule();

    return SUCCESS;
}
