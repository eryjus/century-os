//===================================================================================================================
//
// ProcessReady.cc -- Place a process into the proper queue based on its priority.
//
// This process must be on the ready queue if it is on a queue at all.  Therefore it is the responsibility of any
// calling function to remove it from its queue if it is on it.
//
// Unless the process is ending, the process status will be updated to PROC_RUN.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-24  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "printf.h"
#include "spinlock.h"
#include "process.h"

#include <errno.h>


//
// -- Ready a process by placing it on the proper ready queue
//    -------------------------------------------------------
int ProcessReady(PID_t pid)
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

        case PROC_INIT:
        case PROC_DLYW:
        case PROC_MSGW:
        case PROC_MTXW:
        case PROC_SEMW:
        case PROC_RUN:
            SPIN_BLOCK(readyQueueLock) {
                if (proc->stsQueue.next) ListDelInit(&proc->stsQueue);

                switch(proc->priority) {
                case PTY_OS:
                    ListAddTail(&procOsPtyList, &proc->stsQueue);
                    break;

                case PTY_HIGH:
                    ListAddTail(&procHighPtyList, &proc->stsQueue);
                    break;

                case PTY_NORM:
                    ListAddTail(&procNormPtyList, &proc->stsQueue);
                    break;

                case PTY_LOW:
                    ListAddTail(&procLowPtyList, &proc->stsQueue);
                    break;

                case PTY_IDLE:
                    ListAddTail(&procIdlePtyList, &proc->stsQueue);
                    break;

                default:
                    SpinlockUnlock(&proc->lock);
                    SpinlockUnlock(&readyQueueLock);
                    return -EUNDEF;
                }

                SpinlockUnlock(&readyQueueLock);
            }

            break;

        default:
            SpinlockUnlock(&proc->lock);
            return -EUNDEF;
        }

        SpinlockUnlock(&proc->lock);
    }

    if (pid != currentPID && proc->priority > procs[currentPID]->priority) ProcessSwitch(procs[currentPID], proc);

    return SUCCESS;
}
