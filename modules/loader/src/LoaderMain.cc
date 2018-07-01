//===================================================================================================================
//
//  loader/src/LoaderMain.cc -- The main routine for the loader module
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-06-07  Initial   0.0.0   ADCL  Initial version
//
//===================================================================================================================


#include "hw-disc.h"
#include "pmm.h"
#include "serial.h"
#include "mmu.h"
#include "modules.h"
#include "fb.h"


//
// -- called from assembly language...
//    --------------------------------
extern "C" void LoaderMain(void);


//
// -- The actual loader main function
//    -------------------------------
void LoaderMain(void)
{
    SerialInit();
    HwDiscovery();

    // -- Theoretically, after this point, there should be very little architecture-dependent code
    PmmInit();
    FrameBufferInit();
    FrameBufferPutS("Welcome to Century-OS");
    MmuInit();
    ModuleInit();

    FrameBufferPutS("Initialization Complete");

    while (1) {}
}