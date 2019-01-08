//===================================================================================================================
//
//  HeapCheckHealth.cc -- Check the health of the heap as we make changes
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Perform several sanity checks on the heap in order to verify its integrity is still good.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-May-31  Initial   0.1.0   ADCL  Initial version (copied out of century32 -- HeapDump.c)
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "heap.h"


//
// -- Execute some sanity checks on the overall heap structures
//    ---------------------------------------------------------
void HeapCheckHealth(void)
{
	KHeapHeader_t *block;
	KHeapFooter_t *ftr;

	uint32_t numBlocks = 0;
	uint32_t numAlloc = 0;
	uint32_t numFree = 0;
	uint32_t numCorrupt = 0;
	uint32_t ttlAlloc = 0;
	uint32_t ttlFree = 0;
	uint32_t largeSize = 0;

	block = (KHeapHeader_t *)kHeap->strAddr;

	// guaranteed to be at least 1 block
	do {
		ftr = (KHeapFooter_t *)((char*)block + block->size - sizeof(KHeapFooter_t));

		// count the number of blocks regardless of status
		numBlocks ++;

		// now determine if block is corrupt
		if ((block->_magicUnion.magicHole & 0xfffffffe) != HEAP_MAGIC ||
				(ftr->_magicUnion.magicHole & 0xfffffffe) != HEAP_MAGIC) {
			numCorrupt ++;
		} else if (block->_magicUnion.magicHole != ftr->_magicUnion.magicHole) {
			numCorrupt ++;
		} else if (ftr->hdr != block) {
			numCorrupt ++;
		// now check for free
		} else if (block->_magicUnion.isHole == 1) {
			if (block->entry != 0) {
				numFree ++;
				ttlFree += block->size;

				if (block->size > largeSize) {
					largeSize = block->size;
				}
			} else {
				numCorrupt ++;
			}
		// now check for alloc
		} else if (block->_magicUnion.isHole == 0) {
			if (block->entry == 0) {
				numAlloc ++;
				ttlAlloc += block->size;
			} else {
				numCorrupt ++;
			}
		}

		block = (KHeapHeader_t *)((char *)block + block->size);
	} while ((byte_t *)block < kHeap->endAddr);

	if (!numCorrupt) return;
	else while (1);
}
