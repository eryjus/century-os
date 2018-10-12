//===================================================================================================================
// kernel/src/x86-common/IsrInt01.cc -- This is a reserved interrupt
//
// The most basic int01 handler, which will be installed but should never be called.
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
#include "idt.h"


//
// -- A dummy reserved handler
//    ------------------------
void IsrInt01(isrRegs_t *regs)
{
    kprintf("\nRESERVED INT01\n");
    IsrDumpState(regs);
}
