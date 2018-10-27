;;===================================================================================================================
;;
;; ProcessSwitch.s -- Execute a task switch at the lowest switch
;;
;; This function will perform a voluntary task switch, as opposed to a preemptive switch.  Therefore, there will
;; never need to have an EOI sent to the PIC/APIC.
;;
;;  The following represent the stack structure based on the esp/frame set up at entry:
;;  +-----------+-----------+------------------------------------+
;;  |    EBP    |    ESP    |  Point of reference                |
;;  +-----------+-----------+------------------------------------+
;;  | ebp + 12  | esp + 08  |  The current Process_t structure   |
;;  +-----------+-----------+------------------------------------+
;;  | ebp + 08  | esp + 04  |  The target Process_t structure    |
;;  +-----------+-----------+------------------------------------+
;;  | ebp + 04  |   esp     |  Return EIP                        |
;;  +-----------+-----------+------------------------------------+
;;  |   ebp     | esp - 04  |  Frame pointer EBP                 |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 04  | esp - 08  |  EFLAGS                            |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 08  | esp - 12  |  EAX                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 12  | esp - 16  |  EBX                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 16  | esp - 20  |  ECX                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 20  | esp - 24  |  EDX                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 24  | esp - 28  |  ESI                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 28  | esp - 32  |  EDI                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 32  | esp - 36  |  CR0                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 36  | esp - 40  |  CR3                               |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 40  | esp - 44  |  DS                                |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 44  | esp - 48  |  ES                                |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 48  | esp - 52  |  FS                                |
;;  +-----------+-----------+------------------------------------+
;;  | ebp - 52  | esp - 56  |  GS                                |
;;  +-----------+-----------+------------------------------------+
;;
;; -----------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2012-Sep-23	                         Initial version
;;  2018-May-29  Initial   0.1.0   ADCL  Copied this file from century32 to century-os
;;
;;===================================================================================================================


;;
;; -- Now, expose our function to everyone
;;    ------------------------------------
global  ProcessSwitch


;;
;; -- Some local equates for use with access structure elements
;;    ---------------------------------------------------------
PROC_ESP        EQU     0
PROC_SS         EQU     4

;;
;; -- This is the beginning of the code segment for this file
;;    -------------------------------------------------------
section ._text
cpu		586



ProcessSwitch:
		push	ebp					        ;; save the caller's frame pointer
		mov		ebp,esp				        ;; setup up a frame
        pushfd                              ;; save the current flags register
		cli							        ;; and turn off interrupts so that we do not have a preempt

		push	eax					        ;; save eax
		push	ebx					        ;; save ebx
		push	ecx					        ;; save ecx
		push	edx					        ;; save edx
		push	esi					        ;; save esi
		push	edi					        ;; save edi

		mov		eax,cr0
		push	eax					        ;; save cr0
		mov		eax,cr3
		push	eax					        ;; save cr3

		xor		eax,eax				        ;; clear eax
		mov		ax,ds				        ;; get ds
		push	eax					        ;; and push it
		mov		ax,es				        ;; get es
		push	eax					        ;; and push it
		mov		ax,fs				        ;; get fs
		push	eax					        ;; and push it
		mov		ax,gs				        ;; get gs
		push	eax					        ;; and push it

		mov		esi,[ebp+8]			        ;; get the currentPID Process_t address
		mov		edi,[ebp+12]		        ;; get the target PID Process_t address

		xor		eax,eax				        ;; clear eax
		mov		ax,ss				        ;; capture ss
		mov		[esi+PROC_SS],eax	        ;; save the ss register
		mov		[esi+PROC_ESP],esp	        ;; save the stack pointer

;;
;; -- This completes saving the current process and below we start to restore the target process context
;;    --------------------------------------------------------------------------------------------------
		mov		eax,[edi+PROC_SS]	        ;; get the ss register
		mov		ss,ax				        ;; and restore it
		mov		esp,[edi+PROC_ESP]	        ;; restore the stack pointer

		pop		eax					        ;; pop the gs register
		mov		gs,ax				        ;; and restore it
		pop		eax					        ;; pop the fs register
		mov		fs,ax				        ;; and restore it
		pop		eax					        ;; pop the es register
		mov		es,ax				        ;; and restore it
		pop		eax					        ;; pop the ds register
		mov		ds,ax				        ;; and restore it

		pop		eax					        ;; pop cr3
		mov		cr3,eax				        ;; and restore it
		pop		eax					        ;; pop cr0
		mov		cr0,eax				        ;; and restore it

		pop		edi					        ;; restore edi
		pop		esi					        ;; restore esi
		pop		edx					        ;; restore edx
		pop		ecx					        ;; restore ecx
		pop		ebx					        ;; restore ebx
		pop		eax					        ;; restore eax

		popfd						        ;; pop flags and restore interrupt status
		pop		ebp					        ;; restore ebp
		ret
