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
// -- This is the max IOAPICs that can be defined for this arch
//    ---------------------------------------------------------
#define MAX_IOAPIC          1


//
// -- This is the natural byte alignment for this architecture
//    --------------------------------------------------------
#define BYTE_ALIGNMENT      4


//
// -- These are some addresses we need for this CPU architecture
//    ----------------------------------------------------------
#define HW_DISCOVERY_LOC            0x00003000


//
// -- This is the location where the stack will be initialized
//    --------------------------------------------------------
#define PROCESS_STACK_BUILD 0xff441000


//
// -- This is the location of the frame buffer
//    ----------------------------------------
#define FRAME_BUFFER_VADDR  0xfb000000


//
// -- The address of the MMIO addresses
//    ---------------------------------
#define MMIO_VADDR          0xf8000000
#define MMIO_LOADER_LOC     0x3f000000


//
// -- This is the location of the TTL1/TTL2 Tables in kernel space
//    ------------------------------------------------------------
#define TTL1_KRN_VADDR      0xff404000
#define MGMT_KRN_TTL2       0xfffff000
#define TTL2_KRN_VADDR      0xffc00000


//
// -- This is the location of the TTL1/TTL2 Tables in user space
//    ----------------------------------------------------------
#define TTL1_USR_VADDR      0x20010000
#define MGMT_USR_TTL2       0x7ffff000
#define TTL2_USR_VADDR      0x7fc00000


//
// -- These macros assist with the management of the MMU mappings -- picking the address apart into indexes
//    into the various tables
//    -----------------------------------------------------------------------------------------------------
#define KRN_TTL1_ENTRY(a)       (&((Ttl1_t *)TTL1_KRN_VADDR)[(a) >> 20])
#define KRN_TTL1_ENTRY4(a)      (&((Ttl1_t *)TTL1_KRN_VADDR)[((a) >> 20) & 0xffc])
#define KRN_TTL2_MGMT(a)        (&((Ttl2_t *)MGMT_KRN_TTL2)[(a) >> 22])
#define KRN_TTL2_ENTRY(a)       (&((Ttl2_t *)TTL2_KRN_VADDR)[(a) >> 12])

#define USR_TTL1_ENTRY(a)       (&((Ttl1_t *)TTL1_USR_VADDR)[(a) >> 20])
#define USR_TTL1_ENTRY4(a)      (&((Ttl1_t *)TTL1_USR_VADDR)[((a) >> 20) & 0xffc])
#define USR_TTL2_MGMT(a)        (&((Ttl2_t *)MGMT_USR_TTL2)[(a) >> 22])
#define USR_TTL2_ENTRY(a)       (&((Ttl2_t *)TTL2_USR_VADDR)[(a) >> 12])

#define TTL1_ENTRY(a,f)         (((f)&PG_KRN)?KRN_TTL1_ENTRY(a):USR_TTL1_ENTRY(a))
#define TTL1_ENTRY4(a,f)        (((f)&PG_KRN)?KRN_TTL1_ENTRY4(a):USR_TTL1_ENTRY4(a))
#define TTL2_MGMT(a,f)          (((f)&PG_KRN)?KRN_TTL2_MGMT(a):USR_TTL2_MGMT(a))
#define TTL2_ENTRY(a,f)         (((f)&PG_KRN)?KRN_TTL2_ENTRY(a):USR_TTL2_ENTRY(a))


//
// -- This is the location of the exception vector table
//    --------------------------------------------------
#define EXCEPT_VECTOR_TABLE 0xff401000


//
// -- This is the size of the short exception stacks
//    ----------------------------------------------
#define EXCEPTION_STACK_SIZE  512


//
// -- this is the size of a frame for this architecture
//    -------------------------------------------------
#define FRAME_SIZE              4096


//
// -- Wait the specified number of MICRO-seconds (not milli-)
//    -------------------------------------------------------
void BusyWait(uint32_t microSecs);


//
// -- a lightweight function to halt the cpu
//    --------------------------------------
inline void HaltCpu(void) { __asm("wfi"); }


//
// -- Panic the kernel, dumping the register state
//    --------------------------------------------
inline void Panic(void) { while (1) HaltCpu(); }


//
// -- Synchronization Barriers
//    ------------------------
#define DSB()               __asm volatile("dsb")
#define ISB()               __asm volatile("isb")
#define SEV()               __asm volatile("dsb\nsev\n");


//
// -- a macro to read a 32-bit control register
//    -----------------------------------------
#define MRC(cp15Spec) ({                                \
    uint32_t _val;                                      \
    __asm__ volatile("mrc " cp15Spec : "=r" (_val));    \
    _val;                                               \
})


