//===================================================================================================================
//
//  IsrInt0b.cc -- Segment Not Present
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The most basic Segment Not Present handler
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
void IsrInt0b(isrRegs_t *regs)
{
    kprintf("\nSegment Not Present\n");
    IsrDumpState(regs);
}
