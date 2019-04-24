//===================================================================================================================
//
//  HeapMergeRight.cc -- Merge the freeing block with the block to the right if free as well
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Merge the freeing block with the block to the right if free as well
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Jul-26                          Initial version
//  2012-Sep-16                          Leveraged from Century
//  2012-Sep-23    #90                   Fixed issue with calc'ing the right footer
//  2013-Sep-12   #101                   Resolve issues splint exposes
//  2013-Sep-13    #74                   Rewrite Debug.h to use assertions and write to TTY_LOG
//  2018-Sep-01  Initial   0.1.0   ADCL  Copy this file from century32 to century-os
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "heap.h"


//
// -- Merge a new hole with the existing hols on the right side of this one in memory
//    -------------------------------------------------------------------------------
OrderedList_t *HeapMergeRight(KHeapHeader_t *hdr)
{
    KHeapFooter_t *rightFtr;
    KHeapHeader_t *rightHdr;

    if (!hdr) HeapError("Bad Header passed into HeapMergeRight()", "");

    rightHdr = (KHeapHeader_t *)((byte_t *)hdr + hdr->size);
    rightFtr = (KHeapFooter_t *)((byte_t *)rightHdr + rightHdr->size - sizeof(KHeapFooter_t));

    if ((byte_t *)rightFtr + sizeof(KHeapFooter_t) > kHeap->endAddr) return 0;
    HeapValidateHdr(rightHdr, "rightHeader in HeapMergeRight()");
    if (!rightHdr->_magicUnion.isHole) return 0;        // make sure the left block is a hole

    HeapReleaseEntry(rightHdr->entry);
    hdr->size += rightHdr->size;
    rightFtr->hdr = hdr;
    hdr->_magicUnion.isHole = rightFtr->_magicUnion.isHole = 1;

    return HeapNewListEntry(hdr, 0);
}
