//===================================================================================================================
//
//  StackFind.cc -- Find an available stack for use
//
//        Copyright (c)  2017-2020 -- Adam Clark
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
#include "spinlock.h"
#include "printf.h"
#include "stacks.h"


//
// -- Find an available stack and return its base address
//    ---------------------------------------------------
EXPORT KERNEL
archsize_t StackFind(void)
{
    archsize_t rv = 0;
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(stackBitmapLock) {
        for (int i = 0; i < STACK_COUNT; i ++) {
            if (stacks[i] != (archsize_t)-1) {
                for (int j = 0; j < 32; j ++) {
                    if ((stacks[i] & (1 << j)) == 0) {
                        rv = STACK_LOCATION + (STACK_SIZE * ((i * 32) + j));
                        StackDoAlloc(rv);
                        goto exit;
                    }
                }
            }
        }

exit:
        SPINLOCK_RLS_RESTORE_INT(stackBitmapLock, flags);
    }

    return rv;
}
