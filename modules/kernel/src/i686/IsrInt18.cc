//===================================================================================================================
//
//  IsrInt18.cc -- This is Intel Reserved Interrupt
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  An unused Interrupt
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
// -- An Intel Interrupt
//    --------------
void IsrInt18(isrRegs_t *regs)
{
    kprintf("\nRESERVED INT18\n");
    IsrDumpState(regs);
}
