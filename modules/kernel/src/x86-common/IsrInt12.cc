//===================================================================================================================
// kernel/src/x86-common/IsrInt12.cc -- Machine Check
//
// The most basic Machine Check handler
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
void IsrInt12(isrRegs_t *regs)
{
    kprintf("\nMachine Check\n");
    IsrDumpState(regs);
}