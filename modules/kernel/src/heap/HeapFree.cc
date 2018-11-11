//===================================================================================================================
// kernel/src/HeapFree.cc -- Free a block back into the heap
//
// Free a block back into the heap
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-07-26                          Initial version
//  2012-09-16                          Leveraged from Century
//  2013-09-01    #80                   Re-implement Mutexes (that work now) (2018-05-31: removed)
//  2013-09-12   #101                   Resolve issues splint exposes
//  2013-09-13    #74                   Rewrite Debug.h to use assertions and write to TTY_LOG
//  2018-05-31  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "spinlock.h"
#include "heap.h"


extern Spinlock_t heapLock;


//
// -- Free a block of memory back to the heap
//    ---------------------------------------
void HeapFree(void *mem)
{
	OrderedList_t *entry = 0;
	KHeapHeader_t *hdr;
	KHeapFooter_t *ftr;
	regval_t flags;

	if (!mem) return;

	SPIN_BLOCK(heapLock) {
		flags = DisableInterrupts();

		hdr = (KHeapHeader_t *)((byte_t *)mem - sizeof(KHeapHeader_t));
		ftr = (KHeapFooter_t *)((byte_t *)hdr + hdr->size - sizeof(KHeapFooter_t));
		HeapValidateHdr(hdr, "Heap structures have been overrun by data!!");

		HeapCheckHealth();

		if (hdr->_magicUnion.isHole) goto exit;
		if (hdr->_magicUnion.magicHole != HEAP_MAGIC || ftr->_magicUnion.magicHole != HEAP_MAGIC) goto exit;
		if (ftr->hdr != hdr) goto exit;

		HeapCheckHealth();
		entry = HeapMergeRight(hdr);
		HeapCheckHealth();

		entry = HeapMergeLeft(hdr);
		HeapCheckHealth();
		if (entry) hdr = entry->block;		// reset header if changed

		if (!entry) entry = hdr->entry;		// if nothing changes, get this entry

		hdr->_magicUnion.isHole = ftr->_magicUnion.isHole = 1;
		if (entry) HeapAddToList(entry);	// now add to the ordered list
		else (void)HeapNewListEntry(hdr, 1);

	exit:
		HeapCheckHealth();
		RestoreInterrupts(flags);
		SPIN_RLS(heapLock);
	}
}
