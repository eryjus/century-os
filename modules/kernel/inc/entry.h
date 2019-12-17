//===================================================================================================================
//
//  entry.h -- These are some things presented in the entry section
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Dec-16  Initial   0.5.0   ADCL  Initial version
//
//===================================================================================================================


#pragma once
#include "types.h"


//
// -- Allocate an early frame before the PMM is put in charge
//    -------------------------------------------------------
EXTERN_C ENTRY
frame_t NextEarlyFrame(void);


//
// -- This is the mmu table level 1 physcal address
//    ---------------------------------------------
EXTERN ENTRY_DATA archsize_t mmuLvl1Table;


//
// -- This is the interrupt table physical address
//    --------------------------------------------
EXTERN ENTRY_DATA archsize_t intTableAddr;


//
// -- These are the linker-provided symbols for the entry section
//    -----------------------------------------------------------
EXTERN ENTRY_DATA archsize_t mbStart;
EXTERN ENTRY_DATA archsize_t mbEnd;
EXTERN ENTRY_DATA archsize_t mbSize;
EXTERN ENTRY_DATA archsize_t mbPhys;

#define mbPhysStart (mbPhys)
#define mbPhysEnd   (mbPhys + mbSize)


//
// -- These are the linker-provided symbols for the loader section
//    ------------------------------------------------------------
EXTERN ENTRY_DATA archsize_t ldrStart;
EXTERN ENTRY_DATA archsize_t ldrEnd;
EXTERN ENTRY_DATA archsize_t ldrSize;
EXTERN ENTRY_DATA archsize_t ldrPhys;
EXTERN ENTRY_DATA archsize_t ldrVirt;

#define ldrPhysStart (ldrPhys)
#define ldrPhysEnd   (ldrPhys + ldrSize)
#define ldrVirtStart (ldrVirt)
#define ldrVirtEnd   (ldrEnd)


//
// -- These are the linker-provided symbols for the syscall pergatory section
//    -----------------------------------------------------------------------
EXTERN ENTRY_DATA archsize_t sysStart;
EXTERN ENTRY_DATA archsize_t sysEnd;
EXTERN ENTRY_DATA archsize_t sysSize;
EXTERN ENTRY_DATA archsize_t sysPhys;
EXTERN ENTRY_DATA archsize_t sysVirt;

#define sysPhysStart (sysPhys)
#define sysPhysEnd   (sysPhys + sysSize)
#define sysVirtStart (sysVirt)
#define sysVirtEnd   (sysEnd)


//
// -- These are the linker-provided symbols for the kernel code section
//    -----------------------------------------------------------------
EXTERN ENTRY_DATA archsize_t txtStart;
EXTERN ENTRY_DATA archsize_t txtEnd;
EXTERN ENTRY_DATA archsize_t txtSize;
EXTERN ENTRY_DATA archsize_t txtPhys;
EXTERN ENTRY_DATA archsize_t txtVirt;

#define txtPhysStart (txtPhys)
#define txtPhysEnd   (txtPhys + txtSize)
#define txtVirtStart (txtVirt)
#define txtVirtEnd   (txtEnd)


//
// -- These are the linker-provided symbols for the smp trampline section
//    -------------------------------------------------------------------
EXTERN ENTRY_DATA archsize_t smpStart;
EXTERN ENTRY_DATA archsize_t smpEnd;
EXTERN ENTRY_DATA archsize_t smpSize;
EXTERN ENTRY_DATA archsize_t smpPhys;
EXTERN ENTRY_DATA archsize_t smpVirt;

#define smpPhysStart (smpPhys)
#define smpPhysEnd   (smpPhys + smpSize)
#define smpVirtStart (smpVirt)
#define smpVirtEnd   (smpEnd)


//
// -- These are the linker-provided symbols for the kernel data section
//    -----------------------------------------------------------------
EXTERN ENTRY_DATA archsize_t dataStart;
EXTERN ENTRY_DATA archsize_t dataEnd;
EXTERN ENTRY_DATA archsize_t dataSize;
EXTERN ENTRY_DATA archsize_t dataPhys;
EXTERN ENTRY_DATA archsize_t dataVirt;

#define dataPhysStart (dataPhys)
#define dataPhysEnd   (dataPhys + dataSize)
#define dataVirtStart (dataVirt)
#define dataVirtEnd   (dataEnd)


//
// -- These are the linker-provided symbols for the kernel bss section
//    ----------------------------------------------------------------
EXTERN ENTRY_DATA archsize_t bssStart;
EXTERN ENTRY_DATA archsize_t bssEnd;
EXTERN ENTRY_DATA archsize_t bssSize;
EXTERN ENTRY_DATA archsize_t bssPhys;
EXTERN ENTRY_DATA archsize_t bssVirt;

#define bssPhysStart (bssPhys)
#define bssPhysEnd   (bssPhys + bssSize)
#define bssVirtStart (bssVirt)
#define bssVirtEnd   (bssEnd)


//
// -- These are the linker-provided symbols for the stab section
//    ----------------------------------------------------------
EXTERN ENTRY_DATA archsize_t stabStart;
EXTERN ENTRY_DATA archsize_t stabEnd;
EXTERN ENTRY_DATA archsize_t stabSize;
EXTERN ENTRY_DATA archsize_t stabPhys;
EXTERN ENTRY_DATA archsize_t stabVirt;

#define stabPhysStart (stabPhys)
#define stabPhysEnd   (stabPhys + stabSize)
#define stabVirtStart (stabVirt)
#define stabVirtEnd   (stabEnd)