//
// -- a macro to read a 32-bit floating point register
//    ------------------------------------------------
#define VMRS(vfpspec) ({                                    \
    uint32_t _val;                                          \
    __asm__ volatile("vmrs %0, " vfpspec : "=r" (_val));    \
    _val;                                                   \
})


//
// -- a macro to write a 32-bit control register
//    ------------------------------------------
#define MCR(cp15Spec,val) ({                            \
    __asm__ volatile("mcr " cp15Spec :: "r" (val));     \
    __asm__ volatile("isb");                            \
})


//
// -- a macro to write a 32-bit control register
//    ------------------------------------------
#define VMSR(vfpspec,val) ({                                \
    __asm__ volatile("vmsr " vfpspec ", %0" :: "r" (val));  \
    __asm__ volatile("isb");                                \
})


//
// -- a macro to read a 64-bit control register
//    -----------------------------------------
#define MRRC(cp15Spec) ({                                                   \
    uint32_t _lval, _hval;                                                  \
    __asm__ volatile("mrrc " cp15Spec : "=r" (_lval), "=r" (_hval));        \
    (((uint64_t)(_hval))<<32)|_lval;                                        \
})


//
// -- a macro to write to a 64-bit control register
//    ---------------------------------------------
#define MCRR(cp15Spec,val) ({                                               \
    uint32_t _lval = (uint32_t)(val & 0xffffffff);                          \
    uint32_t _hval = (uint32_t)(val >> 32);                                 \
    __asm__ volatile("mcrr " cp15Spec :: "r" (_lval), "r" (_hval));         \
})


//
// == These are the individual control registers (ordered by CRn, CRM, op1, op2)
//    ==========================================================================


//
// -- Access to the CTR (Cache Type Register)
//    ---------------------------------------
#define CTR                 "p15, 0, %0, c0, c0, 1"
#define READ_CTR()          MRC(CTR)


//
// -- Access to the CLIDR (Cache Level ID Register)
//    ---------------------------------------------
#define CLIDR               "p15, 1, %0, c0, c0, 1"
#define READ_CLIDR()        MRC(CLIDR)


//
// -- Access to the SCTLR (System Control Register)
//    ---------------------------------------------
#define SCTLR               "p15, 0, %0, c1, c0, 0"
#define READ_SCTLR()        MRC(SCTLR)
#define WRITE_SCTLR(val)    MCR(SCTLR,val)


//
// -- Access to the CPACR control register
//    ------------------------------------
#define CPACR               "p15, 0, %0, c1, c0, 2"
#define READ_CPACR()        MRC(CPACR)
#define WRITE_CPACR(val)    MCR(CPACR,val)


//
// -- Access to the NSACR control register
//    ------------------------------------
#define NSACR               "p15, 0, %0, c1, c1, 2"
#define READ_NSACR()        MRC(NSACR)
#define WRITE_NSACR(val)    MCR(NSACR,val)


//
// -- Access to the HCPTR control register
//    ------------------------------------
#define HCPTR               "p15, 4, %0, c1, c1, 2"
#define READ_HCPTR()        MRC(HCPTR)
#define WRITE_HCPTR(val)    MCR(HCPTR,val)


//
// -- Access to the TTBR0 Control Register
//    ------------------------------------
#define TTBR0               "p15, 0, %0, c2, c0, 0"
#define READ_TTBR0()        MRC(TTBR0)
#define WRITE_TTBR0(val)    MCR(TTBR0,val)


//
// -- Access to the TTBR1 Control Register
//    ------------------------------------
#define TTBR1               "p15, 0, %0, c2, c0, 1"
#define READ_TTBR1()        MRC(TTBR1)
#define WRITE_TTBR1(val)    MCR(TTBR1,val)


//
// -- Access to the IFSR (Instruction Faulting Status Register)
//    ---------------------------------------------------------
#define IFSR                "p15, 0, %0, c5, c0, 1"
#define READ_IFSR()         MRC(IFSR)
#define WRITE_IFSR(val)     MCR(IFSR,val)


//
// -- Access to the IFAR (Instruction Faulting Address Register)
//    ----------------------------------------------------------
#define IFAR                "p15, 0, %0, c6, c0, 2"
#define READ_IFAR()         MRC(IFAR)
#define WRITE_IFAR(val)     MCR(IFAR,val)


//
// -- Access to the VBAR (Vector Base Address Register)
//    -------------------------------------------------
#define VBAR                "p15, 0, %0, c12, c0, 0"
#define READ_VBAR()         MRC(VBAR)
#define WRITE_VBAR(val)     MCR(VBAR,val)


//
// -- Perform a TLBIMVAA (TLB Invalidate by MVA ALL ASID)
//    ---------------------------------------------------
#define TLBIMVAA(mem)       MCR("p15, 0, %0, c8, c7, 3",mem)


