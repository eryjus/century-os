//===================================================================================================================
//
//  IsrInt05.cc -- This is a bound exception
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This is a basic bound exception handler
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
// -- Bound Exception Handler
//    -----------------------
EXTERN_C EXPORT KERNEL
void IsrInt05(isrRegs_t *regs)
{
    kprintf("\nBound Exception\n");
    IsrDumpState(regs);
}
