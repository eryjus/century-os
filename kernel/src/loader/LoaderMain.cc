//===================================================================================================================
//
//  LoaderMain.cc -- The main routine for the loader module
//
//        Copyright (c)  2017-2019 -- Adam Clark
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


#include "hw-disc.h"
#include "pmm.h"
#include "serial.h"
#include "mmu.h"
#include "cpu.h"
#include "fb.h"
#include "loader.h"


//
// -- called from assembly language...
//    --------------------------------
__CFUNC void JumpKernel(void (*addr)(), archsize_t stack) __attribute__((noreturn));


//
// -- The actual loader main function
//    -------------------------------
__CFUNC void __ldrtext LoaderMain(archsize_t arg0, archsize_t arg1, archsize_t arg2)
{
    extern void kInit(void);

    LoaderFunctionInit();               // go and initialize all the function locations
    EarlyInit();

    HwDiscovery();

    FrameBufferInit();
    PmmInit();
    MmuInit();

    // -- Theoretically, after this point, there should be very little architecture-dependent code
    FrameBufferClear();
    FrameBufferPutS("Welcome to Century-OS\n");

    SerialPutS("Initialization Complete\n");
    FrameBufferPutS("Initialization Complete\n");

    kprintf("Jumping to the kernel, located at address %p\n", kInit);
    JumpKernel(kInit, STACK_LOCATION + STACK_SIZE);

    while (1) {}
}