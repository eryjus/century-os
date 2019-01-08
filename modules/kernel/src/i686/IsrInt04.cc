//===================================================================================================================
//
//  IsrInt04.cc -- This is an overflow
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "interrupt.h"


//
// -- Overflow Handler
//    ----------------
void IsrInt04(isrRegs_t *regs)
{
    kprintf("\nOverflow\n");
    IsrDumpState(regs);
}
