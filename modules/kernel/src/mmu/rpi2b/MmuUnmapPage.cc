//===================================================================================================================
//
//  MmuUnmapPage.cc -- Unmap a page in virtual address space, returning the frame in case something else needs done.
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-21  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#include "cpu.h"
#include "printf.h"
#include "mmu-kernel.h"


//
// -- Check for the page and unmap if it is mapped.
//    ---------------------------------------------
frame_t MmuUnmapPage(ptrsize_t addr)
{
    kprintf("Unmap a page from the MMU for rpi2b here\n");
    Halt();
}
