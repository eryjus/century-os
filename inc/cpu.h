//===================================================================================================================
//
// inc/cpu.h -- Standard CPU functions
//
// These are the common functions that need to be implemented to manage the CPU resource by the OS.
// 
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-05-25  Initial   0.1.0   ADCL  Initial version as I move functions from century32
//
//===================================================================================================================


#ifndef __CPU_H__
#define __CPU_H__


#include "types.h"


//
// -- Enable interrupts if they are disabled; assembly language function
//    ------------------------------------------------------------------
extern "C" void EnableInterrupts(void);


//
// -- Disable interrupts and return the current flags state; assembly language function
//    ---------------------------------------------------------------------------------
extern "C" regval_t DisableInterrupts(void);


//
// -- Restore the flags state back to the provided state; note all flags are updates; assembly language function
//    ----------------------------------------------------------------------------------------------------------
extern "C" void RestoreInterrupts(regval_t flg);


//
// -- Set a block of memory to the specified  word
//    --------------------------------------------
extern "C" void kMemSetW(void *buf, uint16_t wrd, size_t cnt);

#endif
