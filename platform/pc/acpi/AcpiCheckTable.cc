//===================================================================================================================
//
//  AcpiCheckTable.cc -- Check the table signature and checksum to confirm it is a valid table.
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
// -- Check the table to see if it is what we expect; note that this memory must be mapped before calling
//    ---------------------------------------------------------------------------------------------------
EXTERN_C EXPORT LOADER
bool AcpiCheckTable(archsize_t locn, uint32_t sig)
{
    uint8_t *table = (uint8_t *)locn;
    uint32_t size;
    archsize_t checksum = 0;
    kprintf(".. Checking the ACPI table....\n");
    if (*((uint32_t *)locn) != sig) {
        kprintf(".. (signature check fails)\n");
        return false;
    }

    size = *((uint32_t *)(locn + 4));
    kprintf(".. Checking %x bytes of the table\n", size);
    for (uint32_t i = 0; i < size; i ++) {
        archsize_t loc = (archsize_t)(&table[i]);
        if (!MmuIsMapped(loc)) {
            MmuMapToFrame(loc, loc >> 12, PG_KRN);
        }
        checksum += table[i];
    }

    return (checksum & 0xff) == 0;
}

