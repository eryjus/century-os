//===================================================================================================================
//
//  CpuInit.cc -- Initialize the cpu structures
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Jan-21  Initial  v0.5.0f  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "stacks.h"
#include "mmu.h"
#include "pmm.h"
#include "pic.h"
#include "cpu.h"


//
// -- Initialize the cpus structure
//    -----------------------------
EXTERN_C EXPORT LOADER
void CpuInit(void)
{
    ArchEarlyCpuInit();

    for (int i = 0; i < MAX_CPUS; i ++) {
        // -- start with this stack
        archsize_t stack = STACK_LOCATION;

        // -- other cores get a different stack
        if (i > 0) {
            stack = StackFind();
            frame_t frame = PmmAllocateFrame();
            MmuMapToFrame(stack, frame, PG_KRN | PG_WRT);
        }

        cpus.perCpuData[i].cpuNum = i;
        cpus.perCpuData[i].location = -1;        // will be filled in later
        cpus.perCpuData[i].stackTop = stack + STACK_SIZE;
        cpus.perCpuData[i].state = (i < cpus.cpusDiscovered ? CPU_STOPPED : CPU_BAD);
        cpus.perCpuData[i].kernelLocksHeld = 0;
        cpus.perCpuData[i].reschedulePending = false;
        cpus.perCpuData[i].disableIntDepth = 0;
        cpus.perCpuData[i].cpu = &cpus.perCpuData[i];
        cpus.perCpuData[i].process = NULL;

        kprintf("Calling per cpu(%d)\n", i);
        ArchPerCpuInit(i);
        kprintf("..back\n");
    }

    cpus.perCpuData[0].state = CPU_STARTED;
    cpus.cpusRunning = 1;
    cpus.cpuStarting = -1;
    // -- the location will be done in `CoresStart()`

    ArchLateCpuInit(0);

    kprintf("Done with CPU setup!!  CPU numer is %d\n", thisCpu->cpuNum);
}



