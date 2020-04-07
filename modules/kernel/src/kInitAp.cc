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
#include "platform.h"
#include "timer.h"
#include "process.h"
#include "heap.h"
#include "pic.h"
#include "entry.h"
#include "serial.h"

extern void AtomicsTest(void);
//
// -- This is AP Entry point.  While we have a shared temporary stack and need to get that
//    -----------------------------------------------------------------------------------------------------
extern "C" EXPORT KERNEL
void kInitAp(void)
{
    ArchLateCpuInit(cpus.cpuStarting);
    PlatformApInit();

    ApTimerInit(timerControl, 1000);

    kprintf("CPU %x running...\n", thisCpu->cpuNum);
    Process_t *proc = NEW(Process_t);
    assert(proc != NULL);

    proc->pid = scheduler.nextPID ++;
    proc->ssAddr = thisCpu->stackFrame;
    proc->virtAddrSpace = mmuLvl1Table;

    // -- set the process name
    proc->command = (char *)HeapAlloc(20, false);
    kMemSetB(proc->command, 0, 20);
    kStrCpy(proc->command, "kInitAp( )");
    proc->command[8] = thisCpu->cpuNum + '0';

    proc->policy = POLICY_0;
    proc->priority = PTY_OS;
    proc->status = PROC_RUNNING;
    AtomicSet(&proc->quantumLeft, PTY_OS);
    proc->timeUsed = 0;
    proc->wakeAtMicros = 0;
    ListInit(&proc->stsQueue);

    kprintf("kInitAp() established the current process at %p for CPU%d\n", proc, thisCpu->cpuNum);

//    ProcessCheckQueue();
    CurrentThreadAssign(proc);
    kprintf("Assigning the starting timer for CPU%d\n", thisCpu->cpuNum);
    thisCpu->lastTimer = TimerCurrentCount(timerControl);

    // -- Now we immediately self-terminate to give the scheduler to something else
    kprintf("Enabling interrupts on CPU %d\n",  thisCpu->cpuNum);
    kprintf("Cpus running is %d\n", cpus.cpusRunning);
    EnableInterrupts();
    kprintf("Interrupts enabled on CPU %d\n",  thisCpu->cpuNum);
    NextCpu(cpus.cpuStarting);
//    AtomicsTest();
    kprintf("CPU%d signalled the next CPU to start\n",  thisCpu->cpuNum);

while (true) {}
    ProcessTerminate(currentThread);

    assert(false);
    while (true) {}
}