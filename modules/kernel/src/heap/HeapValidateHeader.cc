//===================================================================================================================
//
//  HeapValidateHdr.cc -- Validate heap header
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Validate the heap header, heap footer, and if is a hole the ordered list entry.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Jul-02                          Initial version
//  2012-Sep-16                          Leveraged from Century
//  2012-Sep-23                          Removed DUMP() define
//  2013-Sep-12   #101                   Resolve issues splint exposes
//  2018-May-31  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "heap.h"


//
// -- Validate a heap header block to ensure it has not been overrun
//    --------------------------------------------------------------
void HeapValidateHdr(KHeapHeader_t *hdr, const char *from)
{
	KHeapFooter_t *ftr;

	if (!hdr) {
		HeapError(from, "Unable to validate NULL header");
	}

	ftr = (KHeapFooter_t *)((char *)hdr + hdr->size - sizeof(KHeapFooter_t));

	if ((hdr->_magicUnion.magicHole & 0xfffffffe) != HEAP_MAGIC) {
		HeapError(from, "Invalid Heap Header Magic Number");
	}

	if ((ftr->_magicUnion.magicHole & 0xfffffffe) != HEAP_MAGIC) {
		HeapError(from, "Invalid Heap Footer Magic Number");
	}

	if (hdr->_magicUnion.magicHole != ftr->_magicUnion.magicHole) {
		HeapError(from, "Header/Footer Magic Number/Hole mismatch");
	}

	if (hdr->_magicUnion.isHole == 1 && hdr->entry == 0) {
		HeapError(from, "Heap hole has no ordered list entry");
	}

	if (hdr->_magicUnion.isHole == 0 && hdr->entry != 0) {
		HeapError(from, "Heap allocated block has an ordered list entry");
	}

	if (hdr->entry && hdr->entry->block != hdr) {
		HeapError(from, "Entry does not point to this header");
	}

	if (hdr->entry && hdr->entry->size != hdr->size) {
		HeapError(from, "Header/Entry size mismatch");
	}
}
