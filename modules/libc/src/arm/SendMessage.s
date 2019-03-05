@@===================================================================================================================
@@
@@  SendMessage.s -- Execute a System Call to send a message
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
    .globl      SendMessage


@@
@@ -- Execute a System Call to Recieve a Message
@@    ------------------------------------------
SendMessage:
    push    {lr}                @@ we want to save this just in case
    svc     #2                  @@ we want function 2 to send a message (registers already hold parameters)
    pop     {pc}                @@ and return

