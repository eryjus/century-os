@@===================================================================================================================
@@
@@  loader.s -- This is the entry point for the Raspberry Pi 2B kernel
@@
@@        Copyright (c)  2017-2018 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ----------------------------------------------------------------------------
@@  2018-Nov-21  Initial   0.2.0   ADCL  Initial version
@@
@@===================================================================================================================


.globl          systemFont
.globl          Halt
.globl          _start


.extern         kInit


@@
@@ -- This is the code for CPU 0 to execute
@@    -------------------------------------
.section    .text


@@
@@ -- This is the main entry point for the kernel.
@@    --------------------------------------------
_start:

initialize:
    ldr     sp,=_stackEnd               @@ set up a stack
    bl      kInit


Halt:
    wfi
    b       Halt                        @@ go back and loop through more


#
# -- This is where we include the binary data for the system font
#    ------------------------------------------------------------
.section        .rodata
systemFont:
.incbin         "system-font.bin"

