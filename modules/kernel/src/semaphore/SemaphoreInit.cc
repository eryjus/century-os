//===================================================================================================================
//
//  SemaphoreInit.cc -- Initialize the internals of the Semaphores
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Apr-30  Initial   0.4.3   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "heap.h"
#include "cpu.h"
#include "semaphore.h"


//
// -- Initialize the Semaphore Set list
//    ---------------------------------
void __ldrtext SemaphoreInit(void)
{
    //
    // -- initialize the list
    //    -------------------
    ListInit(&semaphoreAll.undoList.list);


    //
    // -- Allocate an array from the kernel heap and initialize it to NULL
    //    ----------------------------------------------------------------
    semaphoreAll.semaphoreSets = (SemaphoreSet_t **)HeapAlloc(semmni * sizeof(SemaphoreSet_t *), false);
    kMemSetB(semaphoreAll.semaphoreSets, 0, semmni * sizeof(SemaphoreSet_t *));
}

