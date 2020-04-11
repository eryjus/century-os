//===================================================================================================================
//
//  arch-cpu.h -- This file contains the definitions for setting up the ARM for RPi2b
//
//        Copyright (c)  2017-2020 -- Adam Clark
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
#include "atomic.h"


//
// -- This is the abstraction of the CPU.
//    -----------------------------------
typedef struct ArchCpu_t {
    COMMON_CPU_ELEMENTS
} ArchCpu_t;


//
// -- Perform the Archictecture-Specifc CPU initialization required
//    -------------------------------------------------------------
#define ArchEarlyCpuInit()


//
// -- Complete the final initialization for the CPU
//    ---------------------------------------------
EXTERN_C EXPORT LOADER
void ArchLateCpuInit(int c);


//
// -- Complete the initialization for the arch-specific CPU elements
//    --------------------------------------------------------------
#define ArchPerCpuInit(...)


//
// -- Arch Specific cpu location determination
//    ----------------------------------------
#define ArchCpuLocation()       ReadMPIDR()


//
// -- This is the max IOAPICs that can be defined for this arch
//    ---------------------------------------------------------
#define MAX_IOAPIC          1


//
// -- This is the natural byte alignment for this architecture
//    --------------------------------------------------------
#define BYTE_ALIGNMENT      4


//
// -- These macros assist with the management of the MMU mappings -- picking the address apart into indexes
//    into the various tables
//    -----------------------------------------------------------------------------------------------------
#define KRN_TTL1_ENTRY(a)       (&((Ttl1_t *)ARMV7_TTL1_TABLE_VADDR)[(a) >> 20])
#define KRN_TTL1_ENTRY4(a)      (&((Ttl1_t *)ARMV7_TTL1_TABLE_VADDR)[((a) >> 20) & 0xffc])
#define KRN_TTL2_MGMT(a)        (&((Ttl2_t *)ARMV7_TTL2_MGMT)[(a) >> 22])
#define KRN_TTL2_ENTRY(a)       (&((Ttl2_t *)ARMV7_TTL2_TABLE_VADDR)[(a) >> 12])


//
// -- Wait the specified number of MICRO-seconds (not milli-)
//    -------------------------------------------------------
void BusyWait(uint32_t microSecs);


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
// -- Access to the FPEXC register
//    ----------------------------
#define FPEXC               "fpexc"
#define READ_FPEXC()        VMRS(FPEXC)
#define WRITE_FPEXC(val)    VMSR(FPEXC,val)


//
// -- Initialize the core to use the FPU
//    ----------------------------------
EXTERN_C EXPORT LOADER
void ArchFpuInit(void);


#define ApTimerInit(t,f) TimerInit(t, f)


//
// -- Inlcude the arch-specific CPU operations
//    ----------------------------------------
#include "arch-cpu-ops.h"


//
// -- Some optimizations for the elements we will get to frequently
//    -------------------------------------------------------------
#define thisCpu ((ArchCpu_t *)ReadTPIDRPRW())
#define currentThread ((Process_t *)ReadTPIDRURO())

EXTERN_C EXPORT INLINE
void CurrentThreadAssign(Process_t *p) { thisCpu->process = p; WriteTPIDRURO((archsize_t)p); }


//
// -- Bochs magic breakpoint (which will not work on arm)
//    ---------------------------------------------------
#define BOCHS_BREAK
#define BOCHS_TOGGLE_INSTR


