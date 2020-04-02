//===================================================================================================================
//
//  arch-cpu.h -- This file contains the definitions for setting up the x86 32-bit CPUs
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  This file contains the structures related to managing the x86 32-bit CPUs.  Not included here are the mmu or
//  interrupt handling structures.  This is intended to be the basic strucures for getting the CPU into Protected
//  Mode.
//
//  So, to be clear about interrupts, included here are the structures for the setup of interrupts to be taken
//  (as there may be several errors that need to be handled), but not the actual handling of these interrupts.
//  What I am going for here is the Descriptor Table Entries.
//
//  There are several function that are also needed to be implemented for Arch-specific setup.  Several of these
//  will be `#define`-type macros.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-May-30  Initial   0.1.0   ADCL  Copied this file from century to century-os
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


//
// -- Perform the required housekeeping
//    ---------------------------------
#pragma once

#ifndef __TYPES_H__
#   error "Missing include 'types.h' at the top of the #include list."
#endif

#ifndef __CPU_H__
#   error "Do not include 'arch-cpu.h' directly; include 'cpu.h' instead, which will pick up this file."
#endif


//
// -- This is the Task State-Segment structure
//    ----------------------------------------
typedef struct Tss_t {
    uint32_t prev_tss;
    uint32_t esp0;
    uint32_t ss0;
    uint32_t esp1;
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;
    uint32_t cs;
    uint32_t ss;
    uint32_t ds;
    uint32_t fs;
    uint32_t gs;
    uint32_t ldt;
    uint16_t trap;
    uint16_t ioMap;
    uint32_t ssp;
} __attribute__((packed)) Tss_t;


//
// -- This is the abstraction of the x86 CPU
//    --------------------------------------
typedef struct ArchCpu_t {
    COMMON_CPU_ELEMENTS;
    Tss_t tss;
    archsize_t gsSelector;
    archsize_t tssSelector;
} ArchCpu_t;


//
// -- This is a descriptor used for the GDT and LDT
//    ---------------------------------------------
typedef struct Descriptor_t {
    unsigned int limitLow : 16;         // Low bits (15-0) of segment limit
    unsigned int baseLow : 16;          // Low bits (15-0) of segment base address
    unsigned int baseMid : 8;           // Middle bits (23-16) of segment base address
    unsigned int type : 4;              // Segment type (see GDT_* constants)
    unsigned int s : 1;                 // 0 = system, 1 = application (1 for code/data)
    unsigned int dpl : 2;               // Descriptor Privilege Level
    unsigned int p : 1;                 // Present (must be 1)
    unsigned int limitHi : 4;           // High bits (19-16) of segment limit
    unsigned int avl : 1;               // Unused (available for software use)
    unsigned int bit64 : 1;             // 1 = 64-bit segment
    unsigned int db : 1;                // 0 = 16-bit segment, 1 = 32-bit segment
    unsigned int g : 1;                 // Granularity: limit scaled by 4K when set
    unsigned int baseHi : 8;            // High bits (31-24) of segment base address
} Descriptor_t;


//
// -- A helper macro use to define the NULL Selector
//    ----------------------------------------------
#define NULL_GDT    {0}


//
// -- A helper macro used to define the kernel code
//    0x00 c f 9 a 00 0000 ffff
//    ---------------------------------------------
#define KCODE_GDT        {              \
    .limitLow = 0xffff,                 \
    .baseLow = 0,                       \
    .baseMid = 0,                       \
    .type = 0x0a,                       \
    .s = 1,                             \
    .dpl = 0,                           \
    .p = 1,                             \
    .limitHi = 0xf,                     \
    .avl = 0,                           \
    .bit64 = 0,                         \
    .db = 1,                            \
    .g = 1,                             \
    .baseHi = 0,                        \
}


//
// -- A helper macro used to define the kernel data
//    0x00 c f 9 2 00 0000 ffff
//    ---------------------------------------------
#define KDATA_GDT        {              \
    .limitLow = 0xffff,                 \
    .baseLow = 0,                       \
    .baseMid = 0,                       \
    .type = 0x02,                       \
    .s = 1,                             \
    .dpl = 0,                           \
    .p = 1,                             \
    .limitHi = 0xf,                     \
    .avl = 0,                           \
    .bit64 = 0,                         \
    .db = 1,                            \
    .g = 1,                             \
    .baseHi = 0,                        \
}


