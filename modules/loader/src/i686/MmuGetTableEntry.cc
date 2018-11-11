//===================================================================================================================
//
//  loader/src/i686/MmuGetTableEntry.cc -- For a table, get the entry for the virtual address
//
//  This function will allocate a new table if need be.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-06-27  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "pmm.h"
#include "serial.h"
#include "cpu.h"
#include "mmu.h"

//
// -- From the address, get the table entry shifting properly
//    -------------------------------------------------------
pageEntry_t *MmuGetTableEntry(pageEntry_t *table, ptrsize_t addr, int shift, bool alloc)
{
    uint32_t index = (addr >> shift) & 0x3ff;
    pageEntry_t *rv = &table[index];

    if (rv->p == 0 && shift != 12 && alloc) {       // we will allocate a new frame if appropriate
        SerialPutS("   Making a new table\n");
        uint32_t frame = PmmNewFrame();
        kMemSetB((void *)(frame << 12), 0, 4096);
        rv->frame = frame;
        rv->rw = 1;
        rv->us = 1;
        rv->p = 1;
    }

    return rv;
}
