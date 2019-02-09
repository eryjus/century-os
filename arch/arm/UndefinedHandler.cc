//===================================================================================================================
//
//  UndefinedHandler.cc -- Handle an undefined instruction
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Dec-01  Initial   0.2.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "cpu.h"
#include "interrupt.h"


extern "C" void UndefinedHandler(isrRegs_t *regs)
{
    kprintf("Undefined Instruction:\n");
    IsrDumpState(regs);
    Halt();
}
