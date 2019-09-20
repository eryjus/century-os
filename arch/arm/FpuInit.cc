//===================================================================================================================
//
//  FpuInit.cc -- Initialize the core to handle FPU instructions
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Jun-16  Initial   0.4.6   ADCL  Initial version
//
//===================================================================================================================


#include "loader.h"
#include "types.h"
#include "printf.h"
#include "platform.h"
#include "serial.h"



//
// -- Initialize the core to be able to use FPU instructions
//    ------------------------------------------------------
void __ldrtext FpuInit(void)
{
    //
    // -- prepare the FPU for accepting commands
    //    --------------------------------------
    archsize_t cpacr = READ_CPACR();
    cpacr |= (0b11<<20);
    cpacr |= (0b11<<22);
    WRITE_CPACR(cpacr);

    //
    // -- and enable the fpu
    //    ------------------
    WRITE_FPEXC(1<<30);
}
