//===================================================================================================================
//
//  ElfCreateImg.cc -- Create an img structure and initialize it, returning the structure from the heap
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-20  Initial  v0.7.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "heap.h"
#include "elf.h"


//
// -- Create an initialize but unpopulated ElfImage_t structure
//    ---------------------------------------------------------
EXTERN_C EXPORT KERNEL
ElfImage_t *ElfCreateImg(size_t frameCount)
{
    ElfImage_t *rv = (ElfImage_t *)HeapAlloc(sizeof(size_t) + (frameCount * sizeof(frame_t)), false);

    rv->frameCount = 0;
    for (size_t i = 0; i < frameCount; i ++) rv->frame[i] = 0;

    return rv;
}
