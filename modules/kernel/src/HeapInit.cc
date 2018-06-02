//===================================================================================================================
// kernel/src/HeapInit.cc -- Create and initialize the internal heap structures
// 
// Create and initialize the internal Century heap structures.
//
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-06-30                          Initial version
//  2012-09-16                          Leveraged from Century
//  2013-09-12   #101                   Resolve issues splint exposes
//  2018-06-01  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//
//===================================================================================================================


#include "types.h"
#include "console.h"
#include "heap.h"


extern size_t mbiMaxMem;
extern ptrsize_t end;					// the linker will provide; see linker.ld

//
// -- some local and global variables
//    -------------------------------
ptrsize_t heapStart = (ptrsize_t)(&end);
OrderedList fixedList[ORDERED_LIST_STATIC];
bool fixedListUsed = 0;
KHeap *kHeap = 0;

static KHeap _heap;


//
// -- Initialize the heap structures
void HeapInit(void)
{
	KHeapFooter *tmpFtr;

	if (heapStart & 0x00000fff) {
		heapStart = (heapStart & 0xfffff000) + 0x1000;	// ensure page alignment
	}
	
	kMemSetB(fixedList, 0, sizeof(fixedList));
	
	fixedList[0].block = (KHeapHeader *)heapStart;
	fixedList[0].next = 0;
	fixedList[0].prev = 0;
	fixedList[0].size = ((heapStart & 0xfffff000) - heapStart) + HEAP_MIN_SIZE; //! align to 4K
	
	_heap.strAddr = (char *)heapStart;
	_heap.endAddr = ((char *)_heap.strAddr) + fixedList[0].size;
	_heap.maxAddr = (char *)0xc0000000;
	
	_heap.heapMemory = _heap.heap512 = _heap.heap1K = 
			_heap.heap4K = _heap.heap16K = &fixedList[0];

	fixedList[0].block->_magicUnion.magicHole = HEAP_MAGIC;
	fixedList[0].block->_magicUnion.isHole = 1;
	fixedList[0].block->size = fixedList[0].size;
	fixedList[0].block->entry = &fixedList[0];
	
	tmpFtr = (KHeapFooter *)(((char *)fixedList[0].block) + 
			fixedList[0].size - sizeof(KHeapFooter));
	tmpFtr->_magicUnion.magicHole = fixedList[0].block->_magicUnion.magicHole;
	tmpFtr->hdr = fixedList[0].block;
	
	fixedListUsed = 1;
	kHeap = &_heap;
	
	ConsolePutS("Heap Created\n");
//	kprintf("  Heap Start Location: %lx\n", kHeap->strAddr);
//	kprintf("  Current Heap Size: %lx\n", fixedList[0].size);
//	kprintf("  Heap End Location: %lx\n", kHeap->endAddr);
//	kprintf("  Paging is %s\n", CheckPaging() ? "Enabled" : "Disabled");
}
