//===================================================================================================================
//
//  mmu.h -- The general purpose mmu interface file.
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Jun-24  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __MMU_H__
#define __MMU_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "types.h"
#include "arch-mmu.h"


//
// -- With the page table structures given, map a virtual address to a physical frame
//    -------------------------------------------------------------------------------
void MmuMapToFrame(ptrsize_t cr3, ptrsize_t addr, frame_t frame, bool wrt, bool krn);


//
// -- Unmap a page from the page table
//    --------------------------------
void MmuUnmapPage(ptrsize_t cr3, ptrsize_t addr);


//
// -- Initialize the MMU
//    ------------------
void MmuInit(void);


//
// -- Get a table entry from the table address and the desired virtual address
//    ------------------------------------------------------------------------
pageEntry_t *MmuGetTableEntry(pageEntry_t *table, ptrsize_t addr, int shift, bool alloc);


//
// -- Walk the page tables and get the frame for an address
//    -----------------------------------------------------
frame_t MmuGetFrameForAddr(ptrsize_t cr3Phys, ptrsize_t addr);


//
// -- Dump the paging tables
//    ----------------------
void MmuDumpTables(ptrsize_t addr);


//
// -- Get an address from a page entry
//    --------------------------------
inline pageEntry_t *MmuGetAddrFromEntry(pageEntry_t *e) { return (pageEntry_t *)((e->frame) << 12); }


#endif
