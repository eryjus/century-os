//===================================================================================================================
//
//  ValidateHeapPtr.cc -- Validate the heap pointer.
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Validate the heap pointer.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Jul-02                          Initial version
//  2012-Sep-16                          Leveraged from Century
//  2012-Sep-23                          Removed DUMP() define
//  2013-Sep-12   #101                   Resolve issues splint exposes
//  2018-Jun-01  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
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
