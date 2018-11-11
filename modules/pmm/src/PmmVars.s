;;===================================================================================================================
;;
;;  PmmVars.s -- This file contains the global PMM variables
;;
;;        Copyright (c)  2017-2018 -- Adam Clark
;;        Licensed under "THE BEER-WARE LICENSE"
;;        See License.md for details.
;;
;;  All publicly exposed variables for the Physical Memory Manager (PMM) will be included in this file.
;;
;; ------------------------------------------------------------------------------------------------------------------
;;
;;     Date      Tracker  Version  Pgmr  Description
;;  -----------  -------  -------  ----  ---------------------------------------------------------------------------
;;  2018-Nov-07  Initial   0.1.0   ADCL  Initial version
;;
;;===================================================================================================================


    global pmmBitmap


;;
;; -- This is the number of elements for the physical memory bitmap for 4GB of memory.  The math is kind of messy,
;;    so I will walk through it:
;;    * 4GB = 4096MB.
;;    * 1 4K frame can manage 128 MB of physical memory,  divide to get the number of frames needed.
;;    * 1 frame is 4096 bytes, which will give us the number of bytes we need.
;;    * Finally, we are using 32-bit unsigned integers for comparison (for speed), so divide by that.
;;    The good news is that the compiler will work all this out at compile time.  Notice, however, this is
;;    uninitialized memory -- though the .bss section should be cleared, assume no guarantees please.
;;    ------------------------------------------------------------------------------------------------------------
    section     .data
pmmBitmap:
    times 131072 db 0xff
