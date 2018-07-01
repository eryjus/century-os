//===================================================================================================================
//
//  loader/src/i686/MmuInit.cc -- Initialize the Paging Tables
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-06-27  Initial   0.1.0   ADCL  Initial version
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
    MmuMapToFrame(cr3, GetEbda(), PmmLinearToFrame(GetEbda()));

    // -- Map the memory from 640KB to 1MB
    for (addr = 640 * 1024; addr < 0x100000; addr += 0x1000) {
        MmuMapToFrame(cr3, addr, PmmLinearToFrame(addr));
    }

    // -- Map the GDT/IDT to the final location
    MmuMapToFrame(cr3, 0xff401000, PmmLinearToFrame(0x00000000));

    // -- Identity map the loader
    SerialPutS("Loader Start: ");
    SerialPutHex((uint32_t)&_loaderStart);
    SerialPutS("\nLoader End: ");
    SerialPutHex((uint32_t)&_loaderEnd);
    SerialPutS("\n");

    // -- Identity map the loader
    for (addr = (ptrsize_t)&_loaderStart; addr < (ptrsize_t)&_loaderEnd; addr += 0x1000) {
        MmuMapToFrame(cr3, addr, PmmLinearToFrame(addr));
    }

    // -- Finally, map the frame buffer
    for (frame_t f = PmmLinearToFrame((ptrsize_t)GetFrameBufferAddr()), addr = 0xfb000000;
            f < PmmLinearToFrame((ptrsize_t)GetFrameBufferAddr() + (GetFrameBufferPitch() * GetFrameBufferHeight()));
            f += 0x1000, addr += 0x1000) {
        MmuMapToFrame(cr3, addr, f);
    }

    // -- need to identity-map the stack
    MmuMapToFrame(cr3, 0x200000 - 4096, PmmLinearToFrame(0x200000 - 4096));

    // -- identity map the hardware data strucure
    MmuMapToFrame(cr3, 0x00003000, 3);

    //
    // -- Dump some addresses from the cr3 tables to check validity
    //    ---------------------------------------------------------
    MmuDumpTables(0xfffff000); // Recursive Map
    MmuDumpTables(640 * 1024); // Print 640K
    MmuDumpTables(0xff401000); // The GDT/IDT location
    MmuDumpTables((ptrsize_t)&_loaderStart); // The loader location
    MmuDumpTables(0x28172948);  // An address that should not be mapped
    MmuDumpTables(0xfb000000);  // The start of the frame buffer

    MmuDumpTables((ptrsize_t)LoaderMain);
    MmuDumpTables((ptrsize_t)MmuSwitchPageDir);
}

