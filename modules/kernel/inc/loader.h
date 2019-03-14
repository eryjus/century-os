//===================================================================================================================
//
//  loader.h -- These are the prototypes for function renames for the loader to call at physical locations
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-Feb-10  Initial   0.3.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __LOADER_H__
#define __LOADER_H__


#include "cpu.h"


//
// -- these 2 variable are provided by the linker
//    -------------------------------------------
extern archsize_t phys_loc;
extern archsize_t kern_loc;
extern uint8_t *_loaderEnd;
extern uint8_t *_loaderStart;
extern frame_t pmmEarlyFrame;
extern frame_t pmmEarlyFrameSave;
extern archsize_t mmuLvl1Table;
extern frame_t intTableAddr;


//
// -- This is a call to kMemSetB
//    --------------------------
typedef void (*kMemSetB_t)(void *buf, uint8_t wrd, size_t cnt);
extern kMemSetB_t lMemSetB;


//
// -- This macro is intended to generic enough to convert a virtual address to a physical one.  However,
//    keep in mind that this works on one address only.  Therefore if a function calls another function,
//    this macro will fix the first one, but not the deeper call.
//    --------------------------------------------------------------------------------------------------
#define PHYS_OF(f)  ((archsize_t)(f) - kern_loc + phys_loc + ((archsize_t)&_loaderEnd - (archsize_t)&_loaderStart))


//
// -- This is a special-case macro to print a string to the serial port during early initialization
//    ---------------------------------------------------------------------------------------------
#define LoaderSerialPutS(x)     SerialPutS(&loaderSerial, (char *)PHYS_OF(x))
#define LoaderSerialPutHex(x)   SerialPutHex(&loaderSerial, x)
#define LoaderSerialPutChar(c)  SerialPutChar(&loaderSerial, c)


//
// -- This is the function that will allocate a frame during early initialization (< 4MB)
//    -----------------------------------------------------------------------------------
__CENTURY_FUNC__ frame_t NextEarlyFrame(void);

//
// -- function to initialize the loader functions
//    -------------------------------------------
__CENTURY_FUNC__ void LoaderFunctionInit(void);


//
// -- Early Initialization function to handle this initialization by architecture
//    ---------------------------------------------------------------------------
__CENTURY_FUNC__ void EarlyInit(void);


//
// -- Perform the MMU Early Initialization so that we can use the whole kernel source no matter where it is located
//    -------------------------------------------------------------------------------------------------------------
__CENTURY_FUNC__ void MmuEarlyInit(void);



#endif
