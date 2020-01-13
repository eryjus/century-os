//===================================================================================================================
//
//  PlatformEarlyInit.cc -- Handle the early initialization for the pc platform
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function is called after `MmuEarlyInit()`, so we expect to have access to kernel virtual memory addresses.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-05  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "hardware.h"
#include "hw-disc.h"
#include "serial.h"
#include "mmu.h"
#include "interrupt.h"
#include "printf.h"
#include "platform.h"


//
// -- Handle the early initialization for the pc platform
//    ---------------------------------------------------
EXTERN_C EXPORT LOADER
void PlatformEarlyInit(void)
{
    SerialOpen(&debugSerial);       // initialize the serial port so we can output debug data
    kprintf("Hello...\n");

    if (CheckCpuid() != 0) {
        SetCpuid(true);
        CollectCpuid();
    }

    HwDiscovery();

    RSDP_t *rsdp = AcpiFindRsdp();
    if (rsdp == NULL) return;

    // -- temporarily map the acpi tables
    MmuMapToFrame((archsize_t)rsdp, (frame_t)(((archsize_t)rsdp) >> 12), PG_KRN);

    if ((rsdp->xsdtAddress != 0) && (AcpiReadXsdt(rsdp->xsdtAddress) == true)) {
        // -- do nothing here...
    } else {
        AcpiReadRsdt(rsdp->rsdtAddress);
        kprintf("The APIC base address is at %p\n", READ_APIC_BASE());

    }

    // -- unmap the acpi tables
    MmuUnmapPage((archsize_t)rsdp);


    // -- Complete the CPU initialization
    InitGdt();
    ExceptionInit();
}



