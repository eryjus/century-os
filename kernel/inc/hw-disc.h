//===================================================================================================================
//
//  hw-disc.h -- Locally formatted copies of the multiboot information
//
//        Copyright (c)  2017-2019 -- Adam Clark
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


#ifndef __HW_DISC_H__
#define __HW_DISC_H__


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
    archsize_t cr3;
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
} /*__attribute__((packed))*/ HardwareDiscovery_t;


//
// -- We will work with a local copy and then copy the ending structure to its final location
//    ---------------------------------------------------------------------------------------
extern HardwareDiscovery_t *localHwDisc;


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
void HwDiscovery(void);


//
// -- Read the MB1 structures and place the important bits in the localHwDisc structure
//    ---------------------------------------------------------------------------------
void Mb1Parse(void);


//
// -- Read the MB2 structures and place the important bits in the localHwDisc structure
//    ---------------------------------------------------------------------------------
void Mb2Parse(void);


//
// == These are the inline access functions for the local version of the HardwareDiscovery
//    ====================================================================================


//
// -- BIOS Data Area
//    --------------
inline bool IsEbdaAvail(void) { return localHwDisc->ebdaLocation != 0; }
inline bool IsCom1Avail(void) { return localHwDisc->com1 != 0; }
inline bool IsCom2Avail(void) { return localHwDisc->com2 != 0; }
inline bool IsCom3Avail(void) { return localHwDisc->com3 != 0; }
inline bool IsCom4Avail(void) { return localHwDisc->com4 != 0; }
inline bool IsLpt1Avail(void) { return localHwDisc->lpt1 != 0; }
inline bool IsLpt2Avail(void) { return localHwDisc->lpt2 != 0; }
inline bool IsLpt3Avail(void) { return localHwDisc->lpt3 != 0; }
inline bool IsVideoAvail(void) { return localHwDisc->videoPort != 0; }

inline void SetEbda(archsize_t e) { localHwDisc->ebdaLocation = e; }
inline archsize_t GetEbda(void) { return localHwDisc->ebdaLocation; }

inline void SetCom1(devaddr_t p) { localHwDisc->com1 = p; }
inline devaddr_t GetCom1(void) { return localHwDisc->com1; }

inline void SetCom2(devaddr_t p) { localHwDisc->com2 = p; }
inline devaddr_t GetCom2(void) { return localHwDisc->com2; }

inline void SetCom3(devaddr_t p) { localHwDisc->com3 = p; }
inline devaddr_t GetCom3(void) { return localHwDisc->com3; }

inline void SetCom4(devaddr_t p) { localHwDisc->com4 = p; }
inline devaddr_t GetCom4(void) { return localHwDisc->com4; }

inline void SetLpt1(devaddr_t p) { localHwDisc->lpt1 = p; }
inline devaddr_t GetLpt1(void) { return localHwDisc->lpt1; }

inline void SetLpt2(devaddr_t p) { localHwDisc->lpt2 = p; }
inline devaddr_t GetLpt2(void) { return localHwDisc->lpt2; }

inline void SetLpt3(devaddr_t p) { localHwDisc->lpt3 = p; }
inline devaddr_t GetLpt3(void) { return localHwDisc->lpt3; }

inline void SetVideo(uint16_t p) { localHwDisc->videoPort = p; }
inline uint16_t GetVideo(void) { return localHwDisc->videoPort; }


//
// -- Basic memory limits (where flag 0 is set)
//    -----------------------------------------
inline bool AreMemLimitsAvail(void) { return localHwDisc->memLimitsAvail; }

inline void SetAvailLowerMem(uint32_t l) { localHwDisc->availLowerMem = l; localHwDisc->memLimitsAvail = true; }
inline uint32_t GetAvailLowerMem(void) { return localHwDisc->availLowerMem; }

inline void SetAvailUpperMem(uint32_t l) { localHwDisc->availUpperMem = l; localHwDisc->memLimitsAvail = true; }
inline uint32_t GetAvailUpperMem(void) { return localHwDisc->availUpperMem; }

inline void SetUpperMemLimit(uint64_t l) { localHwDisc->upperMemLimit = l; }
inline uint64_t GetUpperMemLimit(void) { return localHwDisc->upperMemLimit; }


//
// -- Module Data
//    -----------
inline bool HaveModData(void) { return localHwDisc->modAvail; }
inline int GetModCount(void) { return localHwDisc->modCount; }

