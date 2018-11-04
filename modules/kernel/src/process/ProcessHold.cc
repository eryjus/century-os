//===================================================================================================================
//
// ProcessHold.cc -- Hold a process by putting it on the held queue
//
// Perform a number of sanity checks and then remove the process from its current queue and then add it to the
// procHeldList.
//
// Since this may be invoked as part of a user application, we need to assume that it will pass in invalid PIDs in
// crazy states.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-14  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "printf.h"
#include "spinlock.h"
#include "process.h"

#include <errno.h>


//
// -- Hold a process by putting it on the hold list and update the hold flag
//    ----------------------------------------------------------------------
int ProcessHold(PID_t pid)
{
    Process_t *proc;

    SANITY_CHECK_PID(pid, proc);
    SPIN_BLOCK(proc->lock) {
        if (proc->isHeld) {
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
            SPIN_BLOCK(waitListLock) {
                ListDelInit(&proc->stsQueue);
                SpinlockUnlock(&waitListLock);
            }

            break;

        case PROC_RUN:
            SPIN_BLOCK(readyQueueLock) {
                ListDelInit(&proc->stsQueue);
                SpinlockUnlock(&readyQueueLock);
            }

            break;

        default:
            SpinlockUnlock(&proc->lock);
            return -EUNDEF;
        }

        proc->isHeld = true;
        SPIN_BLOCK(heldListLock) {
            ListAddTail(&procHeldList, &proc->stsQueue);
            SpinlockUnlock(&heldListLock);
        }

        SpinlockUnlock(&proc->lock);
    }

    if (pid == currentPID) ProcessReschedule();

    return SUCCESS;
}
