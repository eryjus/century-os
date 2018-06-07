;====================================================================================================================
;
; kernel/inc/i686/arch-console.inc -- Some definitions that control the console layout for the i686 architecture.
;
; These defines will help control the definitions for the console screen.
; 
; ------------------------------------------------------------------------------------------------------------------
;
; IMPORTANT PROGRAMMING NOTE:
; The constants in this file are also duplicated in arch-console.h for the C-language counterpart to this
; file.  EVERY change in this file MUST be reflected in the other and vice-versa.  If not, undesirable results
; will occur.
;
; ------------------------------------------------------------------------------------------------------------------
;                                                                                                                 
;     Date     Tracker  Version  Pgmr  Description                                                                         
;  ----------  -------  -------  ----  ---------------------------------------------------------------------------
;  2018-05-26  Initial   0.1.0   ADCL  Initial version
;
;====================================================================================================================


CONSOLE_ROWS        equ     80
CONSOLE_COLS        equ     24
CONSOLE_VIDEO       equ     0xb8000
CONSOLE_CLEAR       equ     0x0700
