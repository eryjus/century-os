//===================================================================================================================
//
//  butler.h -- The butler process
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-10  Initial  v0.6.1b  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "process.h"
#include "msgq.h"


//
// -- These are the messages that the butler knows how to handle
//    ----------------------------------------------------------
typedef enum {
    BUTLER_CLEAN_PMM,
    BUTLER_CLEAN_PROCESS,
} ButlerTask_t;


//
// -- The name of the butler process
//    ------------------------------
EXTERN EXPORT KERNEL_DATA
const char *butlerName;


//
// -- This is the message queue the butler will use
//    ---------------------------------------------
EXTERN EXPORT KERNEL_BSS
MessageQueue_t *butlerMsgq;


//
// -- These are several memory locations that were provided by the linker that we may want to keep track of
//    -----------------------------------------------------------------------------------------------------
EXTERN EXPORT KERNEL_BSS
uint8_t *krnKernelTextStart;

EXTERN EXPORT KERNEL_BSS
uint8_t *krnKernelTextEnd;

EXTERN EXPORT KERNEL_BSS
archsize_t krnKernelTextPhys;

EXTERN EXPORT KERNEL_BSS
archsize_t krnKernelTextSize;

EXTERN EXPORT KERNEL_BSS
uint8_t *krnKernelDataStart;

EXTERN EXPORT KERNEL_BSS
uint8_t *krnKernelDataEnd;

EXTERN EXPORT KERNEL_BSS
archsize_t krnKernelDataPhys;

EXTERN EXPORT KERNEL_BSS
archsize_t krnKernelDataSize;

EXTERN EXPORT KERNEL_BSS
uint8_t *krnSyscallStart;

EXTERN EXPORT KERNEL_BSS
uint8_t *krnSyscallEnd;

EXTERN EXPORT KERNEL_BSS
archsize_t krnSyscallPhys;

EXTERN EXPORT KERNEL_BSS
archsize_t krnSyscallSize;

EXTERN EXPORT KERNEL_BSS
uint8_t *krnStabStart;

EXTERN EXPORT KERNEL_BSS
uint8_t *krnStabEnd;

EXTERN EXPORT KERNEL_BSS
archsize_t krnStabPhys;

EXTERN EXPORT KERNEL_BSS
archsize_t krnStabSize;


//
// -- Initialize the Butler and perform the initial cleanup
//    -----------------------------------------------------
EXTERN_C EXPORT KERNEL
void ButlerInit(void);


//
// -- The main butler process, dispatching tasks to complete
//    ------------------------------------------------------
EXTERN_C EXPORT KERNEL NORETURN
void Butler(void);


//
// -- Check the memory to see if it is eligible to be freed
//    -----------------------------------------------------
EXTERN_C EXPORT LOADER
bool ButlerMemCheck(frame_t frame);


//
// -- The Butler has been notified of a PMM frame to clean
//    ----------------------------------------------------
void ButlerCleanPmm(void);


//
// -- The Butler has been notified of a Process to clean
//    --------------------------------------------------
void ButlerCleanProcess(void);


