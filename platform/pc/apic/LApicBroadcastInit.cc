//===================================================================================================================
//
//  LApicBroadcastInit.cc -- Broadcast an INIT IPI to all cores
//
//        Copyright (c)  2017-2019 -- Adam Clark
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
void __ldrtext _LApicBroadcastInit(PicDevice_t *dev)
{
    if (!dev) return;

    uint32_t icr = (0b11<<18) | (1<<14) | (1<<0b101);

    MmioWrite(LAPIC_ICR_HI, 0x00);
    MmioWrite(LAPIC_ICR_LO, icr);
}
