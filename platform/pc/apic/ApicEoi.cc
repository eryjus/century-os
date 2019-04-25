//===================================================================================================================
//
//  ApicInit.cc -- Perform an EOI on the Local APIC
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-19  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "mmu.h"
#include "pic.h"


//
// -- End of interrupt signal
//    -----------------------
void _ApicEoi(PicDevice_t *dev, Irq_t irq)
{
    if (!dev) return;
    if (irq < 0 || irq > 23) return;

    ApicDeviceData_t *data = (ApicDeviceData_t *)dev->device.deviceData;
    MmioWrite(data->localApicBase + LAPIC_EOI, 0);
}

