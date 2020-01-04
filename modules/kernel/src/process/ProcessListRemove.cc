//===================================================================================================================
//
// ProcessListRemove.cc -- Remove a process from whatever list it is on
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-30  Initial   0.3.2   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "lists.h"
#include "spinlock.h"
#include "process.h"


//
// -- Remove the process from whatever list it is on, ensuring proper locking
//    -----------------------------------------------------------------------
EXPORT KERNEL
void ProcessListRemove(Process_t *proc)
{
    if (!assert(proc != NULL)) return;


    // -- is it already not on a list?
    if (proc->stsQueue.next == &proc->stsQueue) return;


    //
    // -- Is this process on a queue?
    //    ---------------------------
    if (proc->status != PROC_RUNNING) {
        switch (proc->status) {
        case PROC_DLYW:
        case PROC_MSGW:
        case PROC_MTXW:
        case PROC_SEMW:
        case PROC_TERM:
            ListRemoveInit(&proc->stsQueue);
            break;

        case PROC_READY:
            switch (proc->priority) {
            case PTY_OS:
                ListRemoveInit(&proc->stsQueue);
                break;

            case PTY_HIGH:
                ListRemoveInit(&proc->stsQueue);
                break;

            case PTY_LOW:
                ListRemoveInit(&proc->stsQueue);
                break;

            default:
                ListRemoveInit(&proc->stsQueue);
                break;
            }

            break;

        default:
            // do nothing
            break;
        }
    }
}


