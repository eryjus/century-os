//===================================================================================================================
//
//  HeapAlignToPage.cc -- Align a block to a page boundary
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Split an entry to the first page boundary after allocating the header.  This will result in a free block on the
//  left of the page boundary.  This block may be small and if so will need to be added to the previous block (which
//  is allocated by definition) or at the beginning of the heap memory (special case).
//
//  +------------------------------------------------------------------+
//  |  The entry before splitting.  Split will occur at some location  |
//  |  within the entry.                                               |
//  +------------------------------------------------------------------+
//
//  One of 2 results will occur (as below):
//
//                   Page
//                 Boundary
//                     |
//                     |
//                     V
//  +------------------+-----------------------------------------------+
//  |  A small block   |  A brand new entry inserted into the          |
//  |  too small to    |  ordered list for the remaining free memory.  |
//  |  add as a hole.  |                                               |
//  +------------------+-----------------------------------------------+
//  |  A block of new  |  A brand new entry inserted into the          |
//  |  free memory     |  ordered list for the remaining free memory.  |
//  |  inserted to lst |                                               |
//  +------------------+-----------------------------------------------+
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Jul-04                          Initial version
//  2012-Jul-28    #53                   Fix small blocks corruption
//  2012-Sep-16                          Leveraged from Century
//  2013-Sep-12   #101                   Resolve issues splint exposes
//  2013-Sep-13    #74                   Rewrite Debug.h to use assertions and write to TTY_LOG
//  2018-Jun-01  Initial   0.1.0   ADCL  Copy this file from century32 to century-os
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "heap.h"


//
// -- Align a block to a Page boundary
//    --------------------------------
OrderedList_t *HeapAlignToPage(OrderedList_t *entry)
{
	KHeapHeader_t *newHdr, *oldHdr;
	KHeapFooter_t *newFtr, *oldFtr;
	size_t leftSize, rightSize;
	OrderedList_t *ret;

	if (!entry) HeapError("NULL entry in HeapAlignToPage()", "");
	HeapValidateHdr(entry->block, "HeapAlignToPage()");

	// initialize the working variables
	oldHdr = entry->block;
	newHdr = (KHeapHeader_t *)(HeapCalcPageAdjustment(entry));
	newFtr = (KHeapFooter_t *)((char *)newHdr - sizeof(KHeapFooter_t));
	oldFtr = (KHeapFooter_t *)((char *)oldHdr + oldHdr->size - sizeof(KHeapFooter_t));
	leftSize = (char *)newFtr - (char *)oldHdr + sizeof(KHeapFooter_t);
	rightSize = (char *)oldFtr - (char *)newHdr + sizeof(KHeapFooter_t);

	HeapReleaseEntry(entry);			// will have better one(s) later

	// size the left block properly
	if (leftSize < MIN_HOLE_SIZE) {
		KHeapHeader_t *wrkHdr;

		wrkHdr = ((KHeapFooter_t *)((byte_t *)oldHdr - sizeof(KHeapFooter_t )))->hdr;

		if ((byte_t *)wrkHdr >= kHeap->strAddr) {
			KHeapFooter_t sav;
			KHeapFooter_t *tmp = (KHeapFooter_t *)((char *)wrkHdr + wrkHdr->size - sizeof(KHeapFooter_t));

			sav = *tmp;
			wrkHdr->size += leftSize;

			tmp = (KHeapFooter_t *)((char *)wrkHdr + wrkHdr->size - sizeof(KHeapFooter_t));
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
