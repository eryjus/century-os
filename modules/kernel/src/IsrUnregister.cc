//===================================================================================================================
// kernel/src/IsrUnregister.cc -- Unregister an ISR Handler
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-07-06  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "printf.h"
#include "idt.h"


//
// -- Remove an ISR handler from the handlers table
//    ---------------------------------------------
void IsrUnregister(uint8_t interrupt)
{
	regval_t flags = DisableInterrupts();

    if (isrHandlers[interrupt] == NULL_ISR) {
        kprintf("When unregistering interrupt %d, no handler is registered\n", interrupt);
    } else {
	    isrHandlers[interrupt] = NULL_ISR;
    }

	RestoreInterrupts(flags);
}