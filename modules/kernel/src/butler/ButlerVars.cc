//===================================================================================================================
//
//  ButlerVars.cc -- Variables used by the Butler
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-11  Initial  v0.6.1b  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "msgq.h"
#include "butler.h"


//
// -- This is the message queue the butler will use
//    ---------------------------------------------
EXPORT KERNEL_BSS
MessageQueue_t *butlerMsgq;


//
// -- These are several memory locations that were provided by the linker that we may want to keep track of
//    -----------------------------------------------------------------------------------------------------
EXPORT KERNEL_BSS
uint8_t *krnKernelTextStart;

EXPORT KERNEL_BSS
uint8_t *krnKernelTextEnd;

EXPORT KERNEL_BSS
archsize_t krnKernelTextPhys;

EXPORT KERNEL_BSS
archsize_t krnKernelTextSize;

EXPORT KERNEL_BSS
uint8_t *krnKernelDataStart;

EXPORT KERNEL_BSS
uint8_t *krnKernelDataEnd;

EXPORT KERNEL_BSS
archsize_t krnKernelDataPhys;

EXPORT KERNEL_BSS
archsize_t krnKernelDataSize;

EXPORT KERNEL_BSS
uint8_t *krnSyscallStart;

EXPORT KERNEL_BSS
uint8_t *krnSyscallEnd;

EXPORT KERNEL_BSS
archsize_t krnSyscallPhys;

EXPORT KERNEL_BSS
archsize_t krnSyscallSize;

EXPORT KERNEL_BSS
uint8_t *krnStabStart;

EXPORT KERNEL_BSS
uint8_t *krnStabEnd;

EXPORT KERNEL_BSS
archsize_t krnStabPhys;

EXPORT KERNEL_BSS
archsize_t krnStabSize;


