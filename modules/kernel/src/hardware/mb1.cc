//===================================================================================================================
//
//  mb1.c -- This is the parser for the Multiboot 1 information structure
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Jun-05  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "serial.h"
#include "printf.h"
#include "hw-disc.h"


//
// -- This is the loaded modules block (which will repeat)
//    ----------------------------------------------------
typedef struct Mb1Mods_t {
    uint32_t modStart;
    uint32_t modEnd;
    char *modIdent;
    uint32_t modReserved;
} __attribute__((packed)) Mb1Mods_t;


//
// -- Memory Map entries, which will repeat (pointer points to mmapAddr)
//    ------------------------------------------------------------------
typedef struct Mb1MmapEntry_t {
    uint32_t mmapSize;
    uint64_t mmapAddr;
    uint64_t mmapLength;
    uint32_t mmapType;
} __attribute__((packed)) Mb1MmapEntry_t;


//
// -- This is the Multiboot 1 information structure as defined by the spec
//    --------------------------------------------------------------------
typedef struct MB1 {
    //
    // -- These flags indicate which data elements have valid data
    //    --------------------------------------------------------
    const uint32_t flags;

    //
    // -- The basic memory limits are valid when flag 0 is set; these values are in kilobytes
    //    -----------------------------------------------------------------------------------
    const uint32_t availLowerMem;
    const uint32_t availUpperMem;

    //
    // -- The boot device when flag 1 is set
    //    ----------------------------------
    const uint32_t bootDev;

    //
    // -- The command line for this kernel when flag 2 is set
    //    ---------------------------------------------------
    const uint32_t cmdLine;

    //
    // -- The loaded module list when flag 3 is set
    //    -----------------------------------------
    const uint32_t modCount;
    const uint32_t modAddr;

    //
    // -- The ELF symbol information (a.out-type symbols are not supported) when flag 5 is set
    //    ------------------------------------------------------------------------------------
    const uint32_t shdrNum;                 // may still be 0 if not available
    const uint32_t shdrSize;
    const uint32_t shdrAddr;
    const uint32_t shdrShndx;

    //
    // -- The Memory Map information when flag 6 is set
    //    ---------------------------------------------
    const uint32_t mmapLen;
    const uint32_t mmapAddr;

    //
    // -- The Drives information when flag 7 is set
    //    -----------------------------------------
    const uint32_t drivesLen;
    const uint32_t drivesAddr;

    //
    // -- The Config table when flag 8 is set
    //    -----------------------------------
    const uint32_t configTable;

    //
    // -- The boot loader name when flag 9 is set
    //    ---------------------------------------
    const uint32_t bootLoaderName;

    //
    // -- The APM table location when bit 10 is set
    //    -----------------------------------------
    const uint32_t apmTable;

    //
    // -- The VBE interface information when bit 11 is set
    //    ------------------------------------------------
    const uint32_t vbeControlInfo;
    const uint32_t vbeModeInfo;
    const uint16_t vbeMode;
    const uint16_t vbeInterfaceSeg;
    const uint16_t vbeInterfaceOff;
    const uint16_t vbeInterfaceLen;

    //
    // -- The FrameBuffer information when bit 12 is set
    //    ----------------------------------------------
    const uint64_t framebufferAddr;
    const uint32_t framebufferPitch;
    const uint32_t framebufferWidth;
    const uint32_t framebufferHeight;
    const uint8_t framebufferBpp;
    const uint8_t framebufferType;
    union {
        struct {
            const uint8_t framebufferRedFieldPos;
            const uint8_t framebufferRedMaskSize;
            const uint8_t framebufferGreenFieldPos;
            const uint8_t framebufferGreenMaskSize;
            const uint8_t framebufferBlueFieldPos;
            const uint8_t framebufferBlueMaskSize;
        };
        struct {
            const uint32_t framebufferPalletAddr;
            const uint16_t framebufferPalletNumColors;
        };
    };
} __attribute__((packed)) MB1;


//
// -- This is the address of the MB1 Multiboot information structure
//    --------------------------------------------------------------
extern MB1 *mb1Data;


//
// -- A quick MACRO to help determine if a flag is set
//    ------------------------------------------------
#define CHECK_FLAG(f) (mb1Data->flags & (1<<f))


