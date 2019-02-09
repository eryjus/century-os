//===================================================================================================================
//
//  IsrInt0e.cc -- Page Fault handler
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The most basic Page Fault exception handler
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//  2018-Oct-10  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "interrupt.h"


//
// -- This is the ISR Handler routine
//    -------------------------------
void IsrInt0e(isrRegs_t *regs)
{
    kprintf("\nPage Fault\n");
    IsrDumpState(regs);
}
