//===================================================================================================================
//
//  mb2.c -- This is the parser for the Multiboot 1 information structure
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Jun-07  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "serial-loader.h"
#include "hw-disc.h"


//
// -- MB2 tag types
//    -------------
enum {
    MB2_TAG_LAST_TAG = 0,
    MB2_TAG_CMD_LINE = 1,
    MB2_TAG_LOADER = 2,
    MB2_TAG_MODULE = 3,
    MB2_TAG_BASIC_MEM = 4,
    MB2_TAG_BOOT_DEV = 5,
    MB2_TAG_MMAP = 6,
    MB2_TAG_VBE = 7,
    MB2_TAG_FRAMEBUFFER = 8,
    MB2_TAG_ELF_SYMS = 9,
    MB2_TAG_APM = 10,
};


//
// -- The fixed multiboot info structure elements
//    -------------------------------------------
typedef struct Mb2Fixed_t {
    uint32_t totalSize;
    uint32_t reserved;
} __attribute__((packed)) Mb2Fixed_t;


//
// -- This is the basic tag header information -- every tag has one
//    -------------------------------------------------------------
typedef struct Mb2BasicTag_t {
    uint32_t type;
    uint32_t size;
} __attribute__((packed)) Mb2BasicTag_t;


//
// -- Basic memory info
//    -----------------
typedef struct Mb2BasicMem_t {
    Mb2BasicTag_t tag;         // type == 4; size == 16
    uint32_t memLower;
    uint32_t memUpper;
} __attribute__((packed)) Mb2BasicMem_t;


//
// -- Boot device information
//    -----------------------
typedef struct Mb2BootDevice_t {
    Mb2BasicTag_t tag;         // type == 5; size == 20
    uint32_t biosDev;
    uint32_t partition;
    uint32_t subPartition;
} __attribute__((packed)) Mb2BootDevice_t;


//
// -- The command line for the boot
//    -----------------------------
typedef struct Mb2CmdLine_t {
    Mb2BasicTag_t tag;         // type == 1
    char cmdLine[0];
} __attribute__((packed)) Mb2CmdLine_t;


//
// -- A laoded module
//    ---------------
typedef struct Mb2Module_t {
    Mb2BasicTag_t tag;         // type == 3
    uint32_t modStart;
    uint32_t modEnd;
    char name[0];
} __attribute__((packed)) Mb2Module_t;


//
// -- THe ELF Symbols
//    ---------------
typedef struct Mb2ElfSymbols_t {
    Mb2BasicTag_t tag;         // type == 9
    uint16_t num;
    uint16_t entSize;
    uint16_t shndx;
    uint16_t reserved;
    uint8_t sectionHdrs[0];
} __attribute__((packed)) Mb2ElfSymbols_t;


//
// -- Memory Map
//    ----------
typedef struct Mb2MemMap_t {
    Mb2BasicTag_t tag;         // type == 6
    uint32_t entrySize;
    uint32_t entryVersion;
    struct {
        uint64_t baseAddr;
        uint64_t length;
        uint32_t type;
        uint32_t reserved;
    } entries [0];
} __attribute__((packed)) Mb2MemMap_t;


//
// -- The bootloader name
//    -------------------
typedef struct Mb2Loader_t {
    Mb2BasicTag_t tag;         // type == 2
    char name[0];
} __attribute__((packed)) Mb2Loader_t;


//
// -- The APM Table
//    -------------
typedef struct Mb2Apm_t {
    Mb2BasicTag_t tag;         // type == 10; size == 28
    uint16_t version;
    uint16_t cseg;
    uint32_t offset;
    uint16_t cseg16;
    uint16_t dseg;
    uint16_t flags;
    uint16_t csegLen;
    uint16_t cseg16Len;
    uint16_t dsegLen;
} __attribute__((packed)) Mb2Apm_t;


//
// -- The VBE Table
//    -------------
typedef struct Mb2VbeInfo_t {
    Mb2BasicTag_t tag;         // type == 7; size == 784
    uint16_t vbeMode;
    uint16_t vbeInterfaceSeg;
    uint16_t vbeInterfaceOff;
    uint16_t vbeInterfaceLen;
    uint8_t vbeControlInfo[512];
    uint8_t vbeModeInfo[256];
} __attribute__((packed)) Mb2VbeInfo_t;


//
// -- The FrameBuffer Info
//    -------------
typedef struct Mb2FbInfo_t {
    Mb2BasicTag_t tag;         // type == 8
    uint64_t fbAddr;
    uint32_t fbPitch;
    uint32_t fbWidth;
    uint32_t fbHeight;
    uint8_t fbBpp;
    uint8_t fbType;
    union {
        struct {       // when fbType == 0
            uint32_t palletColors;
            struct {
                uint8_t red;
                uint8_t green;
                uint8_t blue;
            } color [0];
        } pallet;
        struct {          // when fbType == 1
            uint8_t redFieldPos;
            uint8_t redMaskSize;
            uint8_t greenFieldPos;
            uint8_t greenMaskSize;
            uint8_t blueFieldPos;
            uint8_t blueMaskSize;
        } rgb;
    };
} __attribute__((packed)) Mb2FbInfo_t;