//
// -- Parse the Multiboot 1 header
//    ----------------------------
void __ldrtext Mb1Parse(void)
{
    if (!mb1Data) return;

    kprintf("Found the mbi structure at %p\n", mb1Data);
    kprintf("  The flags are: %p\n", mb1Data->flags);

    //
    // -- Check for basic memory information
    //    ----------------------------------
    if (CHECK_FLAG(0)) {
        kprintf("Setting basic memory information\n");
        SetAvailLowerMem(mb1Data->availLowerMem);
        SetAvailUpperMem(mb1Data->availUpperMem);
    }


    //
    // -- Check for boot device information
    //    ---------------------------------
    if (CHECK_FLAG(1)) {
        // TODO: Implement this feature
    }


    //
    // -- Check for the command line -- we might have parameters to the loader
    //    --------------------------------------------------------------------
    if (CHECK_FLAG(2)) {
        kprintf("Identifying command line information: %s\n", mb1Data->cmdLine);
    }


    //
    // -- Check for the module information -- we will need this for the additional loaded modules (i.e. the kernel)
    //    ---------------------------------------------------------------------------------------------------------
    if (CHECK_FLAG(3)) {
        uint32_t i;
        Mb1Mods_t *m;

        kprintf("Module information present\n");

        for (m = (Mb1Mods_t *)mb1Data->modAddr, i = 0; i < mb1Data->modCount; i ++) {
            kprintf("   Found Module: %s\n", m[i].modIdent);
            kprintf("    .. Name is at : %p\n", m[i].modIdent);
            kprintf("    .. Start: %p\n", m[i].modStart);
            kprintf("    .. End: %p\n", m[i].modEnd);

            AddModule(m[i].modStart, m[i].modEnd, m[i].modIdent);
        }
    }


    //
    // -- We skip flag 4 since we will never be an a.out-type executable.  Check for ELF symbols with flag 5
    //    --------------------------------------------------------------------------------------------------
    if (CHECK_FLAG(5)) {
        // TODO: Implement this feature
    }


    //
    // -- Check for Memory Map data, which we will require
    //    ------------------------------------------------
    if (CHECK_FLAG(6)) {
        kprintf("Setting memory map data\n");
        uint32_t size = mb1Data->mmapLen;
        Mb1MmapEntry_t *entry = (Mb1MmapEntry_t *)(((uint32_t)mb1Data->mmapAddr));
        while (size) {
            if (entry->mmapType == 1) {
                kprintf("  iterating in mmap\n");
                kprintf("    entry address is: %p\n", entry);
                kprintf("    entry type is: %x\n", entry->mmapType);
                kprintf("    entry base is: %p : %p\n", (uint32_t)(entry->mmapAddr>>32),
                        (uint32_t)entry->mmapAddr&0xffffffff);
                kprintf("    entry length is: %p : %p\n", (uint32_t)(entry->mmapLength>>32),
                        (uint32_t)entry->mmapLength&0xffffffff);
                kprintf("    entry size is: %p\n", entry->mmapSize);
            }

#if DEBUG_MB==1
            kprintf("  MMap Entry count is: %x\n", GetMMapEntryCount());

            uint32_t *wrk = (uint32_t *)entry;
            for (int i = 0; i < 6; i ++) {
                kprintf("        32-bit entry %x contains %p\n", i, wrk[i]);
            }

            kprintf("  Through all entries...\n");
#endif

            if (entry->mmapType == 1) AddAvailMem(entry->mmapAddr, entry->mmapLength);
            uint64_t newLimit = entry->mmapAddr + entry->mmapLength;
            if (newLimit > GetUpperMemLimit()) SetUpperMemLimit(newLimit);
            size -= (entry->mmapSize + 4);
            entry = (Mb1MmapEntry_t *)(((uint32_t)entry) + entry->mmapSize + 4);
        }

        kprintf("Memory Map is complete\n");
    }


    //
    // -- Check for the drives information
    //    --------------------------------
    if (CHECK_FLAG(7)) {
        // TODO: Implement this feature
    }


    //
    // -- Check for the config data information
    //    -------------------------------------
    if (CHECK_FLAG(8)) {
        // TODO: Implmement this feature
    }


    //
    // -- Check for the boot loader name
    //    ------------------------------
    if (CHECK_FLAG(9)) {
        kprintf("Identifying bootloader: %s\n", mb1Data->bootLoaderName);
    }


    //
    // -- Check for the APM table
    //    -----------------------
    if (CHECK_FLAG(10)) {
        // TODO: Implmement this feature
    }


    //
    // -- Check for the VBE table
    //    -----------------------
    if (CHECK_FLAG(11)) {
        // TODO: Implmement this feature
    }


    //
    // -- Check for the framebuffer information (GRUB specific; see
    //    https://www.gnu.org/software/grub/manual/multiboot/multiboot.html)
    //    ------------------------------------------------------------------
    if (CHECK_FLAG(12)) {
        kprintf("Capturing framebuffer information\n");
        SetFrameBufferAddr((uint16_t *)mb1Data->framebufferAddr);
        SetFrameBufferPitch(mb1Data->framebufferPitch);
        SetFrameBufferWidth(mb1Data->framebufferWidth);
        SetFrameBufferHeight(mb1Data->framebufferHeight);
        SetFrameBufferBpp(mb1Data->framebufferBpp);
        SetFrameBufferType((FrameBufferType)mb1Data->framebufferType);

        kprintf("Frame Buffer is at: %p; The pitch is: %p; The height is: %p\n",
                mb1Data->framebufferAddr, mb1Data->framebufferPitch, mb1Data->framebufferHeight);
    }

    kprintf("Done parsing MB1 information\n");
}
