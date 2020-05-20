//===================================================================================================================
//
//  arch-cpu-ops.h -- Some specific CPU operations that will happen on the x86
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Feb-16  Initial  v0.5.0g  ADCL  Initial version
//
//===================================================================================================================


#ifndef __CPU_H__
#   error "Do not include 'arch-cpu-ops.h' directly; include 'cpu.h' instead, which will pick up this file."
#endif


#include "types.h"


//
// -- CPUID function -- lifted from: https://wiki.osdev.org/CPUID
//    issue a single request to CPUID. Fits 'intel features', for instance note that even if only "eax" and "edx"
//    are of interest, other registers will be modified by the operation, so we need to tell the compiler about it.
//    -------------------------------------------------------------------------------------------------------------
EXPORT LOADER INLINE
void CPUID(int code, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    __asm volatile("cpuid\n" : "=a"(*a),"=b"(*b),"=c"(*c),"=d"(*d) : "a"(code)); }


//
// -- Model Specific Registers
//    ------------------------
EXPORT LOADER INLINE
uint64_t RDMSR(uint32_t r) {
    uint32_t _lo, _hi;
    __asm volatile("rdmsr\n" : "=a"(_lo),"=d"(_hi) : "c"(r) : "%ebx");
    return (((uint64_t)_hi) << 32) | _lo;
}

EXPORT LOADER INLINE
void WRMSR(uint32_t r, uint64_t v) {
    uint32_t _lo = (uint32_t)(v & 0xffffffff);
    uint32_t _hi = (uint32_t)(v >> 32);
    __asm volatile("wrmsr\n" : : "c"(r),"a"(_lo),"d"(_hi));
}


//
// -- Synchronization Barriers
//    ------------------------
#define SoftwareBarrier() __asm volatile("":::"memory")
#define MemoryBarrier() __sync_synchronize()
#define EntireSystemMemoryBarrier() __asm volatile("wbinvd":::"memory")
#define MemoryResynchronization() __asm volatile("wbinvd":::"memory")
#define ClearInstructionPipeline() __asm volatile("mov %%cr3,%%eax\n mov %%eax,%%cr3":::"memory","%eax")


//
// -- a lightweight function to halt the cpu
//    --------------------------------------
EXPORT INLINE
void HaltCpu(void) { __asm("hlt"); }


//
// -- cache maintenance functions
//    ---------------------------
#if defined(ENABLE_CACHE) && ENABLE_CACHE == 1
EXTERN_C EXPORT INLINE
void CleanCache(archsize_t mem, size_t len) { MemoryResynchronization(); }

EXTERN_C EXPORT INLINE
void InvalidateCache(archsize_t mem, size_t len) { MemoryResynchronization(); }
#else
EXTERN_C EXPORT INLINE
void CleanCache(archsize_t mem, size_t len) { }

EXTERN_C EXPORT INLINE
void InvalidateCache(archsize_t mem, size_t len) { }
#endif


