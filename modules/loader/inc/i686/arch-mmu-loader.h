//===================================================================================================================
//
//  arch-mmu-laoder.h -- This is the architecture-specific mmu functions for the i686 architecture
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


#include "cpu.h"


//
// -- define the local type needed to manage the mmu data
//    ---------------------------------------------------
typedef ptrsize_t MmuData_t;


//
// -- Get a table entry from the table address and the desired virtual address
//    ------------------------------------------------------------------------
pageEntry_t *MmuGetTableEntry(pageEntry_t *table, ptrsize_t addr, int shift, bool alloc);


//
// -- Walk the page tables and get the frame for an address
//    -----------------------------------------------------
frame_t MmuGetFrameForAddr(MmuData_t cr3Phys, ptrsize_t addr);


//
// -- Get an address from a page entry
//    --------------------------------
inline pageEntry_t *MmuGetAddrFromEntry(pageEntry_t *e) { return (pageEntry_t *)((e->frame) << 12); }


//
// -- This is the working cr3 variable for i686
//    -----------------------------------------
extern MmuData_t mmuBase;


//
// -- This inline function will install the paging structures to the right place
//    --------------------------------------------------------------------------
inline void SetMmuTopAddr(void) { MmuSwitchPageDir(mmuBase); }


//
// -- This inline function will get the top level paging address
//    ----------------------------------------------------------
inline MmuData_t GetMmuTopAddr(void) { return mmuBase; }