inline frame_t GetModHightestFrame(void) { return localHwDisc->modHighestFrame; }
inline void UpdateModHighestFrame(frame_t frame) {
    if (frame > localHwDisc->modHighestFrame) localHwDisc->modHighestFrame = frame;
}

inline void AddModule(uint64_t at, uint64_t end, char *ident) {
    localHwDisc->mods[localHwDisc->modCount].modStart = at;
    localHwDisc->mods[localHwDisc->modCount].modEnd = end;
    kStrCpy(localHwDisc->mods[localHwDisc->modCount ++].modIdent, ident);
    UpdateModHighestFrame(end >> 12);
    localHwDisc->modAvail = true;
}

inline void SetModuleHdrSize(int i, size_t s) { localHwDisc->mods[i].modHdrSize = s; }
inline void SetModuleCr3(int i, archsize_t cr3) { localHwDisc->mods[i].cr3 = cr3; }
inline void SetModuleEntry(int i, archsize_t entry) { localHwDisc->mods[i].entry = entry; }
inline uint64_t GetAvailModuleStart(int i) { return localHwDisc->mods[i].modStart; }
inline uint64_t GetAvailModuleEnd(int i) { return localHwDisc->mods[i].modEnd; }
inline char *GetAvailModuleIdent(int i) { return localHwDisc->mods[i].modIdent; }


//
// -- Memory Map data
//    ---------------
inline bool HaveMMapData(void) { return localHwDisc->memMapAvail; }
inline int GetMMapEntryCount(void) { return localHwDisc->memMapCount; }

inline void AddAvailMem(uint64_t at, uint64_t len) {
    localHwDisc->mmap[localHwDisc->memMapCount].baseAddr = at;
    localHwDisc->mmap[localHwDisc->memMapCount ++].length = len;
    localHwDisc->memMapAvail = true;
}

inline uint64_t GetAvailMemStart(int i) { return localHwDisc->mmap[i].baseAddr; }
inline uint64_t GetAvailMemLength(int i) { return localHwDisc->mmap[i].length; }


//
// -- Physical Memory Manager Bitmap location
//    ---------------------------------------
inline void SetPmmBitmap(uint32_t *l) { localHwDisc->pmmBitmap = l; }
inline uint32_t *GetPmmBitmap(void) { return localHwDisc->pmmBitmap; }

inline void SetPmmFrameCount(size_t c) { localHwDisc->pmmFrameCount = c; }
inline size_t GetPmmFrameCount(void) { return localHwDisc->pmmFrameCount; }


//
// -- Frame Buffer Management & Screen output management
//    --------------------------------------------------
inline bool IsFrameBufferAvail(void) { return localHwDisc->frameBufferAvail; }

inline void SetFrameBufferAddr(uint16_t *a) { localHwDisc->fbAddr = a; localHwDisc->frameBufferAvail = true; }
inline uint16_t *GetFrameBufferAddr(void) { return localHwDisc->fbAddr; }

inline void SetFrameBufferPitch(uint32_t p) { localHwDisc->fbPitch = p; }
inline uint32_t GetFrameBufferPitch(void) { return localHwDisc->fbPitch; }

inline void SetFrameBufferWidth(uint32_t w) { localHwDisc->fbWidth = w; }
inline uint32_t GetFrameBufferWidth(void) { return localHwDisc->fbWidth; }

inline void SetFrameBufferHeight(uint32_t h) { localHwDisc->fbHeight = h; }
inline uint32_t GetFrameBufferHeight(void) { return localHwDisc->fbHeight; }

inline void SetFrameBufferBpp(uint8_t b) { localHwDisc->fbBpp = b; }
inline uint8_t GetFrameBufferBpp(void) { return localHwDisc->fbBpp; }

inline void SetFrameBufferType(FrameBufferType t) { localHwDisc->fbType = t; }
inline FrameBufferType GetFrameBufferType(void) { return localHwDisc->fbType; }


//
// -- Console properties that are passed to the kernel
//    ------------------------------------------------
inline void SetBgColor(uint16_t c) { localHwDisc->bgColor = c; }
inline uint16_t GetBgColor(void) { return localHwDisc->bgColor; }

inline void SetFgColor(uint16_t c) { localHwDisc->fgColor = c; }
inline uint16_t GetFgColor(void) { return localHwDisc->fgColor; }

inline void SetColPos(uint16_t p) { localHwDisc->colPos = p; }
inline uint16_t GetColPos(void) { return localHwDisc->colPos; }

inline void SetRowPos(uint16_t p) { localHwDisc->rowPos = p; }
inline uint16_t GetRowPos(void) { return localHwDisc->rowPos; }


#endif
