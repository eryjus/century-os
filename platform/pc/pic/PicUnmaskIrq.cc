//===================================================================================================================
//
//  PicUnmaskIrq.cc -- Enable the PIC to pass along an IRQ
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


#include "cpu.h"
#include "hardware.h"
#include "pic.h"


//
// -- Enable the PIC to pass along an IRQ (some call it unmasking)
//    ------------------------------------------------------------
void _PicUnmaskIrq(PicDevice_t *dev, Irq_t i)
{
    if (!dev) return;
    if (i < 0 || i > 15) return;

    uint16_t port;
    int irq = (int)i;

    if (irq < 8) {
        port = PIC1 + PIC_MASTER_DATA;
    } else {
        port = PIC2 + PIC_SLAVE_DATA;
        irq -= 8;
    }

    outb(port, inb(port) & ~(1 << irq));
}
