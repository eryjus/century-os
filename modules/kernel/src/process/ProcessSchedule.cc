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
// -- Find the next process to give the CPU to
//    ----------------------------------------
static Process_t *ProcessNext(void)
{
    if (IsListEmpty(&scheduler.queueOS) == false) {
        return FIND_PARENT(scheduler.queueOS.list.next, Process_t, stsQueue);
    } else if (IsListEmpty(&scheduler.queueHigh) == false) {
        return FIND_PARENT(scheduler.queueHigh.list.next, Process_t, stsQueue);
    } else if (IsListEmpty(&scheduler.queueNormal) == false) {
        return FIND_PARENT(scheduler.queueNormal.list.next, Process_t, stsQueue);
    } else if (IsListEmpty(&scheduler.queueLow) == false) {
        return FIND_PARENT(scheduler.queueLow.list.next, Process_t, stsQueue);
    } else {
        return NULL;
    }
}


//
// -- pick the next process to execute and execute it; ProcessLockScheduler() must be called before calling
//    -----------------------------------------------------------------------------------------------------
void __krntext ProcessSchedule(void)
{
    Process_t *next = NULL;
    ProcessUpdateTimeUsed();

    if (scheduler.schedulerLocksHeld != 0) {
        scheduler.processChangePending = true;
        return;
    }

    next = ProcessNext();
    if (next != NULL) {
        ProcessListRemove(next);
        CLEAN_PROCESS(next);

        if (scheduler.currentProcess->status == PROC_RUNNING) {
            ProcessReady(scheduler.currentProcess);
        }

        CLEAN_SCHEDULER();

        ProcessSwitch(next);
    } else if (scheduler.currentProcess->status == PROC_RUNNING) {
        // -- Do nothing; the current process can continue; reset quantum
        scheduler.currentProcess->quantumLeft += scheduler.currentProcess->priority;
    } else {
        // -- No tasks available; so we go into idle mode
        Process_t *save = scheduler.currentProcess;       // we will save this process for later
        scheduler.currentProcess = NULL;                  // nothing is running!

        do {
            // -- -- temporarily enable interrupts for the timer to fire
            CLEAN_SCHEDULER();
            EnableInterrupts();
            HaltCpu();
            DisableInterrupts();
            next = ProcessNext();
        } while (next == NULL);


        // -- restore the current Process and change if needed
        ProcessUpdateTimeUsed();
        scheduler.currentProcess = save;
        ProcessListRemove(next);

        CLEAN_PROCESS(next);
        CLEAN_SCHEDULER();
        if (next != scheduler.currentProcess) ProcessSwitch(next);
    }

    CLEAN_SCHEDULER();
}

