//===================================================================================================================
//
//  arch-mmu.h -- The rpi2b structures for interfacing with the Memory Management Unit (MMU)
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-11  Initial   0.2.0   ADCL  Initial version
//  2018-Nov-14  Initial   0.2.0   ADCL  Copied the structures from century
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//  2020-Apr-26  Initial  v0.7.0a  ADCL  Replace the MMU code with Long Descriptors
//
//===================================================================================================================


#ifndef __MMU_H__
#   error "Do not include 'arch-mmu-prevalent.h' directly; include 'mmu.h' instead, which will pick up this file."
#endif


#include "types.h"


//
// -- This combined structure is used for a table & page descriptor
//    -------------------------------------------------------------
typedef struct LongDescriptor_t { // Used at:  Pg : Tbl :
    uint32_t present : 1;                   // Y  :  Y  : 1: the table is present; 0: the table is not present
    uint32_t flag : 1;                      // Y  :  Y  : 1: table/page record (0 is not used)
    uint32_t attrIndex : 3;                 // Y  :  N  : indicates the type of mem (device, strongly ordered, etc)
    uint32_t ns : 1;                        // Y  :  N  : 1: non-secure bit (not secured)
    uint32_t ap : 2;                        // Y  :  N  : 01: upper 2 AP bits: use 01 to be able to r/w at any priv
    uint32_t sh : 2;                        // Y  :  N  : 11: inner sharable
    uint32_t af : 1;                        // Y  :  N  : 1: access flag
    uint32_t nG : 1;                        // Y  :  N  : 0: not Global bit
    uint32_t physAddress : 28;              // Y  :  Y  : only the Least Significant 20 bits are used
    uint32_t ignored : 12;                  // N  :  N  : these bits are ignored
    uint32_t contiguous : 1;                // Y  :  N  : 0: part of a contiguous block of memory (only 4K pages)
    uint32_t pxn : 1;                       // Y  :  N  : Privileged eXecute Never
    uint32_t xn : 1;                        // Y  :  N  : eXecute Never (set to 1 when _PG_WRT == 0)
    uint32_t software : 4;                  // Y  :  N  : available for kernel use (will set to 0000)
    uint32_t dftpxn : 1;                    // N  :  Y  : Privileged eXecute Never (set to 0, control at page)
    uint32_t dftxn : 1;                     // N  :  Y  : eXecute Never (set to 0, control at page)
    uint32_t dftap : 2;                     // N  :  Y  : Access Permissions Limit (set to 00, control at page)
    uint32_t dftns : 1;                     // N  :  Y  : Not Secured (set to 0, control at page)
} __attribute((packed)) LongDescriptor_t;


//
// -- These macros will help with determining which entry to use
//    ----------------------------------------------------------
#define LEVEL2ENT(addr)         ((addr >> 21) & 0x3ff)
#define LEVEL3ENT(addr)         ((addr >> 12) & 0x7ffff)



//
// -- Debugging function to dump the MMU Tables for an address
//    --------------------------------------------------------
EXTERN_C EXPORT KERNEL
void MmuDumpTables(archsize_t addr);


//
// -- No need to make a user table -- this is already cleared
//    -------------------------------------------------------
EXTERN_C INLINE
void MmuMakeTopUserTable(frame_t frame) {}


//
// -- Get the current MMU top user table
//    ----------------------------------
EXTERN_C EXPORT KERNEL
archsize_t MmuGetTopUserTable(void);


//
// -- Get the current MMU top kernel table
//    ------------------------------------
EXTERN_C EXPORT KERNEL
archsize_t MmuGetTopKernelTable(void);


//
// -- Get the current MMU top mmu table
//    ---------------------------------
EXTERN_C INLINE
archsize_t MmuGetTopTable(archsize_t addr) {
    return ((addr & 0x80000000) == 0 ? MmuGetTopUserTable() : MmuGetTopKernelTable());
}

