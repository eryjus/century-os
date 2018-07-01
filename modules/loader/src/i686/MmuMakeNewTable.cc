//===================================================================================================================
//
//  loader/src/i686/MmuMakeNewTable.cc -- Create a new Table when we need to allocate a new Page Table.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-06-27  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "cpu.h"
#include "pmm.h"
#include "mmu.h"


//
// -- Make a new Paging table
//    -----------------------
pageEntry_t *MmuMakeNewTable(pageEntry_t *e)
{
    frame_t frame = PmmNewFrame();
    kMemSetB((void *)PmmFrameToLinear(frame), 0, 4096);
    e->frame = frame;
    return e;
}
