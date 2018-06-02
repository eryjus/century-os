//===================================================================================================================
// kernel/src/HeapFindHole.cc -- Find the smallest hole that has the size needed
// 
// Find the smallest hole that has the size required.  Align the block as necessary and ensure the remaining block 
// is big enough.
//
// ------------------------------------------------------------------------------------------------------------------
//
// IMPORTANT PROGRAMMING NOTE:
// The calling function must guarantee that adjustedSize be >= sizeof(KHeapHeader) + sizeof(KHeapFooter) + 1.  
// This function will not check its validity.
//
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-07-02                          Initial version
//  2012-09-16                          Leveraged from Century
//  2013-09-12   #101                   Resolve issues splint exposes
//  2018-06-01  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
// 
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "heap.h"


//
// -- Find the best fit hole in the list of holes
//    -------------------------------------------
OrderedList *HeapFindHole(size_t adjustedSize, bool align)
{
	OrderedList *wrk;
	size_t wrkSize;
	
	// First determine the right starting point for searching.
	if (adjustedSize < 512) wrk = kHeap->heapMemory;
	if (adjustedSize >= 512 && adjustedSize < 1024) wrk = kHeap->heap512;
	if (adjustedSize >= 1024 && adjustedSize < 4096) wrk = kHeap->heap1K;
	if (adjustedSize >= 4096 && adjustedSize < 16384) wrk = kHeap->heap4K;
	if (adjustedSize >= 16384) wrk = kHeap->heap16K;
	
	// in theory, wrk is now optimized for a faster search for the right size
	while (wrk) {	// while we have something to work with...
		if (wrk->size < adjustedSize) {
			wrk = wrk->next;
			continue;
		}
		
		// first entry of sufficient size and we are not aligning; use it
		if (wrk->size >= adjustedSize && !align) return wrk;

		// at this point, guaranteed to be looking for an aligned block	
		// find the real block location; now, calculate the new block size
		wrkSize = wrk->size - (HeapCalcPageAdjustment(wrk) - (ptrsize_t)wrk->block);

		// check if we have overrun the block
		if (wrkSize <= 0) {
			wrk = wrk->next;
			continue;
		}
			
		// wrkSize now has the available memory for the block after adjusting
		// for page alignment; remember we pulled the size of the header out
		// check for a fit
		if (wrkSize >= adjustedSize - sizeof(KHeapHeader)) return wrk;
			
		// not big enough yet, move on
		wrk = wrk->next;
	}
	
	// no memory to allocate
	return 0;
}
