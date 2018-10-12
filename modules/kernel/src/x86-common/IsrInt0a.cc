//===================================================================================================================
// kernel/src/x86-common/IsrInt0a.cc -- Invalid TSS
//
// The most basic Invalid TSS handler
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
void IsrInt0a(isrRegs_t *regs)
{
    kprintf("\nInvalid TSS\n");
    IsrDumpState(regs);
}