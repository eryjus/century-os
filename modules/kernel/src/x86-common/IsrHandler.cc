//===================================================================================================================
// kernel/src/x86-common/IsrHandler.cc -- The common ISR handler routine
//
// All ISRs are handled by a common service program.  This is it.  But it is currently a stub.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-05-29  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "idt.h"


//
// -- A Local prototype to prevent the compiler from name mangling
//    ------------------------------------------------------------
extern "C" void IsrHandler(isrRegs_t regs);


//
// -- The ISR Handler Table
//    ---------------------
isrFunc_t isrHandlers[256] = {NULL_ISR};



//
// -- This is the common ISR Handler entry routine
//    --------------------------------------------
void IsrHandler(isrRegs_t regs)
{
	if (isrHandlers[regs.intno] != NULL) {
		isrFunc_t handler = isrHandlers[regs.intno];
		handler(&regs);
	} else {
		kprintf("Unhandled Interrupt #0x%02.2x (%d)\n", regs.intno, regs.intno);
	}
}
