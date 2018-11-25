//===================================================================================================================
//
//  arch-mmu-kernel.h -- This is the kernel MMU manager header specific to i686
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  --------------------------------------------------------------------------
//  2018-Nov-21  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __MMU_H__
#   error "Do not include 'arch-mmu-kernel.h' directly.  Include 'mmu-kernel.h' and this file will be included"
#endif


//
// -- These are the helper functions to make MMU management nearly painless
//    ---------------------------------------------------------------------
inline int MmuGetPDIndexFromAddr(ptrsize_t addr) { return (addr >> 22) & 0x3ff; }
inline int MmuGetPTIndexFromAddr(ptrsize_t addr) { return (addr >> 12) & 0x3ff; }
inline pageEntry_t *MmuGetPDAddress(void) { return (pageEntry_t *)0xfffff000; }
inline pageEntry_t *MmuGetPTAddress(ptrsize_t addr) {
    return (pageEntry_t *)(0xffc00000 + (MmuGetPDIndexFromAddr(addr) * 0x1000));
}
inline pageEntry_t *MmuGetPDEntry(ptrsize_t addr) { return &MmuGetPDAddress()[MmuGetPDIndexFromAddr(addr)]; }
inline pageEntry_t *MmuGetPTEntry(ptrsize_t addr) { return &MmuGetPTAddress(addr)[MmuGetPTIndexFromAddr(addr)]; }


