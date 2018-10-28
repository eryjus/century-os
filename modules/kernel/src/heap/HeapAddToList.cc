//===================================================================================================================
// kernel/src/HeapAddToList.cc -- Add a new Ordered List Entry into the list in the proper place
// 
// Add a new Ordered List Entry into the list in the proper place
//
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-07-11                          Initial version
//  2012-09-16                          Leveraged from Century
//  2018-06-01  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//
//===================================================================================================================


#include "types.h"
#include "heap.h"


//
// -- Add an ordered list entry to the heap structures
//    ------------------------------------------------
void HeapAddToList(OrderedList *entry)
{
	OrderedList *wrk, *sav;
	size_t size;
	
	if (!entry) HeapError("NULL entry in HeapAddToList()", "");
	HeapValidateHdr(entry->block, "HeapAddToList()");
	// cannot validate heap ptrs as may be empty
	
	size = entry->size;
	
	// assume that we are starting at the beginning
	wrk = kHeap->heapMemory;
	
	if (wrk) {
		if (size >= 512 && kHeap->heap512) wrk = kHeap->heap512;
		if (size >= 1024 && kHeap->heap1K) wrk = kHeap->heap1K;
		if (size >= 4096 && kHeap->heap4K) wrk = kHeap->heap4K;
		if (size >= 16384 && kHeap->heap16K) wrk = kHeap->heap16K;
	} else {
		// special case, nothing in the Ordered List; make it right and leave
		kHeap->heapMemory = entry;
		entry->next = entry->prev = 0;
		
		if (size >= 512) kHeap->heap512 = entry;
		if (size >= 1024) kHeap->heap1K = entry;
		if (size >= 4096) kHeap->heap4K = entry;
		if (size >= 16384) kHeap->heap16K = entry;
		
		goto out;
	}
	
	// in theory, wrk is now optimized for a faster search for the right size
	while (wrk) {	// while we have something to work with...
		if (wrk->size < size) {
			sav = wrk;
			wrk = wrk->next;
			continue;
		}
		
		// at this point, we need to insert before wrk
		entry->next = wrk;
		entry->prev = wrk->prev;
		if (entry->next) entry->next->prev = entry;
		if (entry->prev) entry->prev->next = entry;
		
		break;
	}
	
	// check if we need to add to the end -- special case
	if (!wrk) {
		sav->next = entry;
		entry->prev = sav;
		entry->next = 0;
	}
	
	// entry inserted; now fix-up the optimized pointers; start with NULLs
	if (!kHeap->heap512 && size >= 512) kHeap->heap512 = entry;
	if (!kHeap->heap1K && size >= 1024) kHeap->heap1K = entry;
	if (!kHeap->heap4K && size >= 4096) kHeap->heap4K = entry;
	if (!kHeap->heap16K && size >= 16384) kHeap->heap16K = entry;

	// fixup the pointer for >= 512 bytes
	if (kHeap->heap512) {
		if (kHeap->heap512->prev && kHeap->heap512->prev->size >= 512) {
			kHeap->heap512 = kHeap->heap512->prev;
		}
	}
	
	// fixup the pointer for >= 1024 bytes
	if (kHeap->heap1K) {
		if (kHeap->heap1K->prev && kHeap->heap1K->prev->size >= 1024) {
			kHeap->heap1K = kHeap->heap1K->prev;
		}
	}
	
	// fixup the pointer for >= 4096 bytes
	if (kHeap->heap4K) {
		if (kHeap->heap4K->prev && kHeap->heap4K->prev->size >= 4096) {
			kHeap->heap4K = kHeap->heap4K->prev;
		}
	}
	
	// fixup the pointer for >= 16384 bytes
	if (kHeap->heap16K) {
		if (kHeap->heap16K->prev && kHeap->heap16K->prev->size >= 16384) {
			kHeap->heap16K = kHeap->heap16K->prev;
		}
	}
	
out:
	HeapValidatePtr("HeapAddToList()");
	HeapValidateHdr(entry->block, "HeapAddToList() at exit");
}