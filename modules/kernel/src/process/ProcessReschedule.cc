//===================================================================================================================
//
// ProcessReschedule.cc -- Reschedule to the next task
//
// This function will determine the next task to run and voluntarily switch to it.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-24  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "lists.h"
#include "spinlock.h"
#include "process.h"


//
// -- Determine the next process to run and reschedule to it voluntarily
//    ------------------------------------------------------------------
void ProcessReschedule(void)
{
    kprintf("Looking for a new process\n");
    Process_t *currentProc = procs[currentPID];
    Process_t *tgt = NULL;
    const char *q = "";

    if (!IsListEmpty(&procOsPtyList)) {
        SPIN_BLOCK(procOsPtyList.lock) {
            tgt = FIND_PARENT(procOsPtyList.list.next,Process_t,stsQueue);
            ListMoveTail(&procOsPtyList, &tgt->stsQueue);
            SPIN_RLS(procOsPtyList.lock);
        }
        q = "OS";
    } else if (!IsListEmpty(&procHighPtyList)) {
        SPIN_BLOCK(procHighPtyList.lock) {
            tgt = FIND_PARENT(procHighPtyList.list.next,Process_t,stsQueue);
            ListMoveTail(&procHighPtyList, &tgt->stsQueue);
            SPIN_RLS(procHighPtyList.lock);
        }
        q = "HIGH";
    } else if (!IsListEmpty(&procNormPtyList)) {
        SPIN_BLOCK(procNormPtyList.lock) {
            tgt = FIND_PARENT(procNormPtyList.list.next,Process_t,stsQueue);
            ListMoveTail(&procNormPtyList, &tgt->stsQueue);
            SPIN_RLS(procNormPtyList.lock);
        }
        q = "NORM";
    } else if (!IsListEmpty(&procLowPtyList)) {
        SPIN_BLOCK(procLowPtyList.lock) {
            FIND_PARENT(procLowPtyList.list.next,Process_t,stsQueue);
            ListMoveTail(&procLowPtyList, &tgt->stsQueue);
            SPIN_RLS(procLowPtyList.lock);
        }
        q = "LOW";
    } else if (!IsListEmpty(&procIdlePtyList)) {
        SPIN_BLOCK(procIdlePtyList.lock) {
            tgt = FIND_PARENT(procIdlePtyList.list.next,Process_t,stsQueue);
            ListMoveTail(&procIdlePtyList, &tgt->stsQueue);
            SPIN_RLS(procIdlePtyList.lock);
        }
        q = "IDLE";
    } else {
        kprintf("PANIC: The queues are out of sync!!!\n");
        Halt();
    }

    if (tgt == currentProc) {
        kprintf("The current process is still the highest priority\n");
        currentProc->quantumLeft = currentProc->priority;
    } else {
        kprintf("%s: Executing process switch: %x\n", q, tgt->pid);
        currentPID = tgt->pid;
        ProcessSwitch(currentProc, tgt);
    }
}