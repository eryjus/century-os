//===================================================================================================================
//
//  LApicBroadcastIpi.cc -- Broadcast an IPI to all CPUs
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Jun-08  Initial   0.4.5   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "timer.h"
#include "hardware.h"
#include "pic.h"


//
// -- Broadcast an IPI to all CPUs (including myself)
//    -----------------------------------------------
EXTERN_C EXPORT KERNEL
void _LApicBroadcastIpi(PicDevice_t *dev, int ipi)
{
#if DEBUG_ENABLED(LApicBroadcastIpi)
    kprintf("Entered %s on CPU %d for dev %p\n", __func__, thisCpu->cpuNum, dev);
#endif

    if (!dev) return;
    if (!dev->ipiReady) {
#if DEBUG_ENABLED(LApicBroadcastIpi)
    kprintf("IPI still not ready at %p\n", dev);
#endif

        return;
    }

#if DEBUG_ENABLED(LApicBroadcastIpi)
    kprintf(".. Qualified on CPU %d\n", thisCpu->cpuNum);
#endif

//    uint32_t icr = (0b11<<18) | (1<<14) | ipi;      // all except self | Assert | vector
    uint32_t icr = (0b11<<18) | ipi;      // all except self | vector

    MmioWrite(LAPIC_MMIO + LAPIC_ICR_HI, 0x00);
    MmioWrite(LAPIC_MMIO + LAPIC_ICR_LO, icr);

#if DEBUG_ENABLED(LApicBroadcastIpi)
    kprintf(".. The ESR report %p\n", MmioRead(LAPIC_MMIO + LAPIC_ESR));
    kprintf(".. Delivery status reports %p\n", MmioRead(LAPIC_MMIO + LAPIC_ICR_LO));

    kprintf(".. Completed on CPU %d\n", thisCpu->cpuNum);
#endif
}
