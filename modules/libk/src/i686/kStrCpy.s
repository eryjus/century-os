;;===================================================================================================================
;;
;; libk/src/i686/kStrCpy.s -- copy a string to a buffer
;;
;; copy a string into a buffer.  The buffer must be guaranteed to be big enough to hold the string
;;
;;
;; Prototype:
;; void kStrCpy(char *dest, char *src);
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;    Date     Tracker  Version  Pgmr  Description
;; ----------  -------  -------  ----  ---------------------------------------------------------------------------
;; 2017-10-12                          Initial version
;; 2018-06-23  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
;;
;;===================================================================================================================

;
; -- Expose labels to fucntions that the linker can pick up
;    ------------------------------------------------------
global kStrCpy


;
; -- This is the beginning of the code segment for this file
;    -------------------------------------------------------
section ._text
cpu		586


;
; -- Copy a string to a new location
;    -------------------------------
kStrCpy:
	cld
	push	esi				                                ; save this register
	push	edi				                                ; and this one

	mov		edi,[esp+12]	                                ; get the destination
	mov		esi,[esp+16]	                                ; get the string

.loop:
	cmp		byte [esi],0	                                ; is the string over?
	je		.out			                                ; if so, leave

	mov		al,[esi]		                                ; get the character
	mov		[edi],al		                                ; and set the new character

	inc		esi				                                ; move to the next character
	inc		edi				                                ; and the next loc in the buffer
	jmp		.loop			                                ; and do it again

.out:
	mov		byte [edi],0	                                ; write terminating null
	mov		eax,[esp+12]	                                ; set the return value

	pop		edi				                                ; restore the register
	pop		esi				                                ; and this one
	ret
