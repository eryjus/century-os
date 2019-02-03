/*******************************************************************************************************************/
/*                                                                                                                 */
/*  pmm.ld -- This is the linker script for locating the sections in the target binary                             */
/*                                                                                                                 */
/*        Copyright (c)  2017-2019 -- Adam Clark                                                                   */
/*        Licensed under "THE BEER-WARE LICENSE"                                                                   */
/*        See License.md for details.                                                                              */
/*                                                                                                                 */
/*  Note that this linker script is purpose-build for the PMM.  There are some reasons for this:                   */
/*  1.  When this is started, I do not have a working kernel heap to allocate a stack, so I need to statically     */
/*      allocate this in the binary.                                                                               */
/*  2.  In order to allocate memory frames, I need to have a working PMM.  Well, this is the PMM that needs to     */
/*      be working.                                                                                                */
/*  3.  This is an early process, loaded and executing long before other processes are enabled.                    */
/*                                                                                                                 */
/* --------------------------------------------------------------------------------------------------------------- */
/*                                                                                                                 */
/*     Date      Tracker  Version  Pgmr  Description                                                               */
/*  -----------  -------  -------  ----  ------------------------------------------------------------------------  */
/*  2018-Nov-02  Initial   0.1.0   ADCL  Initial version                                                           */
/*                                                                                                                 */
/*******************************************************************************************************************/

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(PmmMain)

SECTIONS {
    /*
     * -- Set the location where the binary will be loaded
     *    ------------------------------------------------
     */

    . = 0x80000000;

    _stackStart = .;
    /*
     * -- This is the stack and process structure, which is statically allocated in the binary
     *    ------------------------------------------------------------------------------------
     */
    .stack : {
        BYTE(0)
    }
    . = ALIGN(4096);
    _stackTop = .;

    _pageTableStart = .;
    .ptable : {
        BYTE(0)
    }
    . = ALIGN(4096);

    _textStart = .;
    /*
     * -- This is where the code will be placed.  Note that we put the init section right up front.  This
     *    is where we will gain control.
     *    -----------------------------------------------------------------------------------------------
     */
    .text : {
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
    .rodata : {
        *(.rodata .rodata.* .gnu.linkonce.r.*)
    }
    . = ALIGN(4096);
    _roEnd = .;

    /*
     * -- Include debugging information in loader memory
     *    ----------------------------------------------
     */
    _stabStart = .;
	.stab : {
		*(.stab);
		*(.stabstr);
        *(.debug*);
        *(.got*)
		BYTE(0)		/* Force the linker to allocate space for this section */
	}
    . = ALIGN(4096);
	_stabEnd = .;

    _dataStart = .;
    /*
     * -- We drop in the read/write data here
     *    -----------------------------------
     */
    .data : {
        *(.data)
    }
    . = ALIGN(4096);
    _dataEnd = .;

    _bssStart = .;
    /*
     * -- We drop in the 0-init read/write data here
     *    ------------------------------------------
     */
    .bss : {
        *(.bss)
    }
    . = ALIGN(4096);        /* and then align to the next frame */
    _bssEnd = .;


    /DISCARD/ : {
        *(.eh_frame .note.GNU-stack .comment .bootstack)
    }
}
