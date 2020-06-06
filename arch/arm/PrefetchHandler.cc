//===================================================================================================================
//
//  PrefetchHandler.cc -- Handle a prefetch abort
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Dec-01  Initial   0.2.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "cpu.h"
#include "mmu.h"
#include "interrupt.h"



//
// -- Pick up the causes from the Data Abort Handler
//    ----------------------------------------------
EXTERN KERNEL_DATA
const char *causes[];


//
// -- Handle a prefetch exception
//    ---------------------------
EXTERN_C EXPORT KERNEL
void PrefetchHandler(isrRegs_t *regs)
{
    archsize_t ifsr = ReadIFSR();
    int cause = (ifsr & 0x3f);

    kprintf("Prefetch Abort:\n");
    kprintf(".. Data Fault Address: %p\n", ReadIFAR());
    if (ifsr & (1<<12)) kprintf(".. External Abort\n");
    kprintf(".. LPAE is %s\n", ifsr & (1<<9) ? "enabled" : "disabled");
    kprintf(".. Data Fault Status Register: %p\n", ifsr);
    kprintf(".. Fault status %x: %s\n", cause, causes[cause]);

    MmuDumpTables(ReadIFAR());

    IsrDumpState(regs);
}
