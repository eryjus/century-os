//===================================================================================================================
//
//  IsrInt01.cc -- This is a reserved interrupt
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The most basic int01 handler, which will be installed but should never be called.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-11  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "interrupt.h"


//
// -- A dummy reserved handler
//    ------------------------
EXTERN_C EXPORT KERNEL
void IsrInt01(isrRegs_t *regs)
{
    kprintf("\nRESERVED INT01\n");
    IsrDumpState(regs);
}
