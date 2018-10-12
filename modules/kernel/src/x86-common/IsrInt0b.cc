//===================================================================================================================
// kernel/src/x86-common/IsrInt0b.cc -- Segment Not Present
//
// The most basic Segment Not Present handler
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
void IsrInt0b(isrRegs_t *regs)
{
    kprintf("\nSegment Not Present\n");
    IsrDumpState(regs);
}