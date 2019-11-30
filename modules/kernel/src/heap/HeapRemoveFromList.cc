//===================================================================================================================
//
//  HeapRemoveFromList.cc -- Remove an ordered list entry from the list
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Remove an ordered list entry from the list.
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
//  |  prev  -- zero                                                  |
//  |  next  -- zero                                                  |
//  +-----------------------------------------------------------------+
//  |  block->entry -- unchanged (an important fact to remember)      |
//  +-----------------------------------------------------------------+
//  |  hKeap->heapMemory -- may be NULL on return if last entry is    |
//  |                       removed                                   |
//  |  hKeap->heap512    -- may be NULL on return if last entry is    |
//  |                       removed or nothing is >= 512 bytes        |
//  |  hKeap->heap4K     -- may be NULL on return if last entry is    |
//  |                       removed or nothing is >= 4096 bytes       |
//  |  hKeap->heap16K    -- may be NULL on return if last entry is    |
//  |                       removed or nothing is >= 16384 bytes      |
//  +-----------------------------------------------------------------+
//
//  if on entry, entry->next != null, then entry->next->prev is set to
//  entry->prev.
//
//  if on entry, entry->prev != null, then entry->prev->next is set to
//  entry->next.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Jul-12                          Initial version
//  2012-Sep-19                          Leveraged from Century
//  2018-Jun-01  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "heap.h"


//
// -- Remove an entry from the Ordered List
//    -------------------------------------
void HeapRemoveFromList(OrderedList_t *entry)
{
    if (!assert(entry != NULL)) HeapError("NULL entry in HeapRemoveFromList()", "");
    HeapValidateHdr(entry->block, "HeapRemoveFromList()");

    if (kHeap->heapMemory == entry) {
        kHeap->heapMemory = kHeap->heapMemory->next;
    }

    if (kHeap->heap512 == entry) {
        kHeap->heap512 = kHeap->heap512->next;
    }

    if (kHeap->heap1K == entry) {
        kHeap->heap1K = kHeap->heap1K->next;
    }

    if (kHeap->heap4K == entry) {
        kHeap->heap4K = kHeap->heap4K->next;
    }

    if (kHeap->heap16K == entry) {
        kHeap->heap16K = kHeap->heap16K->next;
    }

    if (entry->next) entry->next->prev = entry->prev;
    if (entry->prev) entry->prev->next = entry->next;

    entry->next = entry->prev = 0;
}
