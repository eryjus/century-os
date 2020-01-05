//===================================================================================================================
//
//  hw-disc.h -- Locally formatted copies of the multiboot information
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Jun-03  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#pragma once


#include "types.h"
#include "cpu.h"


//
// -- Since this is all statically allocated, set some reasonable limits
//    ------------------------------------------------------------------
#define NUM_MMAP_ENTRIES        (25)
#define MAX_MODULES             (10)
#define MODULE_IDENT_LEN        (12)


//
// -- The values for the frame buffer type
//    ------------------------------------
typedef enum {
    FB_PALLET = 0,
    FB_RGB = 1,
    FB_EGA = 2,
} FrameBufferType;


//
// -- This is the structure that will hold loaded module info
//    -------------------------------------------------------
typedef struct Module_t {
    uint32_t modStart;
    uint32_t modEnd;
    uint32_t modHdrSize;
//    archsize_t cr3;
    archsize_t entry;
    char modIdent[MODULE_IDENT_LEN];             // this will point to at-risk memory
} Module_t;


//
// -- This is the structure that will hold the memory map data
//    --------------------------------------------------------
typedef struct MMap_t {
    uint64_t baseAddr;
    uint64_t length;
} MMap_t;


//
// -- This structure will hold a stage-3 loader local copy of the provided multiboot information.
//    -------------------------------------------------------------------------------------------
typedef struct HardwareDiscovery_t {
    //
    // -- The BIOS information
    //    --------------------
    archsize_t ebdaLocation;
    uint16_t com1;
    uint16_t com2;
    uint16_t com3;
    uint16_t com4;
    uint16_t lpt1;
    uint16_t lpt2;
    uint16_t lpt3;
    uint16_t videoPort;

    //
    // -- the memory limit information
    //    ----------------------------
    bool memLimitsAvail;
    uint32_t availLowerMem;
    uint32_t availUpperMem;
    uint64_t upperMemLimit;

    //
    // -- the memory map information
    //    --------------------------
    bool memMapAvail;
    int memMapCount;
    MMap_t mmap[NUM_MMAP_ENTRIES];

    //
    // -- the module information
    //    ----------------------
    bool modAvail;
    int modCount;
    Module_t mods[MAX_MODULES];
    frame_t modHighestFrame;

    //
    // -- the Physical Memory Manager location and other relevant info
    //    ------------------------------------------------------------
    uint32_t *pmmBitmap;
    size_t pmmFrameCount;

    //
    // -- FrameBufferInformation
    //    ----------------------
    // -- Frame Buffer Info
    bool frameBufferAvail;
    uint16_t *fbAddr;
    uint32_t fbPitch;
    uint32_t fbWidth;
    uint32_t fbHeight;
    uint8_t fbBpp;
    FrameBufferType fbType;

    //---------------------------------

    //
    // -- The console properties; which will be also passed to the kernel
    //    ---------------------------------------------------------------
    uint16_t bgColor;
    uint16_t fgColor;
    uint16_t rowPos;
    uint16_t colPos;


    //
    // -- The location of the ACPI tables
    //    -------------------------------
    archsize_t rsdp;


    //
    // -- CPUID Data
    //    ----------
    bool cpuidSupported;

    uint32_t cpuid00eax;
    uint32_t cpuid00ebx;
    uint32_t cpuid00ecx;
    uint32_t cpuid00edx;

    uint32_t cpuid01eax;
    uint32_t cpuid01ebx;
    uint32_t cpuid01ecx;
    uint32_t cpuid01edx;

    uint32_t cpuid02eax;
    uint32_t cpuid02ebx;
    uint32_t cpuid02ecx;
    uint32_t cpuid02edx;

    uint32_t cpuid03eax;
    uint32_t cpuid03ebx;
    uint32_t cpuid03ecx;
    uint32_t cpuid03edx;

    uint32_t cpuid04eax;
    uint32_t cpuid04ebx;
    uint32_t cpuid04ecx;
    uint32_t cpuid04edx;

    uint32_t cpuid05eax;
    uint32_t cpuid05ebx;
    uint32_t cpuid05ecx;
    uint32_t cpuid05edx;

    uint32_t cpuid06eax;
    uint32_t cpuid06ebx;
    uint32_t cpuid06ecx;
    uint32_t cpuid06edx;

    uint32_t cpuid07eax;
    uint32_t cpuid07ebx;
    uint32_t cpuid07ecx;
    uint32_t cpuid07edx;

    uint32_t cpuid09eax;
    uint32_t cpuid09ebx;
    uint32_t cpuid09ecx;
    uint32_t cpuid09edx;

    uint32_t cpuid0aeax;
    uint32_t cpuid0aebx;
    uint32_t cpuid0aecx;
    uint32_t cpuid0aedx;

    uint32_t cpuid0beax;
    uint32_t cpuid0bebx;
    uint32_t cpuid0becx;
    uint32_t cpuid0bedx;

    //
    // -- Local APIC info
    //    ---------------
    size_t lapicCount;

    //
    // -- IO APIC Address and Count
    //    -------------------------
    int ioApicCount;
    struct {
        archsize_t addr;
        int gsiBase;
    } ioApic[MAX_IOAPIC];
} HardwareDiscovery_t __attribute__((aligned(4096)));


