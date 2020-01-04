//===================================================================================================================
//
//  arch-mmu-kernel.h -- This is the kernel MMU manager header specific to i686
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  --------------------------------------------------------------------------
//  2018-Nov-21  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#ifndef __MMU_H__
#   error "Do not include 'arch-mmu.h' directly.  Include 'mmu.h' and this file will be included"
#endif


//
// -- This is the recursive mapping location
//    --------------------------------------
const archsize_t RECURSIVE_VADDR = 0xffc00000;
const archsize_t RECURSIVE_PD_VADDR = 0xfffff000;


//
// -- This is a 32-bit page entry for both the page directory and the page tables
//    ---------------------------------------------------------------------------
typedef struct PageEntry_t {
    unsigned int p : 1;                 // Is the page present?
    unsigned int rw : 1;                // set to 1 to allow writes
    unsigned int us : 1;                // 0=Supervisor; 1=user
    unsigned int pwt : 1;               // Page Write Through
    unsigned int pcd : 1;               // Page-level cache disable
    unsigned int a : 1;                 // accessed
    unsigned int d : 1;                 // dirty (needs to be written for a swap)
    unsigned int pat : 1;               // set to 0 for tables, page Page Attribute Table (set to 0)
    unsigned int g : 1;                 // Global (set to 0)
    unsigned int k : 1;                 // Is this a kernel page?
    unsigned int avl : 2;               // Available for software use
    unsigned int frame : 20;            // This is the 4K aligned page frame address (or table address)
} __attribute__((packed)) PageEntry_t;


//
// -- These are the helper functions to make MMU management nearly painless
//    ---------------------------------------------------------------------
inline int MmuGetPDIndexFromAddr(archsize_t addr) { return (addr >> 22) & 0x3ff; }
inline int MmuGetPTIndexFromAddr(archsize_t addr) { return (addr >> 12) & 0x3ff; }
inline PageEntry_t *MmuGetPDAddress(void) { return (PageEntry_t *)RECURSIVE_PD_VADDR; }
inline PageEntry_t *MmuGetPTAddress(archsize_t addr) {
    return (PageEntry_t *)(RECURSIVE_VADDR + (MmuGetPDIndexFromAddr(addr) * 0x1000));
}
inline PageEntry_t *MmuGetPDEntry(archsize_t addr) { return &MmuGetPDAddress()[MmuGetPDIndexFromAddr(addr)]; }
inline PageEntry_t *MmuGetPTEntry(archsize_t addr) { return &MmuGetPTAddress(addr)[MmuGetPTIndexFromAddr(addr)]; }


extern "C" void InvalidatePage(archsize_t addr);
extern "C" void __krntext MmuDumpTables(archsize_t addr);

