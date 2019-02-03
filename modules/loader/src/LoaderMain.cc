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
//
//===================================================================================================================


#include "hw-disc.h"
#include "pmm.h"
#include "serial-loader.h"
#include "mmu-loader.h"
#include "modules.h"
#include "cpu.h"
#include "fb.h"


//
// -- called from assembly language...
//    --------------------------------
extern "C" void LoaderMain(void);
extern "C" void JumpKernel(ptrsize_t addr) __attribute__((noreturn));


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
    ptrsize_t ent = ModuleInit();

    SerialPutS("Initialization Complete\n");
    FrameBufferPutS("Initialization Complete\n");

    SetMmuTopAddr();                                            // also enabled paging
    SetFrameBufferAddr((uint16_t *)FRAME_BUFFER_VADDR);         // re-map the frame buffer now that paging is enabled

    uint32_t *e = (uint32_t *)ent;
    SerialPutS("The instructions at that location are: \n");
    SerialPutS(".. "); SerialPutHex(e[0]); SerialPutChar('\n');
    SerialPutS(".. "); SerialPutHex(e[1]); SerialPutChar('\n');
    SerialPutS(".. "); SerialPutHex(e[2]); SerialPutChar('\n');
    SerialPutS(".. "); SerialPutHex(e[3]); SerialPutChar('\n');


    kMemMove((uint8_t *)HW_DISCOVERY_LOC, localHwDisc, sizeof(HardwareDiscovery_t));
    SerialPutS("Jumping to the kernel, located at address "); SerialPutHex((uint32_t)ent); SerialPutChar('\n');
    JumpKernel(ent);
}