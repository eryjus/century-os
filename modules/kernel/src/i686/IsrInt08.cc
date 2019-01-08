//===================================================================================================================
//
//  IsrInt08.cc -- Double Fault handler
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The most basic Double Fault exception handler
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-10-10  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "interrupt.h"


//
// -- This is the ISR Handler routine
//    -------------------------------
void IsrInt08(isrRegs_t *regs)
{
    kprintf("\nDouble Fault\n");
    IsrDumpState(regs);
}
