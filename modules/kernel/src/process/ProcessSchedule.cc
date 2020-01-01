//===================================================================================================================
//
// ProcessSchedule.cc -- Select the next process to schedule and switch to it
//
//        Copyright (c)  2017-2020 -- Adam Clark
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
HIDDEN KERNEL
Process_t *ProcessNext(void)
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
EXPORT KERNEL
void ProcessSchedule(void)
{
    assert_msg(AtomicRead(&scheduler.schedulerLockCount) > 0,
            "Calling `ProcessSchedule()` without holding the proper lock");

    Process_t *next = NULL;
    ProcessUpdateTimeUsed();

    if (AtomicRead(&scheduler.postponeCount) != 0) {
        scheduler.processChangePending = true;
        return;
    }

    next = ProcessNext();
    if (next != NULL) {
        ProcessListRemove(next);
        ProcessSwitch(next);
    } else if (scheduler.currentProcess->status == PROC_RUNNING) {
        // -- Do nothing; the current process can continue; reset quantum
        AtomicAdd(&scheduler.currentProcess->quantumLeft, scheduler.currentProcess->priority);
        return;
    } else {
        // -- No tasks available; so we go into idle mode
        Process_t *save = scheduler.currentProcess;       // we will save this process for later
        scheduler.currentProcess = NULL;                  // nothing is running!

        do {
            // -- -- temporarily unlock the scheduler and enable interrupts for the timer to fire
            ProcessUnlockScheduler();
            EnableInterrupts();
            HaltCpu();
            DisableInterrupts();
            ProcessLockScheduler(false);     // make sure that this does not overwrite the process's flags
            next = ProcessNext();
        } while (next == NULL);
        ProcessListRemove(next);

        // -- restore the current Process and change if needed
        ProcessUpdateTimeUsed();
        scheduler.currentProcess = save;
        AtomicSet(&next->quantumLeft, next->priority);

        if (next != scheduler.currentProcess) ProcessSwitch(next);
    }
}

