//===================================================================================================================
//
// pmm.h -- The structures and functions for the Physical Memory Manager
//
//        Copyright (c)  2017-2018 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// The PMM will be managed via bitmap.  The bitmap will need to be sufficiently big enough for all the memory on
// the system, which is determined at runtime.
//
// The bitmap is constructed so that a free frame has the value 1 set for the corresponding bit and a allocated
// frame has a 0 for that bit.  This makes comparisons easy when looking for a frame: if the 32-bit value is 0,
// then we know that all the frames are allocates and can move on.  On the other hand, any value other than 0
// means we have at least 1 frame we can allocate.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-10-30  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __PMM_H__
#define __PMM_H__

#include "types.h"


//
// -- This is the number of elements for the physical memory bitmap for 4GB of memory.  The math is kind of messy,
//    so I will walk through it:
//    * 4GB = 4096MB.
//    * 1 4K frame can manage 128 MB of physical memory,  divide to get the number of frames needed.
//    * 1 frame is 4096 bytes, which will give us the number of bytes we need.
//    * Finally, we are using 32-bit unsigned integers for comparison (for speed), so divide by that.
//    The good news is that the compiler will work all this out at compile time.  Notice, however, this is
//    uninitialized memory -- though the .bss section should be cleared, assume no guarantees please.
//    ------------------------------------------------------------------------------------------------------------
#define BITMAP_SIZE     ((((4 * 1024) / 128) * 4096) / sizeof(uint32_t))


//
// -- This the number of frame we can manage, from 0 to FRAME_COUNT - 1
//    -----------------------------------------------------------------
#define FRAME_COUNT     (BITMAP_SIZE * 4096 * 8)


//
// -- This is the bitmap itself
//    -------------------------
extern uint32_t pmmBitmap[BITMAP_SIZE];


//
// -- Free a frame back to the bitmap
//    -------------------------------
int PmmFreeFrame(frame_t frame);


//
// -- Allocate the specified frame if available
//    -----------------------------------------
int PmmAllocFrame(frame_t frame);


//
// -- Allocate a new frame wherever possible
//    --------------------------------------
frame_t PmmNewFrame(void);


//
// -- Check if a frame is allocated (frame must be in range)
//    ------------------------------------------------------
inline bool PmmIsFrameAlloc(frame_t frame) { return (pmmBitmap[frame >> 5] & (1 << (frame & 0x1f))) == 0; }


//
// -- Mark a range of frames as free (set the flag)
//    ---------------------------------------------
inline void PmmFreeFrameRange(frame_t frame, size_t cnt) {
    for (frame_t end = frame + cnt; frame < end; frame ++) PmmFreeFrame(frame);
}


//
// -- Mark a range of frames as allocated (clear the flag)
//    ----------------------------------------------------
inline void PmmAllocFrameRange(frame_t frame, size_t cnt) {
    for (frame_t end = frame + cnt; frame < end; frame ++) PmmAllocFrame(frame);
}


//
// -- These are the messages that the PMM manager will respond to
//    -----------------------------------------------------------
typedef enum {
    PMM_NOOP,
    PMM_FREE_FRAME,
    PMM_ALLOC_FRAME,
    PMM_FREE_RANGE,
    PMM_ALLOC_RANGE,
    PMM_NEW_FRAME,
} PmmMessages_t;


#endif
