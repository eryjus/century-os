//===================================================================================================================
// kernel/src/HeapNewListEntry.cc -- Create an OrderedList entry for the KHeapHeader pointer provided
//
// Create an OrderedList entry for the KHeapHeader pointer provided
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-07-03                          Initial version
//  2012-09-16                          Leveraged from Century
//  2013-09-12   #101                   Resolve issues splint exposes
//  2018-06-01  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//
//===================================================================================================================


#include "types.h"
#include "heap.h"


//
// -- Create a new list entry for the hole
//    ------------------------------------
OrderedList_t *HeapNewListEntry(KHeapHeader_t *hdr, bool add)
{
	int i;
	OrderedList_t *ret;
	extern OrderedList_t fixedList[ORDERED_LIST_STATIC];

	// Assume the hdr to be good; entry does not pass test
	for (i = 0; i < ORDERED_LIST_STATIC; i ++) {
		if (!fixedList[i].block) {
			ret = &fixedList[i];
			ret->block = hdr;
			ret->size = hdr->size;
			ret->next = ret->prev = 0;
			hdr->entry = ret;

			if (add) HeapAddToList(ret);

			HeapValidateHdr(hdr, "Created HeapNewListEntry()");
			return ret;
		}
	}

	HeapError("Unable to allocate a free OrderedList entry", "");
	return 0;
}