//
// -- The multiboot 2 information structure
//    -------------------------------------
void *mb2Data;


//
// -- Mb2Parse() -- Read the multiboot 2 information from the data provided and store it locally
//    ------------------------------------------------------------------------------------------
void Mb2Parse(void)
{
    if (!mb2Data) return;


    uint32_t locn = (uint32_t)mb2Data + sizeof(Mb2Fixed_t);
    bool lastTag = false;

    while (!lastTag) {
        Mb2BasicTag_t *tag = (Mb2BasicTag_t *)locn;
        switch (tag->type) {
        case MB2_TAG_LAST_TAG:
            lastTag = true;
            break;

        case MB2_TAG_CMD_LINE:
            SerialPutS(((Mb2CmdLine_t *)locn)->cmdLine);
            break;

        case MB2_TAG_LOADER:
            SerialPutS(((Mb2Loader_t *)locn)->name);
            SerialPutS("\n");
            break;

        case MB2_TAG_MODULE: {
            SerialPutS("Module information present\n");

            Mb2Module_t *m = (Mb2Module_t *)locn;

            AddModule(m->modStart, m->modEnd, m->name);

            break;
        }

        case MB2_TAG_BASIC_MEM: {
            Mb2BasicMem_t *mem = (Mb2BasicMem_t *)locn;

            SerialPutS("Setting basic memory information\n");
            SetAvailLowerMem(mem->memLower);
            SetAvailUpperMem(mem->memUpper);

            break;
        }

        case MB2_TAG_BOOT_DEV: {
//            Mb2BootDevice_t *dev = (Mb2BootDevice_t *)locn;
//            MbLocalSetBootDev(dev->biosDev, dev->partition, dev->subPartition, 0xffffffff);
            break;
        }

        case MB2_TAG_MMAP: {
            SerialPutS("Setting memory map data\n");
            Mb2MemMap_t *mmap = (Mb2MemMap_t *)locn;
            uint32_t s = tag->size / mmap->entrySize;
            for (uint32_t i = 0; i < s; i ++) {
                if (mmap->entries[i].type == 1) AddAvailMem(mmap->entries[i].baseAddr, mmap->entries[i].length);
                uint64_t newLimit = mmap->entries[i].baseAddr + mmap->entries[i].length;
                if (newLimit > GetUpperMemLimit()) SetUpperMemLimit(newLimit);
            }

            break;
        }

        case MB2_TAG_VBE: {
//            Mb2VbeInfo_t *vbe = (Mb2VbeInfo_t *)locn;
//            MbLocalSetVbe(vbe->vbeMode, vbe->vbeInterfaceSeg, vbe->vbeInterfaceOff, vbe->vbeInterfaceLen,
//                    vbe->vbeControlInfo, vbe->vbeModeInfo);

            break;
        }

        case MB2_TAG_FRAMEBUFFER: {
            Mb2FbInfo_t *fb = (Mb2FbInfo_t *)locn;
            SetFrameBufferAddr((uint16_t *)fb->fbAddr);
            SetFrameBufferPitch(fb->fbPitch);
            SetFrameBufferWidth(fb->fbWidth);
            SetFrameBufferHeight(fb->fbHeight);
            SetFrameBufferBpp(fb->fbBpp);
            SetFrameBufferType((FrameBufferType)fb->fbType);


            SerialPutS("Frame Buffer is at: ");
            SerialPutHex((ptrsize_t)fb->fbAddr);
            SerialPutS("; The pitch is");
            SerialPutHex((ptrsize_t)fb->fbPitch);
            SerialPutS("; The height is");
            SerialPutHex((ptrsize_t)fb->fbHeight);
            SerialPutS("\n");

            break;
        }

        case MB2_TAG_ELF_SYMS: {
//            Mb2ElfSymbols_t *elf = (Mb2ElfSymbols_t *)locn;
//            MbLocalSetElfSyms(elf->num, elf->entSize, elf->shndx);

            break;
        }

        case MB2_TAG_APM: {
//            Mb2Apm_t *apm = (Mb2Apm_t *)locn;
//            MbLocalSetApm(apm->version, apm->cseg, apm->offset, apm->cseg16, apm->dseg, apm->flags, apm->csegLen,
//                    apm->cseg16Len, apm->dsegLen);

            break;
        }

        default:
            SerialPutS("Unimplemented MB2 type\n");
            break;
        }

        locn += (tag->size + (~(tag->size - 1) & 0x7));
    }
}

