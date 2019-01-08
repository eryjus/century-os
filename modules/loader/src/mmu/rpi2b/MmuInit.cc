//===================================================================================================================
//
//  MmuInit.cc -- Initialize the Memory Management Unit for rpi2b
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-18  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "pmm.h"
#include "hw-disc.h"
#include "serial-loader.h"
#include "mmu-loader.h"


extern uint8_t _loaderStart[];
extern uint8_t _loaderEnd[];
extern "C" void LoaderMain(void);

frame_t allocFrom;
ptrsize_t ttl1;


void MmuInit(void)
{
    ptrsize_t addr;
    frame_t f;

    SerialPutS("Set up the TTL1 management table\n");
    allocFrom = PmmLinearToFrame(GetUpperMemLimit()) - 0x1000;

    // -- get 4 frames from the PMM for the ttl1 table; 4 frames = 16K
    ttl1 = PmmFrameToLinear(allocFrom);
    PmmAllocFrameRange(allocFrom, 4);
    allocFrom += 4;

    // -- set up the ttl1 and clear it
    kMemSetB((void *)ttl1, 0, 1024 * 16);

    // -- Map the TTL1 table to location TTL1_VADDR
    MmuMapToFrame(ttl1, TTL1_VADDR, PmmLinearToFrame(ttl1), true, false);
    MmuMapToFrame(ttl1, TTL1_VADDR + 0x1000, PmmLinearToFrame(ttl1) + 1, true, false);
    MmuMapToFrame(ttl1, TTL1_VADDR + 0x2000, PmmLinearToFrame(ttl1) + 2, true, false);
    MmuMapToFrame(ttl1, TTL1_VADDR + 0x3000, PmmLinearToFrame(ttl1) + 3, true, false);

    // -- Identity map the loader
    SerialPutS("Loader Start: ");
    SerialPutHex((uint32_t)&_loaderStart);
    SerialPutS("\nLoader End: ");
    SerialPutHex((uint32_t)&_loaderEnd);
    SerialPutS("\n");

    // -- Identity map the loader
    SerialPutS("Identity Map the Loader");
    for (addr = (ptrsize_t)&_loaderStart; addr < (ptrsize_t)&_loaderEnd; addr += 0x1000) {
        SerialPutChar('.');
        MmuMapToFrame(ttl1, addr, PmmLinearToFrame(addr), true, false);
    }
    SerialPutChar('\n');

    // -- Map the frame buffer
    SerialPutS("Map FrameBuffer");

    // -- when there is a partial frame, we need to get that adjusted here, before we call PmmLinearToFrame()
    ptrsize_t fbEnd = (ptrsize_t)GetFrameBufferAddr() + (GetFrameBufferPitch() * GetFrameBufferHeight());
    if (fbEnd & 0xfff) fbEnd += 0x1000;

    for (f = PmmLinearToFrame((ptrsize_t)GetFrameBufferAddr()), addr = FRAME_BUFFER_VADDR;
            f < PmmLinearToFrame(fbEnd); f ++, addr += 0x1000) {
        SerialPutChar('.');
        MmuMapToFrame(ttl1, addr, f, true, false);
    }
    SerialPutHex(f);
    SerialPutChar('\n');

    // -- need to identity-map the stack
    SerialPutS("Map Stack\n");
    MmuMapToFrame(ttl1, 0x200000 - 4096, PmmLinearToFrame(0x200000 - 4096), true, false);

    // -- identity map the hardware data strucure
    SerialPutS("Map Hardware Discovery Struture\n");
    MmuMapToFrame(ttl1, 0x00003000, 3, true, false);

    // -- Map the interrupt vector table
    SerialPutS("Map Interrupt Vector Table to some new frame for future use\n");
    MmuMapToFrame(ttl1, EXCEPT_VECTOR_TABLE, PmmNewFrame(), true, false);

    // -- identity map the MMIO addresses 0x3f000000 to 0x40000100
    SerialPutS("Identity map MMIO");
    for (addr = 0x3f000000; addr < 0x40000100; addr += 4096) {
        SerialPutChar('.');
        MmuMapToFrame(ttl1, addr, PmmLinearToFrame(addr), true, true);
    }

    // -- also map the MMIO addresses into upper memory
    SerialPutS("\nMap  MMIO to kernel space");
    for (addr = 0xf2000000, f = PmmLinearToFrame(0x3f000000); addr < 0xf3000100; addr += 4096, f ++) {
        SerialPutChar('.');
        MmuMapToFrame(ttl1, addr, f, true, true);
    }

    //
    // -- Dump some addresses from the cr3 tables to check validity
    //    ---------------------------------------------------------
    SerialPutS("\nChecking our work\n");
    MmuDumpTables(TTL1_VADDR); // The TTL1 location
    MmuDumpTables(TTL1_VADDR + 0x1000);
    MmuDumpTables(TTL1_VADDR + 0x2000);
    MmuDumpTables(TTL1_VADDR + 0x3000);
    MmuDumpTables((ptrsize_t)&_loaderStart); // The loader location
    MmuDumpTables(0x28172948);  // An address that should not be mapped
    MmuDumpTables(FRAME_BUFFER_VADDR);  // The start of the frame buffer
    MmuDumpTables(0xfd010000);  // One Page Fault Address
    MmuDumpTables((ptrsize_t)LoaderMain);
    MmuDumpTables(0x80000000);
    MmuDumpTables((ptrsize_t)MmuEnablePaging);
    MmuDumpTables((ptrsize_t)EXCEPT_VECTOR_TABLE);
}