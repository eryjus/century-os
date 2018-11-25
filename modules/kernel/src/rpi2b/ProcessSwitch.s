@@===================================================================================================================
@@
@@  ProcessSwitch.s -- Execute a task switch at the lowest switch
@@
@@  This function will perform a voluntary task switch, as opposed to a preemptive switch.  Therefore, there will
@@  never need to have an EOI sent to the PIC/APIC.
@@
@@  The following represent the stack structure based on the esp/frame set up at entry:
@@  +-----------+-----------+------------------------------------+
@@  |    EBP    |    ESP    |  Point of reference                |
@@  +-----------+-----------+------------------------------------+
@@  | ebp + 12  | esp + 08  |  The current Process_t structure   |
@@  +-----------+-----------+------------------------------------+
@@  | ebp + 08  | esp + 04  |  The target Process_t structure    |
@@  +-----------+-----------+------------------------------------+
@@  | ebp + 04  |   esp     |  Return EIP                        |
@@  +-----------+-----------+------------------------------------+
@@  |   ebp     | esp - 04  |  Frame pointer EBP                 |
@@  +-----------+-----------+------------------------------------+
@@  | ebp - 04  | esp - 08  |  EFLAGS                            |
@@  +-----------+-----------+------------------------------------+
@@  | ebp - 08  | esp - 12  |  EAX                               |
@@  +-----------+-----------+------------------------------------+
@@  | ebp - 12  | esp - 16  |  EBX                               |
@@  +-----------+-----------+------------------------------------+
@@  | ebp - 16  | esp - 20  |  ECX                               |
@@  +-----------+-----------+------------------------------------+
@@  | ebp - 20  | esp - 24  |  EDX                               |
@@  +-----------+-----------+------------------------------------+
@@  | ebp - 24  | esp - 28  |  ESI                               |
@@  +-----------+-----------+------------------------------------+
@@  | ebp - 28  | esp - 32  |  EDI                               |
@@  +-----------+-----------+------------------------------------+
@@  | ebp - 32  | esp - 36  |  CR0                               |
@@  +-----------+-----------+------------------------------------+
@@  | ebp - 36  | esp - 40  |  CR3                               |
@@  +-----------+-----------+------------------------------------+
@@  | ebp - 40  | esp - 44  |  DS                                |
@@  +-----------+-----------+------------------------------------+
@@  | ebp - 44  | esp - 48  |  ES                                |
@@  +-----------+-----------+------------------------------------+
@@  | ebp - 48  | esp - 52  |  FS                                |
@@  +-----------+-----------+------------------------------------+
@@  | ebp - 52  | esp - 56  |  GS                                |
@@  +-----------+-----------+------------------------------------+
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2018-Nov-21	 Initial   0.2.0   ADCL  Initial version
@@
@@===================================================================================================================


@@
@@ -- Now, expose our function to everyone
@@    ------------------------------------
	.global  	ProcessSwitch


@@
@@ -- Some local equates for use with access structure elements
@@    ---------------------------------------------------------
	.equ		PROC_ESP,0
	.equ		PROC_SS,4
	.equ		PROC_CR3,8

@@
@@ -- This is the beginning of the code segment for this file
@@    -------------------------------------------------------
	.section 	.text


@@
@@ -- Execute a process switch
@@    ------------------------
ProcessSwitch:
	mov		pc,lr
