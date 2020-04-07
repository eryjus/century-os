//===================================================================================================================
//
//  arch-cpu-ops.h -- Some specific CPU operations that will happen on the armv7
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Feb-09  Initial  v0.5.0g  ADCL  Initial version
//
//===================================================================================================================


#ifndef __CPU_H__
#   error "Do not include 'arch-cpu-ops.h' directly; include 'cpu.h' instead, which will pick up this file."
#endif


#include "types.h"


//
// -- These are the working marcos that will output the inline functions
//    ------------------------------------------------------------------
#define CP_ADDR(cp,cr1,op1,cr2,op2)   #cp "," #op1 ",%0," #cr1 "," #cr2 "," #op2
#define CP_REG_RO(name,cp,cr1,op1,cr2,op2)                                      \
                                                                                \
    EXTERN_C INLINE                                                      \
    uint32_t Read##name(void) {                                                 \
        uint32_t _val;                                                          \
        __asm__ volatile("mrc " CP_ADDR(cp,cr1,op1,cr2,op2) : "=r" (_val));     \
        return _val;                                                            \
    }                                                                           \
                                                                                \
    EXTERN_C INLINE                                                      \
    uint32_t Read##name##Relaxed(void) {                                        \
        uint32_t _val;                                                          \
        __asm__("mrc " CP_ADDR(cp,cr1,op1,cr2,op2) : "=r" (_val));              \
        return _val;                                                            \
    }                                                                           \


#define CP_REG_WO(name,cp,cr1,op1,cr2,op2)                                      \
                                                                                \
    EXTERN_C INLINE                                                      \
    void Write##name(uint32_t val) {                                            \
        __asm__ volatile("mcr " CP_ADDR(cp,cr1,op1,cr2,op2) :: "r" (val));      \
        __asm__ volatile("isb");                                                \
    }                                                                           \
                                                                                \
    EXTERN_C INLINE                                                      \
    void Write##name##Relaxed(uint32_t val) {                                   \
        __asm__ volatile("mcr " CP_ADDR(cp,cr1,op1,cr2,op2) :: "r" (val));      \
    }                                                                           \


#define CP_REG_RW(name,cp,cr1,op1,cr2,op2)                                      \
    CP_REG_RO(name,cp,cr1,op1,cr2,op2)                                          \
    CP_REG_WO(name,cp,cr1,op1,cr2,op2)                                          \



//
// == These are the cp15 registers that are available to be read/written
//    ==================================================================


//
// -- C0 :: Identification Registers
//    ------------------------------
CP_REG_RO(CTR,  p15,c0,0,c0,1)          // Cache Type Register
CP_REG_RO(MPIDR,p15,c0,0,c0,5)          // Multiprocessor-Affinity Register
CP_REG_RO(CLIDR,p15,c0,1,c0,1)          // Cache Level ID Register


//
// -- C1 :: System Control Registers
//    ------------------------------
CP_REG_RW(SCTLR,p15,c1,0,c0,0)          // System Control Register
CP_REG_RW(CPACR,p15,c1,0,c0,2)          // Coprocessor Access Control Register
CP_REG_RW(NSACR,p15,c1,0,c1,2)          // Non-Secure Access Control Register
CP_REG_RW(HCPTR,p15,c1,4,c1,2)          // Hyp Coprocessor Trap Register


//
// -- C2 & C3 :: Memory Protection and Control Registers
//    --------------------------------------------------
CP_REG_RW(TTBR0,p15,c2,0,c0,0)          // Translation Table Base Register 0
CP_REG_RW(TTBR1,p15,c2,0,c0,1)          // Translation Table Base Register 1


//
// -- C5 & C6 :: Memory System Fault Registers
//    ----------------------------------------
CP_REG_RW(DFSR,p15,c5,0,c0,0)           // The Data Fault Status Register
CP_REG_RW(IFSR,p15,c5,0,c0,1)           // THe Instruction Fault Status Register

CP_REG_RW(DFAR,p15,c6,0,c0,0)           // The Data Fault Address Register
CP_REG_RW(IFAR,p15,c6,0,c0,2)           // The Instruction Fault Address Register


//
// -- C7 :: Cache Maintenance, Address Translation, and other Functions
//    -----------------------------------------------------------------
CP_REG_WO(_ICIALLUIS,p15,c7,0,c1,0)     // ICIALLUIS Branch Predictor
CP_REG_WO(_BPIALLIS,p15,c7,0,c1,6)      // BPIALLIS Branch Predictor
CP_REG_WO(_ICIALLU,p15,c7,0,c5,0)       // ICIALLU Branch Predictor
CP_REG_WO(_ICIMVAU,p15,c7,0,c5,1)       // ICIMVAU Branch Predictor
CP_REG_WO(_BPIALL,p15,c7,0,c5,6)        // BPIALL Branch Predictor
CP_REG_WO(_BPIMVA,p15,c7,0,c5,7)        // BPIMVA Branch Predictor
CP_REG_WO(_DCIMVAC,p15,c7,0,c6,1)       // DCIMVAC Cache Maintnenance
CP_REG_WO(_DCOSW,p15,c7,0,c6,2)         // DCOSW Cache Maintenance
CP_REG_WO(_DCCMVAC,p15,c7,0,c10,1)      // DCCMVAC Cache Maintenance
CP_REG_WO(_DCCSW,p15,c7,0,c10,2)        // DCCSW Cache Maintenance
CP_REG_WO(_DCCMVAU,p15,c7,0,c11,1)      // DCCMVAU Cache Maintenance
CP_REG_WO(_DCCIMVAC,p15,c7,0,c14,1)     // DCCIMVAC Cache Maintenance
CP_REG_WO(_DCCISW,p15,c7,0,c14,2)       // DCCISW Cache Maintenance


