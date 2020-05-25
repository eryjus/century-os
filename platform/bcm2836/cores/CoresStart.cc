//===================================================================================================================
//
//  CoresStart.cc -- Start the cores for the rpi2b
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Jan-04  Initial  v0.5.0d  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "timer.h"
#include "hardware.h"
#include "pic.h"
#include "printf.h"
#include "mmu.h"
#include "process.h"
#include "cpu.h"


//
// -- an definition for the entry point for the cores
//    -----------------------------------------------
EXTERN_C EXPORT KERNEL
void entryAp(void);


//
// -- start the other cores; remains in the kernel since we may want to do this later as well
//    ---------------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void CoresStart(void)
{
    if (cpus.cpusDiscovered < 2) return;

    cpus.perCpuData[0].location = ArchCpuLocation();

    for (int i = 1; i < cpus.cpusDiscovered; i ++) {
        cpus.cpuStarting = i;
        AtomicSet(&cpus.perCpuData[cpus.cpuStarting].state, CPU_STARTING);

        kprintf("Starting core with message to %p\n", IPI_MAILBOX_BASE + 0x0c + (0x10 * i));
        MmioWrite(IPI_MAILBOX_BASE + 0x0c + (0x10 * i), (uint32_t)entryAp);
        SEV();
        kprintf("..  waiting for core to start...\n");

        while (AtomicRead(&cpus.perCpuData[cpus.cpuStarting].state) == CPU_STARTING) { DMB(); }
    }
}

