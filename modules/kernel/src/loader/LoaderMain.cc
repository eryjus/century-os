//===================================================================================================================
//
//  LoaderMain.cc -- The main routine for the loader module
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Jun-07  Initial   0.0.0   ADCL  Initial version
//  2018-Nov-11  Initial   0.2.0   ADCL  Update the architecture abstraction for rpi2b
//  2019-Feb-10  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "hw-disc.h"
#include "pmm.h"
#include "serial.h"
#include "mmu.h"
#include "cpu.h"
#include "heap.h"
#include "fb.h"
#include "platform.h"
#include "entry.h"
#include "loader.h"


//
// -- The actual loader main function
//    -------------------------------
EXTERN_C EXPORT LOADER NORETURN
void LoaderMain(archsize_t arg0, archsize_t arg1, archsize_t arg2)
{
    LoaderFunctionInit();               // go and initialize all the function locations
    MmuInit();                          // Complete the MMU initialization for the loader
    kprintf("Welcome\n");
    PlatformEarlyInit();

    FrameBufferInit();
    HeapInit();
    PmmInit();
    PlatformInit();

    if (!assert_msg(stabEnd >= (4 * 1024 * 1024),
            "The kernel is bigger than 4MB; time to add more page mappings!\n")) {
        Halt();
    }


    // -- Theoretically, after this point, there should be very little architecture-dependent code
    JumpKernel(kInit, STACK_LOCATION);


    // -- if we ever get here, we have some big problems!
    assert_msg(false, "Returned from kInit() back to LoaderMain()!!!");
    while (1) {}
}

