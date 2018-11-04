//===================================================================================================================
//
//  inc/mmu.h -- The general purpose mmu interface file.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-06-24  Initial   0.1.0   ADCL  Initial version
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


//
// -- Some specific memory locations
//    ------------------------------
#define PROCESS_PAGE_DIR    0xff430000
#define PROCESS_PAGE_TABLE  0xff431000

// -- these are dedicated to the function `MmuGetFrameForAddr()`, but documented here.
#define MMU_FRAME_ADDR_PD   0xff432000
#define MMU_FRAME_ADDR_PT   0xff433000

#define PROCESS_STACK_BUILD 0xff441000

#define KERNEL_PAGE_DIR     0xfffff000


#endif
