//===================================================================================================================
//
//  HeapFree.cc -- Free a block back into the heap
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Free a block back into the heap
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Jul-26                          Initial version
//  2012-Sep-16                          Leveraged from Century
//  2013-Sep-01    #80                   Re-implement Mutexes (that work now) (2018-05-31: removed)
//  2013-Sep-12   #101                   Resolve issues splint exposes
//  2013-Sep-13    #74                   Rewrite Debug.h to use assertions and write to TTY_LOG
//  2018-May-31  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "spinlock.h"
#include "heap.h"


//
// -- This is the spinlock used to control single access to the heap
//    --------------------------------------------------------------
extern Spinlock_t heapLock;


//
// -- Free a block of memory back to the heap
//    ---------------------------------------
void HeapFree(void *mem)
{
    OrderedList_t *entry = 0;
    KHeapHeader_t *hdr;
    KHeapFooter_t *ftr;

    if (!mem) return;

    archsize_t flags = SPINLOCK_BLOCK_NO_INT(heapLock) {
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
        if (entry) hdr = entry->block;        // reset header if changed

        if (!entry) entry = hdr->entry;        // if nothing changes, get this entry

        hdr->_magicUnion.isHole = ftr->_magicUnion.isHole = 1;
        if (entry) HeapAddToList(entry);    // now add to the ordered list
        else (void)HeapNewListEntry(hdr, 1);

    exit:
        HeapCheckHealth();
        CLEAN_HEAP();
        SPINLOCK_RLS_RESTORE_INT(heapLock, flags);
    }
}
