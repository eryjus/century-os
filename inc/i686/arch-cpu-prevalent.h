//===================================================================================================================
//
//  arch-cpu.h -- This file contains the definitions for setting up the Intel 32-bit CPUs
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-May-30  Initial   0.1.0   ADCL  Copied this file from century to century-os
//
//===================================================================================================================


#ifndef __CPU_H__
#   error "Do not include 'arch-cpu-prevalent.h' directly; include 'cpu.h' instead, which will pick up this file."
#endif


//
// -- This is the natural byte alignment for this architecture
//    --------------------------------------------------------
#define BYTE_ALIGNMENT      4


//
// -- Change the page directory to the physical address provided
//    ----------------------------------------------------------
extern "C" void MmuSwitchPageDir(ptrsize_t physAddr);


//
// -- These are some addresses we need for this CPU architecture
//    ----------------------------------------------------------
#define HW_DISCOVERY_LOC            0x00003000


//
// -- a lightweight function to halt the cpu
//    --------------------------------------
inline void HaltCpu(void) { __asm("hlt"); }


//
// -- A dummy function to enter system mode, since this is for the ARM
//    ----------------------------------------------------------------
extern "C" inline void EnterSystemMode(void) {}


//
// -- Get a byte from an I/O Port
//    ---------------------------
extern "C" uint8_t inb(uint16_t port);


//
// -- Output a byte to an I/O Port
//    ----------------------------
extern "C" void outb(uint16_t port, uint8_t byte);


//
// -- Get the CR3 value
//    -----------------
extern "C" regval_t GetCr3(void);


