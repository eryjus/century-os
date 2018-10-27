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


#include "errors.h"
#include "printf.h"
#include "spinlock.h"
#include "process.h"


//
// -- Ready a process by placing it on the proper ready queue
//    -------------------------------------------------------
Errors_t ProcessReady(PID_t pid)
{
    Process_t *proc;

    SANITY_CHECK_PID(pid, proc);
    SPIN_BLOCK(proc->lock) {
        if (proc->isHeld) {
            ERROR_80000008(pid);
            SpinlockUnlock(&proc->lock);
            return ERR_80000008;
        }

        switch(proc->status) {
        case PROC_END:
        case PROC_ZOMB:
            ERROR_80000005(pid);
            SpinlockUnlock(&proc->lock);
            return ERR_80000005;

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
                    ERROR_80000009(pid);
                    SpinlockUnlock(&proc->lock);
                    SpinlockUnlock(&readyQueueLock);
                    return ERR_80000009;
                }

                SpinlockUnlock(&readyQueueLock);
            }

            break;

        default:
            ERROR_80000006(pid);
            SpinlockUnlock(&proc->lock);
            return ERR_80000006;
        }

        SpinlockUnlock(&proc->lock);
    }

    if (pid != currentPID && proc->priority > procs[currentPID]->priority) ProcessSwitch(procs[currentPID], proc);

    return SUCCESS;
}
