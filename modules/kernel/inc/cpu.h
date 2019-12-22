//===================================================================================================================
//
//  cpu.h -- Standard CPU functions
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  These are the common low-level functions that need to be implemented to manage the CPU resource by the OS.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-May-25  Initial   0.1.0   ADCL  Initial version as I move functions from century32
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#ifndef __CPU_H__
#define __CPU_H__


#include "types.h"
#include "arch-cpu.h"


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
extern "C" archsize_t DisableInterrupts(void);


//
// -- Restore the flags state back to the provided state; note all flags are updates; assembly language function
//    ----------------------------------------------------------------------------------------------------------
extern "C" void RestoreInterrupts(archsize_t flg);


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
// -- Get the CPU number of the current process
//    -----------------------------------------
__CENTURY_FUNC__ int CpuNum(void);


//
// -- Start any APs that need to be started
//    -------------------------------------
__CENTURY_FUNC__ void CoresStart(void);


//
// -- This structure defines all the data for all the cpus on the system
//    ------------------------------------------------------------------
typedef struct Cpu_t {
    int cpusDiscovered;
    int cpusRunning;
} Cpu_t;


//
// -- This is used to control all the CPUs on the system
//    --------------------------------------------------
extern Cpu_t cpus;


#endif
