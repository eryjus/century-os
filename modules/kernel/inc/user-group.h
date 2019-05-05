//===================================================================================================================
//
//  user-group.h -- The interface into users and groups and permissions thereof
//
//        Copyright (c)  2017-2019 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2019-May-01  Initial   0.4.3   ADCL  Initial version
//
//===================================================================================================================


#ifndef __USER_GROUP__
#define __USER_GROUP__


#include "types.h"


//
// -- This is a quick macro to check for permissions -- currently everyone has all permissions
//    ----------------------------------------------------------------------------------------
#define HasReadPermission(...)       (true)
#define HasWritePermission(...)      (true)
#define IsCreatorOwner(...)          (true)


//
// -- Get the current UID for the user of this process
//    ------------------------------------------------
#define GetUid()                    (0)



#endif


