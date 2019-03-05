@@===================================================================================================================
@@
@@  ReceiveMessage.s -- Execute a System Call to recieve a message
@@
@@        Copyright (c)  2017-2019 -- Adam Clark
@@        Licensed under "THE BEER-WARE LICENSE"
@@        See License.md for details.
@@
@@ -----------------------------------------------------------------------------------------------------------------
@@
@@    Date      Tracker  Version  Pgmr  Description
@@ -----------  -------  -------  ----  ---------------------------------------------------------------------------
@@ 2019-Jan-18  Initial   0.2.0  ADCL  Initial version
@@
@@===================================================================================================================


@@
@@ -- Expose labels to fucntions that the linker can pick up
@@    ------------------------------------------------------
    .globl      ReceiveMessage


@@
@@ -- Execute a System Call to Recieve a Message
@@    ------------------------------------------
ReceiveMessage:
    push    {lr}                @@ we want to save this just in case
    svc     #1                  @@ we want function 1 to receive a message (registers already hold parameters)
    pop     {pc}                @@ and return

