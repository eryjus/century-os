//===================================================================================================================
// kernel/src/x86-common/ConsolePaintChar.cc -- Poke a character and its attribute onto the screen
//
// Poke a character and its attribute onto the screen, checking to make sure we are poking a legitimate location.
// This functions is x86 specific since we are dealing with a text-based console.
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
// -- trivially "poke" a character and its attribute into the video buffer
//    --------------------------------------------------------------------
void ConsolePaintChar(const uint16_t row, const uint16_t col, const uint8_t attr, const uint8_t byte)
{
	if (row > CONSOLE_ROWS || col > CONSOLE_COLS) return;
	
    // -- put the character and then the attribute on the screen
	*(CONSOLE_VIDEO + (col + row * CONSOLE_COLS) * 2) = byte;
	*(CONSOLE_VIDEO + (col + row * CONSOLE_COLS) * 2 + 1) = attr;
}
