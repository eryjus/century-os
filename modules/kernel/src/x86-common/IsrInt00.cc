//===================================================================================================================
// kernel/src/x86-common/IsrInt00.cc -- Divide by 0 handler
//
// The most basic divide by 0 exception handler
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-10-10  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "idt.h"


//
// -- This is the ISR Handler routine
//    -------------------------------
void IsrInt00(isrRegs_t *regs)
{
    kprintf("\nDivide Overflow\n");
    IsrDumpState(regs);
}