//
// -- We will work with a local copy and then copy the ending structure to its final location
//    ---------------------------------------------------------------------------------------
EXTERN EXPORT LOADER_DATA
HardwareDiscovery_t *localHwDisc;


//
// -- A compile time sanity check -- if this throws an error, the structure will not fit in a page and will cause
//    problems for both the loader and the kernel.  The fix will be to move some things around in memory and make
//    room for the larger structure
//    -----------------------------------------------------------------------------------------------------------
static_assert(sizeof(HardwareDiscovery_t) <= 4096, \
        "The size of the Hardware Discovery stucture is more than 1 page long.  Something must be done...");


//
// -- Hardware discovery function to collect the hardware inventory
//    -------------------------------------------------------------
EXTERN_C EXPORT LOADER
void HwDiscovery(void);


//
// -- Read the MB1 structures and place the important bits in the localHwDisc structure
//    ---------------------------------------------------------------------------------
EXTERN_C EXPORT LOADER
void Mb1Parse(void);


//
// -- Read the MB2 structures and place the important bits in the localHwDisc structure
//    ---------------------------------------------------------------------------------
EXTERN_C EXPORT LOADER
void Mb2Parse(void);


//
// -- BIOS Data Area
//    --------------
EXPORT LOADER INLINE
bool IsEbdaAvail(void) { return localHwDisc->ebdaLocation != 0; }

EXPORT LOADER INLINE
bool IsCom1Avail(void) { return localHwDisc->com1 != 0; }

EXPORT LOADER INLINE
bool IsCom2Avail(void) { return localHwDisc->com2 != 0; }

EXPORT LOADER INLINE
bool IsCom3Avail(void) { return localHwDisc->com3 != 0; }

EXPORT LOADER INLINE
bool IsCom4Avail(void) { return localHwDisc->com4 != 0; }

EXPORT LOADER INLINE
bool IsLpt1Avail(void) { return localHwDisc->lpt1 != 0; }

EXPORT LOADER INLINE
bool IsLpt2Avail(void) { return localHwDisc->lpt2 != 0; }

EXPORT LOADER INLINE
bool IsLpt3Avail(void) { return localHwDisc->lpt3 != 0; }

EXPORT LOADER INLINE
bool IsVideoAvail(void) { return localHwDisc->videoPort != 0; }

EXPORT LOADER INLINE
void SetEbda(archsize_t e) { localHwDisc->ebdaLocation = e; }

EXPORT LOADER INLINE
archsize_t GetEbda(void) { return localHwDisc->ebdaLocation; }

EXPORT LOADER INLINE
void SetCom1(devaddr_t p) { localHwDisc->com1 = p; }

EXPORT LOADER INLINE
devaddr_t GetCom1(void) { return localHwDisc->com1; }

EXPORT LOADER INLINE
void SetCom2(devaddr_t p) { localHwDisc->com2 = p; }

EXPORT LOADER INLINE
devaddr_t GetCom2(void) { return localHwDisc->com2; }


EXPORT LOADER INLINE
void SetCom3(devaddr_t p) { localHwDisc->com3 = p; }

EXPORT LOADER INLINE
devaddr_t GetCom3(void) { return localHwDisc->com3; }


EXPORT LOADER INLINE
void SetCom4(devaddr_t p) { localHwDisc->com4 = p; }

EXPORT LOADER INLINE
devaddr_t GetCom4(void) { return localHwDisc->com4; }


EXPORT LOADER INLINE
void SetLpt1(devaddr_t p) { localHwDisc->lpt1 = p; }

EXPORT LOADER INLINE
devaddr_t GetLpt1(void) { return localHwDisc->lpt1; }


EXPORT LOADER INLINE
void SetLpt2(devaddr_t p) { localHwDisc->lpt2 = p; }

EXPORT LOADER INLINE
devaddr_t GetLpt2(void) { return localHwDisc->lpt2; }


