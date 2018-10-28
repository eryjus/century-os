//===================================================================================================================
// kernel/src/HeapAlignToPage.cc -- Align a block to a page boundary
//
// Split an entry to the first page boundary after allocating the header.  This will result in a free block on the 
// left of the page boundary.  This block may be small and if so will need to be added to the previous block (which 
// is allocated by definition) or at the beginning of the heap memory (special case).
//
// +------------------------------------------------------------------+
// |  The entry before splitting.  Split will occur at some location  |
// |  within the entry.                                               |
// +------------------------------------------------------------------+
// 
// One of 2 results will occur (as below):
// 
//                  Page
//                Boundary
//                    |
//                    |
//                    V
// +------------------+-----------------------------------------------+
// |  A small block   |  A brand new entry inserted into the          |
// |  too small to    |  ordered list for the remaining free memory.  |
// |  add as a hole.  |                                               |
// +------------------+-----------------------------------------------+
// |  A block of new  |  A brand new entry inserted into the          |
// |  free memory     |  ordered list for the remaining free memory.  |
// |  inserted to lst |                                               |
// +------------------+-----------------------------------------------+
//
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-07-04                          Initial version
//  2012-07-28    #53                   Fix small blocks corruption
//  2012-09-16                          Leveraged from Century
//  2013-09-12   #101                   Resolve issues splint exposes
//  2013-09-13    #74                   Rewrite Debug.h to use assertions and write to TTY_LOG
//  2018-06-01  Initial   0.1.0   ADCL  Copy this file from century32 to century-os
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "heap.h"


//
// -- Align a block to a Page boundary
//    --------------------------------
OrderedList *HeapAlignToPage(OrderedList *entry)
{
	KHeapHeader *newHdr, *oldHdr;
	KHeapFooter *newFtr, *oldFtr;
	size_t leftSize, rightSize;
	OrderedList *ret;
	
	if (!entry) HeapError("NULL entry in HeapAlignToPage()", "");
	HeapValidateHdr(entry->block, "HeapAlignToPage()");

	// initialize the working variables
	oldHdr = entry->block;
	newHdr = (KHeapHeader *)(HeapCalcPageAdjustment(entry));
	newFtr = (KHeapFooter *)((char *)newHdr - sizeof(KHeapFooter));
	oldFtr = (KHeapFooter *)((char *)oldHdr + oldHdr->size - sizeof(KHeapFooter));
	leftSize = (char *)newFtr - (char *)oldHdr + sizeof(KHeapFooter);
	rightSize = (char *)oldFtr - (char *)newHdr + sizeof(KHeapFooter);
	
	HeapReleaseEntry(entry);			// will have better one(s) later

	// size the left block properly
	if (leftSize < MIN_HOLE_SIZE) {
		KHeapHeader *wrkHdr;

		wrkHdr = ((KHeapFooter*)((char *)oldHdr - sizeof(KHeapFooter)))->hdr;

		if ((char *)wrkHdr >= kHeap->strAddr) {
			KHeapFooter sav;
			KHeapFooter *tmp = (KHeapFooter *)((char *)wrkHdr + wrkHdr->size - sizeof(KHeapFooter));
			
			sav = *tmp;
			wrkHdr->size += leftSize;
			
			tmp = (KHeapFooter *)((char *)wrkHdr + wrkHdr->size - sizeof(KHeapFooter));
			*tmp = sav;
			HeapValidateHdr(wrkHdr, "Work Header in HeapAlignToPage()");
		}
		oldHdr = 0;
	} else {
		oldHdr->_magicUnion.magicHole = HEAP_MAGIC;
		oldHdr->_magicUnion.isHole = 1;
		oldHdr->size = leftSize;
		newFtr->hdr = oldHdr;
		newFtr->_magicUnion.magicHole = oldHdr->_magicUnion.magicHole;
		
		(void)HeapNewListEntry(oldHdr, 1);
		HeapValidateHdr(oldHdr, "Old Header in HeapAlignToPage() else");
	}

	// size the right block properly
	newHdr->_magicUnion.magicHole = HEAP_MAGIC;
	newHdr->_magicUnion.isHole = 1;
	newHdr->size = rightSize;
	oldFtr->hdr = newHdr;
	oldFtr->_magicUnion.magicHole = newHdr->_magicUnion.magicHole;
	
	ret = HeapNewListEntry(newHdr, 1);
	if (oldHdr) HeapValidateHdr(oldHdr, "Old Header in HeapAlignToPage() at return");
	HeapValidateHdr(newHdr, "New Header in HeapAlignToPage() at return");
	return ret;
}
