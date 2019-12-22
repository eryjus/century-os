//===================================================================================================================
//
//  MmuyInit.cc -- Complete the MMU initialization for the arm architecture
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The goal of this function is to make sure that MMU is fully mapped.  Now that we have access to upper memory
//  functions, we will use them to complete the mappings still pening.  These are:
//  * MMIO space
//  * Interrupt Vector Table (map to existing frame but in upper address space)
//  * Frame buffer
//
//  All other addresses should be mapped properly before handing control to the loader.
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
//  2019-Feb-13  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "serial.h"
#include "mmu.h"
#include "hw-disc.h"
#include "printf.h"
#include "entry.h"
#include "loader.h"


//
// -- Complete the initialization of the Mmu for the loader to function properly
//    --------------------------------------------------------------------------
EXTERN_C EXPORT LOADER
void MmuInit(void)
{
    //
    // -- Next up is the VBAR -- which needs to be mapped.  This one is rather trivial.
    //    -----------------------------------------------------------------------------
    MmuMapToFrame(EXCEPT_VECTOR_TABLE, intTableAddr, PG_KRN | PG_WRT);


    //
    // -- Next up is the MMIO locations.  These are currently at physical address `0x3f000000` to `0x4003ffff`, if
    //    you include the BCM2836 extensions for multiple cores.  The goal here is to re-map these to be in kernel
    //    space at `0xf8000000` to `0xf903ffff`.  This should be trivial, almost.
    //    --------------------------------------------------------------------------------------------------------
    for (archsize_t mmioVirt = MMIO_VADDR, mmioPhys = MMIO_LOADER_LOC;
            mmioPhys <= MMIO_LOADER_END;
            mmioPhys ++, mmioVirt += PAGE_SIZE) {
        MmuMapToFrame(mmioVirt, mmioPhys, PG_KRN | PG_DEVICE | PG_WRT);
    }
}

