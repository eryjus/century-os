//===================================================================================================================
//
//  PicEnableIrq.cc -- Enable the PIC to pass along an IRQ
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
void _PicEnableIrq(PicDevice_t *dev, int irq)
{
    if (!dev) return;
    if (irq < 0 || irq > 71) return;

    int shift;
    archsize_t addr;

    if (irq >= 64) {
        shift = irq - 64;
        addr = dev->base1 + INT_IRQENB0;
    } else {
        shift = irq % 32;
        addr = dev->base1 + INT_IRQENB1 + (4 * (irq / 32));
    }

    kprintf("Enabling IRQ %x at address %p\n", 1<<shift, addr);
    MmioWrite(addr, 1 << shift);
}
