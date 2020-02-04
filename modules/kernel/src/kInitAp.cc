//===================================================================================================================
//
//  kInitAp.cc -- Kernel entry point for each AP -- bypassing the major structure init
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Feb-03  Initial  v0.5.0f  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "cpu.h"
#include "timer.h"
#include "process.h"
#include "heap.h"
#include "serial.h"


//
// -- This is AP Entry point.  While we have a shared temporary stack and need to get that
//    -----------------------------------------------------------------------------------------------------
extern "C" EXPORT KERNEL
void kInitAp(void)
{
    ArchLateCpuInit(cpus.cpuStarting);
    NextCpu(cpus.cpuStarting);

    ApTimerInit(timerControl, 1000);

    // -- TODO: this may not work now!
    kprintf("CPU %x running...\n", thisCpu->cpuNum);
while (true) {}
    Process_t *proc = NEW(Process_t);
    assert(proc != NULL);

    proc->pid = scheduler.nextPID ++;
    proc->command = NULL;
    proc->policy = POLICY_0;
    proc->priority = PTY_OS;
    proc->status = PROC_RUNNING;
    AtomicSet(&proc->quantumLeft, PTY_OS);
    proc->timeUsed = 0;
    ListInit(&proc->stsQueue);
    proc->ssAddr = 0;

    // -- Now we immediately self-terminate to give the scheduler to something else
    EnableInterrupts();
    ProcessTerminate(proc);

    while (true) {}
}