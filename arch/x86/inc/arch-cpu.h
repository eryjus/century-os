//===================================================================================================================
//
//  arch-cpu.h -- This file contains the definitions for setting up the Intel 32-bit CPUs
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-May-30  Initial   0.1.0   ADCL  Copied this file from century to century-os
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#pragma once

#ifndef __CPU_H__
#   error "Do not include 'arch-cpu.h' directly; include 'cpu.h' instead, which will pick up this file."
#endif

#include "types.h"


//
// -- This is the max IOAPICs that can be defined for this arch
//    ---------------------------------------------------------
#define MAX_IOAPIC          64


//
// -- This is the natural byte alignment for this architecture
//    --------------------------------------------------------
#define BYTE_ALIGNMENT      4


//
// -- These are some addresses we need for this CPU architecture
//    ----------------------------------------------------------
#define HW_DISCOVERY_LOC            0x00003000


//
// -- This is the size of the short exception stacks
//    ----------------------------------------------
#define EXCEPTION_STACK_SIZE  4096


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
const archsize_t IDT_ADDRESS = 0xff401800;


//
// -- GDT Segment types
//    -----------------
typedef enum {
    GDT_DATA = 0b0010,
    GDT_CODE = 0b1010,
    TSS      = 0b1001,
} DescriptorTypes_t;


//
// -- This is a descriptor
//    --------------------
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
// -- This is the Task State-Segment structure
//    ----------------------------------------
typedef struct tss_t {
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
} __attribute__((packed)) tss_t;


//
// -- This is a short stack specific for the TSS
//    ------------------------------------------
extern byte_t tssStack[TSS_STACK_SIZE];


//
// -- Load the task register
//    ----------------------
EXTERN_C EXPORT KERNEL
void Ltr(uint16_t tr);


//
// -- Initialize the TSS
//    ------------------
void CpuTssInit(void);


//
// -- Change the page directory to the physical address provided
//    ----------------------------------------------------------
EXTERN_C EXPORT KERNEL
void MmuSwitchPageDir(archsize_t physAddr);


//
// -- a lightweight function to halt the cpu
//    --------------------------------------
EXPORT KERNEL INLINE
void HaltCpu(void) { __asm("hlt"); }


//
// -- Panic the kernel, dumping the register state
//    --------------------------------------------
EXPORT KERNEL INLINE
void Panic(void) { __asm("int3"); }


//
// -- cache maintenance functions
//    ---------------------------
#if defined(ENABLE_CACHE) && ENABLE_CACHE == 1
#   define CLEAN_CACHE(mem,len)         WBINVD()
#   define INVALIDATE_CACHE(mem,len)    WBINVD()
#   define WBINVD()                     __asm volatile("wbinvd")
#else
#   define CLEAN_CACHE(mem,len)
#   define INVALIDATE_CACHE(mem,len)
#   define WBINVD()
#endif


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
// -- A dummy function to enter system mode, since this is for the ARM
//    ----------------------------------------------------------------
EXPORT LOADER INLINE
void EnterSystemMode(void) {}


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
void LoadGdt(void *);


//
// -- Load the IDT
//    ------------
EXTERN_C EXPORT KERNEL
void LoadIdt(void *);


