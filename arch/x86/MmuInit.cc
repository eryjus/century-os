//===================================================================================================================
//
//  MmuInit.cc -- Complete the  MMU initialization for the x86 architecture
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  The goal of this function is to make sure that MMU is ready to run all the kernel functions as well as the
//  loader ones.  We have already mapped the lower 4MB of memory and we should be able to use plenty of PMM frames
//  to get the initialization complete.
//
//  One thing I will want to watch for is that I am encroaching on the kernel.  It is a possibility and so I want
//  to build this check into the function so that I can panic the kernel if I encroach on the kernel data.  This
//  will be done by comapring to `_kernelEnd`.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-13  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "serial.h"
#include "mmu.h"
#include "entry.h"
#include "printf.h"
#include "loader.h"


//
// -- Complete the initialization of the Mmu for the loader to function properly
//    --------------------------------------------------------------------------
EXTERN_C EXPORT LOADER
void MmuInit(void)
{
    //
    // -- Next up is the IVT -- which needs to be mapped.  This one is rather trivial.
    //    ----------------------------------------------------------------------------
    MmuMapToFrame(EXCEPT_VECTOR_TABLE, intTableAddr, PG_KRN | PG_WRT);
}

