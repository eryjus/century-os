//===================================================================================================================
//
//  MmuMakeTtl2Table.cc -- Make a new Ttl2 table
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Allocate a new frame and establish a new Ttl2 table (actually 4 of them).  This can get a little messy, so
//  this task is held out to its own function.  We need to:
//  1.  Allocate a new frame and clear it
//  2.  Since 1 Ttl2 table is 1K in length, map all 4 Ttl2 tables to the proper spots in the Ttl1 table
//  3.  Go back to the "recursive" tables and map the table to the correct frame there (this might also create the
//      need to create a new Ttl2 table for is the "recursive" space as well)
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-18  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "serial-loader.h"
#include "pmm.h"
#include "mmu-loader.h"


#ifndef DEBUG_MMU
#   define DEBUG_MMU 0
#endif


//
// -- This is the first frame we are allocating blocks from in the loader
//    -------------------------------------------------------------------
extern frame_t allocFrom;


//
// -- Create a TTL2 table and properly map it to the TTL1 table
//    ---------------------------------------------------------
void MmuMakeTtl2Table(MmuData_t mmu, ptrsize_t addr)
{
#if DEBUG_MMU == 1
    SerialPutS("Creating a new TTL2 table for address "); SerialPutHex(addr); SerialPutChar('\n');
#endif

    frame_t ttl2Frame = allocFrom;         // Adjust to 1K accuracy
    PmmAllocFrame(allocFrom ++);
    kMemSetB((void *)PmmFrameToLinear(ttl2Frame), 0, 4096);

#if DEBUG_MMU == 1
    SerialPutS("  The new frame is "); SerialPutHex(ttl2Frame); SerialPutChar('\n');
#endif

    ptrsize_t ttl2Loc = ttl2Frame << 2;
    int i = (addr >> 20) & 0xffc;   // align this to the "% 4 == 0" TTL1 entry

#if DEBUG_MMU == 1
    SerialPutS("  The base ttl2 1K location is "); SerialPutHex(ttl2Loc); SerialPutChar('\n');
    SerialPutS("  The ttl1 index is "); SerialPutHex(i); SerialPutChar('\n');
#endif

    // -- map the "% 4 == 0" TTL2 table
    Ttl1_t *ttl1Table = (Ttl1_t *)mmu;
    ttl1Table[i].ttl2 = ttl2Loc;
    ttl1Table[i].fault = 0b01;
#if DEBUG_MMU == 1
    SerialPutS("  Set the TTL1 table index "); SerialPutHex(i); SerialPutS(" to 1K location ");
            SerialPutHex(ttl2Loc); SerialPutChar('\n');
#endif

    // -- map the "% 4 == 1" TTL2 table
    ttl1Table[i + 1].ttl2 = ttl2Loc + 1;
    ttl1Table[i + 1].fault = 0b01;
#if DEBUG_MMU == 1
    SerialPutS("  Set the TTL1 table index "); SerialPutHex(i + 1); SerialPutS(" to 1K location ");
            SerialPutHex(ttl2Loc + 1); SerialPutChar('\n');
#endif

    // -- map the "% 4 == 2" TTL2 table
    ttl1Table[i + 2].ttl2 = ttl2Loc + 2;
    ttl1Table[i + 2].fault = 0b01;
#if DEBUG_MMU == 1
    SerialPutS("  Set the TTL1 table index "); SerialPutHex(i + 2); SerialPutS(" to 1K location ");
            SerialPutHex(ttl2Loc + 2); SerialPutChar('\n');
#endif

    // -- map the "% 4 == 3" TTL2 table
    ttl1Table[i + 3].ttl2 = ttl2Loc + 3;
    ttl1Table[i + 3].fault = 0b01;
#if DEBUG_MMU == 1
    SerialPutS("  Set the TTL1 table index "); SerialPutHex(i + 3); SerialPutS(" to 1K location ");
            SerialPutHex(ttl2Loc + 3); SerialPutChar('\n');
#endif

    // Here we need to get the TTL1 entry for the management address.
    int ttl2Offset = (addr >> 20) & 0xfff;
    ptrsize_t mgmtTtl2Addr = TTL2_KRN_VADDR + (ttl2Offset * 1024) + (((addr >> 12) & 0xff) * 4);
//    int mgmtTtl1Index = mgmtTtl2Addr >> 20;
//    Ttl1_t *mgmtTtl1Entry = &ttl1Table[mgmtTtl1Index];

#if DEBUG_MMU == 1
    SerialPutS("  The management address for this Ttl2 table is "); SerialPutHex(mgmtTtl2Addr); SerialPutChar('\n');
    SerialPutS("    The base location is 0xffc00000\n");
    SerialPutS("    The table offset is  "); SerialPutHex(ttl2Offset * 1024); SerialPutChar('\n');
    SerialPutS("    The entry offset is  "); SerialPutHex(((addr >> 12) & 0xff) * 4); SerialPutChar('\n');
    SerialPutS("  The TTL1 management index for this address is "); SerialPutHex(mgmtTtl1Index); SerialPutChar('\n');
#endif

    // -- Map the management address anyway -- will throw errors!!! but can be ignored
    // If the TTL1 Entry for the management address is faulted; create a new TTL2 table
//    if (mgmtTtl1Entry->fault == 0b00) {
//        MmuMakeTtl2Table(mmu, mgmtTtl2Addr);
    SerialPutS("This next call to map address "); SerialPutHex(mgmtTtl2Addr); SerialPutS(" may throw an error\n");
        MmuMapToFrame(mmu, mgmtTtl2Addr, ttl2Frame, true, false);
//    }

#if DEBUG_MMU == 1
    SerialPutS("< Completed the table creation for "); SerialPutHex(addr); SerialPutChar('\n');
#endif
}
