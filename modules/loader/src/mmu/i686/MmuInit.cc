//===================================================================================================================
//
//  MmuInit.cc -- Initialize the Paging Tables
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Jun-27  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "cpu.h"
#include "pmm.h"
#include "hw-disc.h"
#include "serial.h"
#include "mmu.h"


//
// -- Some symbols provided by the linker
//    -----------------------------------
extern uint8_t _loaderStart[];
extern uint8_t _loaderEnd[];
extern "C" void LoaderMain(void);


//
// -- Emulate the CR3 register here so that we can build the tables
//    -------------------------------------------------------------
ptrsize_t cr3 = 0x1000;


//
// -- Perform the MMU structure initialization
//    ----------------------------------------
void MmuInit(void)
{
    // -- clear out the Page Directory
    kMemSetB((void *)cr3, 0, 4096);
    pageEntry_t *recurse = &((pageEntry_t *)cr3)[1023];
    recurse->frame = PmmLinearToFrame(cr3);
    recurse->p = 1;
    recurse->rw = 1;
    recurse->us = 1;

    ptrsize_t addr;

    // -- Map the EBDA and the rest of the memory up to 1MB
    SerialPutS("EBDA: ");
    SerialPutHex((uint32_t)GetEbda());
    SerialPutS("\n");

    // -- Map the EBDA
    SerialPutS("Map the EBDA\n");
    MmuMapToFrame(cr3, GetEbda(), PmmLinearToFrame(GetEbda()), true, false);

    // -- Map the memory from 640KB to 1MB
    SerialPutS("Map Upper Memory");
    for (addr = 640 * 1024; addr < 0x100000; addr += 0x1000) {
        SerialPutChar('.');
        MmuMapToFrame(cr3, addr, PmmLinearToFrame(addr), true, true);
    }

    // -- Map the GDT/IDT to the final location
    SerialPutS("\nMap GDT/IDT\n");
    MmuMapToFrame(cr3, 0xff401000, PmmLinearToFrame(0x00000000), true, false);

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
        MmuMapToFrame(cr3, addr, PmmLinearToFrame(addr), true, false);
    }

    // -- Finally, map the frame buffer
    SerialPutS("\nMap FrameBuffer "); SerialPutHex(PmmLinearToFrame((ptrsize_t)GetFrameBufferAddr() + (GetFrameBufferPitch() * GetFrameBufferHeight())));
    for (frame_t f = PmmLinearToFrame((ptrsize_t)GetFrameBufferAddr()), addr = 0xfb000000;
            f < PmmLinearToFrame((ptrsize_t)GetFrameBufferAddr() + (GetFrameBufferPitch() * GetFrameBufferHeight()));
            f ++, addr += 0x1000) {
        SerialPutChar('.');
        MmuMapToFrame(cr3, addr, f, true, true);
    }

    // -- need to identity-map frame 0
    SerialPutS("\nMap Frame 0\n");
    MmuMapToFrame(cr3, 0, 0, true, false);

    // -- need to identity-map the stack
    SerialPutS("Map Stack\n");
    MmuMapToFrame(cr3, 0x200000 - 4096, PmmLinearToFrame(0x200000 - 4096), true, false);

    // -- identity map the hardware data strucure
    SerialPutS("Map Hardware Discovery Struture\n");
    MmuMapToFrame(cr3, 0x00003000, 3, true, false);

    // -- we need to create a page table for the kernel heap, which will get mapped later
    pageEntry_t *pdTable = (pageEntry_t *)cr3;
    MmuGetTableEntry(pdTable, 0xd0000000, 22, true);

    //
    // -- Dump some addresses from the cr3 tables to check validity
    //    ---------------------------------------------------------
    SerialPutS("Checking our work\n");
    MmuDumpTables(0xfffff000); // Recursive Map
    MmuDumpTables(640 * 1024); // Print 640K
    MmuDumpTables(0xff401000); // The GDT/IDT location
    MmuDumpTables((ptrsize_t)&_loaderStart); // The loader location
    MmuDumpTables(0x28172948);  // An address that should not be mapped
    MmuDumpTables(0xfb000000);  // The start of the frame buffer
    MmuDumpTables(0xfd010000);  // One Page Fault Address

    MmuDumpTables((ptrsize_t)LoaderMain);
    MmuDumpTables((ptrsize_t)MmuSwitchPageDir);
}

