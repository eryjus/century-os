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

    if (IsListEmpty(&roundRobin) == false) {
        Process_t *next = FIND_PARENT(roundRobin.list.next, Process_t, stsQueue);
        ListRemoveInit(&next->stsQueue);
        Enqueue(&roundRobin, &currentProcess->stsQueue);
        ProcessSwitch(next);
    }
}

