//===================================================================================================================
//
//  InitGdt.cc -- Initialize the GDT into its final location
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This function will be used to initialize the GDT to its final location.  From the memory map located here,
//  http://eryjus.ddns.net:3000/projects/century-os/wiki/Low_Memory_Usage_Map, the final GDT will be located at
//  physical address `0x10000`.  It may take up several frames depending on the number of CPUs that we support
//  and/or the number of CPUs we discover.  Currently, this is a small number of CPUs.
//
//  The number of GDT entries we need is easily calculated: 9 + (CPU_count * 3).  Each GDT Entry is 8 bytes long.
//  Therefore, the number of CPUs we can support in a single frame is: floor(((4096 / 8) - 9) / 3) = 167.  167
//  CPUs is quite simply a ridiculous number of CPUs at this juncture.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Jan-05  Initial  v0.5.0e  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "mmu.h"
#include "printf.h"
#include "cpu.h"


//
// -- Initialize the GDT to its final location
//    ----------------------------------------
EXTERN_C EXPORT LOADER
void InitGdt(void)
{
    // -- first, calculate the number of frames we need for the GDT
    size_t gdtEntries = (cpus.cpusDiscovered * 3) + 9;
    size_t gdtFrames = ((gdtEntries * 8) / PAGE_SIZE) + ((gdtEntries * 8) & (PAGE_SIZE - 1) ? 1 : 0);


    // -- Now, we can map the GDT pages and clear the table
    archsize_t vGdt = X86_VIRT_GDT;
    frame_t fGdt = X86_PHYS_GDT >> 12;

    for (size_t i = 0; i < gdtFrames; i ++, fGdt ++, vGdt += PAGE_SIZE) {
        MmuMapToFrame(vGdt, fGdt, PG_KRN | PG_WRT);
    }

    kMemSetB((void *)X86_VIRT_GDT, 0x00, gdtFrames * PAGE_SIZE);


    // -- Now, we start populating the GDT Entries -- first the 9 standard entries
    uint64_t *gdt = (uint64_t *)X86_VIRT_GDT;

    gdt[0] = 0x0000000000000000;        // 0x00: NULL GDT Entry (required)
    gdt[1] = 0x00cf9a000000ffff;        // 0x08: kernel code
    gdt[2] = 0x00cf92000000ffff;        // 0x10: kernel stack (and data)
    gdt[3] = 0x00cffa000000ffff;        // 0x18: user code
    gdt[4] = 0x00cff2000000ffff;        // 0x20: user stack (and data)
    gdt[5] = 0x00cf92000000ffff;        // 0x28: kernel data
    gdt[6] = 0x00cff2000000ffff;        // 0x30: user data
    gdt[7] = 0x00cf9a000000ffff;        // loader code (not used)
    gdt[8] = 0x00cf92000000ffff;        // loader data and stack (not used)


    // -- now the TSS and gs segment selector for each CPU (Redmine #433 goes here)
    for (int i = 0; i < cpus.cpusDiscovered; i ++) {
        gdt[ 9 + (i * 3)] = 0x0000000000000000;     // 32-bit TSS for this CPU
        gdt[10 + (i * 3)] = 0x0000000000000000;     // 64-bit TSS for this CPU
        gdt[11 + (i * 3)] = 0x0000000000000000;     // `gs` for this CPU
    }


    // -- Finally we need to load the new GDT
    struct {
        uint16_t size;
        uintptr_t loc;
    } __attribute__((packed)) gdtRec = {
        (uint16_t)((gdtEntries * 8) - 1),
        X86_VIRT_GDT,
    };


    // -- load the GDT register
    LoadGdt(&gdtRec);
}

