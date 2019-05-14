//===================================================================================================================
//
//  ProcessInit.cc -- Initialize the process structures
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-21  Initial   0.2.0   ADCL  Initial version
//  2018-Dec-02  Initial   0.2.0   ADCL  Provide the details of this function
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//  2019-Mar-15  Initial   0.3.2   ADCL  Update for new Process_t
//
//===================================================================================================================


#include "cpu.h"
#include "heap.h"
#include "timer.h"
#include "process.h"


//
// -- we need the `mmuLvl1Table` variable from the loader, but do not want to include all of
//    what we did in the loader
//    --------------------------------------------------------------------------------------
extern archsize_t mmuLvl1Table;


//
// -- Initialize the process structures
//    ---------------------------------
void __krntext ProcessInit(void)
{
    extern uint64_t lastTimer;

    kprintf("ProcessInit(): mmuLvl1Table = %p\n", mmuLvl1Table);
    scheduler.currentProcess = NEW(Process_t);

    scheduler.currentProcess->topOfStack = 0;
    scheduler.currentProcess->virtAddrSpace = mmuLvl1Table;
    scheduler.currentProcess->pid = scheduler.nextPID ++;          // -- this is the butler process ID
    scheduler.currentProcess->ssAddr = STACK_LOCATION;
    scheduler.currentProcess->command = NULL;
    scheduler.currentProcess->policy = POLICY_0;
    scheduler.currentProcess->priority = PTY_OS;
    scheduler.currentProcess->status = PROC_RUNNING;
    AtomicSet(&scheduler.currentProcess->quantumLeft, PTY_OS);
    scheduler.currentProcess->timeUsed = 0;
    ListInit(&scheduler.currentProcess->stsQueue);
    CLEAN_SCHEDULER();
    CLEAN_PROCESS(scheduler.currentProcess);

    lastTimer = TimerCurrentCount(timerControl);

    kprintf("The address of the OS queue list is %p\n", &scheduler.queueOS.list);
    kprintf(".. and its next pointer is %p\n", scheduler.queueOS.list.next);
}
