//===================================================================================================================
//
//  MmuMakeNewTable.cc -- Create a new Table when we need to allocate a new Page Table.
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Jun-27  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "cpu.h"
#include "pmm.h"
#include "mmu-loader.h"


//
// -- Make a new Paging table
//    -----------------------
pageEntry_t *MmuMakeNewTable(pageEntry_t *e)
{
    frame_t frame = PmmNewFrame(1);
    kMemSetB((void *)PmmFrameToLinear(frame), 0, 4096);
    e->frame = frame;
    return e;
}
