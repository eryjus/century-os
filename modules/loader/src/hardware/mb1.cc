//===================================================================================================================
//
//  loader/src/mb1.c -- This is the parser for the Multiboot 1 information structure
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-06-05  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "serial.h"
#include "hw-disc.h"


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
MB1 *mb1Data = 0;


//
// -- A quick MACRO to help determine if a flag is set
//    ------------------------------------------------
#define CHECK_FLAG(f) (mb1Data->flags && (1<<f))


//
// -- Parse the Multiboot 1 header
//    ----------------------------
void Mb1Parse(void)
{
    if (!mb1Data) return;


    //
    // -- Check for basic memory information
    //    ----------------------------------
    if (CHECK_FLAG(0)) {
        SerialPutS("Setting basic memory information");
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
        // TODO: Implement this feature
    }


    //
    // -- Check for the module information -- we will need this for the additional loaded modules (i.e. the kernel)
    //    ---------------------------------------------------------------------------------------------------------
    if (CHECK_FLAG(3)) {
        // TODO: Implement this feature
    }


    //
    // -- We skip flag 4 since we will never be an a.out-type executable.  Check for ELF symbols
    //    --------------------------------------------------------------------------------------
    if (CHECK_FLAG(5)) {
        // TODO: Implement this feature
    }


    //
    // -- Check for Memory Map data, which we will require
    //    ------------------------------------------------
    if (CHECK_FLAG(6)) {
        // TODO: Implement this feature
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
        // TODO: Implmement this feature
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
        // TODO: Implmement this feature
    }
}
