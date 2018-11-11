//===================================================================================================================
//
//  ProcessTerminate.cc -- Terminate a process and put it on the Reaper queue
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This will operate on the process regardless of status.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-26  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "lists.h"
#include "spinlock.h"
#include "process.h"


//
// -- Terminate a process
//    -------------------
int ProcessTerminate(PID_t pid)
{
    Process_t *proc;

    SANITY_CHECK_PID(pid, proc);
    SPIN_BLOCK(proc->lock) {
        if (proc->status == PROC_END || proc->status == PROC_ZOMB) {
            SPIN_RLS(proc->lock);
            return -EUNDEF;
        }

        if (proc->isHeld) {
            SPIN_BLOCK(procHeldList.lock) {
                ListRemoveInit(&proc->stsQueue);
                SPIN_RLS(procHeldList.lock);
            }
        } else {
            switch(proc->status) {
            case PROC_DLYW:
            case PROC_MSGW:
            case PROC_MTXW:
            case PROC_SEMW:
                SPIN_BLOCK(procWaitList.lock) {
                    ListRemoveInit(&proc->stsQueue);
                    SPIN_RLS(procWaitList.lock);
                }

                break;

            case PROC_RUN:
                switch (proc->priority) {
                case PTY_OS:
                    SPIN_BLOCK(procOsPtyList.lock) {
                        ListRemoveInit(&proc->stsQueue);
                        SPIN_RLS(procOsPtyList.lock);
                    }
                    break;

                case PTY_HIGH:
                    SPIN_BLOCK(procHighPtyList.lock) {
                        ListRemoveInit(&proc->stsQueue);
                        SPIN_RLS(procHighPtyList.lock);
                    }
                    break;

                case PTY_NORM:
                    SPIN_BLOCK(procNormPtyList.lock) {
                        ListRemoveInit(&proc->stsQueue);
                        SPIN_RLS(procNormPtyList.lock);
                    }
                    break;

                case PTY_LOW:
                    SPIN_BLOCK(procLowPtyList.lock) {
                        ListRemoveInit(&proc->stsQueue);
                        SPIN_RLS(procLowPtyList.lock);
                    }
                    break;

                case PTY_IDLE:
                    SPIN_BLOCK(procIdlePtyList.lock) {
                        ListRemoveInit(&proc->stsQueue);
                        SPIN_RLS(procIdlePtyList.lock);
                    }
                    break;

                default:
                    SPIN_RLS(proc->lock);
                    return -EUNDEF;
                }

            default:
                break;
            }
        }

        SPIN_BLOCK(procReaper.lock) {
            ListAddTail(&procReaper, &proc->stsQueue);
            SPIN_RLS(procReaper.lock);
        }

        proc->status = PROC_END;
        SPIN_RLS(proc->lock);
    }

    if (pid == currentPID) ProcessReschedule();

    return SUCCESS;
}
