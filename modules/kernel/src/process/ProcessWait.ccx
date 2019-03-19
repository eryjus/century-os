//===================================================================================================================
//
//  ProcessWait.cc -- Put the process on the wait list and set the current status accordingly.
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function will will never be called by a user process directly.  But with that said, it will also never be
//  called by any other process.  In other words, it will only be called indirectly by the currently running process.
//
//  This makes the checking easier to accomplish.
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

#include <errno.h>


//
// -- Put the process on the wait queue and set its status
//    ----------------------------------------------------
int ProcessWait(ProcStatus_t newStat)
{
    Process_t *proc;

    kprintf("ProcessWait(): Checking\n");
    SANITY_CHECK_PID(currentPID, proc);
    SPIN_BLOCK(proc->lock) {
        if (proc->isHeld) {
            SPIN_RLS(proc->lock);
            return -EUNDEF;
        }

        kprintf("ProcessWait(): Process lock obtained\n");

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


        kprintf("ProcessWait(): removed from ready queue\n");

        SPIN_BLOCK(procWaitList.lock) {
            ListAddTail(&procWaitList, &proc->stsQueue);
            SPIN_RLS(procWaitList.lock);
        }

        kprintf("ProcessWait(): added to wait list\n");

        proc->status = newStat;
        SPIN_RLS(proc->lock);
    }

    kprintf("ProcessWait(): released process lock\n");

    ProcessReschedule();

    return SUCCESS;
}
