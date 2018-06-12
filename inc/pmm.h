//===================================================================================================================
//
//  inc/pmm.h -- The Physical Memory Manager header
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-06-10  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __PMM_H__
#define __PMM_H__


#include "types.h"
#include "hw-disc.h"


//
// -- Some quick worker functions to perform some type conversions
//    ------------------------------------------------------------
inline ptrsize_t PmmFrameToLinear(frame_t f) { return ((ptrsize_t)(f << 12)); }
inline frame_t PmmLinearToFrame(ptrsize_t a) { return ((frame_t)(a >> 12)); }


//
// -- Mark a frame as free (set the flag)
//    -----------------------------------
inline void PmmFreeFrame(frame_t f) { GetPmmBitmap()[f >> 5] |= (1 << (f & 0x1f)); }


//
// -- Mark a frame as allocated (clear the flag)
//    ------------------------------------------
inline void PmmAllocFrame(frame_t f) { GetPmmBitmap()[f >> 5] &= (~(1 << (f & 0x1f))); }


//
// -- Returns if a frame is allocated
//    -------------------------------
inline bool PmmIsFrameAlloc(frame_t f) { return (GetPmmBitmap()[f >> 5] & (1 << (f & 0x1f))) == 0; }


//
// -- Mark a range of frames as free (set the flag)
//    ---------------------------------------------
inline void PmmFreeFrameRange(frame_t f, size_t l) { for (frame_t end = f + l; f < end; f ++) PmmFreeFrame(f); }


//
// -- Mark a range of frames as allocated (clear the flag)
//    ----------------------------------------------------
inline void PmmAllocFrameRange(frame_t f, size_t l) { for (frame_t end = f + l; f < end; f ++) PmmAllocFrame(f); }


//
// -- Initialize the PMM
//    ------------------
void PmmInit(void);


#endif