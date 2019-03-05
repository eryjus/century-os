//===================================================================================================================
//
//  cpu.h -- Standard CPU functions
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  These are the common low-level functions that need to be implemented to manage the CPU resource by the OS.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-May-25  Initial   0.1.0   ADCL  Initial version as I move functions from century32
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#ifndef __CPU_H__
#define __CPU_H__


#include "types.h"
#include "arch-cpu.h"


//
// -- Halt the CPU
//    ------------
extern "C" void Halt(void) __attribute__((noreturn));


//
// -- Enable interrupts if they are disabled; assembly language function
//    ------------------------------------------------------------------
extern "C" void EnableInterrupts(void);


//
// -- Disable interrupts and return the current flags state; assembly language function
//    ---------------------------------------------------------------------------------
extern "C" archsize_t DisableInterrupts(void);


//
// -- Restore the flags state back to the provided state; note all flags are updates; assembly language function
//    ----------------------------------------------------------------------------------------------------------
extern "C" void RestoreInterrupts(archsize_t flg);


//
// -- Set a block of memory to the specified byte
//    -------------------------------------------
extern "C" void kMemSetB(void *buf, uint8_t wrd, size_t cnt);


//
// -- Set a block of memory to the specified word
//    -------------------------------------------
extern "C" void kMemSetW(void *buf, uint16_t wrd, size_t cnt);


//
// -- Move a block of memory from one location to another
//    ---------------------------------------------------
extern "C" void kMemMove(void *tgt, void *src, size_t cnt);


//
// -- Copy a string from one location to another
//    ------------------------------------------
extern "C" void kStrCpy(char *dest, char *src);


//
// -- Get the length of a string
//    --------------------------
extern "C" size_t kStrLen(const char *s);


//
// -- Get the CPU capabilities list for CenturyOS
//    -------------------------------------------
void CpuGetCapabilities(void);


//
// -- Now, arch-cpu.h is required to provide some specific values, perform some sanity checks here
//    to make sure those values have been properly #defined.
//    --------------------------------------------------------------------------------------------
#ifndef BYTE_ALIGNMENT
#   error "BYTE_ALIGNMENT must be #defined and was not"
#endif

#ifndef HW_DISCOVERY_LOC
#   error "HW_DISCOVERY_LOC must be #defined and was not"
#endif

#ifndef PROCESS_PAGE_DIR
#   error "PROCESS_PAGE_DIR must be #defined and was not"
#endif

#ifndef PROCESS_PAGE_TABLE
#   error "PROCESS_PAGE_TABLE must be #defined and was not"
#endif

#ifndef MMU_FRAME_ADDR_PT
#   error "MMU_FRAME_ADDR_PT must be #defined and was not"
#endif

#ifndef MMU_FRAME_ADDR_PD
#   error "MMU_FRAME_ADDR_PD must be #defined and was not"
#endif

#ifndef PROCESS_STACK_BUILD
#   error "PROCESS_STACK_BUILD must be #defined and was not"
#endif

#ifndef FRAME_BUFFER_VADDR
#   error "FRAME_BUFFER_VADDR must be #defined and was not"
#endif

#ifndef EXCEPT_VECTOR_TABLE
#   error "EXCEPT_VECTOR_TABLE must be #defined and was not"
#endif

#ifndef EXCEPTION_STACK_SIZE
#   error "EXCEPTION_STACK_SIZE must be #defined and was not"
#endif

#endif
