//===================================================================================================================
//
//  PicDisableIrq.cc -- Disable the PIC from passing along an IRQ
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
// -- Disable the PIC from passing along an IRQ (some call it masking)
//    ----------------------------------------------------------------
void _PicDisableIrq(PicDevice_t *dev, int irq)
{
    if (!dev) return;
    if (irq < 0 || irq > 15) return;

    uint16_t port;

    if (irq < 8) {
        port = dev->base1 + PIC_MASTER_DATA;
    } else {
        port = dev->base2 + PIC_SLAVE_DATA;
        irq -= 8;
    }

    outb(port, inb(port) | (1 << irq));
}
