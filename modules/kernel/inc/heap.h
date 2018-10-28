//===================================================================================================================
//
// inc/heap.h -- Kernel Heap structures and functions
//
// This files contains the structures and definitions needed to manage and control the heap in Century.
//
// The basis for the design is lifted from Century32 (a 32-bit Hobby OS).
//
// There are several structures that are used and maintained with the heap management.  The heap structure itself is
// nothing more than a doubly linked list of free blocks of memory.  This linked list is also ordered based on the
// size of the free block of memory.  Pointers are setup in the heap structure to point to blocks of certain sizes
// in an attempt to speed up the allocation and deallocation process.  These pointers are at:
// * the beginning of the heap (of course)
// * >= 512 bytes
// * >= 1K bytes
// * >= 4K bytes
// * >= 16K bytes
//
// When a block of memory is requested, the size if first increased to cover the size of the header and footer as
// well as adjusted up to the allocation alignment.  So, if 1 byte is requested (unlikely, but great for
// illustration purposes), the size is increased to HEAP_SMALLEST and then the size of the header (KHeapHdr_size),
// the size of the footer (KHeapFtr_size), and then aligned to the next 8 byte boundary up.
//
// Free blocks are maintained in the heap structure as an ordered list by size, from smallest to biggest.  In
// addition, when the ordered list is searched for the "best fit" (that is the class of algorithm used here), if
// the adjusted request is >= 16K, then the search starts at the 16K pointer; >= 4K but < 16K, then the search starts
// at the 4K pointer; >= 1K but < 4K, then the search starts at the 1K pointer; >= 512 bytes but < 1K, then the
// search starts at the 512 bytes pointer; and, all other searches < 512 bytes are stated at the beginning.
//
// Note that if there are no memory blocks < 512 bytes, but blocks >= 512 bytes, then the beginning of the ordered
// list will point to the first block no matter the size.  The rationale for this is simple: a larger block can
// always be split to fulfill a request.
//
// On the other hand, if there are no blocks >= 16K bytes is size, then the >= 16K pointer will be NULL.  Again,
// the rationale is simple: we cannot add up blocks to make a 16K block, so other measures need to be taken (create
// more heap memory or return failure).
//
// Finally, the dedicated ordered list array is going to be eliminated in this implementation.  Instead it will be
// included as part of the header structure.  This change will allow for more than a fixed number of free blocks.
// This should also simplify the implementation as well.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-06-30                          Initial version
//  2012-09-16                          Leveraged from Century
//  2013-09-12   #101                   Resolve issues splint exposes
//  2018-05-30  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//
//===================================================================================================================


#ifndef __HEAP_H__
#define __HEAP_H__

#include "types.h"
#include "cpu.h"


//
// -- Define some quick macros to help with managing the heap
//    -------------------------------------------------------
#define HEAP_SMALLEST           32
#define HEAP_MAGIC				((uint32_t)0xBAB6BADC)
#define HEAP_CHECK(x)			(((x) & 0xfffffffe) == HEAP_MAGIC)
#define MIN_HOLE_SIZE			(sizeof(KHeapHeader) + sizeof(KHeapHeader) + HEAP_SMALLEST)

#define HEAP_MIN_SIZE	        0x00010000
#define ORDERED_LIST_STATIC		(1024)


//
// -- forward declare the OrderedList structure
//    -----------------------------------------
struct OrderedList;


//
// -- This is the heap block header, used to manage a block of memory in the heap
//    ---------------------------------------------------------------------------
typedef struct KHeapHeader {
	union {
		struct {
			uint32_t isHole : 1;	            // == 1 if this is a hole (not used)
			uint32_t magic : 31;	            // magic number (BAB6 BADC when bit 0 is forced to 0)
		};
		uint32_t magicHole;		                // this is the aggregate of the bit fields
	} _magicUnion;

	struct OrderedList *entry;	                // pointer to the OrderedList entry if hole; NULL if allocated
	size_t size;				                // this size includes the size of the header and footer
} __attribute__ ((packed)) KHeapHeader;


//
// -- This is the beap block footer, used in conjunction with the heap header to makage the heap memory
//    -------------------------------------------------------------------------------------------------
typedef struct KHeapFooter {
	union {
		struct {
			uint32_t isHole : 1;	            // the field is the header is the one used
			uint32_t magic : 31;	            // magic number (0xBAB6_BADC when bit 0 is forced to 0)
		};
		uint32_t magicHole;		                // this is the aggregate of the bit fields
	} _magicUnion;

	KHeapHeader *hdr;			                // pointer back to the header
} __attribute__ ((packed)) KHeapFooter;


//
// -- This is a compare function prototype declaration for ordering blocks
//    --------------------------------------------------------------------
typedef int (*cmpFunc)(KHeapHeader *, KHeapHeader *);


//
// -- The heap is implemented as an ordered list for a bet-fit implementation
//    -----------------------------------------------------------------------
typedef struct OrderedList {
	KHeapHeader *block;			                // pointer to the block of heap memory
	size_t size;				                // the size of the memory pointed to
	struct OrderedList *prev;	                // pointer to the previous entry
	struct OrderedList *next;	                // pointer to the next entry
} OrderedList;


//
// -- This is the heap control structure, maintianing the heap integrity
//    ------------------------------------------------------------------
typedef struct KHeap {
	OrderedList *heapMemory;	                // the start of all heap memory lists < 512 bytes
	OrderedList *heap512;		                // the start of heap memory >= 512 bytes
	OrderedList *heap1K;		                // the start of heap memory >= 1K bytes
	OrderedList *heap4K;		                // the start of heap memory >= 4K bytes
	OrderedList *heap16K;		                // the start of heap memory >= 16K bytes
	char *strAddr;				                // the start address of the heap
	char *endAddr;				                // the ending address of the heap
	char *maxAddr;				                // the max address to which the heap can grow
} KHeap;


//
// -- Global variables
//    ----------------
extern KHeap *kHeap;

//void InitHeap(void);
//uint32 CheckPaging(void);

void HeapAddToList(OrderedList *entry);
OrderedList *HeapAlignToPage(OrderedList *entry);
void *HeapAlloc(size_t size, bool align);
ptrsize_t HeapCalcPageAdjustment(OrderedList *entry);
void HeapCheckHealth(void);
void HeapError(const char *from, const char *desc);
OrderedList *HeapFindHole(size_t adjustedSize, bool align);
void HeapFree(void *mem);
void HeapInit(void);
OrderedList *HeapNewListEntry(KHeapHeader *hdr, bool add);
OrderedList *HeapMergeLeft(KHeapHeader *hdr);
OrderedList *HeapMergeRight(KHeapHeader *hdr);
void HeapReleaseEntry(OrderedList *entry);
void HeapRemoveFromList(OrderedList *entry);
KHeapHeader *HeapSplitAt(OrderedList *entry, size_t adjustToSize);
void HeapValidateHdr(KHeapHeader *hdr, const char *from);
void HeapValidatePtr(const char *from);


//
// -- A quick macro to make coding easier and more readable
//    -----------------------------------------------------
#define NEW(tp) (tp *)HeapAlloc(sizeof(tp), false)

#endif
