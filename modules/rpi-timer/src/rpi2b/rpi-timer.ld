/*******************************************************************************************************************/
/*                                                                                                                 */
/*  rpi-timer.ld -- This is the linker script for locating the sections in the target binary                          */
/*                                                                                                                 */
/*        Copyright (c)  2017-2019 -- Adam Clark                                                                   */
/*        Licensed under "THE BEER-WARE LICENSE"                                                                   */
/*        See License.md for details.                                                                              */
/*                                                                                                                 */
/* --------------------------------------------------------------------------------------------------------------- */
/*                                                                                                                 */
/*     Date      Tracker  Version  Pgmr  Description                                                               */
/*  -----------  -------  -------  ----  ------------------------------------------------------------------------  */
/*  2018-Nov-11  Initial   0.2.0   ADCL  Initial version                                                           */
/*                                                                                                                 */
/*******************************************************************************************************************/


ENTRY(_start)

SECTIONS {
    . = 0x8000;   /* used for real hw */
/*    . = 0x10000;    /* used for qemu */

    .text : {
        *(.entry)
        *(.text)
        *(.rodata)
    }
    . = ALIGN(4096);

    .data : {
        *(.data)
    }

    .bss : {
        *(.bss)
    }
}
