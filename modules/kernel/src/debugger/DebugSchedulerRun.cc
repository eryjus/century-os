//===================================================================================================================
//
//  DebugSchedulerRun.cc -- Dump the information from the processes currently running on each CPU
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The 'interesting' bits of information I want to dump are:
//  * Process Address
//  * PID
//  * Command
//  * Virtual Address Space
//  * Base Stack frame
//  * Status
//  * Priority
//  * Quantum left
//  * Time Used
//  * Wake At (should be 0 or -1 -- I cannot recall)
//  * Whether it is on a queue
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-04  Initial  v0.6.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "serial.h"
#include "process.h"
#include "debugger.h"


//
// -- This function will clear the screen and print out the data headings
//    -------------------------------------------------------------------
EXTERN_C HIDDEN KERNEL
void PrintHeadings(void)
{
#define B ANSI_ATTR_BOLD ANSI_FG_BLUE
#define N ANSI_ATTR_NORMAL

    kprintf(ANSI_CLEAR ANSI_SET_CURSOR(0,0));
    kprintf("+------------------------+\n");
    kprintf("| " B "CPU" N "                    |\n");
    kprintf("+------------------------+\n");
    kprintf("| " B "Process Address:" N "       |\n");
    kprintf("| " B "Process ID:" N "            |\n");
    kprintf("| " B "Command:" N "               |\n");
    kprintf("| " B "Virtual Address Space:" N " |\n");
    kprintf("| " B "Base Stack Frame:" N "      |\n");
    kprintf("| " B "Status:" N "                |\n");
    kprintf("| " B "Priority:" N "              |\n");
    kprintf("| " B "Quantum Left:" N "          |\n");
    kprintf("| " B "Time Used:" N "             |\n");
    kprintf("| " B "Wake At:" N "               |\n");
    kprintf("| " B "Queue Status:" N "          |\n");
    kprintf("+------------------------+\n");

#undef B
#undef N
}


//
// -- Output the interesting values for a CPU
//    ---------------------------------------
EXTERN_C HIDDEN KERNEL
void PrintProcess(int cpu, volatile Process_t *proc)
{
    if (!proc) return;

    int fwd = (cpu * 20) + 25;

    kprintf(ANSI_SET_CURSOR(0,0));
    kprintf("\x1b[%dC+-------------------+\n", fwd);
    kprintf("\x1b[%dC|        " ANSI_ATTR_BOLD "CPU%d" ANSI_ATTR_NORMAL "       |\n", fwd, cpu);
    kprintf("\x1b[%dC+-------------------+\n", fwd);
    kprintf("\x1b[%dC| ", fwd); DbgSpace(17, kprintf("%x", proc)); kprintf("|\n");
    kprintf("\x1b[%dC| ", fwd); DbgSpace(17, kprintf("%d", proc->pid)); kprintf("|\n");
    kprintf("\x1b[%dC| ", fwd); DbgSpace(17, kprintf("%s", proc->command)); kprintf("|\n");
    kprintf("\x1b[%dC| ", fwd); DbgSpace(17, kprintf("%x", proc->virtAddrSpace)); kprintf("|\n");
    kprintf("\x1b[%dC| ", fwd); DbgSpace(17, kprintf("%x", proc->ssAddr)); kprintf("|\n");
    kprintf("\x1b[%dC| ", fwd); DbgSpace(17, kprintf("%s", ProcStatusStr(proc->status))); kprintf("|\n");
    kprintf("\x1b[%dC| ", fwd); DbgSpace(17, kprintf("%s", ProcPriorityStr(proc->priority))); kprintf("|\n");
    kprintf("\x1b[%dC| ", fwd); DbgSpace(17, kprintf("%d", AtomicRead(&proc->quantumLeft))); kprintf("|\n");
    kprintf("\x1b[%dC| ", fwd); DbgSpace(17, kprintf("%d", (uint32_t)proc->timeUsed)); kprintf("|\n");
    kprintf("\x1b[%dC| ", fwd); DbgSpace(17, kprintf("%d", (uint32_t)proc->wakeAtMicros)); kprintf("|\n");

    if (proc->stsQueue.next == &proc->stsQueue) {
        kprintf("\x1b[%dC| " ANSI_FG_GREEN ANSI_ATTR_BOLD "Not on a queue" ANSI_ATTR_NORMAL "    |\n", fwd);
    } else {
        kprintf("\x1b[%dC| " ANSI_FG_RED ANSI_ATTR_BOLD "On some queue" ANSI_ATTR_NORMAL "     |\n", fwd);
    }

    kprintf("\x1b[%dC+-------------------+\n", fwd);
}


//
// -- Dump the interesting values from the running processes on each CPU
//    ------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void DebugSchedulerRunning(void)
{
    DebuggerEngage(DIPI_ENGAGE);

    PrintHeadings();
    for (int i = 0; i < cpus.cpusRunning; i ++) {
        PrintProcess(i, cpus.perCpuData[i].process);
    }

    DebuggerRelease();
}


