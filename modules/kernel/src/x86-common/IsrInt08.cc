//===================================================================================================================
// kernel/src/x86-common/IsrInt0d.cc -- Double Fault handler
//
// The most basic Double Fault exception handler
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
void IsrInt08(isrRegs_t *regs)
{
    kprintf("\nDouble Fault\n");
    IsrDumpState(regs);
}