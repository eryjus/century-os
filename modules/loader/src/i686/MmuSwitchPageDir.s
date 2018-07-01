;;===================================================================================================================
;; loader/src/i686/MmuSwitchPageDir.s -- Working function to set paging directory and enable paging
;;
;; Working function to set paging directory and enable paging.
;;
;;        Copyright (c)  2017-2018 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;    Date     Tracker  Version  Pgmr  Description
;; ----------  -------  -------  ----  ---------------------------------------------------------------------------
;; 2012-07-21                          Initial Version
;; 2012-09-16                          Leveraged from Century
;; 2018-07-01  Initial   0.1.0   ADCL  Copied this file from century32 to centiry-os
;;
;;===================================================================================================================


;;
;; -- Expose some global addresses
;;    ----------------------------
global	MmuSwitchPageDir


;;
;; -- This is the begging of the code segment
;;    ---------------------------------------
section ._text
cpu		586


;;
;; -- Change the page Diretory to the address provided
;;    ------------------------------------------------
MmuSwitchPageDir:
	mov		eax,[esp+4]				; get the paging physical address
	mov		cr3,eax					; and store it in eax

	mov		eax,cr0					; now, get eax
	or		eax,0x80000000			; and turn on the paging bit, bit 31
	mov		cr0,eax					; and store it back in cr0

	ret