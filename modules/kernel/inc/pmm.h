//===================================================================================================================
//
//  pmm.h -- The Physical Memory Manager header
//
//        Copyright (c)  2017-2020 -- Adam Clark
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
//  2020-Apr-12   #405    v0.6.1c  ADCL  Redesign the PMM to store the stack in the freed frames themselves
//
//===================================================================================================================


#pragma once


#define __PMM_H__


#include "types.h"
#include "lists.h"
#include "printf.h"
#include "hw-disc.h"
#include "spinlock.h"
#include "mmu.h"
#include "pmm-msg.h"


//
// -- This is the new PMM frame information structure -- contains info about this block of frames
//    -------------------------------------------------------------------------------------------
typedef struct PmmFrameInfo_t {
    frame_t frame;
    size_t count;
    frame_t prev;
    frame_t next;
} PmmFrameInfo_t;



//
// -- This is the new PMM itself
//    --------------------------
typedef struct Pmm_t {
    AtomicInt_t framesAvail;                // -- this is the total number of frames available in the 3 stacks

    Spinlock_t lowLock;                     // -- This lock protects lowStack
    PmmFrameInfo_t *lowStack;

    Spinlock_t normLock;                    // -- This lock protects normStack
    PmmFrameInfo_t *normStack;

    Spinlock_t scrubLock;                   // -- This lock protects scrubStack
    PmmFrameInfo_t *scrubStack;

    Spinlock_t searchLock;                  // -- This lock protects search only; get lowLock or normLock also
    PmmFrameInfo_t *search;

    Spinlock_t insertLock;                  // -- Protects insert only; one of low-, norm-, or scrubLock as well
    PmmFrameInfo_t *insert;
} Pmm_t;


//
// -- This variable is the actual Physical Memory Manager data
//    --------------------------------------------------------
EXTERN EXPORT KERNEL_DATA
Pmm_t pmm;


//
// -- Has the PMM been initialized properly for use?
//    ----------------------------------------------
EXTERN EXPORT KERNEL_DATA
bool pmmInitialized;


//
// -- The early frame initialization
//    ------------------------------
EXTERN EXPORT LOADER_DATA
archsize_t earlyFrame;


//
// -- Pop a node off the stack; stack must be locked to call this function
//    --------------------------------------------------------------------
void PmmPop(PmmFrameInfo_t *stack);


//
// -- Push a new node onto the stack; stack must be locked to call this function
//    --------------------------------------------------------------------------
void PmmPush(PmmFrameInfo_t *stack, frame_t frame, size_t count);


//
// -- add the frames to an existing block if possible, returning if the operation was successful
//    ------------------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void PmmAddToStackNode(Spinlock_t *lock, PmmFrameInfo_t *stack, frame_t frame, size_t count);


//
// -- This is the worker function to find a block and allocate it
//    -----------------------------------------------------------
EXTERN_C EXPORT KERNEL
frame_t PmmDoAllocAlignedFrames(Spinlock_t *lock, PmmFrameInfo_t *stack, const size_t count, const size_t bitAlignment);


//
// -- This is the worker function to find a block and allocate it
//    -----------------------------------------------------------
EXTERN_C EXPORT KERNEL
frame_t _PmmDoRemoveFrame(PmmFrameInfo_t *stack, bool scrub);


//
// -- Allocate a frame from the pmm
//    -----------------------------
EXTERN_C EXPORT KERNEL
frame_t PmmAllocateFrame(void);


//
// -- Allocate a frame from low memory in the pmm
//    -------------------------------------------
EXTERN_C EXPORT INLINE
frame_t PmmAllocateLowFrame(void) {
    frame_t rv;
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(pmm.lowLock) {
        rv = _PmmDoRemoveFrame(pmm.lowStack, false);
        SPINLOCK_RLS_RESTORE_INT(pmm.lowLock, flags);
    }

    return rv;
}



//
// -- Allocate a block of aligned frames; bitAlignment is the significance of the alignment (min is 12 bits)
//    ------------------------------------------------------------------------------------------------------
EXTERN_C EXPORT INLINE
frame_t PmmAllocAlignedFrames(const size_t count, const size_t bitAlignment) {
    return PmmDoAllocAlignedFrames(&pmm.normLock, pmm.normStack, count, bitAlignment);
}


//
// -- Same as above but from low mem; bitAlignment is significance of the alignment (min is 12 bits)
//    ----------------------------------------------------------------------------------------------
EXTERN_C EXPORT INLINE
frame_t PmmAllocAlignedLowFrames(const size_t count, const size_t bitAlignment) {
    return PmmDoAllocAlignedFrames(&pmm.lowLock, pmm.lowStack, count, bitAlignment);
}


//
// -- Release a block of frames (very useful during initialization)
//    -------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void PmmReleaseFrameRange(const frame_t frame, const size_t count);


//
// -- Release a single frame
//    ----------------------
EXTERN_C EXPORT INLINE
void PmmReleaseFrame(const frame_t frame) { return PmmReleaseFrameRange(frame, 1); }


//
// -- Scrub a frame in preparation the next allocation (includes clearing the frame)
//    ------------------------------------------------------------------------------
EXTERN_C EXPORT INLINE
void PmmScrubFrame(const frame_t frame) { MmuClearFrame(frame); }


//
// -- This is the function to scrub a single block from the scrubStack
//    ----------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void PmmScrubBlock(void);


//
// -- Initialize the PMM
//    ------------------
EXTERN_C EXPORT LOADER
void PmmInit(void);


//
// -- Allocate an early frame before the PMM is put in charge
//    -------------------------------------------------------
EXTERN_C EXPORT ENTRY
frame_t NextEarlyFrame(void);


//
// -- Clean/Invalidate PMM Manager structure
//    --------------------------------------
#define CLEAN_PMM()                 CleanCache((archsize_t)&pmm, sizeof(Pmm_t))
#define INVALIDATE_PMM()            InvalidateCache(&pmm, sizeof(Pmm_t))


