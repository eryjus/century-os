//===================================================================================================================
//
//  IsrInt00.cc -- Divide by 0 handler
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The most basic divide by 0 exception handler
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-10  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "interrupt.h"


//
// -- This is the ISR Handler routine
//    -------------------------------
EXTERN_C EXPORT KERNEL
void IsrInt00(isrRegs_t *regs)
{
    kprintf("\nDivide Overflow\n");
    IsrDumpState(regs);
}
