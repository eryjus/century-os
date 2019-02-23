//===================================================================================================================
//
//  MmuInit.cc -- Complete the initialization for the MMU for ARM
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  While the MMU is up and running, the OS structures to manage the MMU are not set up yet.  The purpose of this
//  function is to make sure the MMU is in a state the kernel can take over this responsibility.  There are still
//  several things that need to still take place before I can turn over responsibilty to the kernel:
//  * The management mappings need to be completed to the proper locations (both the ttl1 and ttl2 tables)
//  * The frame buffer needs to get mapped
//  * The MMIO addresses need to be mapped to the kernel locations
//  * Map the exception vector table (VBAR)
//  * Map a Kernel Stack
//
//  It is prudent to document how to take an address and pick it apart into the different components that are needed
//  for managing the paging tables.  There are several to consider:
//  1.  The index into the TTL1 table
//  2.  Given the TTL2 Table address, the index into the TTL2 table
//  3.  Given the address, the overall index into the TTL2 management table which starts at 0xffc00000
//
//  So for an address, it breaks down like this:
//
//
//           +--------  This is the offset into the TTL2 table -- Each table is 1K, so there are 256 entries
//          --
//     0x12345678
//       ---  ---
//        |    +------  This is the offset into the frame -- handled by the MMU
//        |
//        +-----------  This is the offset into the TTL1 table -- used to determine the address of the TTL2 table
//
//  Now, the index into the overall TTL2 table managed at address 0xffc00000, the top 20 bits will index into that
//  table.  Checking our math here, the TTL2 table is 4MB long; each entry 4 bytes.  So, there are 0x100000
//  entries.  1MB can be represented by 20 bits (5 nibbles).
//
//  So, I also want to make sure I have the structure documented here.  I am going to do my best to draw with ASCII
//  art.
//
//  The TTL1 table is located at over 4 pages from 0xff404000 to 0xff407000 inclusive.  There are 2 blocks of
//  TTL1 entries we will be concerned with here: 0xff4-0xff7 and 0xffc-0xfff.  The first group is needed to map
//  the TTL1 table for management -- keep in mind here we will only map 4 pages.  The second group is needed to
//  map the TTL2 table for management.  This will be 4MB of mappings and will be an entire frame of TTL2 tables
//  (which is really 4 tables by the way).
//
//  +-------//-------++-------//-------++-------//-------++---------------------//---------------------------+
//  |                ||                ||                ||                              |.|F|F|F|F|.|F|F|F|F|
//  |   0xff404000   ||   0xff405000   ||   0xff406000   ||       0xff407000             |.|F|F|F|F|.|F|F|F|F|
//  |                ||                ||                ||                              |.|4|5|6|7|.|C|D|E|F|
//  +-------//-------++-------//-------++-------//-------++---------------------//---------------------------+
//
//  So, the TTL1 management table will look like this:
//
//  0xff400000:
//   ff4___________________ ff5___ ff6___ ff7___
//  +-------------------//-+--//--+--//--+--//--+      * Entry 04 will point to the frame for 0xff404000
//  |-|-|-|-|0|0|0|0|.|    |      |      |      |      * Entry 05 will point to the frame for 0xff405000
//  |-|-|-|-|4|5|6|7|.|    |      |      |      |      * Entry 06 will point to the frame for 0xff406000
//  +-------------------//-+--//--+--//--+--//--+      * Entry 07 will point to the frame for 0xff407000
//
//  This then leaves the TTL2 management addresses.  This is a 4MB block that needs to be managed.  This area can
//  be managed with a single frame or 4 TTL2 tables inserted into the TTL1 table at indices 0xffc, ffd, ffe, fff.
//  So, this is the last group above.  This will look like the following:
//
//  0xffc00000:
//   ffc___ ffd___ ffe___ fff
//  +--//--+--//--+--//--+--//------------------------+      * Entry fc will not point to anything on init
//  |      |      |      |                    |F|F|F|F|      * Entry fd will not point to anything on init
//  |      |      |      |                    |C|D|E|F|      * Entry fe will not point to anything on init
//  +--//--+--//--+--//--+--//------------------------+      * Entry ff will be recursively pointed to this frame
//
//  Now, this is not to say that not other entries will be initialized.  Quite the contrary.  I am just saying that
//  the other entries are not needed for managing the paging tables.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-15  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "loader.h"
#include "pmm.h"
#include "cpu.h"
#include "mmu.h"


