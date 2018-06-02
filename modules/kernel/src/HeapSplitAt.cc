//===================================================================================================================
// kernel/src/HeapSplitAt.cc -- Split an entry to the provided adjustedSize
// 
// Split an entry to the provided adjustToSize.  This includes adding a new entry for the newly created hole into 
// the heap list.
//
// +------------------------------------------------------------------+
// |  The entry before splitting.  Split will occur at some location  |
// |  within the entry.                                               |
// +------------------------------------------------------------------+
// 
// +------------------+-----------------------------------------------+
// |  The new header  |  A brand new entry inserted into the          |
// |  and block of    |  ordered list for the remaining free memory.  |
// |  memory.         |                                               |
// +------------------+-----------------------------------------------+
//
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-07-04                          Initial version
//  2012-09-16                          Leveraged from Century
//  2013-09-12   #101                   Resolve issues splint exposes
//  2018-06-01  Initial   0.1.0   ADCL  Copied this file from century32 tp century-os
//
//===================================================================================================================


#include "types.h"
#include "heap.h"


//
// -- Split a block to the indicated size
//    -----------------------------------
KHeapHeader *HeapSplitAt(OrderedList *entry, size_t adjustToSize)
{
	KHeapHeader *newHdr, *oldHdr;
	KHeapFooter *newFtr, *oldFtr;
	size_t newSize;
	
	if (!entry) HeapError("NULL entry in HeapSplitAt()", "");
	HeapValidateHdr(entry->block, "HeapSplitAt()");
	HeapValidatePtr("HeapSplitAt()");
	
	// initialize the working variables
	oldHdr = entry->block;
	oldFtr = (KHeapFooter *)((char *)oldHdr + oldHdr->size - sizeof(KHeapFooter));
	newHdr = (KHeapHeader *)((char *)oldHdr + adjustToSize);
	newFtr = (KHeapFooter *)((char *)newHdr - sizeof(KHeapFooter));
	newSize = oldHdr->size - adjustToSize;

	HeapReleaseEntry(entry);		// release entry; will replace with back half
	
	// size the allocated block properly
	oldHdr->size = adjustToSize;
	oldHdr->_magicUnion.isHole = 0;
	newFtr->hdr = oldHdr;
	newFtr->_magicUnion.magicHole = oldHdr->_magicUnion.magicHole;

	// create the new hole and add it to the list
	newHdr->_magicUnion.magicHole = HEAP_MAGIC;
	newHdr->_magicUnion.isHole = 1;
	newHdr->size = newSize;
	oldFtr->_magicUnion.magicHole = newHdr->_magicUnion.magicHole;
	oldFtr->hdr = newHdr;

	(void)HeapNewListEntry(newHdr, 1);
	
	// make sure we didn't make a mess
	HeapValidateHdr(oldHdr, "HeapSplitAt [oldHdr]");
	HeapValidateHdr(newHdr, "HeapSplitAt [newHdr]");

	// return the header to the allocated block
	return oldHdr;
}
