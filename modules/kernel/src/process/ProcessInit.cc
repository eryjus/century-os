//===================================================================================================================
//
// ProcessInit.h -- Initialize the process structures
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-28  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "process.h"


//
// -- Initialize the process structures
//    ---------------------------------
void ProcessInit(void)
{
    ListInit(&butler.stsQueue);
    butler.lockList.lock = {0};
    butler.messages.lock = {0};
    ListInit(&butler.lockList.list);
    ListInit(&butler.messages.list);

    // -- no lock needed during initialization
    ListAddTail(&procOsPtyList, &butler.stsQueue);

    procs[0] = &butler;
    currentPID = 0;
}
