//===================================================================================================================
//
//  MmuMapToFrame.cc -- Map an address to a frame, making sure all the tables are set up properly
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The ARM MMU implementation is very different than the x86 version.  But the general concepts are the same.
//
//  I will be checking a TTL1 entry for a TTL2 table existence and if it does not exist, I will allocate a new
//  frame for this table.  Since the TTL2 table is 1K in size, all 4 tables (% 4) will be mapped into this one
//  frame.  I will also need to maintain the tables for the kernel "recursive" part as well.
//
//  Then, there will be the actual mapping of the frame into the proper address space.
//  * The TTL1 table offset will be passed into the function.  This can be easily converted into an array for
//    pointer arithmetic.
//  * The index into the TTL1 table is defined as ((addr >> 20) & 0x0fff).
//
//  With these 2 bits of information, I can check to see if I have a TTL2 table already allocated.
//
//  Once I have the address of the frame for a new the TTL2 table, I will need to update the TTL1 entry for that
//  frame and the other 3 TTL1 entries for the other parts of the frame.  In addition, the "recursive" bits need
//  to be managed as well as these are not magically taken care of in the same way as x86.
//
//  Once I have a TTL2 table location:
//  * The address of the table is (ttl1->ttl2) << 10
//  * The index into that table is ((addr >> 10) & 0x03ff)
//
//  There I can determine if the page is already mapped and map the page.  Note that this change will be
//  automatically updated in the "recursive" part of the tables.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-18  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "serial-loader.h"
#include "mmu-loader.h"


#ifndef DEBUG
#   define DEBUG_MMU 0
#endif


void MmuMapToFrame(ptrsize_t ttl1, ptrsize_t addr, frame_t frame, bool wrt, bool krn)
{
#if DEBUG_MMU == 1
    SerialPutS("Mapping address "); SerialPutHex(addr); SerialPutS(" to frame "); SerialPutHex(frame);
            SerialPutChar('\n');
#endif

    // first, go find the ttl1 entry
    Ttl1_t *ttl1Table = (Ttl1_t *)ttl1;
    int ttl1Index = ((addr >> 20) & 0x0fff);
    Ttl1_t *ttl1Entry = &ttl1Table[ttl1Index];

#if DEBUG_MMU == 1
    SerialPutS("  Ttl1 index is: "); SerialPutHex((uint32_t)ttl1Table); SerialPutChar('[');
            SerialPutHex(ttl1Index); SerialPutS("]\n");
#endif

    // now, do we have a ttl2 table for this address?
    if (ttl1Entry->fault == 0b00) MmuMakeTtl2Table(ttl1, addr);

    // get the ttl2 entry
    Ttl2_t *ttl2Table = (Ttl2_t *)((ttl1Entry->ttl2) << 10);
    int ttl2Index = ((addr >> 12) & 0x00ff);
    Ttl2_t *ttl2Entry = &ttl2Table[ttl2Index];

#if DEBUG_MMU == 1
    SerialPutS("  Ttl2 location is: "); SerialPutHex((uint32_t)ttl2Table); SerialPutChar('[');
            SerialPutHex(ttl2Index); SerialPutS("]\n");
#endif

    // if the page is already mapped, report an error
    if (ttl2Entry->fault != 0b00) {
        SerialPutS("Attempting to map already mapped address "); SerialPutHex(addr); SerialPutS(" (mapped to: ");
                SerialPutHex(ttl2Entry->frame);
                SerialPutS(")\n   The TTL1 Address is: "); SerialPutHex((ptrsize_t)ttl1Table);
                SerialPutS("\n   The Ttl1 Index is: "); SerialPutHex(ttl1Index);
                SerialPutS("\n   The TTL2 Address is: "); SerialPutHex((ptrsize_t)ttl2Table);
                SerialPutS("\n   The Ttl2 Index is: "); SerialPutHex(ttl2Index);
                SerialPutS("\n   Frame is: "); SerialPutHex(ttl2Entry->frame);
                SerialPutS("\n   Fault is: "); SerialPutHex(ttl2Entry->fault);
                SerialPutS("\n   The address of the ttl2 entry is: "); SerialPutHex((uint32_t)ttl2Entry);
                SerialPutS("\n   The overall contents are: "); SerialPutHex(*(uint32_t*)ttl2Entry);
                SerialPutS("\n");
        return;
    }

    ttl2Entry->frame = frame;
    ttl2Entry->s = 1;
    ttl2Entry->apx = 0;
    ttl2Entry->ap = 0b11;
    ttl2Entry->tex = (krn?0x000:0b001);
    ttl2Entry->c = 0;
    ttl2Entry->b = (krn?1:0);
    ttl2Entry->nG = 0;
    ttl2Entry->fault = 0b10;
}