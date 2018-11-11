//===================================================================================================================
//
//  ProcessEnd.cc -- Self-terminate a process by ending normally
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This will always be the running process.
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
#include "cpu.h"
#include "process.h"


//
// -- End a process normally
//    ----------------------
void ProcessEnd(void)
{
    Process_t *proc = procs[currentPID];

    SPIN_BLOCK(proc->lock) {
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
        }

        SPIN_BLOCK(procReaper.lock) {
            ListAddTail(&procReaper, &proc->stsQueue);
            SPIN_RLS(procReaper.lock);
        }

        proc->status = PROC_END;
        SPIN_RLS(proc->lock);
    }

    ProcessReschedule();

    //
    // -- This function should never get here
    //    -----------------------------------
    kprintf("Normally ended process did not end\n");
    Halt();
}