//
// -- A helper macro used to define the kernel code
//    0x00 c f 9 a 00 0000 ffff
//    ---------------------------------------------
#define UCODE_GDT        {              \
    .limitLow = 0xffff,                 \
    .baseLow = 0,                       \
    .baseMid = 0,                       \
    .type = 0x0a,                       \
    .s = 1,                             \
    .dpl = 0,                           \
    .p = 1,                             \
    .limitHi = 0xf,                     \
    .avl = 0,                           \
    .bit64 = 0,                         \
    .db = 1,                            \
    .g = 1,                             \
    .baseHi = 0,                        \
}


//
// -- A helper macro used to define the kernel data
//    0x00 c f 9 2 00 0000 ffff
//    ---------------------------------------------
#define UDATA_GDT        {              \
    .limitLow = 0xffff,                 \
    .baseLow = 0,                       \
    .baseMid = 0,                       \
    .type = 0x02,                       \
    .s = 1,                             \
    .dpl = 0,                           \
    .p = 1,                             \
    .limitHi = 0xf,                     \
    .avl = 0,                           \
    .bit64 = 0,                         \
    .db = 1,                            \
    .g = 1,                             \
    .baseHi = 0,                        \
}


//
// -- A helper macro to define a segment selector specific to the per-cpu data for a given CPU.
//    -----------------------------------------------------------------------------------------
#define GS_GDT(locn)        {           \
    .limitLow = 7,                      \
    .baseLow = ((locn) & 0xffff),       \
    .baseMid = (((locn) >> 16) & 0xff), \
    .type = 0x02,                       \
    .s = 1,                             \
    .dpl = 0,                           \
    .p = 1,                             \
    .limitHi = 0,                       \
    .avl = 0,                           \
    .bit64 = 0,                         \
    .db = 1,                            \
    .g = 0,                             \
    .baseHi = (((locn) >> 24) & 0xff),  \
}


//
// -- A helper macro used to define the kernel code
//    0x00 c f 9 a 00 0000 ffff
//    ---------------------------------------------
#define LCODE_GDT        {              \
    .limitLow = 0xffff,                 \
    .baseLow = 0,                       \
    .baseMid = 0,                       \
    .type = 0x0a,                       \
    .s = 1,                             \
    .dpl = 0,                           \
    .p = 1,                             \
    .limitHi = 0xf,                     \
    .avl = 0,                           \
    .bit64 = 0,                         \
    .db = 1,                            \
    .g = 1,                             \
    .baseHi = 0,                        \
}


//
// -- A helper macro used to define the kernel data
//    0x00 c f 9 2 00 0000 ffff
//    ---------------------------------------------
#define LDATA_GDT        {              \
    .limitLow = 0xffff,                 \
    .baseLow = 0,                       \
    .baseMid = 0,                       \
    .type = 0x02,                       \
    .s = 1,                             \
    .dpl = 0,                           \
    .p = 1,                             \
    .limitHi = 0xf,                     \
    .avl = 0,                           \
    .bit64 = 0,                         \
    .db = 1,                            \
    .g = 1,                             \
    .baseHi = 0,                        \
}


//
// -- A helper macro used to define the kernel data
//    0x00 c f 9 2 00 0000 ffff
//    ---------------------------------------------
#define TSS32_GDT(locn)       {                     \
    .limitLow = ((sizeof(Tss_t) - 1) & 0xffff),     \
    .baseLow = ((locn) & 0xffff),                   \
    .baseMid = (((locn) >> 16) & 0xff),             \
    .type = 0x9,                                    \
    .s = 0,                                         \
    .dpl = 0,                                       \
    .p = 1,                                         \
    .limitHi = (((sizeof(Tss_t) - 1) >> 16) & 0xf), \
    .avl = 0,                                       \
    .bit64 = 0,                                     \
    .db = 0,                                        \
    .g = 0,                                         \
    .baseHi = (((locn) >> 24) & 0xff),              \
}


//
// -- Some cute optimizations for accessing the CPU elements.  The "asm(gs:0)" tells gcc that when you want
//    to read this variable, it is found at the offset 0 from the start of the gs section.  So the key here
//    is going to be to set gs properly.  This will be one during initialization.
//    -----------------------------------------------------------------------------------------------------
EXTERN ArchCpu_t *thisCpu asm("%gs:0");
EXTERN struct Process_t *currentThread asm("%gs:4");

