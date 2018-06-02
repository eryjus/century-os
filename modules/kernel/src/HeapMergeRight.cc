//===================================================================================================================
// kernel/src/HeapMergeRight.cc -- Merge the freeing block with the block to the right if free as well
// 
// Merge the freeing block with the block to the right if free as well
//
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-07-26                          Initial version
//  2012-09-16                          Leveraged from Century
//  2012-09-23    #90                   Fixed issue with calc'ing the right footer
//  2013-09-12   #101                   Resolve issues splint exposes
//  2013-09-13    #74                   Rewrite Debug.h to use assertions and write to TTY_LOG
//  2018-06-01  Initial   0.1.0   ADCL  Copy this file from century32 to century-os
//
//===================================================================================================================


#include "types.h"
#include "heap.h"


//
// -- Merge a new hole with the existing hols on the right side of this one in memory
//    -------------------------------------------------------------------------------
OrderedList *HeapMergeRight(KHeapHeader *hdr)
{
	KHeapFooter *rightFtr;
	KHeapHeader *rightHdr;
	
	if (!hdr) HeapError("Bad Header passed into HeapMergeRight()", "");

	rightHdr = (KHeapHeader *)((char *)hdr + hdr->size);
	rightFtr = (KHeapFooter *)((char*)rightHdr + rightHdr->size - sizeof(KHeapFooter));
	
	if ((char *)rightFtr + sizeof(KHeapFooter) > kHeap->endAddr) return 0;
	HeapValidateHdr(rightHdr, "rightHeader in HeapMergeRight()");
	if (!rightHdr->_magicUnion.isHole) return 0;		// make sure the left block is a hole
	
	HeapReleaseEntry(rightHdr->entry);
	hdr->size += rightHdr->size;
	rightFtr->hdr = hdr;
	hdr->_magicUnion.isHole = rightFtr->_magicUnion.isHole = 1;
	
	return HeapNewListEntry(hdr, 0);
}
