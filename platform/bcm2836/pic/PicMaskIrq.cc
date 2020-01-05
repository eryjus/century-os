//===================================================================================================================
//
//  PicMaskIrq.cc -- Diable the PIC from passing along an IRQ
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
#include "pic.h"


//
// -- Disable the PIC from passing along an IRQ (some call it masking)
//    ----------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void _PicMaskIrq(PicDevice_t *dev, int irq)
{
    if (!dev) return;
    if (irq < 0 || irq > 71) return;
    Bcm2835Pic_t *picData = (Bcm2835Pic_t *)dev->device.deviceData;

    int shift;
    archsize_t addr;

    if (irq >= 64) {
        shift = irq - 64;
        addr = picData->picLoc + INT_IRQDIS0;
    } else {
        shift = irq % 32;
        addr = picData->picLoc + INT_IRQDIS1 + (4 * (irq / 32));
    }

    MmioWrite(addr, 1 << shift);
}
