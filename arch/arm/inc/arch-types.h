//===================================================================================================================
//
//  arch-types.h -- Type definitions specific to rpi2b architectures
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
//  2018-Nov-11  Initial   0.2.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#ifndef __TYPES_H__
#   error "Do not include 'arch-types.h' directly; include 'types.h' instead, which will pick up this file."
#endif


//
// -- This is the address width size for this architecture
//    ----------------------------------------------------
typedef uint32_t archsize_t;


//
// -- This is the equivalent to a port size for x86
//    ---------------------------------------------
typedef archsize_t devaddr_t;


//
// -- This is the order of the registers on the stack
//    -----------------------------------------------
typedef struct isrRegs_t {
    archsize_t sp_usr;
    archsize_t lr_usr;
    archsize_t sp_svc;
    archsize_t lr_svc;
    archsize_t type;
    archsize_t r0;
    archsize_t r1;
    archsize_t r2;
    archsize_t r3;
    archsize_t r4;
    archsize_t r5;
    archsize_t r6;
    archsize_t r7;
    archsize_t r8;
    archsize_t r9;
    archsize_t r10;
    archsize_t r11;
    archsize_t r12;
    archsize_t lr_ret;
    archsize_t spsr_ret;
} isrRegs_t;
