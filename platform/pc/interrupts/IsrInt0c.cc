//===================================================================================================================
//
//  IsrInt0c.cc -- Stack Segment Fault
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The most basic Stack Segment Fault handler
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
void IsrInt0c(isrRegs_t *regs)
{
    kprintf("\nStack Segment Fault\n");
    IsrDumpState(regs);
}
