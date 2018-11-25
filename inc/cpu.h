//===================================================================================================================
//
//  cpu.h -- Standard CPU functions
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  These are the common functions that need to be implemented to manage the CPU resource by the OS.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-May-25  Initial   0.1.0   ADCL  Initial version as I move functions from century32
//
//===================================================================================================================


#ifndef __CPU_H__
#define __CPU_H__


#include "types.h"


//
// -- Include the architecture-specific CPU stuff
//    -------------------------------------------
#include "arch-cpu-prevalent.h"


//
// -- Halt the CPU
//    ------------
extern "C" void Halt(void) __attribute__((noreturn));


//
// -- Enable interrupts if they are disabled; assembly language function
//    ------------------------------------------------------------------
extern "C" void EnableInterrupts(void);


//
// -- Disable interrupts and return the current flags state; assembly language function
//    ---------------------------------------------------------------------------------
extern "C" regval_t DisableInterrupts(void);


//
// -- Restore the flags state back to the provided state; note all flags are updates; assembly language function
//    ----------------------------------------------------------------------------------------------------------
extern "C" void RestoreInterrupts(regval_t flg);


//
// -- Set a block of memory to the specified byte
//    -------------------------------------------
extern "C" void kMemSetB(void *buf, uint8_t wrd, size_t cnt);


//
// -- Set a block of memory to the specified word
//    -------------------------------------------
extern "C" void kMemSetW(void *buf, uint16_t wrd, size_t cnt);


//
// -- Move a block of memory from one location to another
//    ---------------------------------------------------
extern "C" void kMemMove(void *tgt, void *src, size_t cnt);


//
// -- Copy a string from one location to another
//    ------------------------------------------
extern "C" void kStrCpy(char *dest, char *src);


//
// -- Get the length of a string
//    --------------------------
extern "C" size_t kStrLen(const char *s);


//
// -- Get the CPU capabilities list for CenturyOS
//    -------------------------------------------
void CpuGetCapabilities(void);


//
// -- Write to a Memory Mapped I/O Register
//    -------------------------------------
inline void MmioWrite(uint32_t reg, uint32_t data) { *(volatile uint32_t *)reg = data; }


//
// -- Read from a Memory Mapped I/O Register
//    --------------------------------------
inline uint32_t MmioRead(uint32_t reg) { return *(volatile uint32_t *)reg; }


#endif
