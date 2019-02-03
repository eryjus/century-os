//===================================================================================================================
//
//  arch-mmu-laoder.h -- This is the architecture-specific mmu functions for the rpi2b architecture
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-11  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __MMU_H__
#   error "Do not include 'arch-mmu-loader.h' directly.  Instead, include 'mmu.h'"
#endif


#include "types.h"
#include "pmm.h"
#include "cpu.h"


//
// -- define the local structure needed to manage the MMU structure needs
//    -------------------------------------------------------------------
typedef ptrsize_t MmuData_t;


//
// -- This is the working TTL for the rpi2b
//    -------------------------------------
extern MmuData_t mmuBase;


//
// -- This inline function will install the paging structures to the right place
//    --------------------------------------------------------------------------
void SetMmuTopAddr(void);


//
// -- This inline function will get the top level paging address
//    ----------------------------------------------------------
inline MmuData_t GetMmuTopAddr(void) { return mmuBase; }


//
// -- Create a new Ttl2 table
//    -----------------------
void MmuMakeTtl2Table(MmuData_t mmu, ptrsize_t addr);


//
// -- Is paging enabled?
//    ------------------
extern "C" bool MmuIsEnabled(void);


//
// -- Set the TTLB0, TTLB1, and # control bits and enable paging
//    ----------------------------------------------------------
extern "C" void MmuEnablePaging(ptrsize_t ttbr0);


//
// -- Get TTL1 Index from virtual Address
//    -----------------------------------
inline int MmuGetTtl1FromAddr(ptrsize_t addr) { return ((addr >> 20) & 0x0fff); }


//
// -- Get TTL2 Index from virtual Address -- this is into the management table only
//    -----------------------------------------------------------------------------
inline int MmuGetTtl2MgmtIdx(ptrsize_t addr) { return ((addr >> 12) & 0x000fffff); }


//
// -- Get TTL2 Index into frame for Address -- this is into the 4K frame only
//    -----------------------------------------------------------------------
inline int MmuGetTtl2FrameIdx(ptrsize_t addr) { return (MmuGetTtl2MgmtIdx(addr) & 3); }


