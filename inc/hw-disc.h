//===================================================================================================================
//
//  libk/ing/hw-disc.h -- Locally formatted copies of the multiboot information
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-06-03  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================

#ifndef __HW_DISC_H__
#define __HW_DISC_H__


#include "types.h"


#define NUM_MMAP_ENTRIES        (25)


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
    // -- the memory limit information
    //    ----------------------------
    bool memLimitsAvail;
    uint32_t availLowerMem;
    uint32_t availUpperMem;

    //
    // -- the memory map information
    //    --------------------------
    bool memMapAvail;
    int memMapCount;
    MMap_t mmap[NUM_MMAP_ENTRIES];
} __attribute__((packed)) HardwareDiscovery_t;


//
// -- We will work with a local copy and then copy the ending structure to its final location
//    ---------------------------------------------------------------------------------------
extern HardwareDiscovery_t localHwDisc;


//
// -- A compile time sanity check -- if this throws an error, the structure will not fit in a page and will cause
//    problems for both the loader and the kernel.  The fix will be to move somethings around in memory and make
//    room for the larger structure
//    -----------------------------------------------------------------------------------------------------------
static_assert(sizeof(HardwareDiscovery_t) <= 4096, \
        "The size of the Hardware Discovery stucture is more than 1 page long.  Something must be done...");


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
// -- Basic memory limits (where flag 0 is set)
//    -----------------------------------------
inline bool AreMemLimitsAvail(void) { return localHwDisc.memLimitsAvail; }

inline void SetAvailLowerMem(uint32_t l) { localHwDisc.availLowerMem = l; localHwDisc.memLimitsAvail = true; }
inline uint32_t GetAvailLowerMem(void) { return localHwDisc.availLowerMem; }

inline void SetAvailUpperMem(uint32_t l) { localHwDisc.availUpperMem = l; localHwDisc.memLimitsAvail = true; }
inline uint32_t GetAvailUpperMem(void) { return localHwDisc.availUpperMem; }


//
// -- Memory Map data
//    ---------------
inline bool HaveMMapData(void) { return localHwDisc.memMapAvail; }
inline int GetMMapEntryCount(void) { return localHwDisc.memMapCount; }

inline void AddAvailMem(uint64_t at, uint64_t len) {
    localHwDisc.mmap[localHwDisc.memMapCount].baseAddr = at;
    localHwDisc.mmap[localHwDisc.memMapCount ++].length = len;
    localHwDisc.memMapAvail = true;
}

inline uint64_t GetAvailMemStart(int i) { return localHwDisc.mmap[i].baseAddr; }
inline uint64_t GetAvailMemLength(int i) { return localHwDisc.mmap[i].length; }


#endif
