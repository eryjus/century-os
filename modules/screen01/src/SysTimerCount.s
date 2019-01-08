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
@@
@@===================================================================================================================


.section    .text
.globl		SysTimerCount


@@
@@ -- Some constants -- duplicated from arch-hw.h
@@    -------------------------------------------
.equ		HW_BASE,(0x3f000000)
.equ		ST_BASE,(HW_BASE+0x3000)    @@ System Timer base address
.equ		ST_CLO,(ST_BASE+4)        	@@ Counter Lower 32 bits


@@
@@ -- read the system time running count
@@    ----------------------------------
SysTimerCount:
	ldr		r0,=ST_CLO                  @@ load the base address of the system timer
	ldrd	r0,r1,[r0]                  @@ Get the 64-bit timer "count" into r1:r0
	mov		pc,lr						@@ return

