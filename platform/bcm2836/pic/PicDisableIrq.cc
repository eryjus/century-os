//===================================================================================================================
//
//  PicDisableIrq.cc -- Diable the PIC from passing along an IRQ
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
    if (irq < 0 || irq > 71) return;

    int shift;
    archsize_t addr;

    if (irq >= 64) {
        shift = irq - 64;
        addr = dev->base1 + INT_IRQDIS0;
    } else {
        shift = irq % 32;
        addr = dev->base1 + INT_IRQDIS1 + (4 * (irq / 32));
    }

    MmioWrite(addr, 1 << shift);
}
