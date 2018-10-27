//===================================================================================================================
//
// ProcessTerminate.cc -- Terminate a process and put it on the Reaper queue
//
// This will operate on the process regardless of status.
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
// -- Terminate a process
//    -------------------
Errors_t ProcessTerminate(PID_t pid)
{
    Process_t *proc;

    SANITY_CHECK_PID(pid, proc);
    SPIN_BLOCK(proc->lock) {
        if (proc->status == PROC_END || proc->status == PROC_ZOMB) {
            ERROR_8000000A(pid);
            SpinlockUnlock(&proc->lock);
            return ERR_8000000A;
        }

        if (proc->isHeld) {
            SPIN_BLOCK(heldListLock) {
                ListDelInit(&procHeldList);
                SpinlockUnlock(&heldListLock);
            }
        } else {
            switch(proc->status) {
            case PROC_DLYW:
            case PROC_MSGW:
            case PROC_MTXW:
            case PROC_SEMW:
                SPIN_BLOCK(heldListLock) {
                    ListDelInit(&proc->stsQueue);
                    SpinlockUnlock(&heldListLock);
                }

                break;

            case PROC_RUN:
                SPIN_BLOCK(readyQueueLock) {
                    ListDelInit(&proc->stsQueue);
                    SpinlockUnlock(&readyQueueLock);
                }

                break;

            default:
                break;
            }
        }

        SPIN_BLOCK(reaperQueueLock) {
            ListAddTail(&procReaper, &proc->stsQueue);
            SpinlockUnlock(&reaperQueueLock);
        }

        proc->status = PROC_END;
        SpinlockUnlock(&proc->lock);
    }

    if (pid == currentPID) ProcessReschedule();

    return SUCCESS;
}
