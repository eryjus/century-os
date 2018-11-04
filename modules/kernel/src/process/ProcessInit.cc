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
    ListInit(&butler.lockList);
    ListInit(&butler.messages);

    // -- no lock needed during initialization
    ListAddTail(&procIdlePtyList, &butler.stsQueue);

    procs[0] = &butler;
    currentPID = 0;
}
