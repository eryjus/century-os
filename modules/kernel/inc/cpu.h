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


//
// -- This is the state of a CPU
//    --------------------------
typedef enum {
    CPU_STOPPED = 0,
    CPU_STARTING = 1,
    CPU_STARTED = 2,
    CPU_BAD = 0xffff,
} CpuState_t;



//
// -- forward declare the process structure
//    -------------------------------------
struct Process_t;


//
// -- Set up the common CPU elements across all archs.  The actual ArchCpu_t structure will be defined
//    in the arch-cpu.h include.
//    ------------------------------------------------------------------------------------------------
#define COMMON_CPU_ELEMENTS                 \
    int cpuNum;                             \
    archsize_t stackTop;                    \
    archsize_t location;                    \
    SMP_UNSTABLE CpuState_t state;          \
    int kernelLocksHeld;                    \
    bool reschedulePending;                 \
    int disableIntDepth;                    \
    ArchCpu_t *cpu;                         \
    INT_UNSTABLE struct Process_t *process; \
    uint64_t lastTimer;                     \
    uint64_t cpuIdleTime;                   \
    frame_t stackFrame;


#if __has_include("arch-cpu.h")
#   include "arch-cpu.h"
#endif


//
// -- Mark this CPU as started so the next one can be released
//    --------------------------------------------------------
#define NextCpu(c) cpus.perCpuData[c].state = CPU_STARTED


//
// -- Halt the CPU
//    ------------
EXTERN_C EXPORT KERNEL
void Halt(void) __attribute__((noreturn));


//
// -- Panic-halt the OS, reporting the problems and the system state
//    --------------------------------------------------------------
EXTERN_C EXPORT NORETURN KERNEL
void CpuPanic(const char *reason, isrRegs_t *regs);


//
// -- Panic-halt the OS, pushing the registers onto the stack
//    -------------------------------------------------------
EXTERN_C EXPORT NORETURN KERNEL
void CpuPanicPushRegs(const char *reason);


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
void kStrCpy(char *dest, const char *src);


//
// -- Copy a string from one location to another
//    ------------------------------------------
EXTERN_C EXPORT KERNEL
int kStrCmp(const char *str1, const char *str2);


//
// -- Get the length of a string
//    --------------------------
EXTERN_C EXPORT KERNEL
size_t kStrLen(const char *s);


//
// -- Start any APs that need to be started
//    -------------------------------------
EXTERN_C EXPORT KERNEL
void CoresStart(void);


//
// -- Perform the initialization of the cpu data structure
//    ----------------------------------------------------
EXTERN_C EXPORT LOADER
void CpuInit(void);


//
// -- A do-nothing function for use with drivers
//    ------------------------------------------
EXTERN_C EXPORT KERNEL
void EmptyFunction(void);


//
// -- This structure defines all the data for all the cpus on the system
//    ------------------------------------------------------------------
typedef struct Cpu_t {
    int cpusDiscovered;
    SMP_UNSTABLE int cpusRunning;
    SMP_UNSTABLE int cpuStarting;
    ArchCpu_t perCpuData[MAX_CPUS];
} Cpu_t;


//
// -- A function to initialize the CPU structure for the cpu starting
//    ---------------------------------------------------------------
EXTERN_C EXPORT KERNEL
archsize_t CpuMyStruct(void);


//
// -- This is the cpu abstraction variable structure
//    ----------------------------------------------
EXTERN EXPORT KERNEL_BSS
Cpu_t cpus;


#include "atomic.h"

