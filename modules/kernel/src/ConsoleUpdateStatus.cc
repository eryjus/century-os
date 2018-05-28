//===================================================================================================================
// kernel/src/ConsoleUpdateStatus.cc -- Update the status line with the current contents
//
// Update the status line with the most current information.  This information is stored in a locally held array.
//
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-05-27  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "console.h"


//
// -- This is the contents of the status line -- initially blank
//    ----------------------------------------------------------
char statusLine[CONSOLE_COLS] = {0};


//
// -- Poke the chars of the status line to update them
//    ------------------------------------------------
void ConsoleUpdateStatus(void)
{
    for (uint16_t i = 0; i < CONSOLE_COLS; i ++) {
        ConsolePaintChar(CONSOLE_ROWS, i, CONSOLE_STATUS_ATTR, statusLine[i]);
    }
}
