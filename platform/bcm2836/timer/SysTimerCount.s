@@===================================================================================================================
@@
@@  SysTimerCount.s -- This file contains low-level asm for interacting with the system timer.
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@
@@ ------------------------------------------------------------------------------------------------------------------1
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2017-Apr-04  Initial   0.0.0   ADCL  Initial version
@@  2018-Nov-13  Initial   0.2.0   ADCL  Leveraged this file from century and updated.
@@  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
@@
@@===================================================================================================================


.section    .text
.globl		SysTimerCount


@@
@@ -- Some constants -- duplicated from arch-hw.h
@@    -------------------------------------------
.equ		BASE,0                      @@ The offset into the TimerDriver_t structure for the base
.equ		ST_CLO,4                	@@ Counter Lower 32 bits


@@
@@ -- read the system time running count
@@    ----------------------------------
SysTimerCount:
	ldr		r0,[r0,#BASE]               @@ load the base address of the system timer
    add     r0,#ST_CLO
	ldrd	r0,r1,[r0]                  @@ Get the 64-bit timer "count" into r1:r0
	mov		pc,lr						@@ return

