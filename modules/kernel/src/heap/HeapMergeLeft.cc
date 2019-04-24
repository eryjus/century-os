//===================================================================================================================
//
//  HeapMergeLeft.cc -- Merge the freeing block with the block to the left if free as well
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Merge the freeing block with the block to the left if free as well
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Jul-26                          Initial version
//  2012-Sep-16                          Leveraged from Century
//  2013-Sep-12   #101                   Resolve issues splint exposes
//  2013-Sep-13    #74                   Rewrite Debug.h to use assertions and write to TTY_LOG
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "heap.h"


//
// -- Merge this hole with the one on the left
//    ----------------------------------------
OrderedList_t *HeapMergeLeft(KHeapHeader_t *hdr)
{
    KHeapFooter_t *leftFtr = NULL;
    KHeapHeader_t *leftHdr = NULL;
    KHeapFooter_t *thisFtr = NULL;

    if (!hdr) HeapError("Bad Header passed into HeapMergeLeft()", "");

    thisFtr = (KHeapFooter_t *)((char *)hdr + hdr->size - sizeof(KHeapFooter_t));
    leftFtr = (KHeapFooter_t *)((char *)hdr - sizeof(KHeapFooter_t));

    // -- Check of this fits before dereferencing the pointer -- may end in `#PF` if first block
    if ((byte_t *)leftHdr < kHeap->strAddr) return 0;
    leftHdr = leftFtr->hdr;

    if (!leftHdr->_magicUnion.isHole) return 0;        // make sure the left block is a hole

    HeapReleaseEntry(leftHdr->entry);

    leftHdr->size += hdr->size;
    thisFtr->hdr = leftHdr;
    leftHdr->_magicUnion.isHole = thisFtr->_magicUnion.isHole = 1;

    return HeapNewListEntry(leftHdr, 0);
}
