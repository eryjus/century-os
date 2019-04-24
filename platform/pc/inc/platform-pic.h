//===================================================================================================================
//
//  platform-pic.h -- Programmable Interrupt Controller definitions and functions for the x86
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
#   error "Use #include \"pic.h\" and it will pick up this file; do not #include this file directly."
#endif


//
// -- on x86, this is the type we use to refer to the pic port
//    --------------------------------------------------------
typedef uint16_t PicBase_t;


//
// -- These are the possible pic drivers for the computer
//    ---------------------------------------------------
extern struct PicDevice_t pic8259;
extern struct PicDevice_t apicDriver;

//
// -- This is the base location of the timer on x86
//    ---------------------------------------------
#define PIC1         0x20
#define PIC2         0xa0


//
// -- Here are the offsets for the different ports of interest
//    --------------------------------------------------------
#define PIC_MASTER_COMMAND      0x00
#define PIC_MASTER_DATA         0x01
#define PIC_SLAVE_COMMAND       0x00
#define PIC_SLAVE_DATA          0x01


//
// -- Here are the function prototypes that the operation functions need to conform to
//    --------------------------------------------------------------------------------
__CENTURY_FUNC__ void _PicInit(PicDevice_t *dev, const char *name);
__CENTURY_FUNC__ void _PicUnmaskIrq(PicDevice_t *dev, int irq);
__CENTURY_FUNC__ void _PicMaskIrq(PicDevice_t *dev, int irq);
__CENTURY_FUNC__ void _PicEoi(PicDevice_t *dev, int irq);


