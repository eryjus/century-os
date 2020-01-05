//===================================================================================================================
//
//  PicInit.cc -- Initialize the PIC
//
//        Copyright (c)  2017-2020 -- Adam Clark
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


#include "types.h"
#include "hardware.h"
#include "pic.h"


//
// -- Initialize the x86 8259 PIC -- note, this is not the IOAPIC
//    -----------------------------------------------------------
EXTERN_C EXPORT KERNEL
void _PicInit(PicDevice_t *dev, const char *name)
{
    if (!dev) return;

    archsize_t flags = DisableInterrupts();

    // -- Remap the irq table, even though we may not be using it.
    outb(PIC1 + PIC_MASTER_DATA, 0xff);            // Disable all IRQs
    outb(PIC2 + PIC_SLAVE_DATA, 0xff);             // Disable all IRQs
    outb(PIC1 + PIC_MASTER_COMMAND, 0x11);
    outb(PIC2 + PIC_SLAVE_COMMAND, 0x11);
    outb(PIC1 + PIC_MASTER_DATA, 0x20);
    outb(PIC2 + PIC_SLAVE_DATA, 0x28);
    outb(PIC1 + PIC_MASTER_DATA, 0x04);
    outb(PIC2 + PIC_SLAVE_DATA, 0x02);
    outb(PIC1 + PIC_MASTER_DATA, 0x01);
    outb(PIC2 + PIC_SLAVE_DATA, 0x01);

    RestoreInterrupts(flags);
}


