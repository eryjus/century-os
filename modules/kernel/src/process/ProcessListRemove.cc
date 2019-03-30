//===================================================================================================================
//
// ProcessListRemove.cc -- Remove a process from whatever list it is on
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
void __krntext ProcessListRemove(Process_t *proc)
{
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
            SPIN_BLOCK(scheduler.listBlocked.lock) {
                ListRemoveInit(&proc->stsQueue);
                SpinlockUnlock(&scheduler.listBlocked.lock);
            }

            break;

        case PROC_READY:
            switch (proc->priority) {
            case PTY_OS:
                SPIN_BLOCK(scheduler.queueOS.lock) {
                    ListRemoveInit(&proc->stsQueue);
                    SpinlockUnlock(&scheduler.queueOS.lock);
                }

                break;

            case PTY_HIGH:
                SPIN_BLOCK(scheduler.queueHigh.lock) {
                    ListRemoveInit(&proc->stsQueue);
                    SpinlockUnlock(&scheduler.queueHigh.lock);
                }

                break;

            case PTY_LOW:
                SPIN_BLOCK(scheduler.queueLow.lock) {
                    ListRemoveInit(&proc->stsQueue);
                    SpinlockUnlock(&scheduler.queueLow.lock);
                }

                break;

            default:
                SPIN_BLOCK(scheduler.queueNormal.lock) {
                    ListRemoveInit(&proc->stsQueue);
                    SpinlockUnlock(&scheduler.queueNormal.lock);
                }

                break;
            }

            break;

        default:
            // do nothing
            break;
        }
    }
}


