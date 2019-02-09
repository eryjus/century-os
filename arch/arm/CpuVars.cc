//===================================================================================================================
//
//  CpuVars.cc -- Various variables used by the rpi2b CPU
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-30  Initial   0.2.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "cpu.h"

//
// -- This is the stack for the exception handlers
//    --------------------------------------------
byte_t exceptionStack[EXCEPTION_STACK_SIZE];


