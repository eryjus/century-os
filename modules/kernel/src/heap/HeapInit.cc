//===================================================================================================================
//
//  HeapInit.cc -- Create and initialize the internal heap structures
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Create and initialize the internal Century heap structures.
//
//  Please note that we are allocating a starting block of memory statically.  This block is called
//  `heapMemoryBlock`.  The loader will have allocated frames for it but this heap is not located in the right area
//  of virtual address.  So,  part of the responsibility of this initialization step is to unmap these from the
//  kernel binary and then remap them into the Heap virtual address space at 0xd0000000.  By doing this, the
//  kernel should be able to get enough heap operational to begin to send messages, and then add more then the
//  PMM is operational.
//
//  Now, since I am moving the heap from the end of the kernel (which is how this was originally written), to a
//  standalone block of virtual address space, there are some chages that will need to be made.  Fortunately, the
//  design allows for this change relatively easily.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Jun-30                          Initial version
//  2012-Sep-16                          Leveraged from Century
//  2013-Sep-12   #101                   Resolve issues splint exposes
//  2018-Jun-01  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//  2018-Nov-10  Initial   0.1.0   ADCL  Move the heap memory into its own dedicate virtual address space
//
//===================================================================================================================


#include "types.h"
#include "serial.h"
#include "printf.h"
#include "mmu-kernel.h"
#include "heap.h"


//
// -- This is how much of the heap we will allocate at compile time.  This will really be frames that will be moved
//    during initialization
//    -------------------------------------------------------------------------------------------------------------
#define INITIAL_HEAP		(4096*16)


//
// -- We will fake an allocation in the bss that will be remapped into the heap space
//    -------------------------------------------------------------------------------
char __attribute__((aligned(4096))) heapMemoryBlock[INITIAL_HEAP];


//
// -- some local and global variables
//    -------------------------------
ptrsize_t heapStart = 0xd0000000;				// this is the start in virtual address space
OrderedList_t fixedList[ORDERED_LIST_STATIC];
bool fixedListUsed = 0;
static KHeap_t _heap;
KHeap_t *kHeap = &_heap;


//
// -- Initialize the heap structures
//    ------------------------------
void HeapInit(void)
{
	// -- The first order of business here is to move the frames from `heapMemoryBlock` to 0xd0000000
	ptrsize_t vAddr = (ptrsize_t)heapMemoryBlock;
	ptrsize_t vLimit = vAddr + INITIAL_HEAP;
	ptrsize_t nAddr = heapStart;		// This is the new location

	for ( ; vAddr < vLimit; nAddr += 0x1000, vAddr += 0x1000) {
		frame_t frame = MmuUnmapPage(vAddr);
		MmuMapToFrame(nAddr, frame, PG_KRN | PG_WRT);
	}

	// -- Set up the heap structure and list of open blocks
	KHeapFooter_t *tmpFtr;

	kMemSetB(fixedList, 0, sizeof(fixedList));

	// -- Build the first free block which is all allocated
	fixedList[0].block = (KHeapHeader_t *)heapStart;
	fixedList[0].next = 0;
	fixedList[0].prev = 0;
	fixedList[0].size = INITIAL_HEAP;

	_heap.strAddr = (byte_t *)heapStart;
	_heap.endAddr = ((byte_t *)_heap.strAddr) + fixedList[0].size;
	_heap.maxAddr = (byte_t *)0xfb000000;

	_heap.heapMemory = _heap.heap512 = _heap.heap1K =
			_heap.heap4K = _heap.heap16K = &fixedList[0];

	fixedList[0].block->_magicUnion.magicHole = HEAP_MAGIC;
	fixedList[0].block->_magicUnion.isHole = 1;
	fixedList[0].block->size = fixedList[0].size;
	fixedList[0].block->entry = &fixedList[0];

	tmpFtr = (KHeapFooter_t *)(((char *)fixedList[0].block) +
			fixedList[0].size - sizeof(KHeapFooter_t));
	tmpFtr->_magicUnion.magicHole = fixedList[0].block->_magicUnion.magicHole;
	tmpFtr->hdr = fixedList[0].block;

	fixedListUsed = 1;
	kHeap = &_heap;

	kprintf("Heap Created\n");
	kprintf("  Heap Start Location: %p\n", kHeap->strAddr);
	kprintf("  Current Heap Size..: %p\n", fixedList[0].size);
	kprintf("  Heap End Location..: %p\n", kHeap->endAddr);
}
