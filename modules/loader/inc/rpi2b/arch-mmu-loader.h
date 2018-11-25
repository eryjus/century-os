//===================================================================================================================
//
//  arch-mmu-laoder.h -- This is the architecture-specific mmu functions for the rpi2b architecture
//
//        Copyright (c)  2017-2018 -- Adam Clark
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
// -- This is the working TTL for the rpi2b
//    -------------------------------------
extern ptrsize_t ttl1;


//
// -- This inline function will install the paging structures to the right place
//    --------------------------------------------------------------------------
void SetMmuTopAddr(void);


//
// -- This inline function will get the top level paging address
//    ----------------------------------------------------------
inline ptrsize_t GetMmuTopAddr(void) { return ttl1; }


//
// -- Create a new Ttl2 table
//    -----------------------
void MmuMakeTtl2Table(ptrsize_t ttl1, ptrsize_t addr);


//
// -- Is paging enabled?
//    ------------------
extern "C" bool MmuIsEnabled(void);


//
// -- Set the TTLB0, TTLB1, and # control bits and enable paging
//    ----------------------------------------------------------
extern "C" void MmuEnablePaging(ptrsize_t ttl1);
