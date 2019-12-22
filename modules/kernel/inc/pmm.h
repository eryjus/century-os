//===================================================================================================================
//
//  pmm.h -- The Physical Memory Manager header
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Jun-10  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-14  Initial   0.3.0   ADCL  Relocated
//  2019-Mar-10  Initial   0.3.1   ADCL  Rebuild the PMM to be managed by a stack
//
//===================================================================================================================


#ifndef __PMM_H__
#define __PMM_H__


#include "types.h"
#include "lists.h"
#include "printf.h"
#include "hw-disc.h"
#include "mmu.h"
#include "pmm-msg.h"


//
// -- This structure is used on the stack and scrubStack to keep track of the frame
//    -----------------------------------------------------------------------------
typedef struct PmmBlock_t {
    ListHead_t::List_t list;
    frame_t frame;
    size_t count;
} PmmBlock_t;


//
// -- This structure (there is only one of these on the system) manages all the PMM
//    -----------------------------------------------------------------------------
typedef struct PmmManager_t {
    StackHead_t normalStack;
    StackHead_t lowStack;
    StackHead_t scrubStack;
} PmmManager_t;


//
// -- This variable is the actual Physical Memory Manager data
//    --------------------------------------------------------
EXTERN EXPORT KERNEL_DATA
PmmManager_t pmm;


//
// -- Has the PMM been initialized properly for use?
//    ----------------------------------------------
EXTERN EXPORT KERNEL_DATA
bool pmmInitialized;

//
// -- add the frames to an existing block if possible, returning if the operation was successful
//    ------------------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
bool _PmmAddToStackNode(StackHead_t *stack, frame_t frame, size_t count);


//
// -- This is the worker function to find a block and allocate it
//    -----------------------------------------------------------
EXTERN_C EXPORT KERNEL
frame_t _PmmDoAllocAlignedFrames(StackHead_t *stack, const size_t count, const size_t bitAlignment);


//
// -- This is the worker function to find a block and allocate it
//    -----------------------------------------------------------
EXTERN_C EXPORT KERNEL
frame_t _PmmDoRemoveFrame(StackHead_t *stack, bool scrub);


//
// -- Allocate a frame from the pmm
//    -----------------------------
EXTERN_C EXPORT KERNEL
frame_t PmmAllocateFrame(void);


//
// -- Allocate a frame from low memory in the pmm
//    -------------------------------------------
EXTERN_C EXPORT KERNEL
inline frame_t PmmAllocateLowFrame(void) { return _PmmDoRemoveFrame(&pmm.lowStack, false); }



//
// -- Allocate a block of aligned frames; bitAlignment is the significance of the alignment (min is 12 bits)
//    ------------------------------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
inline frame_t PmmAllocAlignedFrames(const size_t count, const size_t bitAlignment) {
    return _PmmDoAllocAlignedFrames(&pmm.normalStack, count, bitAlignment);
}


//
// -- Same as above but from low mem; bitAlignment is significance of the alignment (min is 12 bits)
//    ----------------------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
inline frame_t PmmAllocAlignedLowFrames(const size_t count, const size_t bitAlignment) {
    return _PmmDoAllocAlignedFrames(&pmm.lowStack, count, bitAlignment);
}


//
// -- Release a block of frames (very useful during initialization)
//    -------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void PmmReleaseFrameRange(const frame_t frame, const size_t count);


//
// -- Release a single frame
//    ----------------------
EXTERN_C EXPORT KERNEL
inline void PmmReleaseFrame(const frame_t frame) { return PmmReleaseFrameRange(frame, 1); }


//
// -- Scrub a frame in preparation the next allocation (includes clearing the frame)
//    ------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
inline void PmmScrubFrame(const frame_t frame) { MmuClearFrame(frame); }


//
// -- This is the function to scrub a single block from the scrubStack
//    ----------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void __krntext PmmScrubBlock(void);


//
// -- Initialize the PMM
//    ------------------
EXTERN_C EXPORT LOADER
void PmmInit(void);


//
// -- For debugging purposes, dump the state of the PMM manager
//    ---------------------------------------------------------
EXTERN_C EXPORT KERNEL
void PmmDumpState(void);


//
// -- Allocate an early frame before the PMM is put in charge
//    -------------------------------------------------------
EXTERN_C EXPORT ENTRY
frame_t NextEarlyFrame(void);


//
// -- Clean/Invalidate PMM Manager structure
//    --------------------------------------
#define CLEAN_PMM()                 CLEAN_CACHE(&pmm, sizeof(PmmManager_t))
#define INVALIDATE_PMM()            INVALIDATE_CACHE(&pmm, sizeof(PmmManager_t))


//
// -- Clean/Invalidate PMM Block Structure
//    ------------------------------------
#define CLEAN_PMM_BLOCK(blk)        CLEAN_CACHE(blk, sizeof(PmmBlock_t))
#define INVALIDATE_PMM_BLOCK(blk)   INVALIDATE_CACHE(blk, sizeof(PmmBlock_t))


#endif