//===================================================================================================================
//
//  MmuMapToFrame.cc -- Map a page to point to a physical frame
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function will walk the current paging tables and insert tables as needed to map a virtual address or
//  page to a physical frame.  If the page is already mapped, it will not be replaced.  Also, frame 0 is
//  explicitly not allowed to be mapped.  The loader takes care of this and there is no reason whatsoever why any
//  other task should need to map to this frame.
//
//  Note that this function is called before any serial port mapping/setup is complete; therefore, not debugging
//  code can exist in this function yet.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-10  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "cpu.h"
#include "printf.h"
#include "pmm.h"
#include "spinlock.h"
#include "mmu.h"


//
// -- Helper function to create and map a new table
//    ----------------------------------------------------------------------------------------------
EXTERN_C HIDDEN KERNEL
frame_t MmuMakeTtl2Table(archsize_t addr, int flags)
{
    //
    // -- We have been asked to create a new TTL2 table.  We got here, so we know we need a frame.
    //    Go get it.
    //    ----------------------------------------------------------------------------------------
    frame_t frame = PmmAllocateFrame();
    MmuClearFrame(frame);


    //
    // -- The next order of business is to map this into the Management table.  This needs to be done for
    //    every new table, so there is nothing to check -- we know we need to do this.
    //    -----------------------------------------------------------------------------------------------
    Ttl2_t *ttl2Entry = KRN_TTL2_ENTRY(MMU_CLEAR_FRAME);

    INVALIDATE_PAGE(ttl2Entry, addr);

    ttl2Entry = TTL2_MGMT(addr, flags);
    ttl2Entry->frame = frame;
    ttl2Entry->s = 1;
    ttl2Entry->apx = 0;
    ttl2Entry->ap = 0b11;
    ttl2Entry->tex = 0b001;
    ttl2Entry->c = 1;
    ttl2Entry->b = 1;
    ttl2Entry->nG = 0;
    ttl2Entry->fault = 0b10;

    INVALIDATE_PAGE(ttl2Entry, addr);


    //
    // -- Note that we will not actually map this into the TTL1 table.  The calling function holds that
    //    responsibility.  Therefore, the only thing left to do is to return the frame we have allocated
    //    and prepared to be a TTL2 table.
    //    ----------------------------------------------------------------------------------------------
    BPIALLIS();
    DSB();
    return frame;
}


//
// -- Map a page to a frame
//    ---------------------
EXTERN_C EXPORT KERNEL
void MmuMapToFrame(archsize_t addr, frame_t frame, int flags)
{
    // -- refuse to map frame 0 for security reasons
    if (!frame || !addr) {
        return;
    }


    //
    // -- The first order of business is to check if we have a TTL2 table for this address.  We will know this
    //    by checking the TTL1 Entry and checking the fault field.
    //    ----------------------------------------------------------------------------------------------------
    if (TTL1_ENTRY(addr, flags)->fault == 0b00) {
        kprintf("TTL1 entry is not mapped to a TTL2 table; creating\n");
        frame_t ttl2 = MmuMakeTtl2Table(addr, flags);
        Ttl1_t *ttl1Entry = TTL1_ENTRY4(addr, flags);

        for (int i = 0; i < 4; i ++) {
            ttl1Entry[i].ttl2 = (ttl2 << 2) + i;
            ttl1Entry[i].fault = 0b01;

            INVALIDATE_PAGE(&ttl1Entry[i], &ttl1Entry[i]);
            DSB();
        }
    }


    //
    // -- At this point, we know we have a ttl2 table and the management entries are all set up properly.  It
    //    is just a matter of mapping the address.
    //    ---------------------------------------------------------------------------------------------------
    Ttl2_t *ttl2Entry = TTL2_ENTRY(addr, flags);

    if (ttl2Entry->fault != 0b00) return;

    INVALIDATE_PAGE(ttl2Entry, addr);

    ttl2Entry->frame = frame;
    ttl2Entry->s = 1;
    ttl2Entry->apx = 0;
    ttl2Entry->ap = 0b11;
    ttl2Entry->tex = (flags&PG_DEVICE?0b000:0b001);
    ttl2Entry->c = (flags&PG_DEVICE?0:1);
    ttl2Entry->b = (flags&PG_DEVICE?0:1);
    ttl2Entry->nG = 0;
    ttl2Entry->fault = 0b10;

    INVALIDATE_PAGE(ttl2Entry, addr);
    DSB();
}

