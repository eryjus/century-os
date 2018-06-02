//===================================================================================================================
// kernel/src/HeapCalcPageAdjustment.cc -- Calculate the adjustment to align the pointer to a page boundary
// 
// Calculate the adjustment to align the pointer to a page boundary, not including the header (i.e. after the header)
//
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-07-26                          Initial version
//  2012-09-16                          Leveraged from Century
//  2013-09-12   #101                   Resolve issues splint exposes
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "heap.h"


ptrsize_t HeapCalcPageAdjustment(OrderedList *entry)
{
	ptrsize_t wrkPtr;
	
	wrkPtr = (ptrsize_t)entry->block + sizeof(KHeapHeader);
		
	// if not a page aligned block, align it
	if (wrkPtr & 0x00000fff) {
		wrkPtr = (wrkPtr & 0xfffff000) + 0x1000; //! next page
	}
	
	return wrkPtr - sizeof(KHeapHeader);
}
