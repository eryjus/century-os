//===================================================================================================================
//
//  ElfInit.cc -- Initialize the ELF loader with an image located at the specified frame (not virtual address)
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2020-Apr-20  Initial  v0.7.0a  ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "spinlock.h"
#include "mmu.h"
#include "process.h"
#include "heap.h"
#include "printf.h"
#include "elf.h"


//
// -- Process the load of an ELF executable into memory
//    -------------------------------------------------
EXTERN_C EXPORT KERNEL
void ElfInit(ElfImage_t *img, const char *name)
{
    Process_t *proc = NULL;

    // -- The first thing we need to do is map the elf header into a temporary location
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(elfHdrLock) {
        MmuMapToFrame(ELF_TEMP_HEADER, img->frame[0], PG_KRN);

        //
        // -- TODO: Can I restore interrupts here?  Need to think about deadlock prevention
        //    -----------------------------------------------------------------------------
        ElfHdrCommon_t *hdrShort = (ElfHdrCommon_t *)ELF_TEMP_HEADER;

        // -- Check for the ELF signature
        if (!HAS_ELF_MAGIC(hdrShort)) {
            kprintf("Invalid ELF Signature\n");
            goto exit;
        }

        if (hdrShort->eIdent[EI_CLASS] != ELFCLASS_32) {
            kprintf("Module is not 32-bit\n");
            goto exit;
        }

        if (hdrShort->eIdent[EI_DATA] != ELFDATA_LSB) {
            kprintf("Module is not in Little Endian byte order\n");
            goto exit;
        }

        if (hdrShort->eType != ET_EXEC) {
            kprintf("Module is not an executable file\n");
            goto exit;
        }


        // -- Here we prepare to create a new user process with its own memory space
        proc = ProcessPrepareFromImage(img, hdrShort, name);
        assert(proc != NULL);

exit:
        // -- clean up to exit
        MmuUnmapPage(ELF_TEMP_HEADER);
        SPINLOCK_RLS_RESTORE_INT(elfHdrLock, flags);

//        if (proc) ProcessReady(proc);
    }
}
