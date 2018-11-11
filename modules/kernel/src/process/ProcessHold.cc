//===================================================================================================================
//
//  ProcessHold.cc -- Hold a process by putting it on the held queue
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Perform a number of sanity checks and then remove the process from its current queue and then add it to the
//  procHeldList.
//
//  Since this may be invoked as part of a user application, we need to assume that it will pass in invalid PIDs in
//  crazy states.
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
            SPIN_RLS(proc->lock);
            return -EUNDEF;
        }

        switch(proc->status) {
        case PROC_END:
        case PROC_ZOMB:
            SPIN_RLS(proc->lock);
            return -EUNDEF;

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

            break;

        default:
            SPIN_RLS(proc->lock);
            return -EUNDEF;
        }

        proc->isHeld = true;
        SPIN_BLOCK(procHeldList.lock) {
            ListAddTail(&procHeldList, &proc->stsQueue);
            SPIN_RLS(procHeldList.lock);
        }

        SPIN_RLS(proc->lock);
    }

    if (pid == currentPID) ProcessReschedule();

    return SUCCESS;
}
