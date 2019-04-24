//===================================================================================================================
//
//  ApicRegisterHandler.cc -- Register a handler to take care of an IRQ
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
// -- Register an IRQ handler
//    -----------------------
isrFunc_t __krntext _ApicRegisterHandler(PicDevice_t *dev, int irq, int vector, isrFunc_t handler)
{
    if (!dev) return (isrFunc_t)-1;
    if (!handler) return (isrFunc_t)-1;
    if (irq < 0 || irq > 23) return (isrFunc_t)-1;
    if (vector < 0 || vector > 255) return (isrFunc_t)-1;

    kprintf("Processing an audited request to map irq %x to vector %x\n", irq, vector);

    PicMaskIrq(dev, irq);

    Ioapicredtbl_t redir;
    redir.reg = 0;
    redir.intvec = vector;
    redir.delmod = DELMODE_FIXED;
    redir.destmod = 0;      // physical cpu delivery
    redir.intpol = 0;       // active high
    redir.triggerMode = 1;  // level triggered
    redir.intMask = 1;      // leave this masked!!
    redir.dest = 0;         // apic id 0 for now

    ApicDeviceData_t *data = (ApicDeviceData_t *)dev->device.deviceData;
    IOAPIC_WRITE(data->ioapicBase, IOREDTBL0 + (irq * 2), redir.reg0);
    IOAPIC_WRITE(data->ioapicBase, IOREDTBL0 + (irq * 2) + 1, redir.reg1);

    isrFunc_t rv = IsrRegister(vector, handler);
    PicUnmaskIrq(dev, irq);

    kprintf(".. Request complete\n");

    return rv;
}


