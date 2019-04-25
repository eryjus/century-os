//===================================================================================================================
//
//  PicRegisterHandler.cc -- Register a handler to take care of an IRQ
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-24  Initial   0.4.1   ADCL  Initial version
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
isrFunc_t __krntext _PicRegisterHandler(PicDevice_t *dev, Irq_t irq, int vector, isrFunc_t handler)
{
    if (!dev) return (isrFunc_t)-1;
    if (!handler) return (isrFunc_t)-1;
    if (irq < 0 || irq > 15) return (isrFunc_t)-1;
    if (vector < 0 || vector > 255) return (isrFunc_t)-1;

    kprintf("Processing an audited request to map irq %x to vector %x\n", irq, vector);

    PicMaskIrq(dev, irq);
    isrFunc_t rv = IsrRegister(vector, handler);
    PicUnmaskIrq(dev, irq);

    kprintf(".. Request complete\n");

    return rv;
}


