//===================================================================================================================
//
//  IsrInt04.cc -- This is an overflow
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This is a basic overflow handler
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
// -- Overflow Handler
//    ----------------
EXTERN_C EXPORT KERNEL
void IsrInt04(isrRegs_t *regs)
{
    kprintf("\nOverflow\n");
    IsrDumpState(regs);
}