//
// -- complete the mmu initialization
//    -------------------------------
void __ldrtext MmuInit(void)
{
    //
    // -- the first order of business is to map the ttl1 table into its management address.  This will mean
    //    creating a new ttl2 table for this location since it most likely does nto exist.  The target location
    //    for the table is `TTL1_KRN_VADDR`.
    //    -----------------------------------------------------------------------------------------------------
    Ttl1_t *ttl1Table = (Ttl1_t *)mmuLvl1Table;
    Ttl1_t *ttl1Entry = &ttl1Table[(TTL1_KRN_VADDR >> 20) & 0xffc];     // adjusted for %4 == 0
    Ttl2_t *ttl2Table = 0;
    int idx;
    frame_t ttl1Frame;

    // -- it has not been touched yet, so we need a frame for the TTL2 table
    if (ttl1Entry->fault == 0b00) {
        frame_t ttl2 = NextEarlyFrame();
        PmmAllocFrame(ttl2);
        void *addr = (void *)(ttl2 << 12);
        kMemSetB(addr, 0, FRAME_SIZE);

        kprintf("Against the TTL1 table at %p:\n", mmuLvl1Table);

        for (int i = 0; i < 4; i ++) {
            kprintf("Mapping a new ttl2 table at address %p to table %p\n", &ttl1Entry[i], (ttl2<<2) + i);
            ttl1Entry[i].ttl2 = (ttl2 << 2) + i;
            ttl1Entry[i].fault = 0b01;
        }
    }

    // -- now, get the right table and map the page
    ttl1Entry = &ttl1Table[TTL1_KRN_VADDR >> 20];
    ttl2Table = (Ttl2_t *)((ttl1Entry->ttl2) << 10);

    kprintf("Preparing to populate the TTL2 table at %p\n", ttl2Table);

    // -- for the ttl1 table, we need to map 4 pages
    ttl1Frame = (((archsize_t)mmuLvl1Table) >> 12);
    idx = (TTL1_KRN_VADDR >> 12) & 0xff;
    for (int i = 0; i < 4; i ++) {
        ttl2Table[idx + i].frame = ttl1Frame + i;
        ttl2Table[idx + i].s = 1;
        ttl2Table[idx + i].apx = 0;
        ttl2Table[idx + i].ap = 0b11;
        ttl2Table[idx + i].tex = 0b001;
        ttl2Table[idx + i].c = 1;
        ttl2Table[idx + i].b = 1;
        ttl2Table[idx + i].nG = 0;
        ttl2Table[idx + i].fault = 0b10;
    }


    //
    // -- From here on, we can use the proper kernel address for the TTL1 table.
    //
    //    The next order of business is to build out the ttl2 tables for the management address for the ttl2 tables.
    //    This seems recursive, but I plan to use the address space from 0xffc00000 to 0xffffffff to manage all of
    //    the ttl2 tables at least for the kernel space.  Therefore, for ttl1 entries 0xffc, 0xffd, 0xffe, and 0xfff,
    //    I need to map those into a frame.  The good news here is that it all fits in one frame.  As a result, this
    //    is going to look a LOT like the code above.  Why not do it as its own function?  I should only need to
    //    do this 3 times and for specific instances.  The general function is another story, but it requires this
    //    table to work.  Doing this now will also make the next step trivial.
    //    -----------------------------------------------------------------------------------------------------------
    ttl1Entry = KRN_TTL1_ENTRY4(TTL2_KRN_VADDR);     // adjusted for %4 == 0
    kprintf("Build out the TTL2 table for address %p\n", TTL2_KRN_VADDR);
    frame_t ttl2;

    // -- it has not been touched yet, so we need a frame for the TTL2 table
    if (ttl1Entry->fault == 0b00) {
        ttl2 = NextEarlyFrame();
        PmmAllocFrame(ttl2);
        void *addr = (void *)(ttl2 << 12);
        kMemSetB(addr, 0, FRAME_SIZE);

        for (int i = 0; i < 4; i ++) {
            kprintf(".. Setting the ttl2 table for MB %x\n", (TTL2_KRN_VADDR >> 20) + i);
            ttl1Entry[i].ttl2 = (ttl2 << 2) + i;
            ttl1Entry[i].fault = 0b01;
        }
    } else ttl2 = ttl1Entry->ttl2;


    //
    // -- The next task here is to recursively map the last entry in the TTL2 table to itself.  It is this mapping
    //    that makes management possible from kernel space.  Note that there are actually 4 tables in this frame
    //    and we need to get to the last entry of the last table.  in the index 0x3ff, the '3' is the table and
    //    the 'ff' is the index -- conceptually anyway.
    //    ---------------------------------------------------------------------------------------------------------
    Ttl2_t *ttl2Mgmt;

    ttl2Mgmt = (Ttl2_t *)(ttl2 << 12);

    ttl2Mgmt[0x3ff].frame = ttl2;
    ttl2Mgmt[0x3ff].s = 1;
    ttl2Mgmt[0x3ff].apx = 0;
    ttl2Mgmt[0x3ff].ap = 0b11;
    ttl2Mgmt[0x3ff].tex = 0b001;
    ttl2Mgmt[0x3ff].c = 1;
    ttl2Mgmt[0x3ff].b = 1;
    ttl2Mgmt[0x3ff].nG = 0;
    ttl2Mgmt[0x3ff].fault = 0b10;


    //
    // -- From here, we can use the TTL2 management addresses to map frames as well.
    //
    //    now that we have the ttl2 table to manage all the ttl2 tables, we need to map the existing ttl2 tables
    //    into these entries to be managed.  However, we only want the addresses that are `>= 0x80000000`.  First,
    //    everything that is in low memory will be abandonned when we clean up after the kernel.  Second, this
    //    particular table will be used only for the kernel space -- there is another table that will be built
    //    for the user space addresses.
    //    --------------------------------------------------------------------------------------------------------

    // -- since there are 4 ttl2 tables per frame, we can jump by 4 tables (0x100 * 4)
    for (unsigned int i = 0x80000000; i >= 0x80000000; i += 0x400000) {    // will loop through 0; code for it
        ttl1Entry = KRN_TTL1_ENTRY(i);

        // -- now is there a ttl2 table attached?
        if (ttl1Entry->fault == 0b01) {
            kprintf("Writing a management mapping for %p\n", i);
            ttl2Mgmt = KRN_TTL2_MGMT(i);

            ttl2Mgmt->frame = ttl1Entry->ttl2 >> 2;
            ttl2Mgmt->s = 1;
            ttl2Mgmt->apx = 0;
            ttl2Mgmt->ap = 0b11;
            ttl2Mgmt->tex = 0b001;
            ttl2Mgmt->c = 1;
            ttl2Mgmt->b = 1;
            ttl2Mgmt->nG = 0;
            ttl2Mgmt->fault = 0b10;
        }
    }


    //
    // -- from here on, we can use the general-purpose functions to perform our work since the management tables
    //    are fully assembled.  The next step is to map the frame buffer.
    //    ------------------------------------------------------------------------------------------------------
    archsize_t fbAddr = ((archsize_t)GetFrameBufferAddr());        // better be aligned to frame boundary!!!
    size_t fbSize = GetFrameBufferPitch() * GetFrameBufferHeight();
    fbSize += (fbSize&0xfff?0x1000:0);      // -- this is adjusted so that when we `>> 12` we are mapping enough
    fbSize >>= 12;                          // -- now, the number of frames to map
    archsize_t off = 0;

    kprintf("MMU: Mapping the frame buffer at %p for %x frames\n", fbAddr, fbSize);

    while (fbSize) {
        MmuMapToFrame(MMU_FRAMEBUFFER + off, fbAddr >> 12, 0);
        off += 0x1000;
        fbAddr += 0x1000;
        fbSize --;
    }

    // -- goose the config to the correct fb address
    SetFrameBufferAddr((uint16_t *)MMU_FRAMEBUFFER);


    //
    // -- Next up is the MMIO locations.  These are currently at physical address `0x3f000000` to `0x4003ffff`, if
    //    you include the BCM2836 extensions for multiple cores.  The goal here is to re-map these to be in kernel
    //    space at `0xf8000000` to `0xf903ffff`.  This should be trivial, almost.
    //    --------------------------------------------------------------------------------------------------------
    archsize_t mmioPhys = 0x3f000;                      // address converted to frame
    archsize_t mmioVirt = 0xf8000000;
    int count = (0x40040000 - 0x3f000000) >> 12;

    kprintf("MMU: Mapping the mmio addresses to %p for %x frames\n", mmioVirt, count + 1);

    while (count >= 0) {
        MmuMapToFrame(mmioVirt, mmioPhys, 0);
        mmioVirt += 0x1000;
        mmioPhys ++;
        count --;
    }


    //
    // -- Next up is the VBAR -- which needs to be mapped.  This one is rather trivial.
    //    -----------------------------------------------------------------------------
    MmuMapToFrame(EXCEPT_VECTOR_TABLE, intTableAddr, 0);


    //
    // -- the next order of business is to set up the stack (but do not yet change to it -- we are in a function
    //    and will need to clean up and return from this function and getting that right is risky).
    //    ------------------------------------------------------------------------------------------------------
    archsize_t stackLoc = STACK_LOCATION;
    for (int i = 0; i < STACK_SIZE; i += 0x1000, stackLoc += 0x1000) {
        MmuMapToFrame(stackLoc, PmmNewFrame(1), 0);
    }


    kprintf("MMU: The MMU is initialized\n");
}