//
// -- Access to the FPEXC register
//    ----------------------------
#define FPEXC               "fpexc"
#define READ_FPEXC()        VMRS(FPEXC)
#define WRITE_FPEXC(val)    VMSR(FPEXC,val)


//
// -- branch prediction maintenance
//    -----------------------------
#if defined(ENABLE_BRANCH_PREDICTOR) && ENABLE_BRANCH_PREDICTOR == 1
#   define BPIMVA(mem)          MCR("p15, 0, %0, c7, c5, 7",mem)
#   define BPIALL()             MCR("p15, 0, %0, c7, c5, 6",0)
#   define BPIALLIS()           MCR("p15, 0, %0, c7, c1, 6",0)
#else
#   define BPIMVA(mem)
#   define BPIALL()
#   define BPIALLIS()
#endif


//
// -- cache maintenance functions
//    ---------------------------
#if defined(ENABLE_CACHE) && ENABLE_CACHE == 1
#   define DCIMVAC(mem)         MCR("p15, 0, %0, c7, c6, 1",mem)
#   define DCOSW(sw)            MCR("p15, 0, %0, c7, c6, 2",sw)
#   define DCCMVAC(mem)         MCR("p15, 0, %0, c7, c10, 1",mem)
#   define DCCSW(sw)            MCR("p15, 0, %0, c7, c10, 2",sw)
#   define DCCMVAU(mem)         MCR("p15, 0, %0, c7, c11, 1",mem)
#   define DCCIMVAC(mem)        MCR("p15, 0, %0, c7, c14, 1",mem)
#   define DCCISW(sw)           MCR("p15, 0, %0, c7, c14, 2",sw)

#   define ICIALLUIS()          MCR("p15, 0, %0, c7, c1, 0",0)
#   define ICIALLU()            MCR("p15, 0, %0, c7, c5, 0",0)
#   define ICIMVAU(mem)         MCR("p15, 0, %0, c7, c5, 1",mem)

#   define CLEAN_CACHE(mem,len)                                                         \
        do {                                                                            \
            DSB();                                                                      \
            archsize_t loc = ((archsize_t)(mem)) & ~(CACHE_LINE_SIZE - 1);              \
            archsize_t end = ((archsize_t)(mem)) + len;                                 \
            for ( ; loc <= end; loc += CACHE_LINE_SIZE) {                               \
                DCCMVAC(loc);                                                           \
            }                                                                           \
            DSB();                                                                      \
        } while(0)

#   define INVALIDATE_CACHE(mem,len)                                                    \
        do {                                                                            \
            DSB();                                                                      \
            archsize_t loc = ((archsize_t)(mem)) & ~(CACHE_LINE_SIZE - 1);              \
            archsize_t end = ((archsize_t)(mem)) + len;                                 \
            for ( ; loc <= end; loc += CACHE_LINE_SIZE) {                               \
                DCIMVAC(loc);                                                           \
            }                                                                           \
            DSB();                                                                      \
        } while(0)

#else
#   define DCIMVAC(mem)
#   define DCOSW(sw)
#   define DCCMVAC(mem)
#   define DCCSW(sw)
#   define DCCMVAU(mem)
#   define DCCIMVAC(mem)
#   define DCCISW(sw)

#   define ICIALLUIS()
#   define ICIALLU()
#   define ICIMVAU(mem)

#   define CLEAN_CACHE(mem,len)
#   define INVALIDATE_CACHE(mem,len)
#endif


//
// -- This is a well-defined sequence to clean up after changing the translation tables
//    ---------------------------------------------------------------------------------
#define INVALIDATE_PAGE(ent,vma)                                    \
        do {                                                        \
            DCCMVAC(ent);                                           \
            DSB();                                                  \
            TLBIMVAA(((uint32_t)vma) & 0xfffff000);                 \
            BPIALL();                                               \
            DSB();                                                  \
            ISB();                                                  \
        } while (0)


//
// -- A dummy function to enter system mode, since this is for the ARM
//    ----------------------------------------------------------------
extern "C" void EnterSystemMode(void);


//
// -- Get the CBAR
//    ------------
extern "C" archsize_t GetCBAR(void);


//
// -- Get the Data Fault Address Register (DFAR)
//    ------------------------------------------
extern "C" archsize_t GetDFAR(void);


//
// -- Get the Data Fault Status Register (DFSR)
//    ------------------------------------------
extern "C" archsize_t GetDFSR(void);


//
// -- Initialize the core to use the FPU
//    ----------------------------------
__CENTURY_FUNC__ void FpuInit(void);


#define CpuTssInit()

