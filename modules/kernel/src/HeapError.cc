//===================================================================================================================
// kernel/src/HeapError.cc -- When a Heap Error occurs, kill the kernel
// 
// When a Heap Error occurs, kill the kernel printing the problem description
//
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-09-23                          Initial Version
//  2018-05-31  Initial   0.1.0   ADCL  Copied this file from century32 (__HeapError.c)
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "console.h"
#include "heap.h"


//
// -- Panic the kernel as the result of a heap error
//    ----------------------------------------------
void HeapError(const char *from, const char *desc) 
{
    DisableInterrupts();
    ConsolePutS(from);
    ConsolePutS(desc);
    Halt();
}
