//===================================================================================================================
//
//  LoaderFunctionsInit.cc -- Set up the physical address for any functions called by the loader.
//
//        Copyright (c)  2017-2019 -- Adam Clark
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


#include "loader.h"
#include "types.h"
#include "cpu.h"
#include "serial.h"


//
// -- This is an array of functions that need to be called right away to initialize the data
//    --------------------------------------------------------------------------------------
typedef void (*FunctionPtr_t)(void);


//
// -- These 2 addresses bound the array
//    ---------------------------------
extern FunctionPtr_t const init_start[], init_end[];

//
// -- Perform this function initialization
//    ------------------------------------
void __ldrtext LoaderFunctionInit(void)
{
    FunctionPtr_t *wrk = (FunctionPtr_t *)init_start;

    while (wrk != (FunctionPtr_t *)init_end) {
        (*wrk)();                   // -- call the function
        wrk ++;
    }

    lMemSetB = (kMemSetB_t)PHYS_OF(kMemSetB);
}

