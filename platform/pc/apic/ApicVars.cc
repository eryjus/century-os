//===================================================================================================================
//
//  ApicVars.cc -- These are the variables for the x86 APIC
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
#include "timer.h"
#include "pic.h"


//
// -- This is the structure for the data needed by this driver
//    --------------------------------------------------------
__krndata IoApicDeviceData_t ioapicData = {
    .redirTableEntry = {
        IOREDTBL2,      // IRQ0
        IOREDTBL1,      // IRQ1
        0,              // IRQ2
        IOREDTBL3,      // IRQ3
        IOREDTBL4,      // IRQ4
        IOREDTBL5,      // IRQ5
        IOREDTBL6,      // IRQ6
        IOREDTBL7,      // IRQ7
        IOREDTBL8,      // IRQ8
        IOREDTBL9,      // IRQ9
        IOREDTBL10,     // IRQ10
        IOREDTBL11,     // IRQ11
        IOREDTBL12,     // IRQ12
        IOREDTBL13,     // IRQ13
        IOREDTBL14,     // IRQ14
        IOREDTBL15,     // IRQ15
        IOREDTBL16,     // PIRQ0
        IOREDTBL17,     // PIRQ1
        IOREDTBL18,     // PIRQ2
        IOREDTBL19,     // PIRQ3
        IOREDTBL20,     // MIRQ0
        IOREDTBL21,     // MIRQ1
        IOREDTBL22,     // GPIRQ
        IOREDTBL23,     // SMI
        IOREDTBL0,      // INTR
    },
};


//
// -- This is the device description for the IO APIC
//    ----------------------------------------------
__krndata PicDevice_t ioapicDriver = {
    .device = {
        .name = {'a', 'p', 'i', 'c', '\0'},
        .deviceData = (DeviceData_t)&ioapicData,
    },
    .PicInit = _IoApicInit,
    .PicRegisterHandler = _IoApicRegisterHandler,
    .PicMaskIrq = _IoApicMaskIrq,
    .PicUnmaskIrq = _IoApicUnmaskIrq,
    .PicEoi = _IoApicEoi,
};


//
// -- This is the device description for the local apic timer
//    -------------------------------------------------------
__krndata TimerDevice_t lapicTimerControl = {
    .TimerCallBack = TimerCallBack,
    .TimerInit = _LApicInit,
    .TimerEoi = _LApicEoi,
    .TimerPlatformTick = _TimerPlatformTick,
    .TimerCurrentCount = _TimerCurrentCount,
};