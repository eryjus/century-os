//===================================================================================================================
//
//  PicInit.cc -- Initialize the PIC
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-24  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#include "cpu.h"
#include "hardware.h"
#include "pic.h"


//
// -- Initialize the x86 8259 PIC -- note, this is not the IOAPIC
//    -----------------------------------------------------------
void _PicInit(PicDevice_t *dev)
{
    if (!dev) return;

	archsize_t flags = DisableInterrupts();
    PicBase_t base1 = dev->base1;
    PicBase_t base2 = dev->base2;

    // -- Remap the irq table, even though we may not be using it.
	outb(base1 + PIC_MASTER_DATA, 0xff);			// Disable all IRQs
	outb(base2 + PIC_SLAVE_DATA, 0xff);			// Disable all IRQs
	outb(base1 + PIC_MASTER_COMMAND, 0x11);
	outb(base2 + PIC_SLAVE_COMMAND, 0x11);
	outb(base1 + PIC_MASTER_DATA, 0x20);
	outb(base2 + PIC_SLAVE_DATA, 0x28);
	outb(base1 + PIC_MASTER_DATA, 0x04);
	outb(base2 + PIC_SLAVE_DATA, 0x02);
	outb(base1 + PIC_MASTER_DATA, 0x01);
	outb(base2 + PIC_SLAVE_DATA, 0x01);

	RestoreInterrupts(flags);
}


