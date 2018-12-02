//===================================================================================================================
//
//  arch-mmu.h -- The rpi2b structures for interfacing with the Memory Management Unit (MMU)
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The ARM architecture is different than the x86-family architecture.  The MMU tables are called Translation
//  Tables and there are 2 Levels: 1 and 2.  There is 1 TTL1 table that is 16K long and up to 4096 TTL2 tables
//  that are 1K long each.  We are going to stuff 4 X 1K (consecutive) tables into a single 4K frame, mapping all
//  4K as a single operation, and we will aggregate 4 X 4K frames into one 16K aligned TTL1 table.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-11  Initial   0.2.0   ADCL  Initial version
//  2018-Nov-14  Initial   0.2.0   ADCL  Copied the structures from century
//
//===================================================================================================================


#ifndef __MMU_H__
#   error "Do not include 'arch-mmu-prevalent.h' directly; include 'mmu.h' instead, which will pick up this file."
#endif


#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "types.h"


//
// -- The Translation Table Level 1 structure (TTL1)
//    ----------------------------------------------
typedef struct Ttl1_t {
    unsigned int fault : 2;             // 00=fault; 01=TTL2 table address; 01 and 11 unused
    unsigned int sbz : 3;               // sbz = should be zero
    unsigned int domain : 4;            // domain -- we will use 0b0000 for now
    unsigned int p : 1;                 // unimplemented in the rpi2b arch; use 0
    unsigned int ttl2 : 22;             // the frame address of the ttl2 table (notice aligned to 1K)
} __attribute__((packed)) Ttl1_t;


//
// -- The Translation Table Level 2 structure (TTL2)
//    ----------------------------------------------
typedef struct Ttl2_t {
    unsigned int fault : 2;             // 00=fault; 01=large page(not used); 1x=small page (x sets execute never)
    unsigned int b : 1;                 // buffered
    unsigned int c : 1;                 // cached
    unsigned int ap : 2;                // access permissions
    unsigned int tex : 3;               // Type Extension
    unsigned int apx : 1;               // access permission extension
    unsigned int s : 1;                 // sharable
    unsigned int nG : 1;                // not Global
    unsigned int frame : 20;            // this is the final 4K frame address
} __attribute__((packed)) Ttl2_t;


//
// -- This is the location of the frame buffer
//    ----------------------------------------
const ptrsize_t FRAME_BUFFER_VADDR = 0xfb000000;


//
// -- This is the location of the TTL1/TTL2 Tables
//    --------------------------------------------
const ptrsize_t TTL1_VADDR = 0x80400000;
const ptrsize_t TTL2_VADDR = 0x80000000;


//
// -- This is the location of the exception vector table
//    --------------------------------------------------
const ptrsize_t EXCEPT_VECTOR_TABLE = 0xff401000;

