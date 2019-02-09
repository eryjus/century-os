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
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#ifndef __CPU_H__
#   error "Do not include 'arch-cpu.h' directly; include 'cpu.h' instead, which will pick up this file."
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
// -- Some specific memory locations
//    ------------------------------
#define PROCESS_PAGE_DIR    0xff430000
#define PROCESS_PAGE_TABLE  0xff434000

// -- these are dedicated to the function `MmuGetFrameForAddr()`, but documented here.
#define MMU_FRAME_ADDR_PD   0xff436000
#define MMU_FRAME_ADDR_PT   0xff43a000

#define PROCESS_STACK_BUILD 0xff441000


//
// -- This is the location of the frame buffer
//    ----------------------------------------
#define FRAME_BUFFER_VADDR  0xfb000000


//
// -- This is the location of the TTL1/TTL2 Tables
//    --------------------------------------------
#define TTL1_KRN_VADDR      0xff404000
#define TTL2_KRN_VADDR      0xffc00000


//
// -- This is the location of the exception vector table
//    --------------------------------------------------
#define EXCEPT_VECTOR_TABLE 0xff401000


//
// -- This is the size of the short exception stacks
//    ----------------------------------------------
#define EXCEPTION_STACK_SIZE  512


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
extern "C" archsize_t GetTTBR0(void);


//
// -- Get the CBAR
//    ------------
extern "C" archsize_t GetCBAR(void);


#define CpuTssInit()

