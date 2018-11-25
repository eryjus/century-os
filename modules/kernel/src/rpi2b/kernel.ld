/*******************************************************************************************************************/
/*                                                                                                                 */
/*  kernel.ld -- This is the linker script for locating the sections in the target binary                          */
/*                                                                                                                 */
/*        Copyright (c)  2017-2018 -- Adam Clark                                                                   */
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
    /*
     * -- Set the location where the binary will be loaded
     *    ------------------------------------------------
     */

    . = 0x80000000;
    _kernelStart = .;
    _textStart = .;

    /*
     * -- This is where the code will be placed.  Note that we put the init section right up front.  This
     *    is where we will gain control.
     *    -----------------------------------------------------------------------------------------------
     */
    .text : AT(ADDR(.text) - 0x7fe80000) {
        *(.mboot)
        *(.init)
        *(.text .text.* .stub .gnu.linkonce.t.*)
    }
    . = ALIGN(4096);
    _textEnd = .;

    _roStart = .;
    /*
     * -- We drop in the read only data here
     *    ----------------------------------
     */
    .rodata : AT(ADDR(.rodata) - 0x7fe80000) {
        *(.rodata .rodata.* .gnu.linkonce.r.*)
    }
    . = ALIGN(4096);
    _roEnd = .;

    /*
     * -- Include debugging information in kernel memory
     *    ----------------------------------------------
     */
    _stabStart = .;
	.stab : AT(ADDR(.stab) - 0x7fe80000) {
		*(.stab);
		*(.stabstr);
        *(.debug*);
        *(.got*)
		BYTE(0)		/* Force the linker to allocate space for this section */
	}
    . = ALIGN(4096);
	_stabEnd = .;

    /*
     * -- Allocate a stack
     *    ----------------
     */
    _stackStart = .;
    .stack : {
        *(.stack)
    }
    . = ALIGN(4096);
    _stackEnd = .;

    _dataStart = .;
    /*
     * -- We drop in the read/write data here
     *    -----------------------------------
     */
    .data : AT(ADDR(.data) - 0x7fe80000) {
        *(.data*)
    }
    . = ALIGN(4096);
    _dataEnd = .;

    _bssStart = .;
    /*
     * -- We drop in the 0-init read/write data here
     *    ------------------------------------------
     */
    .bss : AT(ADDR(.bss) - 0x7fe80000) {
        *(.bss)
    }
    . = ALIGN(4096);
    _bssEnd = .;
    _kernelEnd = .;

    /DISCARD/ : {
        *(.eh_frame .note.GNU-stack .comment .bootstack)
    }
}
