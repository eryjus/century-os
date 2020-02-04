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
    if (cpus.cpusDiscovered < 2) return;

    //
    // -- Load the trampoline code into the low 1MB of memory
    //    ---------------------------------------------------
    uint8_t *trampoline = (uint8_t *)X86_TRAMPOLINE;        // for S&G, start at 32K
    extern uint8_t _smpStart[];
    extern uint8_t _smpEnd[];

    MmuUnmapPage(X86_TRAMPOLINE);
    MmuMapToFrame(X86_TRAMPOLINE, X86_TRAMPOLINE >> 12, PG_KRN | PG_WRT);

    kprintf("Copying the AP entry code to %p\n", trampoline);
    kprintf("... start at %p\n", _smpStart);
    kprintf("... length is %p\n", _smpEnd - _smpStart);

    kMemMove(trampoline, _smpStart, _smpEnd - _smpStart);  // something in here is overwriting MMU tables

    kprintf("... moved...\n");


    // -- remap as read only!!
//    MmuUnmapPage(X86_TRAMPOLINE);
//    MmuMapToFrame(X86_TRAMPOLINE, X86_TRAMPOLINE >> 12, PG_KRN | PG_DEVICE);

    kprintf("Memory remapped\n");

    cpus.perCpuData[0].location = ArchCpuLocation();

    for (int i = 1; i < cpus.cpusDiscovered; i ++) {
        cpus.cpuStarting = i;
        cpus.perCpuData[cpus.cpuStarting].state = CPU_STARTING;

        kprintf("Starting core %d \n", i);
        picControl->PicBroadcastInit(picControl, i);
        picControl->PicBroadcastSipi(picControl, i, (archsize_t)trampoline);

        while (cpus.perCpuData[cpus.cpuStarting].state == CPU_STARTING) {}
    }
}
