//===================================================================================================================
//
//  IsrInt1b.cc -- This is Intel Reserved Interrupt
//
//        Copyright (c)  2017-2020 -- Adam Clark
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
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "interrupt.h"


//
// -- An Intel Interrupt
//    --------------
void IsrInt1b(isrRegs_t *regs)
{
    kprintf("\nRESERVED INT1B\n");
    IsrDumpState(regs);
}
