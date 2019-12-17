
#include "types.h"
#include "printf.h"
#include "cpu.h"
#include "timer.h"
#include "process.h"
#include "heap.h"
#include "serial.h"


bool bootAp = false;

extern "C" {
    EXPORT LOADER void kInitAp(void);
}


//
// -- This is AP Entry point.  While we have a shared temporary stack and need to get that
//    -----------------------------------------------------------------------------------------------------
extern "C" EXPORT KERNEL
void kInitAp(void)
{
    FpuInit();

    TimerInit(timerControl, 1000);
    EnableInterrupts();

    kprintf("CPU %x running\n", CpuNum());

    Process_t *proc = NEW(Process_t);
    assert(proc != NULL);

    proc->pid = scheduler.nextPID ++;
    proc->command = NULL;
    proc->policy = POLICY_0;
    proc->priority = PTY_OS;
    proc->status = PROC_INIT;
    AtomicSet(&proc->quantumLeft, PTY_OS);
    proc->timeUsed = 0;
    ListInit(&proc->stsQueue);
    proc->ssAddr = 0;

    // -- Now we immediately self-terminate to give the scheduler to something else
    ProcessTerminate(proc);

    while (true) {}
}