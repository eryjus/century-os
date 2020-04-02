//===================================================================================================================
//
//  timer.h -- The structures and function prototypes for the PIT timer
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Note that the use of the PIT is dependent on the use ouf the PIC (Programmable Interrupt Controller)
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-28  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#pragma once


#define __TIMER_H__


#include "types.h"


struct TimerDevice_t;


//
// -- get the platform-specific definitions
//    -------------------------------------
#if __has_include("platform-timer.h")
#   include "platform-timer.h"
#endif


#include "pic.h"


//
// -- This is a control structure for the timer, all functions will be registered in this structure
//    ---------------------------------------------------------------------------------------------
typedef struct TimerDevice_t {
    TimerBase_t base;
    PicDevice_t *pic;
    archsize_t reloadValue;
    float factor;

    void (*TimerCallBack)(isrRegs_t *reg);
    void (*TimerInit)(struct TimerDevice_t *, uint32_t);
    void (*TimerEoi)(struct TimerDevice_t *);
    void (*TimerPlatformTick)(struct TimerDevice_t *);
    uint64_t (*TimerCurrentCount)(struct TimerDevice_t *);
} TimerDevice_t;


//
// -- The global timer control structure holding pointers to all the proper functions.
//    --------------------------------------------------------------------------------
EXTERN KERNEL_DATA
TimerDevice_t *timerControl;


//
// -- These are the common interface functions we will use to interact with the timer.  These functions are
//    not safe in that they will not check for nulls before calling the function.  Therefore, caller beware!
//    ------------------------------------------------------------------------------------------------------
EXPORT INLINE
void TimerInit(TimerDevice_t *dev, uint32_t freq) { dev->TimerInit(dev, freq); }

EXPORT INLINE
void TimerEoi(TimerDevice_t *dev) { dev->TimerEoi(dev); }

EXPORT INLINE
void TimerPlatformTick(TimerDevice_t *dev) { dev->TimerPlatformTick(dev); }

EXPORT INLINE
uint64_t TimerCurrentCount(TimerDevice_t *dev) { return dev->TimerCurrentCount(dev); }


//
// -- Here are the function prototypes that the operation functions need to conform to
//    --------------------------------------------------------------------------------
EXTERN_C EXPORT KERNEL
void TimerCallBack(isrRegs_t *reg);

EXTERN_C EXPORT KERNEL
void _TimerInit(TimerDevice_t *dev, uint32_t freq);

EXTERN_C EXPORT KERNEL
void _TimerEoi(TimerDevice_t *dev);

EXTERN_C EXPORT KERNEL
void _TimerPlatformTick(TimerDevice_t *dev);

EXTERN_C EXPORT KERNEL
uint64_t _TimerCurrentCount(TimerDevice_t *dev);


//
// -- Pick the correct Timer given what we have available
//    ---------------------------------------------------
EXTERN_C EXPORT LOADER
TimerDevice_t *TimerPick(void);