EXPORT LOADER INLINE
void SetLpt3(devaddr_t p) { localHwDisc->lpt3 = p; }

EXPORT LOADER INLINE
devaddr_t GetLpt3(void) { return localHwDisc->lpt3; }


EXPORT LOADER INLINE
void SetVideo(uint16_t p) { localHwDisc->videoPort = p; }

EXPORT LOADER INLINE
uint16_t GetVideo(void) { return localHwDisc->videoPort; }


//
// -- Basic memory limits (where flag 0 is set)
//    -----------------------------------------
EXPORT LOADER INLINE
bool AreMemLimitsAvail(void) { return localHwDisc->memLimitsAvail; }


EXPORT LOADER INLINE
void SetAvailLowerMem(uint32_t l) { localHwDisc->availLowerMem = l; localHwDisc->memLimitsAvail = true; }

EXPORT LOADER INLINE
uint32_t GetAvailLowerMem(void) { return localHwDisc->availLowerMem; }


EXPORT LOADER INLINE
void SetAvailUpperMem(uint32_t l) { localHwDisc->availUpperMem = l; localHwDisc->memLimitsAvail = true; }

EXPORT LOADER INLINE
uint32_t GetAvailUpperMem(void) { return localHwDisc->availUpperMem; }


EXPORT LOADER INLINE
void SetUpperMemLimit(uint64_t l) { localHwDisc->upperMemLimit = l; }

EXPORT LOADER INLINE
uint64_t GetUpperMemLimit(void) { return localHwDisc->upperMemLimit; }


//
// -- Module Data
//    -----------
EXPORT LOADER INLINE
bool HaveModData(void) { return localHwDisc->modAvail; }

EXPORT LOADER INLINE
int GetModCount(void) { return localHwDisc->modCount; }


EXPORT LOADER INLINE
frame_t GetModHightestFrame(void) { return localHwDisc->modHighestFrame; }

EXPORT LOADER INLINE
void UpdateModHighestFrame(frame_t frame) {
    if (frame > localHwDisc->modHighestFrame) localHwDisc->modHighestFrame = frame;
}


EXPORT LOADER INLINE
void AddModule(uint64_t at, uint64_t end, char *ident) {
    localHwDisc->mods[localHwDisc->modCount].modStart = at;
    localHwDisc->mods[localHwDisc->modCount].modEnd = end;
    kStrCpy(localHwDisc->mods[localHwDisc->modCount ++].modIdent, ident);
    UpdateModHighestFrame(end >> 12);
    localHwDisc->modAvail = true;
}


EXPORT LOADER INLINE
void SetModuleHdrSize(int i, size_t s) { localHwDisc->mods[i].modHdrSize = s; }
//EXPORT LOADER INLINE
//void SetModuleCr3(int i, archsize_t cr3) { localHwDisc->mods[i].cr3 = cr3; }

EXPORT LOADER INLINE
void SetModuleEntry(int i, archsize_t entry) { localHwDisc->mods[i].entry = entry; }

EXPORT LOADER INLINE
uint64_t GetAvailModuleStart(int i) { return localHwDisc->mods[i].modStart; }

EXPORT LOADER INLINE
uint64_t GetAvailModuleEnd(int i) { return localHwDisc->mods[i].modEnd; }

EXPORT LOADER INLINE
char *GetAvailModuleIdent(int i) { return localHwDisc->mods[i].modIdent; }


//
// -- Memory Map data
//    ---------------
EXPORT LOADER INLINE
bool HaveMMapData(void) { return localHwDisc->memMapAvail; }

EXPORT LOADER INLINE
int GetMMapEntryCount(void) { return localHwDisc->memMapCount; }


EXPORT LOADER INLINE
void AddAvailMem(uint64_t at, uint64_t len) {
    localHwDisc->mmap[localHwDisc->memMapCount].baseAddr = at;
    localHwDisc->mmap[localHwDisc->memMapCount ++].length = len;
    localHwDisc->memMapAvail = true;
}


EXPORT LOADER INLINE
uint64_t GetAvailMemStart(int i) { return localHwDisc->mmap[i].baseAddr; }

EXPORT LOADER INLINE
uint64_t GetAvailMemLength(int i) { return localHwDisc->mmap[i].length; }


//
// -- Physical Memory Manager Bitmap location
//    ---------------------------------------
EXPORT LOADER INLINE
void SetPmmBitmap(uint32_t *l) { localHwDisc->pmmBitmap = l; }

EXPORT LOADER INLINE
uint32_t *GetPmmBitmap(void) { return localHwDisc->pmmBitmap; }


