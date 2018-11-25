//===================================================================================================================
//
//  LoaderMain.cc -- The main routine for the loader module
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-Jun-07  Initial   0.0.0   ADCL  Initial version
//  2018-Nov-11  Initial   0.2.0   ADCL  Update the architecture abstraction for rpi2b
//
//===================================================================================================================


#include "hw-disc.h"
#include "pmm.h"
#include "serial.h"
#include "mmu-loader.h"
#include "modules.h"
#include "cpu.h"
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
    FrameBufferPutS("Welcome to Century-OS\n");
    MmuInit();
    kernEntry_t ent = ModuleInit();

    FrameBufferPutS("Initialization Complete");

    SetMmuTopAddr();                                            // also enabled paging
    SetFrameBufferAddr((uint16_t *)FRAME_BUFFER_ADDRESS);       // re-map the frame buffer now that paging is enabled


    kMemMove((uint8_t *)HW_DISCOVERY_LOC, localHwDisc, sizeof(HardwareDiscovery_t));
    SerialPutS("Jumping to the kernel\n");
    ent();

    // -- We should never get to this place
    SerialPutS("Uh-Oh!!\n");

    while (1) {}
}