//===================================================================================================================
//
//  pic.h -- These are the hardware abstractions for interacting with the Programmable Interrupt Controller
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-24  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __PIC_H__
#define __PIC_H__


#include "types.h"
#include "hardware.h"

//
// -- get the platform-specific definitions
//    -------------------------------------
#if __has_include("platform-pic.h")
#   include "platform-pic.h"
#endif


//
// -- This is a control structure for the timer, all functions will be registered in this structure
//    ---------------------------------------------------------------------------------------------
typedef struct PicDevice_t {
    PicBase_t base1;
    PicBase_t base2;

    void (*PicInit)(struct PicDevice_t *);
    void (*PicEnableIrq)(struct PicDevice_t *, int);
    void (*PicDisableIrq)(struct PicDevice_t *, int);
    void (*PicEoi)(struct PicDevice_t *, int);
    archsize_t (*PicGetIrq)(struct PicDevice_t *);
} PicDevice_t;


//
// -- The global PIC control structure holding pointers to all the proper functions.  Before we reference this
//    global variable, we are already in the kernel.  There is no need for a loader version of this device.
//    ----------------------------------------------------------------------------------------------------------
extern PicDevice_t picControl;


//
// -- These are the common interface functions we will use to interact with the PIC.  These functions are
//    not safe in that they will not check for nulls before calling the function.  Therefore, caller beware!
//    ------------------------------------------------------------------------------------------------------
inline void PicInit(PicDevice_t *dev) { dev->PicInit(dev); }
inline void PicEnableIrq(PicDevice_t *dev, int irq) { dev->PicEnableIrq(dev, irq); }
inline void PicDisableIrq(PicDevice_t *dev, int irq) { dev->PicDisableIrq(dev, irq); }
inline void PicEoi(PicDevice_t *dev, int irq) { dev->PicEoi(dev, irq); }
inline archsize_t PicGetIrq(PicDevice_t *dev) { return dev->PicGetIrq(dev); }


//
// -- Here are the function prototypes that the operation functions need to conform to
//    --------------------------------------------------------------------------------
extern void _PicInit(PicDevice_t *dev);
extern void _PicEnableIrq(PicDevice_t *dev, int irq);
extern void _PicDisableIrq(PicDevice_t *dev, int irq);
extern void _PicEoi(PicDevice_t *dev, int irq);
extern archsize_t _PicGetIrq(PicDevice_t *dev);


#endif

