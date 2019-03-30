//===================================================================================================================
//
// ProcessSchedule.cc -- Select the next process to schedule and switch to it
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-18  Initial   0.3.2   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "lists.h"
#include "process.h"


//
// -- pick the next process to execute and execute it; ProcessLockScheduler() must be called before calling
//    -----------------------------------------------------------------------------------------------------
void __krntext ProcessSchedule(void)
{
    ProcessUpdateTimeUsed();

    if (schedulerLocksHeld != 0) {
        processChangePending = true;
        return;
    }

    if (IsListEmpty(&roundRobin) == false) {
        Process_t *next = FIND_PARENT(roundRobin.list.next, Process_t, stsQueue);
        ListRemoveInit(&next->stsQueue);
        if (currentProcess->status == PROC_RUNNING) Enqueue(&roundRobin, &currentProcess->stsQueue);
        ProcessSwitch(next);
    } else if (currentProcess->status == PROC_RUNNING) {
        // -- Do nothing; the current process can continue; reset quantum
        currentProcess->quantumLeft += currentProcess->priority;
    } else {
        // -- No tasks available; so we go into idle mode
        Process_t *save = currentProcess;       // we will save this process for later
        currentProcess = NULL;                  // nothing is running!

        do {
            // -- -- temporarily enable interrupts for the timer to fire
            EnableInterrupts();
            HaltCpu();
            DisableInterrupts();
        } while (IsListEmpty(&roundRobin) == true);


        // -- restore the current Process
        ProcessUpdateTimeUsed();
        currentProcess = save;
        Process_t *next = FIND_PARENT(roundRobin.list.next, Process_t, stsQueue);
        ListRemoveInit(&next->stsQueue);
        if (next != currentProcess) ProcessSwitch(next);
    }
}

