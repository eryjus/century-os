//===================================================================================================================
// kernel/src/ConsolePutChar.cc -- Put a character on the screen at the current location
//
// Put a character on the screen at the current cursor location and advance the cursor position. 
//
// This function is written to be aligned closely with the C library function putchar().  The intent is that any
// place putchar() can be used, ConsolePutChar can be used in its place.  I do not want to duplicate the function 
// name because it might create confusion for the reader on whether the library files are included or not.
//
// ------------------------------------------------------------------------------------------------------------------
//                                                                                                                 
//     Date     Tracker  Version  Pgmr  Description                                                                         
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-09-15  Initial                 Initial version -- leveraged from Royalty
//  2013-08-21    #57                   Align the kConsole & tty screen handling
//  2013-09-12   #101                   Resolve issues splint exposes
//  2013-09-12    #61                   HeapDump() and other functions do not handle tty properly
//  2018-05-26  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
//  2018-05-27   #347     0.1.0   ADCL  Turn this into an architecture-independent function
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "console.h"


//
// -- declare a couple of variables that are not public but we want here
//    ------------------------------------------------------------------
uint16_t _colPos;
uint16_t _rowPos;


//
// -- some external function prototypes that are not global
//    -----------------------------------------------------
extern "C" void ConsoleScrollUp(void);
extern "C" void ConsoleSetCursorPos(const uint16_t row, const uint16_t col);


//
// -- Put a single character on the screen
//    ------------------------------------
void ConsolePutChar(int c)
{
	regval_t flags = DisableInterrupts();
	
	if (c == '\n' || c == '\r') {
newline:
		_colPos = 0;
		_rowPos ++;
		if (_rowPos >= CONSOLE_ROWS) {
			ConsoleScrollUp();
			_rowPos = CONSOLE_ROWS - 1;
			_colPos = 0;
		}
		
		goto out;
	}
	
	ConsolePaintChar(_rowPos, _colPos ++, CONSOLE_ATTR, (unsigned char)(c & 0xff));
	
	if (_colPos >= CONSOLE_COLS) goto newline;
	
out:
	ConsoleSetCursorPos(_rowPos, _colPos);
	RestoreInterrupts(flags);
}
