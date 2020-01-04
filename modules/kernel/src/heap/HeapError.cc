//===================================================================================================================
//
//  HeapError.cc -- When a Heap Error occurs, kill the kernel
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  When a Heap Error occurs, kill the kernel printing the problem description
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-Sep-23                          Initial Version
//  2018-May-31  Initial   0.1.0   ADCL  Copied this file from century32 (__HeapError.c)
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "printf.h"
#include "heap.h"


//
// -- Panic the kernel as the result of a heap error
//    ----------------------------------------------
void HeapError(const char *from, const char *desc)
{
    DisableInterrupts();
    kprintf("Heap Error!!! %s - %s\n", from, desc);
    Halt();
}
