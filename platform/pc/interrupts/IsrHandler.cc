//===================================================================================================================
//
//  IsrHandler.cc -- The common ISR handler routine
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  All ISRs are handled by a common service program.  This is it.  But it is currently a stub.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-May-29  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "printf.h"
#include "interrupt.h"


//
// -- The ISR Handler Table
//    ---------------------
EXPORT KERNEL_BSS
isrFunc_t isrHandlers[256] = {NULL_ISR};



//
// -- This is the common ISR Handler entry routine
//    --------------------------------------------
EXTERN_C EXPORT KERNEL
void IsrHandler(isrRegs_t regs)
{
    if (isrHandlers[regs.intno] != NULL) {
        isrFunc_t handler = isrHandlers[regs.intno];
        handler(&regs);
    } else {
        kprintf("PANIC: Unhandled Interrupt #%x\n", regs.intno);
        CpuPanic("", &regs);
    }
}
