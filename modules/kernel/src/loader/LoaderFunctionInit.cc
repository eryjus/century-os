//===================================================================================================================
//
//  LoaderFunctionsInit.cc -- Set up the physical address for any functions called by the loader.
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-12  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "serial.h"
#include "entry.h"
#include "loader.h"


//
// -- Perform this function initialization
//    ------------------------------------
EXTERN_C EXPORT LOADER
void LoaderFunctionInit(void)
{
    FunctionPtr_t *wrk = (FunctionPtr_t *)init_start;

    while (wrk != (FunctionPtr_t *)init_end) {
        (*wrk)();                   // -- call the function
        wrk ++;
    }
}

