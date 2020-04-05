//===================================================================================================================
//
//  DebugSchedulerStat.cc -- Show the status of the scheduler (counts)
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-03  Initial  v0.6.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "process.h"
#include "printf.h"
#include "debugger.h"


//
// -- Show the status of the scheduler queues
//    ---------------------------------------
EXTERN_C EXPORT KERNEL
void DebugSchedulerStat(void)
{
    DebuggerEngage(DIPI_ENGAGE);

    kprintf(ANSI_FG_BLUE ANSI_ATTR_BOLD "Dumping the status of the scheduler:\n" ANSI_ATTR_NORMAL);

    // -- check the overall status of the scheduler (technically should never be locked)
    if (schedulerLock.lock) {
        kprintf("The scheduler is " ANSI_FG_RED ANSI_ATTR_BOLD "locked" ANSI_ATTR_NORMAL " by CPU%d\n",
                scheduler.lockCpu);
        kprintf("   The process running on this CPU is %p (%s)\n",
                cpus.perCpuData[scheduler.lockCpu].process, cpus.perCpuData[scheduler.lockCpu].process->command);
    } else {
        kprintf("The scheduler is " ANSI_FG_GREEN ANSI_ATTR_BOLD "unlocked" ANSI_ATTR_NORMAL"\n");
    }


    // -- check each CPU for a running process
    for (int i = 0; i < cpus.cpusRunning; i ++) {
        if (cpus.perCpuData[i].process) {
            kprintf(ANSI_ATTR_BOLD "CPU%d" ANSI_FG_GREEN " does " ANSI_ATTR_NORMAL
                    "have a process running on it\n", i);
        } else {
            kprintf(ANSI_ATTR_BOLD "CPU%d" ANSI_FG_RED " does not " ANSI_ATTR_NORMAL
                    "have a process running on it\n", i);
        }
    }


    // -- check the status of each queue
    kprintf(ANSI_ATTR_BOLD "       OS Queue" ANSI_ATTR_NORMAL " process count: " ANSI_ATTR_BOLD "%d\n"
            ANSI_ATTR_NORMAL, ListCount(&scheduler.queueOS));
    kprintf(ANSI_ATTR_BOLD "     High Queue" ANSI_ATTR_NORMAL " process count: " ANSI_ATTR_BOLD "%d\n"
            ANSI_ATTR_NORMAL, ListCount(&scheduler.queueHigh));
    kprintf(ANSI_ATTR_BOLD "   Normal Queue" ANSI_ATTR_NORMAL " process count: " ANSI_ATTR_BOLD "%d\n"
            ANSI_ATTR_NORMAL, ListCount(&scheduler.queueNormal));
    kprintf(ANSI_ATTR_BOLD "      Low Queue" ANSI_ATTR_NORMAL " process count: " ANSI_ATTR_BOLD "%d\n"
            ANSI_ATTR_NORMAL, ListCount(&scheduler.queueLow));
    kprintf(ANSI_ATTR_BOLD "     Idle Queue" ANSI_ATTR_NORMAL " process count: " ANSI_ATTR_BOLD "%d\n"
            ANSI_ATTR_NORMAL, ListCount(&scheduler.queueIdle));
    kprintf(ANSI_ATTR_BOLD "   Blocked List" ANSI_ATTR_NORMAL " process count: " ANSI_ATTR_BOLD "%d\n"
            ANSI_ATTR_NORMAL, ListCount(&scheduler.listBlocked));
    kprintf(ANSI_ATTR_BOLD "  Sleeping List" ANSI_ATTR_NORMAL " process count: " ANSI_ATTR_BOLD "%d\n"
            ANSI_ATTR_NORMAL, ListCount(&scheduler.listSleeping));
    kprintf(ANSI_ATTR_BOLD "Terminated List" ANSI_ATTR_NORMAL " process count: " ANSI_ATTR_BOLD "%d\n"
            ANSI_ATTR_NORMAL, ListCount(&scheduler.listTerminated));

    DebuggerRelease();
}


