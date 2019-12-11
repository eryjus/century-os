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


#pragma once

#include "cpu.h"


//
// -- these 2 variable are provided by the linker
//    -------------------------------------------
EXTERN archsize_t phys_loc;
EXTERN archsize_t kern_loc;
EXTERN uint8_t *_loaderEnd;
EXTERN uint8_t *_loaderStart;
EXTERN archsize_t mmuLvl1Table;
EXTERN frame_t intTableAddr;


//
// -- This is a call to kMemSetB
//    --------------------------
typedef void (*kMemSetB_t)(void *buf, uint8_t wrd, size_t cnt);
EXTERN kMemSetB_t lMemSetB;


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
// -- Function prototypes
//    -------------------
extern "C" {


    //
    // -- This is the function that will allocate a frame during early initialization (< 4MB)
    //    -----------------------------------------------------------------------------------
    EXPORT LOADER frame_t NextEarlyFrame(void);

    //
    // -- function to initialize the loader functions
    //    -------------------------------------------
    EXPORT LOADER void LoaderFunctionInit(void);


    //
    // -- Early Initialization function to handle this initialization by architecture
    //    ---------------------------------------------------------------------------
    EXPORT LOADER void EarlyInit(void);


    //
    // -- Perform the MMU Early Initialization so that we can use the whole kernel
    //    source no matter where it is located
    //    ------------------------------------------------------------------------
    EXPORT LOADER void MmuEarlyInit(void);


    //
    // -- This is the loader main entry point
    //    -----------------------------------
    EXPORT LOADER void LoaderMain(archsize_t arg0, archsize_t arg1, archsize_t arg2);
}
