//===================================================================================================================
//
// kernel/inc/x86-common/cpu.h -- This file contains the common definitions for setting up the x86 CPUs
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-05-06  Initial   0.0.0   ADCL  Initial version
//  2018-05-28  Initial   0.1.0   ADCL  Copied this file from century (gdt.h) to century-os
//
//===================================================================================================================


#ifndef __CPU_H__
#error "Do not include 'x86-common-cpu.h' directly; include 'cpu.h' instead, which will pick up this file."
#endif


#include "types.h"

#define PORT 0x3f8   /* COM1 */


enum GdtTypes {
    GDT_DATA = 0b0010,
    GDT_CODE = 0b1010,
};


enum Selectors {
    SEL_NULL = 0x00 << 3,
    SEL_KCODE = 0x01 << 3,
    SEL_KSTACK = 0x02 << 3,
    SEL_UCODE = 0x03 << 3,
    SEL_USTACK = 0x04 << 3,
    SEL_UDATA = 0x05 << 3,              // unused at this time
    SEL_KDATA = 0x06 << 3,              // unused at this time
    SEL_LCODE = 0x07 << 3,
    SEL_LDATA = 0x08 << 3,
    SEL_TSS = 0x09 << 3,
    SEL_TSS2 = 0x0a << 3,
    SEL_CALL1 = 0x0b << 3,              // Possible Future use
    SEL_CALL2 = 0x0c << 3,              // Possible Future use
    SEL_CALL3 = 0x0d << 3,              // Possible Future use
    SEL_CALL4 = 0x0e << 3,              // Possible Future use
    SEL_CALL5 = 0x0f << 3,              // Possible Future use
};


//
// -- This is a segment descriptor.
//    -----------------------------
struct SegmentDescriptor {
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
};


//
// -- This macro will be used to statically define the NULL selector in the loader.
//    -----------------------------------------------------------------------------
#define SEG_NULL                                                                            \
(struct SegmentDescriptor) {                                                                \
    0x0000,                                 /* limitLow */                                  \
    0x0000,                                 /* baseLow */                                   \
    0x00,                                   /* baseMid */                                   \
    0x0,                                    /* type (see GDT_* constants) */                \
    0,                                      /* s: application segment */                    \
    0x00,                                   /* dpl */                                       \
    0,                                      /* p: always present */                         \
    0x0,                                    /* limitHi */                                   \
    0,                                      /* avl: keep as 0 for now */                    \
    0,                                      /* bit64: 32-bit */                             \
    0,                                      /* db: 1 for 32-bit segment */                  \
    0,                                      /* g: 1 for limit * 4K */                       \
    0x00                                    /* baseHi */                                    \
}


//
// -- This macro will be used to statically define the segments used by the loader.  Note that the segments
//    used by the kernel and OS will be dynamically built and therefore will not use these macros.
//    -----------------------------------------------------------------------------------------------------
#define SEG32(type,base,lim,dpl)                                                            \
(struct SegmentDescriptor) {                                                                \
    ((lim) >> 12) & 0xffff,                 /* limitLow */                                  \
    (unsigned int)(base) & 0xffff,          /* baseLow */                                   \
    ((unsigned int)(base) >> 16) & 0xff,    /* baseMid */                                   \
    type,                                   /* type (see GDT_* constants) */                \
    1,                                      /* s: application segment */                    \
    dpl,                                    /* dpl */                                       \
    1,                                      /* p: always present */                         \
    (unsigned int)(lim) >> 28,              /* limitHi */                                   \
    0,                                      /* avl: keep as 0 for now */                    \
    0,                                      /* bit64: 32-bit */                             \
    1,                                      /* db: 1 for 32-bit segment */                  \
    1,                                      /* g: 1 for limit * 4K */                       \
    (unsigned int)(base) >> 24              /* baseHi */                                    \
}


//
// -- This macro will be used to statically define the segments used by the loader.  Note that the segments
//    used by the kernel and OS will be dynamically built and therefore will not use these macros.
//    -----------------------------------------------------------------------------------------------------
#define SEG64(type,dpl)                                                                     \
(struct SegmentDescriptor) {                                                                \
    0x0000,                                 /* limitLow */                                  \
    0x0000,                                 /* baseLow */                                   \
    0x00,                                   /* baseMid */                                   \
    type,                                   /* type (see GDT_* constants) */                \
    1,                                      /* s: application segment */                    \
    dpl,                                    /* dpl */                                       \
    1,                                      /* p: always present */                         \
    0x0,                                    /* limitHi */                                   \
    0,                                      /* avl: keep as 0 for now */                    \
    1,                                      /* bit64: 64-bit */                             \
    1,                                      /* db: 1 for 32-bit segment */                  \
    1,                                      /* g: 1 for limit * 4K */                       \
    0x00                                    /* baseHi */                                    \
}


//
// -- The Interrupt Descriptor Table Entry
//    ------------------------------------
typedef struct IdtEntry {
	uint16_t baseLow;
	uint16_t sel;
	uint8_t always0;
	uint8_t flags;
	uint16_t baseHigh;
} __attribute__((packed)) IdtEntry;

