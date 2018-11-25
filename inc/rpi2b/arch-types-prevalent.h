//===================================================================================================================
//
//  arch-types.h -- Type definitions specific to rpi2b architectures
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
//  2018-Nov-11  Initial   0.2.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __TYPES_H__
#   error "Do not include 'arch-types-prevalent.h' directly; include 'types.h' instead, which will pick up this file."
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
	uint32_t r0;
	uint32_t r1;
	uint32_t r2;
	uint32_t r3;
	uint32_t r4;
	uint32_t r5;
	uint32_t r6;
	uint32_t r7;
	uint32_t r8;
	uint32_t r9;
	uint32_t r10;
	uint32_t r11;
	uint32_t r12;
	uint32_t r13;
	uint32_t r14;
	uint32_t r15;
} isrRegs_t;
