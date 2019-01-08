;;===================================================================================================================
;;
;;  kMemSetW.s -- Set a block of memory to the specified value by words
;;
;;        Copyright (c)  2017-2019 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  Set a block of memory to the specified value.  This function operates with words being passed into the function,
;;  so cnt contains the number of words to fill.
;;
;;  On entry, the stack has the following structure:
;;  +-----------+-----------+------------------------------------+
;;  |  via ebp  |  via esp  |  description of the contents       |
;;  +-----------+-----------+------------------------------------+
;;  | ebp + 16  | esp + 12  |  Number of words to set            |
;;  +-----------+-----------+------------------------------------+
;;  | ebp + 12  | esp + 08  |  The word to set in the memory     |
;;  +-----------+-----------+------------------------------------+
;;  | ebp + 08  | esp + 04  |  The memory location to set        |
;;  +-----------+-----------+------------------------------------+
;;  | ebp + 04  |   esp     |  Return EIP                        |
;;  +-----------+-----------+------------------------------------+
;;  |   ebp     | esp - 04  |  EBP                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 04  | esp - 08  |  EAX                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 08  | esp - 12  |  ECX                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 12  | esp - 16  |  EDI                               |
;;  +-----------+-----------+------------------------------------+
;;
;;  Prototype:
;;  void kMemSetW(void *buf, uint16_t wrd, size_t cnt);
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2012-May-24                          Initial version
;;  2012-May-24    #21                   Created function as a tool to resolve defect 21
;;  2012-Sep-15                          Leveraged from Century Code
;;  2018-May-28  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
;;
;;===================================================================================================================


;;
;; -- Expose labels to fucntions that the linker can pick up
;;    ------------------------------------------------------
	global	kMemSetW


;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section .text
cpu		586


;;
;; -- Clear or set a block of memory to the specified value
;;    -----------------------------------------------------
kMemSetW:
	push	ebp						                        ;; create a frame
	mov		ebp,esp					                        ;; ... create a frame
	push	eax						                        ;; save eax
	push	ecx						                        ;; save ecx
	push	edi						                        ;; save edi

	mov		eax,[ebp+8]				                        ;; get the memory location to set
	mov		edi,eax					                        ;; and put it in edi
	mov		ecx,[ebp+16]			                        ;; get the number of bytes to set
	mov		ax,[ebp+12]				                        ;; get the word to set
	cld								                        ;; make sure we are incrementing
	rep		stosw					                        ;; store the word

	pop		edi						                        ;; restore edi
	pop		ecx						                        ;; restore ecx
	pop		eax						                        ;; restore eax
	pop		ebp						                        ;; restore previous frame
	ret
