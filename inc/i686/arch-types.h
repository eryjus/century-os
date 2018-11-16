//===================================================================================================================
//
//  arch-types.h -- Type definitions specific to i686 architectures
//
//        Copyright (c)  2017-2018 -- Adam Clark
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
//
//===================================================================================================================


#ifndef __TYPES_H__
# 	error "Do not include 'arch-types.h' directly; include 'types.h' instead, which will pick up this file."
#endif


//
// -- This is the address width size for this architecture
//    ----------------------------------------------------
typedef uint32_t ptrsize_t;


//
// -- This is the size of a frame for the PMM (which is tied to the address width for this architecture)
//    --------------------------------------------------------------------------------------------------
typedef uint32_t frame_t;


//
// -- This is the size of a general purpose register in this architecture
//    -------------------------------------------------------------------
typedef uint32_t regval_t;


//
// -- This is the order of the registers on the stack
//    -----------------------------------------------
typedef struct isrRegs_t {
	uint32_t ss;
	uint32_t gs;
	uint32_t fs;
	uint32_t es;
	uint32_t ds;
	uint32_t cr3;
	uint32_t cr2;
	uint32_t cr0;
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t esp;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	uint32_t intno;
	uint32_t ackIRQ;
	uint32_t errcode;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
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


//
// -- This is the prototype definition for an ISR handler routine
//    -----------------------------------------------------------
typedef void (*isrFunc_t)(isrRegs_t *);


//
// -- The definition of a NULL ISR Handler Function
//    ---------------------------------------------
const isrFunc_t NULL_ISR = (isrFunc_t)NULL;


//
// -- The ISR Handlers
//    ----------------
extern isrFunc_t isrHandlers[256];


