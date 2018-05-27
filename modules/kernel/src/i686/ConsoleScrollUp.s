;====================================================================================================================
;
; kernel/src/i686/ConsoleScrollUp.s -- Scroll the screen up one line
;
; Scroll the screen up one line -- low level kernel function
;
;  Prototype:
;  void ConsoleScrollUp(void);
;
; ------------------------------------------------------------------------------------------------------------------
;                                                                                                                 
;     Date     Tracker  Version  Pgmr  Description                                                                         
;  ----------  -------  -------  ----  ----------------------------------------------------------------------------
;  2012-05-24                          Initial version
;  2012-05-24                          Created function as a tool to resolve defect #21
;  2012-09-15                          Leveraged from Royalty Code
;  2013-08-21    #57                   Align the kConsole & tty screen handling
;  2013-09-01    #82                   Add a mutex for screen operations (removed: 2018-05-26)
;
;====================================================================================================================


%include "arch-console.inc"


;
; -- Expose labels to functions that the linker can pick up
;    ------------------------------------------------------
global		ConsoleScrollUp


;
; -- This is the beginning of the code segment for this file
;    -------------------------------------------------------
section .text
cpu		586


;
; -- Scroll the screen up a row and clear the bottom line
;    ----------------------------------------------------
ConsoleScrollUp:
	push	edi						                        ; save edi
	push	esi						                        ; save esi
	push	ecx						                        ; save ecx
	push	eax						                        ; save eax
	pushfd							                        ; save flags
	cli								                        ; disable interrupts for a moment

	mov		edi,CONSOLE_VIDEO		                        ; need to work with the screen
	mov		esi,CONSOLE_VIDEO+(CONSOLE_COLS*2)	            ; our source is from line #2 (0-based)
	mov		ecx,(CONSOLE_ROWS-1)*CONSOLE_COLS               ; scroll the right number of rows
	cld								                        ; make sure we increment
	rep		movsw					                        ; move the words

	mov		ecx,CONSOLE_COLS		                        ; now, prepare to blank the last line
	mov		ax,CONSOLE_CLEAR		                        ; with this word
	rep		stosw					                        ; blank the line

	popfd							                        ; restore the flags
	pop		eax						                        ; restore eax
	pop		ecx						                        ; restore ecx
	pop		esi						                        ; restore esi
	pop		edi						                        ; restore edi

	ret
