;====================================================================================================================
;
; kernel/src/i686/ConsoleSetCursorPos.s -- Set the porition of the cursor on the console screen
;
; Set the position of the cursor on the screen -- low level kernel function
;
; On entry, the stack has the following structure:
;  +-----------+-----------+------------------------------------+
;  |  via ebp  |  via esp  |  description of the contents       |
;  +-----------+-----------+------------------------------------+
;  | ebp + 12  | esp + 08  |  Column to move the cursor to      |
;  +-----------+-----------+------------------------------------+
;  | ebp + 08  | esp + 04  |  Row to move the cursor to         |
;  +-----------+-----------+------------------------------------+
;  | ebp + 04  |   esp     |  Return EIP                        |
;  +-----------+-----------+------------------------------------+
;  |   ebp     | esp - 04  |  EBP                               |
;  +-----------+-----------+------------------------------------+
;  | ebp - 04  | esp - 08  |  EAX                               |
;  +-----------+-----------+------------------------------------+
;  | ebp - 08  | esp - 12  |  EBX                               |
;  +-----------+-----------+------------------------------------+
;  | ebp - 12  | esp - 16  |  ECX                               |
;  +-----------+-----------+------------------------------------+
;  | ebp - 16  | esp - 20  |  EDX                               |
;  +-----------+-----------+------------------------------------+
;
;  Prototype:
;  void ConsoleSetCursorPos(const uint16_t row, const uint16_t col);
;
; ------------------------------------------------------------------------------------------------------------------
;                                                                                                                 
;     Date     Tracker  Version  Pgmr  Description                                                                         
;  ----------  -------  -------  ----  ---------------------------------------------------------------------------
;  2012-05-24                          Initial version
;  2012-05-24    #21                   Created function as a tool to resolve defect #21
;  2012-09-15                          Leveraged from Century Code
;  2013-04-16                          Bug #1: Aligned the stack with ebp (not esp)
;  2018-05-27                          Copied this file from century32 to century-os
;
;====================================================================================================================


;
; -- Expose labels to fucntions that the linker can pick up
;    ------------------------------------------------------
global		ConsoleSetCursorPos


;
; -- This is the beginning of the code segment for this file
;    -------------------------------------------------------
section .text
cpu		586


;
; -- Set the cursor position on the screen
;    -------------------------------------
ConsoleSetCursorPos:
	push	ebp				                                ; create a stack frame
	mov		ebp,esp			                                ; ... stack frame
	push	eax				                                ; save eax
	push	ebx				                                ; save ebx
	push	ecx				                                ; save ecx
	push	edx				                                ; save edx

	mov		eax,[ebp+12]		                            ; get the column
	mov		ebx,[ebp+8]		                                ; get the row

;
; -- the location will be rows * 80 + cols (we will use an optimization trick of shifting a multiplication of 64 and
;    another by 16 and adding the 2 results together -- MUCH faster than multiplying by 80)
;    ---------------------------------------------------------------------------------------------------------------
	mov		ecx,ebx			                                ; we will manipulate both
	shl		ebx,6			                                ; multiply by 64
	shl		ecx,4			                                ; multiply by 16
	add		ebx,ecx			                                ; add them together to get *80
	add		eax,ebx			                                ; add the col; eax is the address

;
; -- set up to write the LSB to the video card
;    -----------------------------------------
	mov		bx,ax			                                ; get the result
	and		bx,0x00ff		                                ; get the LSB
	shr		ax,8			                                ; move the MSB
	and		ax,0x00ff		                                ; ensure just the byte

	push	ax				                                ; save our work
	mov		dx,0x3d4		                                ; write to port 0x3d4
	mov		al,0x0e			                                ; we want register 0x0e
	out		dx,al			                                ; want to set Cursor MSB
	pop		ax				                                ; get our work back
	inc		dx				                                ; write data
	out		dx,al			                                ; set the byte

;
; -- and now the MSB
;    ---------------
	dec		dx				                                ; back to 3d4
	mov		al,0x0f			                                ; want register 0x0f
	out		dx,al			                                ; want to set cursor LSB
	inc		dx				                                ; write data
	mov		al,bl			                                ; move to al
	out		dx,al                                           ; set the byte

;
; -- clean up and exit
;    -----------------
	pop		edx				                                ; save edx
	pop		ecx				                                ; save ecx
	pop		ebx				                                ; save ebx
	pop		eax				                                ; save eax
	pop		ebp				                                ; restore previous frame

	ret
