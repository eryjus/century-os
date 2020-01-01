//===================================================================================================================
//
//  IsrInt06.cc -- This is an invalid Opcode exception
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This is a basic invalid opcode exception handler
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
// -- Invalid Opcode Exception Handler
//    --------------------------------
void IsrInt06(isrRegs_t *regs)
{
    kprintf("\nInvalid Opcode Exception\n");
    IsrDumpState(regs);
}
