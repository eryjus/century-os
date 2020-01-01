//===================================================================================================================
//
//  IsrInt03.cc -- This is a breakpoint
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This is the breakpoint and will be the entry into the debugger
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
// -- Breakpoint Handler
//    ------------------
void IsrInt03(isrRegs_t *regs)
{
    kprintf("\nBreakpoint\n");
    IsrDumpState(regs);
}