EXPORT LOADER INLINE
void SetPmmFrameCount(size_t c) { localHwDisc->pmmFrameCount = c; }

EXPORT LOADER INLINE
size_t GetPmmFrameCount(void) { return localHwDisc->pmmFrameCount; }


//
// -- Frame Buffer Management & Screen output management
//    --------------------------------------------------
EXPORT LOADER INLINE
bool IsFrameBufferAvail(void) { return localHwDisc->frameBufferAvail; }


EXPORT LOADER INLINE
void SetFrameBufferAddr(uint16_t *a) { localHwDisc->fbAddr = a; localHwDisc->frameBufferAvail = true; }

EXPORT LOADER INLINE
uint16_t *GetFrameBufferAddr(void) { return localHwDisc->fbAddr; }


EXPORT LOADER INLINE
void SetFrameBufferPitch(uint32_t p) { localHwDisc->fbPitch = p; }

EXPORT LOADER INLINE
uint32_t GetFrameBufferPitch(void) { return localHwDisc->fbPitch; }


EXPORT LOADER INLINE
void SetFrameBufferWidth(uint32_t w) { localHwDisc->fbWidth = w; }

EXPORT LOADER INLINE
uint32_t GetFrameBufferWidth(void) { return localHwDisc->fbWidth; }


EXPORT LOADER INLINE
void SetFrameBufferHeight(uint32_t h) { localHwDisc->fbHeight = h; }

EXPORT LOADER INLINE
uint32_t GetFrameBufferHeight(void) { return localHwDisc->fbHeight; }


EXPORT LOADER INLINE
void SetFrameBufferBpp(uint8_t b) { localHwDisc->fbBpp = b; }

EXPORT LOADER INLINE
uint8_t GetFrameBufferBpp(void) { return localHwDisc->fbBpp; }


EXPORT LOADER INLINE
void SetFrameBufferType(FrameBufferType t) { localHwDisc->fbType = t; }

EXPORT LOADER INLINE
FrameBufferType GetFrameBufferType(void) { return localHwDisc->fbType; }


//
// -- Console properties that are passed to the kernel
//    ------------------------------------------------
EXPORT LOADER INLINE
void SetBgColor(uint16_t c) { localHwDisc->bgColor = c; }

EXPORT LOADER INLINE
uint16_t GetBgColor(void) { return localHwDisc->bgColor; }


EXPORT LOADER INLINE
void SetFgColor(uint16_t c) { localHwDisc->fgColor = c; }

EXPORT LOADER INLINE
uint16_t GetFgColor(void) { return localHwDisc->fgColor; }


EXPORT LOADER INLINE
void SetColPos(uint16_t p) { localHwDisc->colPos = p; }

EXPORT LOADER INLINE
uint16_t GetColPos(void) { return localHwDisc->colPos; }


EXPORT LOADER INLINE
void SetRowPos(uint16_t p) { localHwDisc->rowPos = p; }

EXPORT LOADER INLINE
uint16_t GetRowPos(void) { return localHwDisc->rowPos; }


//
// -- access to the rsdp member
//    -------------------------
EXPORT LOADER INLINE
void SetRsdp(archsize_t p) { localHwDisc->rsdp = p; }

EXPORT LOADER INLINE
archsize_t GetRsdp(void) { return localHwDisc->rsdp; }


//
// -- access to cpuid support
//    -----------------------
EXPORT LOADER INLINE
void SetCpuid(bool c) { localHwDisc->cpuidSupported = c; }

EXPORT LOADER INLINE
bool GetCpuid(void) { return localHwDisc->cpuidSupported; }

EXPORT LOADER INLINE
bool HasLocalApic(void) { return !!(localHwDisc->cpuid01edx & (1<<9)); }


//
// -- access to the local APIC fields
//    -------------------------------
EXPORT LOADER INLINE
void IncLocalApic(void) { localHwDisc->lapicCount ++; }

EXPORT LOADER INLINE
size_t GetLocalApicCount(void) { return localHwDisc->lapicCount; }

//
// -- access to the IO APIC
//    ---------------------
EXPORT LOADER INLINE
void AddIoapic(archsize_t addr, int gsi) { localHwDisc->ioApic[localHwDisc->ioApicCount ++] = { addr, gsi}; }

EXPORT LOADER INLINE
int GetIoapicCount(void) { return localHwDisc->ioApicCount; }

EXPORT LOADER INLINE
archsize_t GetIoapicAddr(int i) { return localHwDisc->ioApic[i].addr; }

EXPORT LOADER INLINE
int GetIoapicGsi(int i) { return localHwDisc->ioApic[i].gsiBase; }

