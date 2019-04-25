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
// -- A forward declaration
//    ---------------------
struct PicDevice_t;


//
// -- get the platform-specific definitions
//    -------------------------------------
#if __has_include("platform-pic.h")
#   include "platform-pic.h"
#endif

#if __has_include("platform-apic.h")
#   include "platform-apic.h"
#endif


//
// -- This is a control structure for the pic, all functions will be registered in this structure
//    -------------------------------------------------------------------------------------------
typedef struct PicDevice_t {
    GenericDevice_t device;

    void (*PicInit)(PicDevice_t *, const char *);
    isrFunc_t (*PicRegisterHandler)(PicDevice_t *, Irq_t, int, isrFunc_t);
    void (*PicMaskIrq)(PicDevice_t *, Irq_t);
    void (*PicUnmaskIrq)(PicDevice_t *, Irq_t);
    void (*PicEoi)(PicDevice_t *, Irq_t);
    int (*PicDetermineIrq)(PicDevice_t *);
} PicDevice_t;


//
// -- The global PIC control structure holding pointers to all the proper functions.  Before we reference this
//    global variable, we are already in the kernel.  There is no need for a loader version of this device.
//    ----------------------------------------------------------------------------------------------------------
extern PicDevice_t *picControl;


//
// -- These are the common interface functions we will use to interact with the PIC.  These functions are
//    not safe in that they will not check for nulls before calling the function.  Therefore, caller beware!
//    ------------------------------------------------------------------------------------------------------
inline void PicInit(PicDevice_t *dev, const char *name) { dev->PicInit(dev, name); }
inline isrFunc_t PicRegisterHandler(PicDevice_t *dev, Irq_t irq, int vector, isrFunc_t handler) {
                    return dev->PicRegisterHandler(dev, irq, vector, handler); }
inline void PicUnmaskIrq(PicDevice_t *dev, Irq_t irq) { dev->PicUnmaskIrq(dev, irq); }
inline void PicMaskIrq(PicDevice_t *dev, Irq_t irq) { dev->PicMaskIrq(dev, irq); }
inline void PicEoi(PicDevice_t *dev, Irq_t irq) { dev->PicEoi(dev, irq); }
inline archsize_t PicDetermineIrq(PicDevice_t *dev) { return dev->PicDetermineIrq(dev); }


//
// -- Pick the correct PIC given what we have available
//    -------------------------------------------------
__CENTURY_FUNC__ PicDevice_t *PicPick(void);


#endif

