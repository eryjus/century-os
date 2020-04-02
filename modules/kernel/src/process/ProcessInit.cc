//===================================================================================================================
//
//  ProcessInit.cc -- Initialize the process structures
//
//        Copyright (c)  2017-2020 -- Adam Clark
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
EXPORT LOADER
void ProcessInit(void)
{
    extern uint64_t lastTimer;

    Process_t *proc = NEW(Process_t);
    CurrentThreadAssign(proc);

    if (!assert(proc != NULL)) {
        CpuPanicPushRegs("Unable to allocate Current Process structure");
    }

    proc->topOfStack = 0;
    proc->virtAddrSpace = mmuLvl1Table;
    proc->pid = scheduler.nextPID ++;          // -- this is the butler process ID
    proc->ssAddr = STACK_LOCATION;
    proc->command = NULL;
    proc->policy = POLICY_0;
    proc->priority = PTY_OS;
    proc->status = PROC_RUNNING;
    AtomicSet(&proc->quantumLeft, PTY_OS);
    proc->timeUsed = 0;
    ListInit(&proc->stsQueue);
    CLEAN_SCHEDULER();
    CLEAN_PROCESS(proc);

    kprintf("ProcessInit() established the current process at %p for CPU%d\n", proc, thisCpu->cpuNum);


    //
    // -- Create an idle process for each CPU
    //    -----------------------------------
    for (int i = 0; i < cpus.cpusDiscovered; i ++) {
        // -- Note ProcessCreate() creates processes at the OS priority...
        kprintf("starting idle process %d\n", i);
        ProcessCreate(ProcessIdle);
    }


    lastTimer = TimerCurrentCount(timerControl);
    kprintf("ProcessInit() complete\n");
}
