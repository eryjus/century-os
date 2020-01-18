//===================================================================================================================
//
//  mb2.c -- This is the parser for the Multiboot 1 information structure
//
//        Copyright (c)  2017-2020 -- Adam Clark
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
#include "serial.h"
#include "mmu.h"
#include "printf.h"
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
    MB2_TAG_EFI32 = 11,
    MB2_TAG_EFI64 = 12,
    MB2_TAG_SMBIOS = 13,
    MB2_TAG_RSDP_V1 = 14,
    MB2_TAG_RSDP_V2 = 15,
    MB2_TAG_NET_INFO = 16,
    MB2_TAG_EFI_MMAP = 17,
    MB2_TAG_EFI_BOOT_SRV = 18,
    MB2_TAG_EFI_IMG_32 = 19,
    MB2_TAG_EFI_IMG_64 = 20,
    MB2_TAG_LOAD_ADDR = 21,
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
// -- The command line for the boot
//    -----------------------------
typedef struct Mb2CmdLine_t {
    Mb2BasicTag_t tag;         // type == 1
    char cmdLine[0];
} __attribute__((packed)) Mb2CmdLine_t;


//
// -- The bootloader name
//    -------------------
typedef struct Mb2Loader_t {
    Mb2BasicTag_t tag;         // type == 2
    char name[0];
} __attribute__((packed)) Mb2Loader_t;


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
//    --------------------
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
// -- The ELF Symbols
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
// -- EFI 32-bit system table pointer
//    -------------------------------
typedef struct Mb2Efi32_t {
    Mb2BasicTag_t tag;          // type == 11; size == 12
    uint32_t pointer;
} __attribute__((packed)) Mb2Efi32_t;


//
// -- EFI 64-bit system table pointer
//    -------------------------------
typedef struct Mb2Efi64_t {
    Mb2BasicTag_t tag;          // type == 12; size == 16
    uint64_t pointer;
} __attribute__((packed)) Mb2Efi64_t;


//
// -- SMBIOS Tables
//    -------------
typedef struct Mb2SmBios_t {
    Mb2BasicTag_t tag;          // type == 13
    uint8_t major;
    uint8_t minor;
    uint8_t reserved[6];
    uint8_t smBiosTables[0];
} __attribute__((packed)) Mb2SmBios_t;


//
// -- RSDPv1 Tables
//    -------------
typedef struct Mb2RsdpV1_t {
    Mb2BasicTag_t tag;          // type == 14
    uint8_t rsdpV1Copy[0];
} __attribute__((packed)) Mb2RsdpV1_t;


//
// -- RSDPv2 Tables
//    -------------
typedef struct Mb2RsdpV2_t {
    Mb2BasicTag_t tag;          // type == 15
    uint8_t rsdpV2Copy[0];
} __attribute__((packed)) Mb2RsdpV2_t;


//
// -- Networking Information
//    ----------------------
typedef struct Mb2NetInfo_t {
    Mb2BasicTag_t tag;          // type == 16
    uint8_t dhcpAck[0];
} __attribute__((packed)) Mb2NetInfo_t;


//
// -- EFI Memory Map
//    --------------
typedef struct Mb2EfiMemMap_t {
    Mb2BasicTag_t tag;          // type == 17
    uint32_t descriptorSize;
    uint32_t descriptorVer;
    uint8_t efiMemMap[0];
} __attribute__((packed)) Mb2EfiMemMap_t;


//
// -- EFI Boot Services Not Terminated (EFI Boot Services still available)
//    --------------------------------------------------------------------
typedef struct Mb2EfiBootServ_t {
    Mb2BasicTag_t tag;          // type == 18; size == 8
} __attribute__((packed)) Mb2EfiBootServ_t;


//
// -- EFI 32-bit image handle pointer
//    -------------------------------
typedef struct Mb2EfiImage32_t {
    Mb2BasicTag_t tag;          // type == 19; size == 12
    uint32_t pointer;
} __attribute__((packed)) Mb2EfiImage32_t;


//
// -- EFI 64-bit image handle pointer
//    -------------------------------
typedef struct Mb2EfiImage64_t {
    Mb2BasicTag_t tag;          // type == 20; size == 16
    uint64_t pointer;
} __attribute__((packed)) Mb2EfiImage64_t;


//
// -- Image load phys address
//    -----------------------
typedef struct Mb2LoadPhysAddr_t {
    Mb2BasicTag_t tag;          // type == 21; size == 12
    uint32_t baseAddr;
} __attribute__((packed)) Mb2LoadPhysAddr_t;


//
// -- The multiboot 2 information structure
//    -------------------------------------
EXTERN LOADER_BSS
void *mb2Data;

EXTERN LOADER_BSS
void *mb1Data;


