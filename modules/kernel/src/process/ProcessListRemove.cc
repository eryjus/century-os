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
    archsize_t flags;


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
            flags = SPINLOCK_BLOCK_NO_INT(scheduler.listBlocked.lock) {
                ListRemoveInit(&proc->stsQueue);
                SPINLOCK_RLS_RESTORE_INT(scheduler.listBlocked.lock, flags);
            }

            break;

        case PROC_READY:
            switch (proc->priority) {
            case PTY_OS:
                flags = SPINLOCK_BLOCK_NO_INT(scheduler.queueOS.lock) {
                    ListRemoveInit(&proc->stsQueue);
                    SPINLOCK_RLS_RESTORE_INT(scheduler.queueOS.lock, flags);
                }

                break;

            case PTY_HIGH:
                flags = SPINLOCK_BLOCK_NO_INT(scheduler.queueHigh.lock) {
                    ListRemoveInit(&proc->stsQueue);
                    SPINLOCK_RLS_RESTORE_INT(scheduler.queueHigh.lock, flags);
                }

                break;

            case PTY_LOW:
                flags = SPINLOCK_BLOCK_NO_INT(scheduler.queueLow.lock) {
                    ListRemoveInit(&proc->stsQueue);
                    SPINLOCK_RLS_RESTORE_INT(scheduler.queueLow.lock, flags);
                }

                break;

            default:
                flags = SPINLOCK_BLOCK_NO_INT(scheduler.queueNormal.lock) {
                    ListRemoveInit(&proc->stsQueue);
                    SPINLOCK_RLS_RESTORE_INT(scheduler.queueNormal.lock, flags);
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


