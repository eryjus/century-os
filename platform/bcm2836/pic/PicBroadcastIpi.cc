//===================================================================================================================
//
//  PicBroadcastIpi.cc -- Broadcast an IPI to all CPUs
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
#include "pic.h"


//
// -- Broadcast an IPI to all CPUs (including myself)
//    -----------------------------------------------
EXTERN_C EXPORT KERNEL
void _PicBroadcastIpi(PicDevice_t *dev, int ipi)
{
    if (!dev) return;
    if (!dev->ipiReady) return;

#if DEBUG_ENABLED(PicBroadcastIpi)
    kprintf("For IPI broadcast Qualified on CPU %d\n", thisCpu->cpuNum);
#endif

    for (int i = 0; i < cpus.cpusRunning; i ++) {
        if (i != thisCpu->cpuNum) {
#if DEBUG_ENABLED(PicBroadcastIpi)
            kprintf("Sending to mailbox for cpu %d\n", i);
#endif
            MmioWrite(IPI_MAILBOX_BASE + (0x10 * i), (archsize_t)ipi);
        }
    }

#if DEBUG_ENABLED(PicBroadcastIpi)
    kprintf(".. Completed on CPU %d\n", thisCpu->cpuNum);
#endif
}
