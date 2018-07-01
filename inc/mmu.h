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
void MmuMapToFrame(ptrsize_t cr3, ptrsize_t addr, frame_t frame);


//
// -- Initialize the MMU
//    ------------------
void MmuInit(void);


//
// -- Get a table entry from the table address and the desired virtual address
//    ------------------------------------------------------------------------
pageEntry_t *MmuGetTableEntry(pageEntry_t *table, ptrsize_t addr, int shift);


//
// -- Dump the paging tables
//    ----------------------
void MmuDumpTables(ptrsize_t addr);


#endif
