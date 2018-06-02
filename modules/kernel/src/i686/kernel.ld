/*******************************************************************************************************************/
/*                                                                                                                 */ 
/*  kernel/src/i686/loader.ld -- This is the linker script for locating the sections in the target binary          */
/*                                                                                                                 */ 
/*        Copyright (c)  2017-2018 -- Adam Clark                                                                   */
/*                                                                                                                 */ 
/* --------------------------------------------------------------------------------------------------------------- */
/*                                                                                                                 */ 
/*     Date     Tracker  Version  Pgmr  Description                                                                */       
/*  ----------  -------  -------  ----  -------------------------------------------------------------------------  */
/*  2017-04-14  Initial   0.0.0   ADCL  Initial version                                                            */
/*  2018-05-25  Initial   0.1.0   ADCL  Copy this file from the loader of century                                  */
/*                                                                                                                 */ 
/*******************************************************************************************************************/

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)

SECTIONS {
    /*
     * -- Set the location where the binary will be loaded
     *    ------------------------------------------------
     */
     
    . = 0x100000; 
    _loaderStart = .;
    _textStart = .;
    
    /*
     * -- This is where the code will be placed.  Note that we put the init section right up front.  This
     *    is where we will gain control.
     *    -----------------------------------------------------------------------------------------------
     */
    .text : {
        *(.mbheader)
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
    . = ALIGN(4096);
    _bssEnd = .;
    _loaderEnd = .;

    /DISCARD/ : {
        *(.eh_frame .note.GNU-stack .comment .bootstack)
    }
       
    end = .; _end = .; __end = .;
}
