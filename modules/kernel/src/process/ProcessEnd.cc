//===================================================================================================================
//
// ProcessEnd.cc -- Self-terminate a process by ending normally
//
// This will always be the running process.
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
#include "cpu.h"
#include "process.h"


//
// -- End a process normally
//    ----------------------
void ProcessEnd(void)
{
    Process_t *proc = procs[currentPID];

    SPIN_BLOCK(proc->lock) {
        SPIN_BLOCK(readyQueueLock) {
            ListDelInit(&proc->stsQueue);
            SpinlockUnlock(&readyQueueLock);
        }

        SPIN_BLOCK(reaperQueueLock) {
            ListAddTail(&procReaper, &proc->stsQueue);
            SpinlockUnlock(&reaperQueueLock);
        }

        proc->status = PROC_END;
        SpinlockUnlock(&proc->lock);
    }

    ProcessReschedule();

    //
    // -- This function should never get here
    //    -----------------------------------
    ERROR_80000007(currentPID);
    Halt();
}
