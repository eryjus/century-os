//===================================================================================================================
//
//  HeapCalcPageAdjustment.cc -- Calculate the adjustment to align the pointer to a page boundary
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Calculate the adjustment to align the pointer to a page boundary, not including the header (i.e. after the
//  header)
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Jul-26                          Initial version
//  2012-Sep-16                          Leveraged from Century
//  2013-Sep-12   #101                   Resolve issues splint exposes
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "heap.h"


//
// -- Calculate the adjustment needed to align to a page
//    --------------------------------------------------
ptrsize_t HeapCalcPageAdjustment(OrderedList_t *entry)
{
	ptrsize_t wrkPtr;

	wrkPtr = (ptrsize_t)entry->block + sizeof(KHeapHeader_t);

	// if not a page aligned block, align it
	if (wrkPtr & 0x00000fff) {
		wrkPtr = (wrkPtr & 0xfffff000) + 0x1000; //! next page
	}

	return wrkPtr - sizeof(KHeapHeader_t);
}
