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
#include "loader.h"
#include "hw-disc.h"
#include "pmm.h"
#include "serial.h"
#include "mmu.h"
#include "cpu.h"
#include "heap.h"
#include "fb.h"
#include "platform.h"


//
// -- called from assembly language...
//    --------------------------------
extern "C" EXPORT LOADER
void JumpKernel(void (*addr)(), archsize_t stack) __attribute__((noreturn));

extern "C" EXPORT LOADER
void SerialEarlyPutChar(uint8_t);

extern "C" EXPORT KERNEL
void kInit(void);


//
// -- The actual loader main function
//    -------------------------------
EXPORT LOADER
void LoaderMain(archsize_t arg0, archsize_t arg1, archsize_t arg2)
{
    LoaderFunctionInit();               // go and initialize all the function locations
    EarlyInit();
    FrameBufferInit();
    MmuInit();                          // after this call, all kernel memory can be accessed
    HeapInit();
    PmmInit();
    PlatformInit();


    // -- Theoretically, after this point, there should be very little architecture-dependent code
    FrameBufferClear();
    FrameBufferPutS("Welcome to Century-OS\n");
    kprintf("Jumping to %p (%smapped) with stack %p (%smapped)\n", kInit, (MmuIsMapped((archsize_t)kInit)?"":"not "),
            STACK_LOCATION, (MmuIsMapped(STACK_LOCATION)?"":"not "));
    JumpKernel(kInit, STACK_LOCATION + STACK_SIZE);

    // -- if we ever get here, we have some big problems!
    assert_msg(false, "Returned from kInit() back to LoaderMain()!!!");
    while (1) {}
}

