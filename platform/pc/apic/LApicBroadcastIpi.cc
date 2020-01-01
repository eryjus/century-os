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


#include "printf.h"
#include "timer.h"
#include "hardware.h"
#include "pic.h"


//
// -- Broadcast an IPI to all CPUs (including myself)
//    -----------------------------------------------
void __krntext _LApicBroadcastIpi(PicDevice_t *dev, int ipi)
{
    if (ipi < 0 || ipi > 31) return;
    if (!dev) return;

    uint32_t icr = (0b11<<18) | (1<<14) | ipi;

    MmioWrite(LAPIC_ICR_HI, 0x00);
    MmioWrite(LAPIC_ICR_LO, icr);
}
