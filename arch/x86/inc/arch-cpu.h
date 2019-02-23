//===================================================================================================================
//
//  arch-cpu.h -- This file contains the definitions for setting up the Intel 32-bit CPUs
//
//        Copyright (c)  2017-2019 -- Adam Clark
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


#ifndef __CPU_H__
#   error "Do not include 'arch-cpu.h' directly; include 'cpu.h' instead, which will pick up this file."
#endif


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
// -- This is the location of the exception vector table
//    --------------------------------------------------
#define EXCEPT_VECTOR_TABLE 0xff401000


//
// -- This is the size of the short TSS stack
//    ---------------------------------------
#define TSS_STACK_SIZE  512


//
// -- this is the size of a frame for this architecture
//    -------------------------------------------------
#define FRAME_SIZE          4096


//
// -- this is the location of the kernel stack
//    ----------------------------------------
#define STACK_LOCATION          0xff800000
#define STACK_SIZE              0x1000


//
// -- These are critical CPU structure locations
//    ------------------------------------------
const archsize_t GDT_ADDRESS = 0xff401000;
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
// -- This is the structure and layout of Frame 0, accessed through kernel address 0xff401000
//    ---------------------------------------------------------------------------------------
typedef struct Frame0_t {
    Descriptor_t gdt[16];               // The GDT, for 128 bytes       (8*16)      128
    tss_t tss;                          // The TSS                      (104)       104
    byte_t unused1[384];                // Room to grow                 (384)       384
    byte_t unused2[920];                // Room for the ioBitmap        (920)       920
    byte_t unused3[512];                // Simply more room to grow     (512)       512
    Descriptor_t idt[256];              // The IDT                      (8*256)    2048
} __attribute__((packed)) Frame0_t;


//
// -- I'm going to let the compiler do the work for me here.  In case of an error, look for something like the
//    following (gcc):
//
//      /home/adam/workspace/century-os/inc/types.h:59:70: error: division by zero [-Werror=div-by-zero]
//       #define ct_assert(e) enum { ASSERT_CONCAT(assert_line_, __LINE__) = 1/(!!(e)) }
//                                                                           ~^~~~~~~~
//      /home/adam/workspace/century-os/modules/kernel/inc/tss.h:104:1: note: in expansion of macro 'ct_assert'
//       ct_assert(sizeof(Frame0_t) == 4096);
//       ^~~~~~~~~
//    ---------------------------------------------------------------------------------------------------------
ct_assert(sizeof(Frame0_t) == 4096);


//
// -- This is a pointer to the CPU structures
//    ---------------------------------------
extern Frame0_t *cpuStructs;


//
// -- This is a short stack specific for the TSS
//    ------------------------------------------
extern byte_t tssStack[TSS_STACK_SIZE];


//
// -- Load the task register
//    ----------------------
extern "C" void Ltr(uint16_t tr);


//
// -- Initialize the TSS
//    ------------------
void CpuTssInit(void);



//
// -- Change the page directory to the physical address provided
//    ----------------------------------------------------------
extern "C" void MmuSwitchPageDir(archsize_t physAddr);


//
// -- a lightweight function to halt the cpu
//    --------------------------------------
inline void HaltCpu(void) { __asm("hlt"); }


//
// -- A dummy function to enter system mode, since this is for the ARM
//    ----------------------------------------------------------------
extern "C" inline void EnterSystemMode(void) {}


//
// -- Get a byte from an I/O Port
//    ---------------------------
inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    asm volatile ( "inb %1, %0" : "=a"(ret) : "Nd"(port) );
    return ret;
}


//
// -- Output a byte to an I/O Port
//    ----------------------------
inline void outb(uint16_t port, uint8_t val) { asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) ); }

//
// -- Get the CR3 value
//    -----------------
extern "C" archsize_t GetCr3(void);


