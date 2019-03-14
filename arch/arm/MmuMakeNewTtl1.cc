//===================================================================================================================
//
//  MmuMakeNewTtl1.cc -- Create a new TTL1 table prepared for use
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Mar-07  Initial   0.3.0   ADCL  Initial Version
//
//===================================================================================================================


#include "types.h"
#include "spinlock.h"
#include "pmm.h"
#include "mmu.h"


//
// -- This is the lock that controls access to the address space for initializing the table
//    -------------------------------------------------------------------------------------
Spinlock_t tableInitLock = {0};


//
// -- Create a new TTL1 table, with the necessary TTL2 table for management
//    ---------------------------------------------------------------------
frame_t MmuMakeNewTtl1Table(void)
{
#if 0
    //
    // -- This deserves some comment...  We need a 16K aligned table for 4 frames, so if we allocate 7 frames
    //    we are guaranteed to get one that is aligned properly.  The key here is going to be to find the
    //    correct frame and then free the other 3 frames.
    //    ---------------------------------------------------------------------------------------------------
    frame_t ttl1 = PmmNewFrame(7);          // allocate a ttl1 table
    frame_t ttl2Mgmt = PmmAllocateFrame();      // allocate a ttl2 table for management

    //
    // -- properly align the 4 frames
    //    ---------------------------
    switch (ttl1 & 3) {
    case 0:
        PmmFreeFrame(ttl1 + 4);
        PmmFreeFrame(ttl1 + 5);
        PmmFreeFrame(ttl1 + 6);
        break;

    case 1:
        PmmFreeFrame(ttl1 + 0);
        PmmFreeFrame(ttl1 + 5);
        PmmFreeFrame(ttl1 + 6);
        ttl1 += 1;
        break;

    case 2:
        PmmFreeFrame(ttl1 + 0);
        PmmFreeFrame(ttl1 + 1);
        PmmFreeFrame(ttl1 + 6);
        ttl1 += 2;
        break;

    case 3:
        PmmFreeFrame(ttl1 + 0);
        PmmFreeFrame(ttl1 + 1);
        PmmFreeFrame(ttl1 + 2);
        ttl1 += 3;
        break;
    }


    //
    // -- The first thing to do is to create the management ttl2 recursive map
    //    --------------------------------------------------------------------
    SPIN_BLOCK(tableInitLock) {
        MmuMapToFrame(MMU_NEW_TABLE_INIT, ttl2Mgmt, PG_KRN);

        Ttl2_t *e = (Ttl2_t *)MMU_NEW_TABLE_INIT;
        e[511].frame = ttl2Mgmt;
        e[511].s = 1;
        e[511].apx = 0;
        e[511].ap = 0b11;
        e[511].tex = 0b001;
        e[511].c = 1;
        e[511].b = 1;
        e[511].nG = 0;
        e[511].fault = 0b10;

        MmuUnmapPage(MMU_NEW_TABLE_INIT);
        SpinlockUnlock(&tableInitLock);
    }


    //
    // -- The next thing is to take the 4 ttl2 tables in ttl2Mgmt and map them into ttl1;
    //    separate the activities so that something else can get in when needed
    //    -------------------------------------------------------------------------------
    SPIN_BLOCK(tableInitLock) {
        MmuMapToFrame(MMU_NEW_TABLE_INIT, ttl1 + 1, PG_KRN);

        Ttl1_t *e = &((Ttl1_t *)MMU_NEW_TABLE_INIT)[2044];
        for (int i = 0; i < 4; i ++) {
            e[i].ttl2 = (ttl2Mgmt << 2) + i;
            e[i].fault = 0b01;
        }

        MmuUnmapPage(MMU_NEW_TABLE_INIT);
        SpinlockUnlock(&tableInitLock);
    }


    return ttl1;
#endif
    return 0;
}