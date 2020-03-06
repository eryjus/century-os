//===================================================================================================================
//
//  IsrInt13.cc -- SIMD Floating Point Exception
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The most basic SIMD Floating Point Exception handler
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-10  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "interrupt.h"


//
// -- This is the ISR Handler routine
//    -------------------------------
EXTERN_C EXPORT KERNEL
void IsrInt13(isrRegs_t *regs)
{
    kprintf("\nSIMD Floating Point Exception\n");
    IsrDumpState(regs);
}
