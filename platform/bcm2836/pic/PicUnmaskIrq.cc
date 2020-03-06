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


#include "types.h"
#include "printf.h"
#include "pic.h"
#include "printf.h"
#include "debug.h"


//
// -- Enable the PIC to pass along an IRQ (some call it unmasking)
//    ------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void _PicUnmaskIrq(PicDevice_t *dev, int irq)
{
#if DEBUG_ENABLED(PicUnmaskIrq)
    kprintf("Entering PicUnmaskIrq\n");
#endif

    if (!dev) return;
    if (irq < 0 || irq > BCM2836_LAST_IRQ) return;
    Bcm2835Pic_t *picData = (Bcm2835Pic_t *)dev->device.deviceData;
    int shift;
    archsize_t addr;

#if DEBUG_ENABLED(PicUnmaskIrq)
    kprintf("Sanity checks qualify PicUnmaskIrq, irq %d\n", irq);
#endif

    if (irq >= BCM2836_CORE_BASE) {
        shift = irq - BCM2836_CORE_BASE;
        addr = (MMIO_VADDR + 0x01000060) + (thisCpu->cpuNum * 4);

#if DEBUG_ENABLED(PicUnmaskIrq)
        kprintf("... bcm2836 local core interrupt\n");
#endif
    } else if (irq >= BCM2835_ARM_BASE) {
        shift = irq - BCM2835_GPU_BASE1;
        addr = picData->picLoc + INT_IRQDIS0;

#if DEBUG_ENABLED(PicUnmaskIrq)
        kprintf("... arm processor interrupt\n");
#endif
    } else {        // GPU IRQ 0-63
        shift = irq % 32;
        addr = picData->picLoc + INT_IRQDIS1 + (4 * (irq / 32));

#if DEBUG_ENABLED(PicUnmaskIrq)
        kprintf("... bcm2835 GPU interrupt\n");
#endif
    }

#if DEBUG_ENABLED(PicUnmaskIrq)
    kprintf("Enabling IRQ bit %x at address %p\n", shift, addr);
#endif
    MmioWrite(addr, 1 << shift);
#if DEBUG_ENABLED(PicUnmaskIrq)
    kprintf("Done\n");
#endif
}
