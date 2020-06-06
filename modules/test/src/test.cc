//===================================================================================================================
//
//  test.cc -- perform a user-mode test
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-18  Initial  v0.7.0a  ADCL  Initial version
//
//===================================================================================================================


#include "stdlib.h"


//
// -- this is a test
//    --------------
extern "C" void _start(void)
{
    exit(0);
}
