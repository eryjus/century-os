//===================================================================================================================
//
//  HeapReleaseEntry.cc -- Release an OrderedList Entry and put it back in the pool
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Release an OrderedList Entry and put it back in the pool of available entries to use.
//
//  On calling, the contents of entry have the following characteristics:
//  +-----------------------------------------------------------------+
//  | Entry                                                           |
//  +-----------------------------------------------------------------+
//  |  block -- points to proper header                               |
//  |  size  -- the size of the block, with header/footer             |
//  |  prev  -- points to the previous (smaller) block (may be null)  |
//  |  next  -- points to the next (larger) block (may be null)       |
//  +-----------------------------------------------------------------+
//  |  block->entry -- is equal to the parm entry                     |
//  +-----------------------------------------------------------------+
//
//  On exit, the following characteristics:
//  +-----------------------------------------------------------------+
//  | Entry                                                           |
//  +-----------------------------------------------------------------+
//  |  block -- zero                                                  |
//  |  size  -- zero                                                  |
//  |  prev  -- zero [through RemoveFromList()]                       |
//  |  next  -- zero [through RemoveFromList()]                       |
//  +-----------------------------------------------------------------+
//  |  block->entry -- zero                                           |
//  +-----------------------------------------------------------------+
//
//  if on entry, entry->next != null, then entry->next->prev is set to entry->prev.
//
//  if on entry, entry->prev != null, then entry->prev->next is set to entry->next.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Jul-03                          Initial version
//  2012-Sep-16                          Leveraged from Century
//  2018-Jun-01  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//
//===================================================================================================================


#include "types.h"
#include "heap.h"


//
// -- Release an entry from the ordered list
//    --------------------------------------
void HeapReleaseEntry(OrderedList_t *entry)
{
	if (!entry) HeapError("NULL entry in HeapReleaseEntry()", "");
	HeapValidateHdr(entry->block, "HeapReleaseEntry()");

	// verify removed from list and remove if necessary
	if (entry->next || entry->prev || entry->block->entry) {
		HeapRemoveFromList(entry);
	}

	// clear out the data
	entry->block->entry = 0;
	entry->block = 0;
	entry->size = 0;
}