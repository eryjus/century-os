//===================================================================================================================
// kernel/src/HeapMergeLeft.cc -- Merge the freeing block with the block to the left if free as well
//
// Merge the freeing block with the block to the left if free as well
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-07-26                          Initial version
//  2012-09-16                          Leveraged from Century
//  2013-09-12   #101                   Resolve issues splint exposes
//  2013-09-13    #74                   Rewrite Debug.h to use assertions and write to TTY_LOG
//
//===================================================================================================================


#include "types.h"
#include "heap.h"


//
// -- Merge this hole with the one on the left
//    ----------------------------------------
OrderedList_t *HeapMergeLeft(KHeapHeader_t *hdr)
{
	KHeapFooter_t *leftFtr;
	KHeapHeader_t *leftHdr;
	KHeapFooter_t *thisFtr;

	if (!hdr) HeapError("Bad Header passed into HeapMergeLeft()", "");

	thisFtr = (KHeapFooter_t *)((char *)hdr + hdr->size - sizeof(KHeapFooter_t));
	leftFtr = (KHeapFooter_t *)((char *)hdr - sizeof(KHeapFooter_t));
	leftHdr = leftFtr->hdr;

	if ((byte_t *)leftHdr < kHeap->strAddr) return 0;
	if (!leftHdr->_magicUnion.isHole) return 0;		// make sure the left block is a hole

	HeapReleaseEntry(leftHdr->entry);

	leftHdr->size += hdr->size;
	thisFtr->hdr = leftHdr;
	leftHdr->_magicUnion.isHole = thisFtr->_magicUnion.isHole = 1;

	return HeapNewListEntry(leftHdr, 0);
}
