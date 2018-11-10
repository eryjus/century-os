//===================================================================================================================
//
// tss.h -- This is the structure for managing the TSS along with its companion structures
//
// This file contains the structures for managing the TSS primarily.  However, since the GDT and the IDT also
// both exist in the same frame, this file will cover those as well.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Nov-04  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __TSS_H__
#define __TSS_H__


#include "types.h"


//
// -- This is the size of the short TSS stack
//    ---------------------------------------
#define TSS_STACK_SIZE  512


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
    byte_t unused1[384];                // Room to grow                 (384)       384
    tss_t tss;                          // The TSS                      (104)       104
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


#endif
