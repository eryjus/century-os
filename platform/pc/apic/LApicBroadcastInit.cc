//===================================================================================================================
//
//  LApicBroadcastInit.cc -- Broadcast an INIT IPI to all cores
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Jun-16  Initial   0.4.6   ADCL  Initial version
//
//===================================================================================================================


#include "printf.h"
#include "timer.h"
#include "hardware.h"
#include "pic.h"


//
// -- Broadcast an INIT to all CPUs (including myself)
//    ------------------------------------------------
void __ldrtext _LApicBroadcastInit(PicDevice_t *dev, uint32_t core)
{
    if (!dev) return;

    LapicIcrHi_t hi = {
        .destination = (uint8_t)core,
    };

    LapicIcrLo_t lo = {0};
    lo.deliveryMode = DELMODE_INIT;
    lo.destinationMode = 0;
    lo.deliveryStatus = 1;
    lo.level = 1;
    lo.trigger = 1;
    lo.destinationShorthand = 0b00;

    MmioWrite(LAPIC_MMIO + LAPIC_ICR_HI, hi.raw);
    MmioWrite(LAPIC_MMIO + LAPIC_ICR_LO, lo.raw);
}
