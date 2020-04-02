//===================================================================================================================
//
// ProcessCheckQueue.cc -- Debugging function used to output the state of the scheduler queues
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-28  Initial  v0.5.1a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "lists.h"
#include "printf.h"
#include "process.h"


//
// -- Output the state of the scheduler
//    ---------------------------------
void ProcessDoCheckQueue(void)
{
    ProcessLockAndPostpone();
    kprintf("Dumping the status of the scheduler on CPU%d\n", thisCpu->cpuNum);
    kprintf("The scheduler is %s\n", schedulerLock.lock?"locked":"unlocked");
    if (schedulerLock.lock) kprintf("...  on CPU%d\n", scheduler.lockCpu);
    assert(schedulerLock.lock != 0);
    assert(scheduler.lockCpu == thisCpu->cpuNum);
    kprintf(".. postpone count %d\n", AtomicRead(&scheduler.postponeCount));
    kprintf(".. currently, a reschedule is %spending\n", scheduler.processChangePending ? "" : "not ");
    kprintf("..     OS Queue process count: %d\n", ListCount(&scheduler.queueOS));
    kprintf("..   High Queue process count: %d\n", ListCount(&scheduler.queueHigh));
    kprintf(".. Normal Queue process count: %d\n", ListCount(&scheduler.queueNormal));
    kprintf("..    Low Queue process count: %d\n", ListCount(&scheduler.queueLow));
    kprintf("..   Idle Queue process count: %d\n", ListCount(&scheduler.queueIdle));
    kprintf(".. There are %d processes on the terminated list\n", ListCount(&scheduler.listTerminated));
    ProcessUnlockAndSchedule();
}


