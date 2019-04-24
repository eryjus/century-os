//===================================================================================================================
//
//  ApicUnmaskIrq.cc -- Unmask an IRQ so that it is effectively enabled
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-20  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "hw-disc.h"
#include "mmu.h"
#include "interrupt.h"
#include "pic.h"


//
// -- Enable an IRQ by unmasking it
//    -----------------------------
void __krntext _ApicUnmaskIrq(PicDevice_t *dev, int irq)
{
    if (!dev) return;
    if (irq < 0 || irq > 23) return;

    ApicDeviceData_t *data = (ApicDeviceData_t *)dev->device.deviceData;
    archsize_t addr = data->ioapicBase;
    int reg = IOREDTBL0 + (2 * irq);

    IOAPIC_WRITE(addr, reg, IOAPIC_READ(addr, reg) & ~(1<<16));
}

