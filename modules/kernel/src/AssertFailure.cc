//===================================================================================================================
//
//  AssertFailure.cc -- Handle outputting that an assertion failed
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Nov-29  Initial  0.4.6b   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "printf.h"


//
// -- Handle outputting that an assertion failed
//    ------------------------------------------
EXPORT KERNEL
bool AssertFailure(const char *expr, const char *msg, const char *file, int line)
{
    kprintf("\n!!! ASSERT FAILURE !!!\n%s(%d) %s %s\n\n", file, line, expr, (msg?msg:""));

    // -- always return false in case this is used in a conditional
    return false;
}

