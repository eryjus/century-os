//===================================================================================================================
//
// IsrInt12.cc -- Machine Check
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The most basic Machine Check handler
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
void IsrInt12(isrRegs_t *regs)
{
    kprintf("\nMachine Check\n");
    IsrDumpState(regs);
}
