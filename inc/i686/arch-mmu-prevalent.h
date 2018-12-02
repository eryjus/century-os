//===================================================================================================================
//
//  arch-mmu.h -- The i686 structures for interfacing with the Memory Management Unit (MMU)
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  PAE is not going to be supported.  This is going to be considered another architecture if it is supported.
//  This makes sense since the address space would change from 32-bits to 36-bits, and therefore requiring 64-bits
//  to store an address.  This requires a cross-compiler different from the i686-elf to do.
//
//  So, with the 32-bit address space, we have a 32-bit entry that will reference a 4K page at the Page Table
//  level.  Therefore, we can have 1024 entries per Page Table, or 4MB per Page Directory Entry.  The Page
//  Directory has 1024 entries, resulting in 4GB addressable space.  It is also important to note that with
//  recursive mapping, we give up 4MB of virtual address space.
//
//  I will leverage the paging structure to take one of the avl bits to indicate a kernel page.  This type of
//  page will have the following attributes:
//  * Will be mapped into other processes for use by SYSCALLS
//  * Will not be eligible for page swapping
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-May-14  Initial   0.0.0   ADCL  Initial version
//  2018-Jun-24  Initial   0.1.0   ADCL  Copied this file from century to century-os
//
//===================================================================================================================


#ifndef __MMU_H__
#   error "Do not include 'arch-mmu-prevalent.h' directly; include 'mmu.h' instead, which will pick up this file."
#endif


//
// -- This is a 32-bit page entry for both the page directory and the page tables
//    ---------------------------------------------------------------------------
typedef struct pageEntry_t {
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
} __attribute__((packed)) pageEntry_t;


//
// -- Make a new table by allocating a new frame and initializing it to 0
//    -------------------------------------------------------------------
pageEntry_t *MmuMakeNewTable(pageEntry_t *e);


//
// -- This is the location of the frame buffer
//    ----------------------------------------
const ptrsize_t FRAME_BUFFER_VADDR = 0xfb000000;


//
// -- This is the recursive mapping location
//    --------------------------------------
const ptrsize_t RECURSIVE_VADDR = 0xffc00000;
const ptrsize_t RECURSIVE_PD_VADDR = 0xfffff000;


//
// -- These are critical CPU structure locations
//    ------------------------------------------
const ptrsize_t GDT_ADDRESS = 0xff401000;
const ptrsize_t IDT_ADDRESS = 0xff401800;
