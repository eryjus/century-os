//===================================================================================================================
//
//  printf.h -- A printf()-like function to write output to the serial port
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-11  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#pragma once

#include "types.h"


//
// -- used to "turn on" kprintf() output
//    ----------------------------------
EXTERN EXPORT KERNEL_DATA bool kPrintfEnabled;



//
// -- This function operates like printf()
//    ------------------------------------
EXTERN_C EXPORT KERNEL
int kprintf(const char *fmt, ...);


