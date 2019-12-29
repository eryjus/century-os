//===================================================================================================================
//
//  PmmDumpState.cc -- Dump the current state of the PMM, respecting locks as this may crash the debugger
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function cannot be completed until I have a timeout for Spinlock.  The reason is that this function might
//  deadlock when running as part of the intended debugger.  We need to attempt an immediate lock.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-13  Initial   0.3.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "pmm.h"


//
// -- This function will dump as much of the current state of the PMM as possible (given locks)
//    -----------------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void PmmDumpState(void)
{
    kprintf("\n");
    kprintf("=============================================================================================\n");
    kprintf("\n");
    kprintf("Dumping the current state of the PMM structures\n");
    kprintf("  (Note: these data elements may be in motion at the time the state is being dumped)\n");
    kprintf("------------------------------------------------------------------------------------\n");
    kprintf("\n");
    kprintf("The number of frames in each stack are:\n");
    kprintf("  Scrub.: %x\n", pmm.scrubStack.count);
    kprintf("  Normal: %x\n", pmm.normalStack.count);
    kprintf("  Low...: %x\n", pmm.lowStack.count);
    kprintf("\n");
    kprintf("=============================================================================================\n");
}

