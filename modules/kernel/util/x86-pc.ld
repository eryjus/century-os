/*******************************************************************************************************************/
/*                                                                                                                 */
/*  kernel.ld -- This is the linker script for locating the sections in the target binary                          */
/*                                                                                                                 */
/*        Copyright (c)  2017-2019 -- Adam Clark                                                                   */
/*                                                                                                                 */
/* --------------------------------------------------------------------------------------------------------------- */
/*                                                                                                                 */
/*  As of v0.5.0, I am working on changing the entire kernel location map.  For more information, see these wikis: */
/*  http://eryjus.ddns.net:3000/projects/century-os/wiki/Low_Memory_Usage_Map and                                  */
/*  http://eryjus.ddns.net:3000/projects/century-os/wiki/High-Level_Virtual_Memory_Map.                            */
/*                                                                                                                 */
/*  For this to be successful, I need the entry point to be loaded at 1MB (MultiBoot requirement).  Then, with     */
/*  the MMU completely set up, I will be able to jump to the kernel in high memory.  The success of locating all   */
/*  this stuff lies here.                                                                                          */
/*                                                                                                                 */
/*     Date      Tracker  Version  Pgmr  Description                                                               */
/*  -----------  -------  -------  ----  ------------------------------------------------------------------------  */
/*  2017-Apr-14  Initial   0.0.0   ADCL  Initial version                                                           */
/*  2018-May-25  Initial   0.1.0   ADCL  Copy this file from the loader of century                                 */
/*  2018-Oct-28  Initial   0.1.0   ADCL  Set the PMM into its own sections                                         */
/*  2019-Feb-09  Initial   0.3.0   ADCL  Reformat this for an all-in-one loader/kernel                             */
/*  2019-Dec-09   #428    0.5.0a   ADCL  Map all kernel functions during entry                                     */
/*                                                                                                                 */
/*******************************************************************************************************************/

OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(entry)

PHYS = 0x00100000;
LOADER = 0x80000000;
SYSCALL = 0x80400000;
KERN_CODE = 0x80800000;
KERN_DATA = 0x81000000;

