//===================================================================================================================
// kernel/src/x86-common/IsrInt03.cc -- This is a breakpoint
//
// This is the breakpoint and will be the entry into the debugger
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
// -- Breakpoint Handler
//    ------------------
void IsrInt03(isrRegs_t *regs)
{
    kprintf("\nBreakpoint\n");
    IsrDumpState(regs);
}
