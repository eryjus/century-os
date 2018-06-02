//===================================================================================================================
// kernel/src/HeapAlloc.cc -- Allocate a number of bytes from the heap
// 
// Allocate a number of bytes from the heap, returning a pointer to the block of memory requested.  This block of 
// memory is adjusted for the number of bytes in the header block; the pointer is the first byte beyond the header.
// 
// The following conditions are accounted for in this function:
// 1.  A hole is found that is EXACTLY the size needed (rare) -- allocate it
// 2.  A hole is found that is slightly larger than needed, but not enough space to realistically leave another 
//     hole behind -- allocate the hole
// 3.  A hole is found to be too big -- split the hole and allocate the correct amount of heap
// 4.  A hole that is not enough can be found -- return 0
//
// When a request for memory must be page aligned:
// 5.  A hole before the allocated memory is too small -- add it to the previous block
// 6.  A hole after the allocated memory is too small -- allocate it with the requested memory
// 7.  Both the 2 situations above -- completed both actions
// 8.  A hole is too big -- split it accordingly taking into account the above
//
// TODO: Fix potential memory leak when multiple small alignments get added to previous blocks that will never 
//       be deallocated.
//
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-07-02                          Initial version
//  2012-08-15    #44                   Implement Enhancement #44 (Use Mutex to lock) (2018-05-31 - removed)
//  2012-09-16                          Leveraged from Century
//  2013-09-01    #80                   Re-implement Mutexes (that work now) (commented again)
//  2013-09-12   #101                   Resolve issues splint exposes
//  2013-09-13    #74                   Rewrite Debug.h to use assertions and write to TTY_LOG
//  2018-05-31  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "heap.h"


//
// -- Alloc a block of memory from the heap
//    -------------------------------------
void *HeapAlloc(size_t size, bool align)
{
	regval_t flags = DisableInterrupts();
	size_t adjustedSize;
	OrderedList *entry;
	KHeapHeader *hdr;
	
	if (size < HEAP_SMALLEST) size = HEAP_SMALLEST;		    // must allocate at least 1 byte

	if (size & (BYTE_ALIGNMENT - 1)) {	                    // Check for alignment 
		size += BYTE_ALIGNMENT;
		size &= ~(BYTE_ALIGNMENT - 1);
	}

	adjustedSize = size + sizeof(KHeapHeader) + sizeof(KHeapFooter);
	
	entry = HeapFindHole(adjustedSize, align);

	if (!entry) {
		HeapCheckHealth();
		RestoreInterrupts(flags);

		return 0;
	}

	HeapValidateHdr(entry->block, "HeapAlloc()");
	hdr = entry->block;
	
	// if we are aligning, take care of it now
	if (align) {
		entry = HeapAlignToPage(entry);		// must reset entry
		
		if (!entry) {
			HeapCheckHealth();
			RestoreInterrupts(flags);

			return 0;
		}

		HeapValidateHdr(entry->block, "HeapAlloc() after alignment");
		hdr = entry->block;
	}

	// perfect fit -OR- just a little too big
	if (hdr->size == adjustedSize || adjustedSize - hdr->size < MIN_HOLE_SIZE) {
		KHeapFooter *ftr;
		
		ftr = (KHeapFooter *)((char *)hdr + hdr->size - sizeof(KHeapFooter));
		
		HeapReleaseEntry(entry);
		hdr->_magicUnion.isHole = 0;
		ftr->_magicUnion.isHole = 0;
		HeapValidateHdr(hdr, "Resulting Header before return (good size)");
		HeapCheckHealth();
		RestoreInterrupts(flags);

		return (void *)((char *)hdr + sizeof(KHeapHeader));
	}
	
	// the only thing left is that it is too big and needs to be split
	hdr = HeapSplitAt(entry, adjustedSize);		// var entry is no longer valid after call
	HeapValidatePtr("HeapAlloc()");
	HeapValidateHdr(hdr, "Resulting Header before return (big size)");
	HeapCheckHealth();
	RestoreInterrupts(flags);

	return (void *)((char *)hdr + sizeof(KHeapHeader));
}
