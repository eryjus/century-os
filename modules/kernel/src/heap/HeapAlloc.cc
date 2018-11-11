//===================================================================================================================
//
//  HeapAlloc.cc -- Allocate a number of bytes from the heap
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Allocate a number of bytes from the heap, returning a pointer to the block of memory requested.  This block of
//  memory is adjusted for the number of bytes in the header block; the pointer is the first byte beyond the header.
//
//  The following conditions are accounted for in this function:
//  1.  A hole is found that is EXACTLY the size needed (rare) -- allocate it
//  2.  A hole is found that is slightly larger than needed, but not enough space to realistically leave another
//      hole behind -- allocate the hole
//  3.  A hole is found to be too big -- split the hole and allocate the correct amount of heap
//  4.  A hole that is not enough can be found -- return 0
//
//  When a request for memory must be page aligned:
//  5.  A hole before the allocated memory is too small -- add it to the previous block
//  6.  A hole after the allocated memory is too small -- allocate it with the requested memory
//  7.  Both the 2 situations above -- completed both actions
//  8.  A hole is too big -- split it accordingly taking into account the above
//
//  TODO: Fix potential memory leak when multiple small alignments get added to previous blocks that will never
//        be deallocated.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Jul-02                          Initial version
//  2012-Aug-15    #44                   Implement Enhancement #44 (Use Mutex to lock) (2018-05-31 - removed)
//  2012-Sep-16                          Leveraged from Century
//  2013-Sep-01    #80                   Re-implement Mutexes (that work now) (commented again)
//  2013-Sep-12   #101                   Resolve issues splint exposes
//  2013-Sep-13    #74                   Rewrite Debug.h to use assertions and write to TTY_LOG
//  2018-May-31  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "spinlock.h"
#include "heap.h"


Spinlock_t heapLock = {0};

//
// -- Alloc a block of memory from the heap
//    -------------------------------------
void *HeapAlloc(size_t size, bool align)
{
	SPIN_BLOCK(heapLock) {
		regval_t flags = DisableInterrupts();
		size_t adjustedSize;
		OrderedList_t *entry;
		KHeapHeader_t *hdr;

		if (size < HEAP_SMALLEST) size = HEAP_SMALLEST;		    // must allocate at least 1 byte

		if (size & (BYTE_ALIGNMENT - 1)) {	                    // Check for alignment
			size += BYTE_ALIGNMENT;
			size &= ~(BYTE_ALIGNMENT - 1);
		}

		adjustedSize = size + sizeof(KHeapHeader_t) + sizeof(KHeapFooter_t);

		entry = HeapFindHole(adjustedSize, align);

		// -- are we out of memory?
		if (!entry) {
			HeapCheckHealth();
			RestoreInterrupts(flags);
			SPIN_RLS(heapLock);

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
				SPIN_RLS(heapLock);

				return 0;
			}

			HeapValidateHdr(entry->block, "HeapAlloc() after alignment");
			hdr = entry->block;
		}

		// perfect fit -OR- just a little too big
		if (hdr->size == adjustedSize || adjustedSize - hdr->size < MIN_HOLE_SIZE) {
			KHeapFooter_t *ftr;

			ftr = (KHeapFooter_t *)((byte_t *)hdr + hdr->size - sizeof(KHeapFooter_t));

			HeapReleaseEntry(entry);
			hdr->_magicUnion.isHole = 0;
			ftr->_magicUnion.isHole = 0;
			HeapValidateHdr(hdr, "Resulting Header before return (good size)");
			HeapCheckHealth();
			RestoreInterrupts(flags);
			SPIN_RLS(heapLock);

			return (void *)((byte_t *)hdr + sizeof(KHeapHeader_t));
		}

		// the only thing left is that it is too big and needs to be split
		hdr = HeapSplitAt(entry, adjustedSize);		// var entry is no longer valid after call
		HeapValidatePtr("HeapAlloc()");
		HeapValidateHdr(hdr, "Resulting Header before return (big size)");
		HeapCheckHealth();
		RestoreInterrupts(flags);
		SPIN_RLS(heapLock);

		return (void *)((byte_t *)hdr + sizeof(KHeapHeader_t));
	}
}
