//===================================================================================================================
//
//  kernel/src/i686/CpuGdtInit.c -- This is the GDT for the 32-bit architectures
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-05-06  Initial   0.0.0   ADCL  Initial version
//  2018-05-28  Initial   0.1.0   ADCL  Copied this file from century to century-os
//
//===================================================================================================================

#include "types.h"
#include "console.h"
#include "cpu.h"


//
// -- The kernel's GDT
//    ----------------
struct SegmentDescriptor gdt[16] = {
    SEG_NULL,                                   // 0x00<<3: NULL descriptor
    SEG32(GDT_CODE, 0, 0xffffffff, 0),          // 0x01<<3: Kernel Code Selector
    SEG32(GDT_DATA, 0, 0xffffffff, 0),          // 0x02<<3: Kernel Stack (Data) Selector
    SEG32(GDT_CODE, 0, 0xffffffff, 3),          // 0x03<<3: User Code Selector
    SEG32(GDT_DATA, 0, 0xffffffff, 3),          // 0x04<<3: User Stack (Data) Selector
    SEG_NULL,                                   // 0x05<<3: Reserved for user data if desired
    SEG_NULL,                                   // 0x06<<3: Reserved for kernel data if desired
    SEG32(GDT_CODE, 0, 0xffffffff, 0),          // 0x07<<3: Loader Code Selector
    SEG32(GDT_DATA, 0, 0xffffffff, 0),          // 0x08<<3: Loader Data & Stack Selector
    SEG_NULL,                                   // 0x09<<3: TSS Part 1
    SEG_NULL,                                   // 0x0a<<3: TSS Part 2
    SEG_NULL,                                   // 0x0b<<3: Future use call gate
    SEG_NULL,                                   // 0x0c<<3: Future use call gate
    SEG_NULL,                                   // 0x0d<<3: Future use call gate
    SEG_NULL,                                   // 0x0e<<3: Future use call gate
    SEG_NULL,                                   // 0x0f<<3: Future use call gate
};  


