//===================================================================================================================
//
//  PlatformEarlyInit.cc -- Handle the early initialization for the pc platform
//
//        Copyright (c)  2017-2019 -- Adam Clark
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


#include "hardware.h"
#include "hw-disc.h"
#include "fb.h"
#include "printf.h"
#include "platform.h"


//
// -- Handle the early initialization for the pc platform
//    ---------------------------------------------------
void __ldrtext PlatformEarlyInit(void)
{
    HwDiscovery();
    RSDP_t *rsdp = AcpiFindRsdp();
    if (rsdp == NULL) return;

    if ((rsdp->xsdtAddress != 0) && (AcpiReadXsdt(rsdp->xsdtAddress) == true)) {
        return;
    }

    cpus.cpuCount = 0;

    AcpiReadRsdt(rsdp->rsdtAddress);

    kprintf("The APIC base address is at %p\n", READ_APIC_BASE());
}



