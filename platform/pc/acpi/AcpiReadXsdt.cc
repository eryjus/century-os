//===================================================================================================================
//
//  AcpiReadXsdt.cc -- Validate and read the XSDT table
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
// -- read the xsdt table
//    -------------------
bool __ldrtext AcpiReadXsdt(archsize_t loc)
{
    kprintf("Reading the XSDT\n");
    CHK_ACPI(loc);

    if (!AcpiCheckTable(loc, MAKE_SIG("XSDT"))) {
        kprintf("The XSDT does not match the required checks\n");
        return false;
    }

    XSDT_t *xsdt = (XSDT_t *)loc;
    uint32_t entries = (xsdt->length - ACPI_HDR_SIZE) / sizeof(uint64_t);
    kprintf("... checking %x entries\n", entries);

    for (uint32_t i = 0; i < entries; i ++) {
        kprintf("The address for entry %x is %p\n", i, xsdt->entry[i]);
        if (xsdt->entry[i]) AcpiGetTableSig(xsdt->entry[i]);
    }

    return true;
}

