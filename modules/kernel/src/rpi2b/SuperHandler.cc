//===================================================================================================================
//
//  SuperHandler.cc -- Handle a Supervisor Request
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


extern "C" void SuperHandler(isrRegs_t *regs)
{
    kprintf("Supervisor Request:\n");
    IsrDumpState(regs);
}