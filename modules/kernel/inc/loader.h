//===================================================================================================================
//
//  loader.h -- These are functions that used to perform the loader functions
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Dec-16  Initial   0.5.0   ADCL  Initial version
//
//===================================================================================================================


#pragma once


#include "types.h"


//
// -- this function will call the global initialization functions
//    -----------------------------------------------------------
EXTERN_C EXPORT LOADER
void LoaderFunctionInit(void);


//
// -- Complete the initialization of the MMU
//    --------------------------------------
EXTERN_C EXPORT LOADER
void MmuInit(void);


//
// -- This is the prototype for the loader main entry point
//    -----------------------------------------------------
EXTERN_C EXPORT LOADER NORETURN
void LoaderMain(archsize_t arg0, archsize_t arg1, archsize_t arg2);


//
// -- This is the prototype function to jump into the kernel proper
//    -------------------------------------------------------------
EXTERN_C EXPORT LOADER NORETURN
void JumpKernel(void (*addr)(), archsize_t stack) __attribute__((noreturn));


//
// -- This is the kernel function that will gain control (kernel entry point)
//    -----------------------------------------------------------------------
EXTERN_C EXPORT KERNEL NORETURN
void kInit(void);