EXTERN_C EXPORT INLINE
void CurrentThreadAssign(Process_t *p) { currentThread = p; }


//
// -- Perform the architecture-specific CPU initialization
//    ----------------------------------------------------
EXTERN_C EXPORT LOADER
void ArchEarlyCpuInit(void);

EXTERN_C EXPORT LOADER
void ArchLateCpuInit(int c);


//
// -- Perform some arch-specific initialization
//    -----------------------------------------
EXTERN_C EXPORT LOADER
void ArchPerCpuInit(int i);


//
// -- Perform the setup for the permanent GDT
//    ---------------------------------------
EXTERN_C EXPORT LOADER
void ArchGdtSetup(void);


//
// -- Perform the setup for the permanent IDT
//    ---------------------------------------
EXTERN_C EXPORT LOADER
void ArchIdtSetup(void);


//
// -- Load GS from the per-cpu struct
//    -------------------------------
EXTERN_C EXPORT LOADER
void ArchGsLoad(archsize_t sel);


//
// -- Load TSS from the per-cpu struct
//    --------------------------------
EXTERN_C EXPORT LOADER
void ArchTssLoad(archsize_t sel);


//
// -- Arch Specific cpu location determination
//    ----------------------------------------
#define ArchCpuLocation()   MmioRead(LAPIC_MMIO + LAPIC_ID)











// -- TODO: relocate these constants


//
// -- This is the max IOAPICs that can be defined for this arch
//    ---------------------------------------------------------
#define MAX_IOAPIC          64


//
// -- This is the natural byte alignment for this architecture
//    --------------------------------------------------------
#define BYTE_ALIGNMENT      4


//
// -- This is the location of the Page Directory and Page Tables
//    ----------------------------------------------------------
#define PAGE_DIR_VADDR      0xfffff000
#define PAGE_TBL_VADDR      0xffc00000


//
// -- These macros help assist with the management of the MMU mappings -- separating the address components
//    into the indexes of the separate tables
//    -----------------------------------------------------------------------------------------------------
#define PD_ENTRY(a)         (&((PageEntry_t *)PAGE_DIR_VADDR)[(a) >> 22])
#define PT_ENTRY(a)         (&((PageEntry_t *)PAGE_TBL_VADDR)[(a) >> 12])


//
// -- This is the size of the short TSS stack
//    ---------------------------------------
#define TSS_STACK_SIZE  512


//
// -- These are critical CPU structure locations
//    ------------------------------------------
const archsize_t TSS_ADDRESS = 0xff401080;


//
// -- Load the task register
//    ----------------------
EXTERN_C EXPORT KERNEL
void Ltr(uint16_t tr);


//
// -- Change the page directory to the physical address provided
//    ----------------------------------------------------------
EXTERN_C EXPORT KERNEL
void MmuSwitchPageDir(archsize_t physAddr);


//
// -- Access macros for the APIC
//    --------------------------
#define APIC_BASE               (0x1b)
#define READ_APIC_BASE()        RDMSR(APIC_BASE)
#define WRITE_APIC_BASE(v)      WRMSR(APIC_BASE,v)


//
// -- Bochs magic breakpoint
//    ----------------------
#define BOCHS_BREAK             __asm volatile("xchg %bx,%bx")
#define BOCHS_TOGGLE_INSTR      __asm volatile("xchg %edx,%edx")


//
// -- Get the CR3 value
//    -----------------
EXTERN_C EXPORT KERNEL
archsize_t GetCr3(void);


//
// -- Check if CPUID is supported
//    ---------------------------
EXTERN_C EXPORT KERNEL
int CheckCpuid(void);


//
// -- Collect the CPUID information
//    -----------------------------
EXTERN_C EXPORT KERNEL
void CollectCpuid(void);


//
// -- Load the GDT and set it up
//    --------------------------
EXTERN_C EXPORT KERNEL
void ArchLoadGdt(void *);


//
// -- Load the IDT
//    ------------
EXTERN_C EXPORT KERNEL
void ArchLoadIdt(void *);


#define ApTimerInit(t,f)


//
// -- Inlcude the arch-specific CPU operations
//    ----------------------------------------
#include "arch-cpu-ops.h"

