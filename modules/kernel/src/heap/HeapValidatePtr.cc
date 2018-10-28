//===================================================================================================================
// kernel/src/ValidateHeapPtr.cc -- Validate the heap pointer.
// 
// Validate the heap pointer.
//
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-07-02                          Initial version
//  2012-09-16                          Leveraged from Century
//  2012-09-23                          Removed DUMP() define
//  2013-09-12   #101                   Resolve issues splint exposes
//  2018-06-01  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//
//===================================================================================================================


#include "types.h"
#include "heap.h"


//
// -- Check the heap structure
//    ------------------------
void HeapValidatePtr(const char *from)
{
	if (!kHeap->heapMemory) {
		HeapError(from, "Start of heapMemory is empty");
	}
	
	if (!kHeap->heapMemory) return;

	HeapValidateHdr(kHeap->heapMemory->block, from);
}
