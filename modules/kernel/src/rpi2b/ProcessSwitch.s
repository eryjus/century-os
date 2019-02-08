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
    push    {r0-r12}                        @@ save all the registers
    push    {lr}

    mrs     r2,cpsr                         @@ Get the Current Program Status Register
    str     r2,[r0,#PROC_SS]                @@ and store it in the structure

    @@ -- r0 has the current Process_t; r1 has the target Process_t
    mrc     p15,0,r2,c2,c0,0                @@ read the ttl1 table to the TTLR0 register
    str     r2,[r0,#PROC_CR3]               @@ save the paging tables
    str     sp,[r0,#PROC_ESP]               @@ save the stack

    @@ -- from here we are dealing with the target process
    ldr     r2,[r1,#PROC_CR3]               @@ get the paging table
    mcr     p15,0,r2,c2,c0,0                @@ write the ttl1 table to the TTLR0 register

    ldr     sp,[r1,#PROC_ESP]               @@ get the stack

    pop     {lr}

    ldr     r2,[r1,#PROC_SS]                @@ get the cspr from the structure
    msr     cpsr,r2                         @@ And put it in place

    pop     {r0-r12}                        @@ pop all the registers

	mov		pc,lr

