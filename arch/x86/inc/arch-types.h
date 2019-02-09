//===================================================================================================================
//
//  arch-types.h -- Type definitions specific to i686 architectures
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  These types are architecture dependent.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-May-25  Initial   0.1.0   ADCL  Initial
//  2018-Nov-11  Initial   0.2.0   ADCL  Address architecture abstraction issues
//  2019-Feb-09  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#ifndef __TYPES_H__
# 	error "Do not include 'arch-types.h' directly; include 'types.h' instead, which will pick up this file."
#endif


//
// -- This is the address width size for this architecture
//    ----------------------------------------------------
typedef uint32_t archsize_t;


//
// -- This is the size of a frame for the PMM (which is tied to the address width for this architecture)
//    --------------------------------------------------------------------------------------------------
typedef archsize_t frame_t;


//
// -- This is the equivalent to a port size for x86
//    ---------------------------------------------
typedef archsize_t devaddr_t;


//
// -- This is the order of the registers on the stack
//    -----------------------------------------------
typedef struct isrRegs_t {
	archsize_t ss;
	archsize_t gs;
	archsize_t fs;
	archsize_t es;
	archsize_t ds;
	archsize_t cr3;
	archsize_t cr2;
	archsize_t cr0;
	archsize_t edi;
	archsize_t esi;
	archsize_t ebp;
	archsize_t esp;
	archsize_t ebx;
	archsize_t edx;
	archsize_t ecx;
	archsize_t eax;
	archsize_t intno;
	archsize_t ackIRQ;
	archsize_t errcode;
	archsize_t eip;
	archsize_t cs;
	archsize_t eflags;
} isrRegs_t;


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


