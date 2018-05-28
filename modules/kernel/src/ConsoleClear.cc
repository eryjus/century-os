//===================================================================================================================
// kernel/src/ConsoleClear.cc -- Clear the screen
// 
// Clear the screen -- this is used at the low level kernel only, and is not a tty function
//
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-09-15                          Initial version -- leveraged from Royalty
//  2013-09-01                          Add a mutex for screen operations (removed 2018-05-28)
//  2013-09-12    #61                   HeapDump() and other functions do not handle tty properly
//  2018-05-28  Initial   0.1.0   ADCL  Copy this file from century32 to century-os
//
//===================================================================================================================

#include "types.h"
#include "cpu.h"
#include "console.h"


//
// -- Declare some external resources we will use in this function
//    ------------------------------------------------------------
extern uint16_t _colPos;
extern uint16_t _rowPos;

extern "C" void ConsoleSetCursorPos(const uint16_t row, const uint16_t col);


//
// -- Clear the screen
//    ----------------
void ConsoleClear (void)
{
	kMemSetW(CONSOLE_VIDEO, CONSOLE_CLEAR, CONSOLE_COLS * CONSOLE_ROWS);
    
	_colPos = 0;
	_rowPos = 0;
	ConsoleSetCursorPos(_rowPos, _colPos);
}
