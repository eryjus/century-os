//===================================================================================================================
//
//  StackAlloc.cc -- Allocate the stack associated with the address handed in
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Dec-01  Initial   0.4.6d  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"
#include "stacks.h"


//
// -- allocate a stack by setting the proper bit
//    ------------------------------------------
EXPORT KERNEL
void StackDoAlloc(archsize_t stackBase)
{
    if (!assert(stackBase >= STACK_LOCATION)) return;
    if (!assert(stackBase < STACK_LOCATION + (4 * 1024 * 1024))) return;

    stackBase &= ~(STACK_SIZE - 1);           // align to stack
    stackBase -= STACK_LOCATION;
    stackBase /= STACK_SIZE;

    int index = stackBase / 32;
    int bit = stackBase % 32;

    stacks[index] |= (1 << bit);
}

