//===================================================================================================================
//
//  PicUnmaskIrq.cc -- Enable the PIC to pass along an IRQ
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


#include "printf.h"
#include "cpu.h"
#include "hardware.h"
#include "pic.h"


//
// -- Enable the PIC to pass along an IRQ (some call it unmasking)
//    ------------------------------------------------------------
void _PicUnmaskIrq(PicDevice_t *dev, int irq)
{
    if (!dev) return;
    if (irq < 0 || irq > 71) return;
    Bcm2835Pic_t *picData = (Bcm2835Pic_t *)dev->device.deviceData;

    int shift;
    archsize_t addr;

    if (irq >= 64) {
        shift = irq - 64;
        addr = picData->picLoc + INT_IRQENB0;
    } else {
        shift = irq % 32;
        addr = picData->picLoc + INT_IRQENB1 + (4 * (irq / 32));
    }

    kprintf("Enabling IRQ bit %x at address %p\n", shift, addr);
    MmioWrite(addr, 1 << shift);
    kprintf("Done\n");
}
