//===================================================================================================================
//
//  IsrInt02.cc -- This is the NMI interrupt
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The most basic NMI Handler
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
// -- An NMI handler
//    --------------
EXTERN_C EXPORT KERNEL
void IsrInt02(isrRegs_t *regs)
{
    kprintf("\nNMI\n");
    IsrDumpState(regs);
}
