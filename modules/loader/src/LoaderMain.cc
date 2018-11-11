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
    extern ptrsize_t cr3;

    SerialInit();
    HwDiscovery();

    // -- Theoretically, after this point, there should be very little architecture-dependent code
    PmmInit();
    FrameBufferInit();
    FrameBufferPutS("Welcome to Century-OS");
    MmuInit();
    kernEntry_t ent = ModuleInit();

    FrameBufferPutS("Initialization Complete");

    MmuSwitchPageDir(cr3);                  // also enabled paging
    SetFrameBufferAddr((uint16_t *)0xfb000000);     // re-map the frame buffer now that paging is enabled


    SerialPutS("Jumping to the kernel\n");
    kMemMove((uint8_t *)0x00003000, localHwDisc, sizeof(HardwareDiscovery_t));
    ent();
    SerialPutS("Uh-Oh!!\n");

    while (1) {}
}