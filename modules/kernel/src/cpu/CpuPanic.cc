//===================================================================================================================
//
//  CpuPanic.cc -- Panic halt all CPUs, printing the register contents of this CPU
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Mar-04  Initial  v0.5.0h  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "pic.h"
#include "interrupt.h"
#include "cpu.h"


//
// -- Panic halt all CPUs
//    -------------------
EXTERN_C EXPORT NORETURN KERNEL
void CpuPanic(const char *reason, isrRegs_t *regs)
{
    DisableInterrupts();
    PicBroadcastIpi(picControl, IPI_PANIC);
    kprintf("\n%s\n\n", reason);
    IsrDumpState(regs);
}


