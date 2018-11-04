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
#include "lists.h"
#include "spinlock.h"
#include "process.h"


//
// -- Determine the next process to run and reschedule to it voluntarily
//    ------------------------------------------------------------------
void ProcessReschedule(void)
{
    Process_t *currentProc = procs[currentPID];

    SPIN_BLOCK(readyQueueLock) {
        if (!IsListEmpty(&procOsPtyList)) {
            Process_t *tgt = FIND_PARENT(procOsPtyList.next,Process_t,stsQueue);
            if (tgt == currentProc) return;
            ListMoveTail(&tgt->stsQueue, &procOsPtyList);
            SpinlockUnlock(&readyQueueLock);
            ProcessSwitch(currentProc, tgt);
        } else if (!IsListEmpty(&procHighPtyList)) {
            Process_t *tgt = FIND_PARENT(procHighPtyList.next,Process_t,stsQueue);
            if (tgt == currentProc) return;
            ListMoveTail(&tgt->stsQueue, &procHighPtyList);
            SpinlockUnlock(&readyQueueLock);
            ProcessSwitch(currentProc, tgt);
        } else if (!IsListEmpty(&procNormPtyList)) {
            Process_t *tgt = FIND_PARENT(procNormPtyList.next,Process_t,stsQueue);
            if (tgt == currentProc) return;
            ListMoveTail(&tgt->stsQueue, &procNormPtyList);
            SpinlockUnlock(&readyQueueLock);
            ProcessSwitch(currentProc, tgt);
        } else if (!IsListEmpty(&procLowPtyList)) {
            Process_t *tgt = FIND_PARENT(procLowPtyList.next,Process_t,stsQueue);
            if (tgt == currentProc) return;
            ListMoveTail(&tgt->stsQueue, &procLowPtyList);
            SpinlockUnlock(&readyQueueLock);
            ProcessSwitch(currentProc, tgt);
        } else {
            Process_t *tgt = FIND_PARENT(procIdlePtyList.next,Process_t,stsQueue);
            if (tgt == currentProc) return;
            ListMoveTail(&tgt->stsQueue, &procIdlePtyList);
            SpinlockUnlock(&readyQueueLock);
            ProcessSwitch(currentProc, tgt);
        }
    }
}