//
// -- Mb2Parse() -- Read the multiboot 2 information from the data provided and store it locally
//    ------------------------------------------------------------------------------------------
EXTERN_C EXPORT LOADER
void Mb2Parse(void)
{
    if (!mb2Data) return;

    archsize_t mb2Page = (archsize_t)mb2Data;
    MmuMapToFrame(mb2Page, mb2Page >> 12, PG_KRN);
    MmuMapToFrame(mb2Page + PAGE_SIZE, (mb2Page + PAGE_SIZE) >> 12, PG_KRN);

    kprintf("Parsing MB2 Info at %p (MB1 info at %p)\n", mb2Data, mb1Data);
    kprintf(".. size = %x\n", ((uint32_t *)mb2Data)[0]);
    kprintf(".. resv = %x\n", ((uint32_t *)mb2Data)[1]);

    uint32_t locn = (uint32_t)mb2Data + sizeof(Mb2Fixed_t);
    bool lastTag = false;

    while (!lastTag) {
        Mb2BasicTag_t *tag = (Mb2BasicTag_t *)locn;
        kprintf("MB2 info: at %p: %x\n", tag, tag->type);
        switch (tag->type) {
        case MB2_TAG_LAST_TAG:
            kprintf(".. Last Tag\n");
            lastTag = true;
            break;

        case MB2_TAG_CMD_LINE:
            kprintf("%s\n", ((Mb2CmdLine_t *)locn)->cmdLine);
            break;

        case MB2_TAG_LOADER:
            kprintf("%s\n", ((Mb2Loader_t *)locn)->name);
            break;

        case MB2_TAG_MODULE: {
            kprintf("Module information present\n");

            Mb2Module_t *m = (Mb2Module_t *)locn;

            AddModule(m->modStart, m->modEnd, m->name);

            break;
        }

        case MB2_TAG_BASIC_MEM: {
            Mb2BasicMem_t *mem = (Mb2BasicMem_t *)locn;

            kprintf("Setting basic memory information\n");
            SetAvailLowerMem(mem->memLower);
            SetAvailUpperMem(mem->memUpper);

            break;
        }

        case MB2_TAG_BOOT_DEV: {
            kprintf(".. Boot Device\n");
//            Mb2BootDevice_t *dev = (Mb2BootDevice_t *)locn;
//            MbLocalSetBootDev(dev->biosDev, dev->partition, dev->subPartition, 0xffffffff);
            break;
        }

        case MB2_TAG_MMAP: {
            kprintf("Setting memory map data\n");
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
            kprintf(".. VBE info\n");
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


            kprintf("Frame Buffer is at: %p; The pitch is: %p; The height is: %p\n",
                    (archsize_t)fb->fbAddr, (archsize_t)fb->fbPitch, (archsize_t)fb->fbHeight);

            break;
        }

        case MB2_TAG_ELF_SYMS: {
            kprintf(".. Elf Syms\n");
//            Mb2ElfSymbols_t *elf = (Mb2ElfSymbols_t *)locn;
//            MbLocalSetElfSyms(elf->num, elf->entSize, elf->shndx);

            break;
        }

        case MB2_TAG_APM: {
            kprintf(".. APM\n");
//            Mb2Apm_t *apm = (Mb2Apm_t *)locn;
//            MbLocalSetApm(apm->version, apm->cseg, apm->offset, apm->cseg16, apm->dseg, apm->flags, apm->csegLen,
//                    apm->cseg16Len, apm->dsegLen);

            break;
        }


        case MB2_TAG_EFI32:
            kprintf(".. EFI32 System Table\n");
            break;


        case MB2_TAG_EFI64:
            kprintf(".. EFI64 System Table\n");
            break;


        case MB2_TAG_SMBIOS:
            kprintf(".. SMBIOS Table\n");
            break;


        case MB2_TAG_RSDP_V1: {
            Mb2RsdpV1_t *rdsp = (Mb2RsdpV1_t *)locn;

            kprintf(".. RSDPV1 Table: %c%c%c%c%c%c%c%c\n", rdsp->rsdpV1Copy[0], rdsp->rsdpV1Copy[1],
                    rdsp->rsdpV1Copy[2], rdsp->rsdpV1Copy[3], rdsp->rsdpV1Copy[4], rdsp->rsdpV1Copy[5],
                    rdsp->rsdpV1Copy[6], rdsp->rsdpV1Copy[7]);
            break;
        }


        case MB2_TAG_RSDP_V2:{
            Mb2RsdpV2_t *rdsp = (Mb2RsdpV2_t *)locn;

            kprintf(".. RSDPV2Table: %c%c%c%c%c%c%c%c\n", rdsp->rsdpV2Copy[0], rdsp->rsdpV2Copy[1],
                    rdsp->rsdpV2Copy[2], rdsp->rsdpV2Copy[3], rdsp->rsdpV2Copy[4], rdsp->rsdpV2Copy[5],
                    rdsp->rsdpV2Copy[6], rdsp->rsdpV2Copy[7]);
            break;
        }


        case MB2_TAG_NET_INFO:
            kprintf(".. Network Information Table\n");
            break;


        case MB2_TAG_EFI_MMAP:
            kprintf(".. EFI Memory Map\n");
            break;


        case MB2_TAG_EFI_BOOT_SRV:
            kprintf(".. EFI Boot Services not Terminated\n");
            break;


        case MB2_TAG_EFI_IMG_32:
            kprintf(".. EFI 32-bit Image Pointer\n");
            break;


        case MB2_TAG_EFI_IMG_64:
            kprintf(".. EFI 64-bit Image Pointer\n");
            break;


        case MB2_TAG_LOAD_ADDR: {
            Mb2LoadPhysAddr_t *addr = (Mb2LoadPhysAddr_t *)locn;

            kprintf(".. Load Base Address: %p\n", addr->baseAddr);
            break;
        }


        default:
            kprintf("Unimplemented MB2 type: %x\n", tag->type);
            break;
        }

        locn += (tag->size + (~(tag->size - 1) & 0x7));
    }

    MmuUnmapPage(mb2Page);
    MmuUnmapPage(mb2Page + PAGE_SIZE);
}

