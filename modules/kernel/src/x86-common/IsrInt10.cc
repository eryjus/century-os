//===================================================================================================================
// kernel/src/x86-common/IsrInt10.cc -- Math Fault Handler
//
// The most basic Math Fault handler
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-10  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "idt.h"


//
// -- This is the ISR Handler routine
//    -------------------------------
void IsrInt10(isrRegs_t *regs)
{
    kprintf("\nMath Fault\n");
    IsrDumpState(regs);
}