//===================================================================================================================
//
//  AcpiReadRsdt.cc -- Validate and read the RSDT table
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-06  Initial   0.4.1   ADCL  Initial version
//
//===================================================================================================================


#include "printf.h"
#include "mmu.h"
#include "hardware.h"



//
// -- read the rsdt table
//    -------------------
EXTERN_C EXPORT LOADER
bool AcpiReadRsdt(archsize_t loc)
{
    kprintf("Reading the RSDT\n");
    CHK_ACPI(loc);

    if (!AcpiCheckTable(loc, MAKE_SIG("RSDT"))) {
        kprintf("The RSDT does not match the required checks\n");
        return false;
    }

    RSDT_t *rsdt = (RSDT_t *)loc;
    uint32_t entries = (rsdt->length - ACPI_HDR_SIZE) / sizeof(uint32_t);

    for (uint32_t i = 0; i < entries; i ++) {
        kprintf("The address for entry %x is %p\n", i, rsdt->entry[i]);
        if (rsdt->entry[i]) AcpiGetTableSig(rsdt->entry[i]);
    }

    return true;
}

