//===================================================================================================================
//
// ProcessRelease.cc -- Release a process that has been previously held and put it on the proper queue.
//
// Perform a number of sanity checks and then remove the process from the held queue and then add it to the
// proper new queue.
//
// Since this may be invoked as part of a user application, we need to assume that it will pass in invalid PIDs in
// crazy states.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-22  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "printf.h"
#include "spinlock.h"
#include "process.h"


//
// -- Release a process on the held list and place it in the proper queue
//    -------------------------------------------------------------------
int ProcessRelease(PID_t pid)
{
    Process_t *proc;

    SANITY_CHECK_PID(pid, proc);
    SPIN_BLOCK(proc->lock) {
        if (!proc->isHeld) {
            SpinlockUnlock(&proc->lock);
            return -EUNDEF;
        }

        switch(proc->status) {
        case PROC_END:
        case PROC_ZOMB:
            SpinlockUnlock(&proc->lock);
            return -EUNDEF;

        case PROC_DLYW:
        case PROC_MSGW:
        case PROC_MTXW:
        case PROC_SEMW:
            SPIN_BLOCK(heldListLock) {
                ListDelInit(&proc->stsQueue);
                SpinlockUnlock(&heldListLock);
            }

            SPIN_BLOCK(waitListLock) {
                ListAddTail(&procWaitList, &proc->stsQueue);
                SpinlockUnlock(&waitListLock);
            }

            break;

        case PROC_RUN:
            SPIN_BLOCK(heldListLock) {
                ListDelInit(&proc->stsQueue);
                SpinlockUnlock(&heldListLock);
            }

            ProcessReady(pid);

            break;

        default:
            SpinlockUnlock(&proc->lock);
            return -EUNDEF;
        }

        proc->isHeld = false;
        SpinlockUnlock(&proc->lock);
    }

    if (pid != currentPID && proc->priority > procs[currentPID]->priority) ProcessSwitch(procs[currentPID], proc);

    return SUCCESS;
}
