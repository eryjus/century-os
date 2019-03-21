//===================================================================================================================
//
//  timer.h -- The structures and function prototypes for the PIT timer
//
//        Copyright (c)  2017-2019 -- Adam Clark
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


#ifndef __TIMER_H__
#define __TIMER_H__


#include "types.h"


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

    void (*TimerCallBack)(isrRegs_t *reg);
    void (*TimerInit)(struct TimerDevice_t *, uint32_t);
    void (*TimerEoi)(struct TimerDevice_t *);
    void (*TimerPlatformTick)(struct TimerDevice_t *);
    uint64_t (*TimerCurrentCount)(struct TimerDevice_t *);
} TimerDevice_t;


//
// -- The global timer control structure holding pointers to all the proper functions.  Before we reference this
//    global variable, we are already in the kernel.  There is no need for a loader version of this device.
//    ----------------------------------------------------------------------------------------------------------
extern TimerDevice_t timerControl;


//
// -- These are the common interface functions we will use to interact with the timer.  These functions are
//    not safe in that they will not check for nulls before calling the function.  Therefore, caller beware!
//    ------------------------------------------------------------------------------------------------------
inline void TimerInit(TimerDevice_t *dev, uint32_t freq) { dev->TimerInit(dev, freq); }
inline void TimerEoi(TimerDevice_t *dev) { dev->TimerEoi(dev); }
inline void TimerPlatformTick(TimerDevice_t *dev) { dev->TimerPlatformTick(dev); }
inline uint64_t TimerCurrentCount(TimerDevice_t *dev) { return dev->TimerCurrentCount(dev); }


//
// -- Here are the function prototypes that the operation functions need to conform to
//    --------------------------------------------------------------------------------
__CENTURY_FUNC__ void TimerCallBack(isrRegs_t *reg);
__CENTURY_FUNC__ void _TimerInit(TimerDevice_t *dev, uint32_t freq);
__CENTURY_FUNC__ void _TimerEoi(TimerDevice_t *dev);
__CENTURY_FUNC__ void _TimerPlatformTick(TimerDevice_t *dev);
__CENTURY_FUNC__ uint64_t _TimerCurrentCount(TimerDevice_t *dev);


#endif
