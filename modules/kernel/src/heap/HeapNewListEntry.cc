//===================================================================================================================
//
//  HeapNewListEntry.cc -- Create an OrderedList entry for the KHeapHeader pointer provided
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Create an OrderedList entry for the KHeapHeader pointer provided
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Jul-03                          Initial version
//  2012-Sep-16                          Leveraged from Century
//  2013-Sep-12   #101                   Resolve issues splint exposes
//  2018-Jun-01  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
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
