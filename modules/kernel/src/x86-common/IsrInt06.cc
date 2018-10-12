//===================================================================================================================
// kernel/src/x86-common/IsrInt06.cc -- This is an invalid Opcode exception
//
// This is a basic invalid opcode exception handler
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
// -- Invalid Opcode Exception Handler
//    --------------------------------
void IsrInt06(isrRegs_t *regs)
{
    kprintf("\nInvalid Opcode Exception\n");
    IsrDumpState(regs);
}
