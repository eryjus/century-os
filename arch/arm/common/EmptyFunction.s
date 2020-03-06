@@===================================================================================================================
@@
@@  EmptyFunction.s -- This function does absolutely nothing
@@
@@        Copyright (c)  2017-2020 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@ ------------------------------------------------------------------------------------------------------------------
@@
@@     Date      Tracker  Version  Pgmr  Description
@@  -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@  2020-Feb-23  Initial  v0.5.0h  ADCL  Initial version
@@
@@===================================================================================================================



.globl      EmptyFunction


.section    .text


EmptyFunction:
    mov     pc,lr                       @@ just return