SECTIONS {
    . = PHYS;                           /* Multiboot will place the kernel here in physical memory */

    /*
     * -- Start with the multiboot required code, and all the entry code needed to get the MMU initialized
     *    ------------------------------------------------------------------------------------------------
     */
    .entry : {
        _mbStart = .;
        *(.mboot)                       /* lead with the multiboot header */
        *(.text.entry)                  /* followed by the code to get the mmu initialized */
        *(.data.entry)                  /* followed by the data needed to initialized */

        /*
         * -- add some linker-provided variables here
         *    ---------------------------------------
         */
        phys_loc = .;           LONG(PHYS);
        kern_loc = .;           LONG(LOADER);

        /* -- the start/end of the multiboot code/data -- needed for the 1MB MMU mappings */
        mbStart = .;            LONG(_mbStart);
        mbEnd = .;              LONG(_mbEnd);
        mbSize = .;             LONG(_mbEnd - _mbStart);
        mbPhys = .;             LONG(_mbStart);
        /* note, mb will not exist in virtual address space */

        /* -- now add in the loader positions -- will be mapped to upper memory */
        ldrStart = .;           LONG(_loaderStart);
        ldrEnd = .;             LONG(_loaderEnd);
        ldrSize = .;            LONG(_loaderEnd - _loaderStart);
        ldrPhys = .;            LONG(_loaderStart - LOADER + LOADER_OFFSET);
        ldrVirt = .;            LONG(_loaderStart);

        /* -- the kernel syscall pergatory section */
        sysStart = .;           LONG(_syscallStart);
        sysEnd = .;             LONG(_syscallEnd);
        sysSize = .;            LONG(_syscallEnd - _syscallStart);
        sysPhys = .;            LONG(_syscallStart - SYSCALL + SYSCALL_OFFSET);
        sysVirt = .;            LONG(_syscallStart);

        /* -- the kernel text and rodata sections */
        txtStart = .;           LONG(_textStart);
        txtEnd = .;             LONG(_textEnd);
        txtSize = .;            LONG(_textEnd - _textStart);
        txtPhys = .;            LONG(_textStart - KERN_CODE + KCODE_OFFSET);
        txtVirt = .;            LONG(_textStart);

        /* -- the SMP trampoline code */
        smpStart = .;           LONG(_smpStart);
        smpEnd = .;             LONG(_smpEnd);
        smpSize = .;            LONG(_smpEnd - _smpStart);
        smpPhys = .;            LONG(_smpStart - KERN_CODE + KCODE_OFFSET);
        smpVirt = .;            LONG(_smpStart);

        /* -- kernel data */
        dataStart = .;          LONG(_dataStart);
        dataEnd = .;            LONG(_dataEnd);
        dataSize = .;           LONG(_dataEnd - _dataStart);
        dataPhys = .;           LONG(_dataStart - KERN_DATA + KDATA_OFFSET);
        dataVirt = .;           LONG(_dataStart);

        /* -- kernel bss */
        bssStart = .;           LONG(_bssStart);
        bssEnd = .;             LONG(_bssEnd);
        bssSize = .;            LONG(_bssEnd - _bssStart);
        bssPhys = .;            LONG(_bssStart - KERN_DATA + KDATA_OFFSET);
        bssVirt = .;            LONG(_bssStart);

        /* -- the debugging data */
        stabStart = .;          LONG(_stabStart);
        stabEnd = .;            LONG(_stabEnd);
        stabSize = .;           LONG(_stabEnd - _stabStart);
        stabPhys = .;           LONG(_stabStart - KERN_DATA + KDATA_OFFSET);
        stabVirt = .;           LONG(_stabStart);


        *(.bss.entry)                   /* the stack for the first function call */
        . = ALIGN(4096);
        _mbEnd = .;
    }

    /*
     * -- each of the following input sections need to be decorated with the "ax" flags in order to be placed
     *    at the top of the file the way we want.  This will "break" the rules for data and bss, but we should
     *    be able to deal with it since we are not letting anything map this file for us -- just load it.
     *    ----------------------------------------------------------------------------------------------------
     */
    LOADER_OFFSET = .;
    . = LOADER;
    .loader : AT(ADDR(.loader) - LOADER + LOADER_OFFSET) {
        _loaderStart = .;
        *(.ldrtext)                     /* -- loader text (code) */
        *(.text.startup)                /* -- this is additional initialization code */
        . = ALIGN(8);                   /* -- the next table needs to align to 8 bytes */

        /*
         * -- Initialization Array here
         *    -------------------------
         */
        init_start = .;                 /* -- here we have an array of functions that need to be called */
        *(.ctors)
        *(.ctors.*)
        *(.init_array)
        *(SORT_BY_INIT_PRIORITY(.init_array.*))
        init_end = .;

        . = ALIGN(8);                   /* -- re-align to 8 bytes */
        *(.ldrrodata)                   /* -- loader rodata (like strings) */
        *(.ldrdata)                     /* -- loader data (things that will not get passed to the kernel */
        *(.ldrbss)                      /* -- loader uninitialized data */
        . = ALIGN(4096);
        _loaderEnd = .;
    }


    /*
     * -- This is where the kernel syscall code will be placed.
     *    -----------------------------------------------------
     */
    SYSCALL_OFFSET = . - LOADER + LOADER_OFFSET;
    . = SYSCALL;
    .syscall : AT(ADDR(.syscall) - SYSCALL + SYSCALL_OFFSET) {
        _kernelStart = .;
        _syscallStart = .;
        *(.text.syscall)
        *(.data.syscall)
        *(.bss.syscall)
        BYTE(0)                 /* force the section to be written, even if nothing in it yet */
        . = ALIGN(4096);
        _syscallEnd = .;
    }


    /*
     * -- This is where the kernel code will be placed.
     *    ---------------------------------------------
     */
    KCODE_OFFSET = . - SYSCALL + SYSCALL_OFFSET;
    . = KERN_CODE;
    .text : AT(ADDR(.text) - KERN_CODE + KCODE_OFFSET) {
        _textStart = .;
        *(.init)
        *(.text .text.* .stub .gnu.linkonce.t.*)
        *(.rodata .rodata.* .gnu.linkonce.r.*)
        . = ALIGN(4096);

        /*
         * -- This is the trampoline code for the APs
         *    ---------------------------------------
         */
        _smpStart = .;
        *(.smptext)
        *(.smpdata)
        . = ALIGN(4096);
        _smpEnd = .;
        _textEnd = .;
    }


    /*
     * -- We drop in the read/write data here
     *    -----------------------------------
     */
    KDATA_OFFSET = . - KERN_CODE + KCODE_OFFSET;
    . = KERN_DATA;
    .data : AT(ADDR(.data) - KERN_DATA + KDATA_OFFSET) {
        _dataStart = .;
        *(.data)
        . = ALIGN(4);

        _bssStart = .;
        *(.bss)
        . = ALIGN(4096);
        _bssEnd = .;
        _dataEnd = .;
    }


    /*
     * -- Include debugging information in loader memory
     *    ----------------------------------------------
     */
    .stab : {
        _stabStart = .;
        *(.stab);
        *(.stabstr);
        *(.debug*);
        *(.got*)
        BYTE(0)        /* Force the linker to allocate space for this section */
        . = ALIGN(4096);
        _stabEnd = .;
        _kernelEnd = .;
    }


    /DISCARD/ : {
        *(.eh_frame .note.GNU-stack .comment .bootstack)
    }
}