//
// -- C8 :: TLB Maintenance Functions
//    -------------------------------
CP_REG_WO(TLBIMVAA,p15,c8,0,c7,3)       // TLB Invalidate by MVA ALL ASID


//
// -- C9 :: TCM Control and Performance Monitors
//    ------------------------------------------


//
// -- C10 :: Memory Mapping and TLB Control Registers
//    -----------------------------------------------


//
// -- C11 :: TCM DMA Registers
//    ------------------------


//
// -- C12 :: Security Extensions Registers
//    ------------------------------------
CP_REG_RW(VBAR,p15,c12,0,c0,0)          // Vector Base Address Register


//
// -- C13 :: Process, Context, and Thread ID Registers
//    ------------------------------------------------
CP_REG_RW(TPIDRURO,p15,c13,0,c0,3)      // User Read-Only Thread ID Register (current process)
CP_REG_RW(TPIDRPRW,p15,c13,0,c0,4)      // Privilege Read-Write Thread ID Register (current CPU)


//
// -- C14 :: Generic Timer Extensions
//    -------------------------------


//
// -- branch prediction maintenance
//    -----------------------------
#if defined(ENABLE_BRANCH_PREDICTOR) && ENABLE_BRANCH_PREDICTOR == 1
#   define WriteBPIMVA(mem)          Write_BPIMVA(mem)
#   define WriteBPIALL()             Write_BPIALL(0)
#   define WriteBPIALLIS()           Write_BPIALLIS(0)
#else
#   define WriteBPIMVA(mem)
#   define WriteBPIALL()
#   define WriteBPIALLIS()
#endif


//
// -- Synchronization Barriers
//    ------------------------
#define SoftwareBarrier() __asm volatile("":::"memory")
#define MemoryBarrier() __sync_synchronize()
#define EntireSystemMemoryBarrier() __asm volatile("dmb sy":::"memory")
#define MemoryResynchronization() __asm volatile("dsb":::"memory")
#define ClearInsutructionPipeline() __asm volatile("isb":::"memory")


EXTERN_C INLINE
void SEV(void) { __asm volatile("dsb\nsev\n"); }


EXTERN_C INLINE
void DMB(void) { __asm volatile("dmb\n"); }


//
// -- a lightweight function to halt the cpu
//    --------------------------------------
EXTERN_C INLINE
void HaltCpu(void) { __asm("wfi"); }


//
// -- cache maintenance functions
//    ---------------------------
#if defined(ENABLE_CACHE) && ENABLE_CACHE == 1
#   define WriteDCIMVAC(mem)         Write_DCIMVAC(mem)
#   define WriteDCOSW(sw)            Write_DCOSW(sw)
#   define WriteDCCMVAC(mem)         Write_DCCMVAC(mem)
#   define WriteDCCSW(sw)            Write_DCCSW(sw)
#   define WriteDCCMVAU(mem)         Write_DCCMVAU(mem)
#   define WriteDCCIMVAC(mem)        Write_DCCIMVAC(mem)
#   define WriteDCCISW(sw)           Write_DCCISW(sw)

#   define WriteICIALLUIS()          Write_ICIALLUIS(0)
#   define WriteICIALLU()            Write_ICIALLU(0)
#   define WriteICIMVAU(mem)         Write_ICIMVAU(mem)


//
// -- Clean the cache for a block of memory
//    -------------------------------------
EXTERN_C INLINE
void CleanCache(archsize_t mem, size_t len) {
    MemoryBarrier();
    archsize_t loc = mem & ~(CACHE_LINE_SIZE - 1);
    archsize_t end = mem + len;
    for ( ; loc <= end; loc += CACHE_LINE_SIZE) {
        WriteDCCMVAC(loc);
    }
    MemoryBarrier();
}


//
// -- Force Cache Synchronization for a block of memory
//    -------------------------------------------------
EXTERN_C INLINE
void InvalidateCache(archsize_t mem, size_t len)
{
    MemoryBarrier();
    archsize_t loc = mem & ~(CACHE_LINE_SIZE - 1);
    archsize_t end = mem + len;
    for ( ; loc <= end; loc += CACHE_LINE_SIZE) {
        WriteDCIMVAC(loc);
    }
    MemoryBarrier();
}

#else
#   define WriteDCIMVAC(mem)
#   define WriteDCOSW(sw)
#   define WriteDCCMVAC(mem)
#   define WriteDCCSW(sw)
#   define WriteDCCMVAU(mem)
#   define WriteDCCIMVAC(mem)
#   define WriteDCCISW(sw)

#   define WriteICIALLUIS()
#   define WriteICIALLU()
#   define WriteICIMVAU(mem)

#   define CleanCache(mem,len)
#   define INVALIDATE_CACHE(mem,len)
#endif


//
// -- This is a well-defined sequence to clean up after changing the translation tables
//    ---------------------------------------------------------------------------------
EXTERN_C INLINE
void InvalidatePage(archsize_t vma) {
    MemoryBarrier();
    WriteTLBIMVAA(vma & 0xfffff000);
    WriteBPIALL();
    MemoryBarrier();
    ClearInsutructionPipeline();
}




