//===================================================================================================================
//
//  loader/src/modules/ModuleInit.cc -- Initialize all the loaded modules provided by Multiboot
//
//  This will read all the ELF header information and build the proper locations in memory.
//
// -----------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2017-06-27  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "hw-disc.h"
#include "pmm.h"
#include "mmu.h"
#include "serial.h"
#include "elf.h"
#include "modules.h"


extern ptrsize_t cr3;


//
// -- Initialize the modules multiboot handed off to us
//    -------------------------------------------------
kernEntry_t ModuleInit(void)
{
    kernEntry_t entry;

    if (!HaveModData()) {
        SerialPutS("Unable to locate Kernel...  Halting!\n");
        Halt();
    }

    bool haveKernel = false;

    for (int i = 0; i < GetModCount(); i ++) {
        bool thisIsKernel = false;

        SerialPutS("Initializing Modules:\n");
        char *modName = GetAvailModuleIdent(i);
        SerialPutS(modName);
        SerialPutS("\n   Starting Address: ");
        SerialPutHex(GetAvailModuleStart(i));
        SerialPutS("\n");

        ElfHdrCommon_t* hdr = (ElfHdrCommon_t *)GetAvailModuleStart(i);

        if (modName[0] == 'k' && modName[1] == 'e' && modName[2] == 'r' && modName[3] == 'n'
                && modName[4] == 'e' && modName[5] == 'l') {
            haveKernel = true;
            thisIsKernel = true;
        }

        // -- Check for the ELF signature
        if (!HAS_ELF_MAGIC(hdr)) {
            SerialPutS("Invalid ELF Signature\n");
            continue;
        }

        if (hdr->eIdent[EI_CLASS] != ELFCLASS_32) {
            SerialPutS("Module is not 32-bit\n");
            continue;
        }

        if (hdr->eIdent[EI_DATA] != ELFDATA_LSB) {
            SerialPutS("Module is not in Little Endian byte order\n");
            continue;
        }

        if (hdr->eType != ET_EXEC) {
            SerialPutS("Module is not an executable file\n");
            continue;
        }

        // -- At this point, we know we have a proper ELF file we can read
        Elf32EHdr_t *hdr32 = (Elf32EHdr_t *)hdr;
        Elf32PHdr_t *phdr32 = (Elf32PHdr_t *)((char *)hdr + hdr32->ePhOff);

        for (int j = 0; j < hdr32->ePhNum; j ++) {
            // -- So now we need to go through and map the kernel pages to the frames
            SerialPutS("   FileSize = ");
            SerialPutHex(phdr32[j].pFileSz);
            SerialPutS("; MemSize = ");
            SerialPutHex(phdr32[j].pMemSz);
            SerialPutS("; FileOffset = ");
            SerialPutHex(phdr32[j].pOffset);
            SerialPutS("\n");

            if (thisIsKernel) entry = (kernEntry_t)hdr32->eEntry;

            ptrsize_t offset;
            for (offset = 0; offset < phdr32[j].pMemSz; offset += 0x1000) {
                frame_t f;

                if (offset > phdr32[j].pFileSz) {
                    f = PmmNewFrame();
                } else {
                    f = PmmLinearToFrame(GetAvailModuleStart(i) + phdr32[j].pOffset + offset);
                }

                SerialPutS("      Attempting to map page ");
                SerialPutHex(phdr32[j].pVAddr + offset);
                SerialPutS(" to frame ");
                SerialPutHex(f);
                SerialPutS("\n");

                MmuMapToFrame(cr3, phdr32[j].pVAddr + offset, f);
            }
        }

        // -- The last thing to do with this is to clean up the ELF headers
        for (ptrsize_t j = 0; j < phdr32[i].pOffset; j += 0x1000) {
            PmmFreeFrame(PmmLinearToFrame(0x100000 + j));
        }
    }

    // -- Make sure we have a kernel and panic if we do not.
    if (!haveKernel) {
        SerialPutS("Unable to locate Kernel...  Halting!\n");
        Halt();
    }

    SerialPutS("Returning kernel entry point: ");
    SerialPutHex((ptrsize_t)entry);
    SerialPutS("\n");

    return entry;
}

