//===================================================================================================================
//
//  arch-cpu.h -- This file contains the definitions for setting up the ARM for RPi2b
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-11  Initial   0.2.0   ADCL  Initial version
//  2018-Nov-13  Initial   0.2.0   ADCL  Copy the MMIO functions from century into this file
//
//===================================================================================================================


#ifndef __CPU_H__
#   error "Do not include 'arch-cpu-prevalent.h' directly; include 'cpu.h' instead, which will pick up this file."
#endif


#include "types.h"


//
// -- This is the natural byte alignment for this architecture
//    --------------------------------------------------------
#define BYTE_ALIGNMENT      4


//
// -- These are some addresses we need for this CPU architecture
//    ----------------------------------------------------------
#define HW_DISCOVERY_LOC            0x00003000
#define FRAME_BUFFER_ADDRESS        0xfb000000


//
// -- Read the low level timer value
//    ------------------------------
extern "C" uint64_t SysTimerCount(void);


//
// -- Wait the specified number of MICRO-seconds (not milli-)
//    -------------------------------------------------------
void BusyWait(uint32_t microSecs);


//
// -- a lightweight function to halt the cpu
//    --------------------------------------
inline void HaltCpu(void) { __asm("wfi"); }


//
// -- A dummy function to enter system mode, since this is for the ARM
//    ----------------------------------------------------------------
extern "C" void EnterSystemMode(void);


//
// -- Get the TTBR0
//    -------------
extern "C" regval_t GetTTBR0(void);


//
// -- Get the CBAR
//    ------------
extern "C" regval_t GetCBAR(void);


