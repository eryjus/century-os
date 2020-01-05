//===================================================================================================================
//
//  cpu.h -- Standard CPU functions
//
//        Copyright (c)  2017-2020 -- Adam Clark
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


#pragma once
#define __CPU_H__


#include "types.h"

#if __has_include("arch-cpu.h")
#   include "arch-cpu.h"
#endif


//
// -- Halt the CPU
//    ------------
EXTERN_C EXPORT KERNEL
void Halt(void) __attribute__((noreturn));


//
// -- Enable interrupts if they are disabled; assembly language function
//    ------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void EnableInterrupts(void);


//
// -- Disable interrupts and return the current flags state; assembly language function
//    ---------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
archsize_t DisableInterrupts(void);


//
// -- Restore the flags state back to the provided state; note all flags are updates; assembly language function
//    ----------------------------------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void RestoreInterrupts(archsize_t flg);


//
// -- Set a block of memory to the specified byte
//    -------------------------------------------
EXTERN_C EXPORT KERNEL
void kMemSetB(void *buf, uint8_t wrd, size_t cnt);


//
// -- Set a block of memory to the specified word
//    -------------------------------------------
EXTERN_C EXPORT KERNEL
void kMemSetW(void *buf, uint16_t wrd, size_t cnt);


//
// -- Move a block of memory from one location to another
//    ---------------------------------------------------
EXTERN_C EXPORT KERNEL
void kMemMove(void *tgt, void *src, size_t cnt);


//
// -- Copy a string from one location to another
//    ------------------------------------------
EXTERN_C EXPORT KERNEL
void kStrCpy(char *dest, char *src);


//
// -- Get the length of a string
//    --------------------------
EXTERN_C EXPORT KERNEL
size_t kStrLen(const char *s);


//
// -- Get the CPU number of the current process
//    -----------------------------------------
EXTERN_C EXPORT KERNEL
int CpuNum(void);


//
// -- Start any APs that need to be started
//    -------------------------------------
EXTERN_C EXPORT KERNEL
void CoresStart(void);


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
EXTERN KERNEL_DATA
Cpu_t cpus;

