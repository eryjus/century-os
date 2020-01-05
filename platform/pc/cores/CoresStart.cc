//===================================================================================================================
//
//  CoresStart.cc -- Start the cores for the x86-pc
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Jan-04  Initial  v0.5.0d  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "timer.h"
#include "hardware.h"
#include "process.h"
#include "printf.h"
#include "pmm.h"
#include "pic.h"


//
// -- an definition for the entry point for the cores
//    -----------------------------------------------
EXTERN_C EXPORT KERNEL
void entryAp(void);


//
// -- start the other cores; remains in the kernel since we may want to do this later as well
//    ---------------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void CoresStart(void)
{
    //
    // -- Load the trampoline code into the low 1MB of memory
    //    ---------------------------------------------------
    uint8_t *trampoline = (uint8_t *)0x8000;        // for S&G, start at 32K
    extern uint32_t intTableAddr;
    extern uint8_t _smpStart[];
    extern uint8_t _smpEnd[];

    kprintf("Copying the AP entry code to %p\n", trampoline);
    kprintf("... start at %p\n", _smpStart);
    kprintf("... length is %p\n", _smpEnd - _smpStart);
    kMemMove(trampoline, _smpStart, _smpEnd - _smpStart);

    // -- Patch up some memory locations
    *((uint32_t *)(&trampoline[14])) = intTableAddr;
    *((uint32_t *)(&trampoline[20])) = intTableAddr + 0x800;

    for (int i = 1; i < cpus.cpusDiscovered; i ++) {
        picControl->PicBroadcastInit(picControl, i);
        picControl->PicBroadcastSipi(picControl, i, (archsize_t)trampoline);
    }
}
