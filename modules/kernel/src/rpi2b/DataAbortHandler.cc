//===================================================================================================================
//
//  DataAbortHandler.cc -- Handle a data abort
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Dec-01  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "cpu.h"
#include "interrupt.h"


extern "C" void DataAbortHandler(isrRegs_t *regs)
{
    kprintf("Data Exception:\n");
    IsrDumpState(regs);
    Halt();
}
