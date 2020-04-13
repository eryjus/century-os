//===================================================================================================================
//
//  HeapExpand.cc -- Expand the size of the heap
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-12  Initial  v0.6.1b  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "pmm.h"
#include "printf.h"
#include "mmu.h"
#include "heap.h"


//
// -- Expand the heap size (we have the heap lock)
//    --------------------------------------------
EXTERN_C EXPORT KERNEL
size_t HeapExpand(void)
{
    // TODO: remove the following line
    return 0;


    if (!assert_msg(kHeap->endAddr < kHeap->maxAddr, "All Heap memory allocated; unable to create more")) {
        return 0;
    }

    kprintf("Expanding heap...\n");

    size_t rv = 0;
    byte_t *newEnd = kHeap->endAddr + HEAP_SIZE_INCR;

    if (newEnd > kHeap->maxAddr) newEnd = kHeap->maxAddr;

    kprintf(".. new end will be %p (%d additional pages)\n", newEnd, (newEnd - kHeap->endAddr) >> 12);

    while (kHeap->endAddr < newEnd) {
        kprintf(".. getting a frame...\n");
        frame_t frame = PmmAllocateFrame();
        kprintf(".. mapping\n");
        MmuMapToFrame((archsize_t)kHeap->endAddr, frame, PG_KRN | PG_WRT);
        kprintf(".. done\n");
        kHeap->endAddr += PAGE_SIZE;
        rv += PAGE_SIZE;
    }

    kprintf("Heap expanded by %d bytes\n", rv);

    return rv;
}

