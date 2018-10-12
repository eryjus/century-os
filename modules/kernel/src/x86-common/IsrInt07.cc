//===================================================================================================================
// kernel/src/x86-common/IsrInt07.cc -- This is a device not available fault
//
// This is a basic device not available handler
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
// -- Device not available Handler
//    ----------------------------
void IsrInt07(isrRegs_t *regs)
{
    kprintf("\nDevice Not Available\n");
    IsrDumpState(regs);
}
