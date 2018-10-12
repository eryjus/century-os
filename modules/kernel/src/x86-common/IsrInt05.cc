//===================================================================================================================
// kernel/src/x86-common/IsrInt05.cc -- This is a bound exception
//
// This is a basic bound exception handler
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
// -- Bound Exception Handler
//    -----------------------
void IsrInt05(isrRegs_t *regs)
{
    kprintf("\nBound Exception\n");
    IsrDumpState(regs);
}
