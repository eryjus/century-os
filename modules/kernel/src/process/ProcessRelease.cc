//===================================================================================================================
//
//  ProcessRelease.cc -- Release a process that has been previously held and put it on the proper queue.
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Perform a number of sanity checks and then remove the process from the held queue and then add it to the
//  proper new queue.
//
//  Since this may be invoked as part of a user application, we need to assume that it will pass in invalid PIDs in
//  crazy states.
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
            // -- we will ready the process here in case the wait condition was satisfied
        case PROC_RUN:
            SPIN_BLOCK(procHeldList.lock) {
                ListRemoveInit(&proc->stsQueue);
                SPIN_RLS(procHeldList.lock);
            }

            ProcessReady(pid);

            break;

        default:
            SPIN_RLS(proc->lock);
            return -EUNDEF;
        }

        proc->isHeld = false;
        SPIN_RLS(proc->lock);
    }

    if (pid != currentPID && proc->priority > procs[currentPID]->priority) ProcessSwitch(procs[currentPID], proc);

    return SUCCESS;
}
