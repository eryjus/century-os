//===================================================================================================================
//
// kernel/inc/i686/types.h -- Type definitions specific to i686 architectures
//
// These types are architecture dependent.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-05-25  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __TYPES_H__
#error "Do not include 'arch-types.h' directly; include 'types.h' instead, which will pick up this file."
#endif


//
// -- This is the address width size for this architecture
//    ----------------------------------------------------
typedef uint32_t ptrsize_t;


//
// -- This is the size of a frame for the PMM (which is tied to the address width for this architecture)
//    --------------------------------------------------------------------------------------------------
typedef uint32_t frame_t;


//
// -- This is the size of a general purpose register in this architecture
//    -------------------------------------------------------------------
typedef uint32_t regval_t;
