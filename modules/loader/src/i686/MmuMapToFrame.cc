//===================================================================================================================
//
//  loader/src/i686/MmuMapToFrame.cc -- Given the Paging tables, map the address to the frame provided
//
//  This function is a bit complicated.  We need to walk through the tables and entries in an organized manner in
//  order to check if the address has already been mapped.  We need a few bits of information to make this work:
//  * The address of the Page Directory Table; this is in `cr3`
//  * The index of the Page Directory Entry we are concerned with
//  * The Page Directory Entry for the address in question which is used to get the frame of the physical Page Table
//  * The Page Table address; from the Page Directory Entry above
//  * The index of the Page Table Entry we want for the address in question
//  * The Page Table Entry for the address we are concerned with
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-06-24  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "serial.h"
#include "pmm.h"
#include "mmu.h"


//
// -- Map a virtual address page to a physical frame in the paging tables
//    -------------------------------------------------------------------
void MmuMapToFrame(ptrsize_t cr3, ptrsize_t addr, frame_t frame, bool wrt, bool krn)
{
    // -- Make sure the frame is really allocated
    if (!PmmIsFrameAlloc(frame)) {
        SerialPutS("MMU: Frame ");
        SerialPutHex(frame);
        SerialPutS(" is not allocated\n");
        return;
    }

    // -- Walk the tables, making sure we have all the entries we need
    pageEntry_t *pdTable = (pageEntry_t *)cr3;
    pageEntry_t *pdEntry = MmuGetTableEntry(pdTable, addr, 22, true);
    pageEntry_t *pTable = MmuGetAddrFromEntry(pdEntry);
    pageEntry_t *pEntry = MmuGetTableEntry(pTable, addr, 12, true);

    // -- Now, check if the page is already mapped
    if (pEntry->p) {
        SerialPutS("MMU: Address ");
        SerialPutHex(addr);
        SerialPutS(" is already mapped to frame ");
        SerialPutHex(pEntry->frame);
        SerialPutS("\n");
        return;
    }

    // -- Finally complete the mapping
    pEntry->frame = frame;
    pEntry->p = 1;
    pEntry->rw = wrt;
    pEntry->us = 1;
    pEntry->k = krn;
}