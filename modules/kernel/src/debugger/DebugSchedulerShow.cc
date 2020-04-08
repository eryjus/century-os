//===================================================================================================================
//
//  DebugSchedulerShow.cc -- List all running processes
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-07  Initial  v0.6.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "process.h"
#include "printf.h"
#include "debugger.h"


//
// -- Print the details of this process
//    ---------------------------------
EXTERN_C HIDDEN KERNEL
void PrintProcessRow(Process_t *proc)
{
    kprintf("| " ANSI_ATTR_BOLD);
    DbgSpace(25, kprintf("%s ", proc->command));
    kprintf(ANSI_ATTR_NORMAL);
    DbgSpace( 8, kprintf("| %d ", proc->pid));
    DbgSpace(10, kprintf("| %s ", ProcPriorityStr(proc->priority)));
    DbgSpace(10, kprintf("| %s ", ProcStatusStr(proc->status)));
    DbgSpace(12, kprintf("| %p ", proc));
    DbgSpace(20, kprintf("| %p %p ", (uint32_t)(proc->timeUsed >> 32), (uint32_t)proc->timeUsed));
    kprintf("|\n");
}


//
// -- Show the status of the scheduler queues
//    ---------------------------------------
EXTERN_C EXPORT KERNEL
void DebugSchedulerShow(void)
{
    DebuggerEngage(DIPI_ENGAGE);

    kprintf(ANSI_CLEAR ANSI_SET_CURSOR(0,0));
    kprintf("+---------------------------+--------+----------+----------+------------+-----------------------+\n");
    kprintf("| " ANSI_ATTR_BOLD ANSI_FG_BLUE "Command" ANSI_ATTR_NORMAL "                   | "
            ANSI_ATTR_BOLD ANSI_FG_BLUE "PID" ANSI_ATTR_NORMAL "    | " ANSI_ATTR_BOLD ANSI_FG_BLUE "Priority"
            ANSI_ATTR_NORMAL " | " ANSI_ATTR_BOLD ANSI_FG_BLUE "Status" ANSI_ATTR_NORMAL "   | "
            ANSI_ATTR_BOLD ANSI_FG_BLUE "Address" ANSI_ATTR_NORMAL "    | " ANSI_ATTR_BOLD ANSI_FG_BLUE
            "Time Used" ANSI_ATTR_NORMAL "             |\n");
    kprintf("+---------------------------+--------+----------+----------+------------+-----------------------+\n");

    ListHead_t::List_t *wrk = scheduler.globalProcesses.list.next;

    while (wrk != &scheduler.globalProcesses.list) {
        Process_t *proc = FIND_PARENT(wrk, Process_t, globalList);

        PrintProcessRow(proc);

        wrk = wrk->next;
    }

    kprintf("+---------------------------+--------+----------+----------+------------+-----------------------+\n");

    DebuggerRelease();
}

