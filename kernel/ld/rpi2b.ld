/*******************************************************************************************************************/
/*                                                                                                                 */
/*  rpi2b.ld -- This is the linker script for locating the sections in the target binary                           */
/*                                                                                                                 */
/*        Copyright (c)  2017-2019 -- Adam Clark                                                                   */
/*                                                                                                                 */
/* --------------------------------------------------------------------------------------------------------------- */
/*                                                                                                                 */
/*     Date      Tracker  Version  Pgmr  Description                                                               */
/*  -----------  -------  -------  ----  ------------------------------------------------------------------------  */
/*  2018-Nov-11  Initial   0.2.0   ADCL  Initial version                                                           */
/*  2019-Feb-08  Initial   0.3.0   ADCL  Relocated                                                                 */
/*                                                                                                                 */
/*******************************************************************************************************************/

ENTRY(entry)

PHYS = 0x00100000;                      /* We will need this value for the loader */
KERN = 0x80000000;                      /* We will need this value for the loader */

SECTIONS {
    . = PHYS;                           /* Multiboot will place the kernel here */

    /*
     * -- each of the following input sections need to be decorated with the "ax" flags in order to be placed
     *    at the top of the file the way we want.  This will "break" the rules for data and bss, but we should
     *    be able to deal with it since we are not letting anything map this file for us -- just load it.
     *    ----------------------------------------------------------------------------------------------------
     */
    _loaderStart = .;
    .loader : {                         /* This is where the loader will be located -- things to be reclaimed: */
        *(.mboot)                       /* -- multiboot header comes first */
        *(.ldrtext)                     /* -- loader text (code) */
        *(.text.startup)                /* -- this is additional initialization code */
        . = ALIGN(8);                   /* -- the next table needs to align to 8 bytes */

        init_start = .;                 /* -- here we have an array of functions that need to be called */
        *(.ctors)
        *(.ctors.*)
        *(.init_array)
        *(SORT_BY_INIT_PRIORITY(.init_array.*))
        init_end = .;

        . = ALIGN(8);                   /* -- re-align to 8 bytes */
        *(.ldrrodata)                   /* -- loader rodata (like strings -- I hope anyway) */
        *(.ldrdata)                     /* -- loader data (things that will not get passed to the kernel */

        phys_loc = .;                   /* -- provide variable `phys_loc` with the value of PHYS */
        LONG(PHYS);

        kern_loc = .;                   /* -- provide variable `kern_loc` wiht the value of KERN */
        LONG(KERN);

        *(.ldrbss)                      /* -- loader bss (again, things that will not get passed to the kernel */
        . = ALIGN(4096);

        _localHwDisc = .;               /* -- this will be the location of the hardware discovery structire */
        BYTE(0);                        /* -- be sure something is allocated */
    }
    . = ALIGN(4096);
    _loaderEnd = .;
    PHYS_OFFSET = .;                    /* -- this is used to get the physical addresses correct for the elf loader */




    /*
     * -- Set the location where the binary will be loaded
     *    ------------------------------------------------
     */

    . = KERN;
    _kernelStart = .;
    _textStart = .;

    /*
     * -- This is where the code will be placed.  Note that we put the init section right up front.  This
     *    is where we will gain control.
     *    -----------------------------------------------------------------------------------------------
     */
    .text : AT(ADDR(.text) - KERN + PHYS_OFFSET) {
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
     * -- Include debugging information in kernel memory
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

    /*
     * -- Allocate a stack
     *    ----------------
     */
    . = ALIGN(4096);
    _stackStart = .;
    .stack : {
        *(.stack)
        BYTE(0)
    }
    . = ALIGN(4096);
    _stackEnd = .;

    _dataStart = .;
    /*
     * -- We drop in the read/write data here
     *    -----------------------------------
     */
    .data : {
        *(.data*)
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
    _kernelEnd = .;

    /DISCARD/ : {
        *(.eh_frame .note.GNU-stack .comment .bootstack)
    }
}
