//===================================================================================================================
//
// inc/elf.h -- The ELF Structures for loading ELF files
//
// These are the common structures for 32- and 64-bit ELF executables
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-06-27  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __ELF_H__
#define __ELF_H__


#include "types.h"


//
// -- The number of identifying bytes
//    -------------------------------
#define ELF_NIDENT      16


//
// -- The following are the meanings of the different positions in the eIdent field
//    -----------------------------------------------------------------------------
enum {
    EI_MAG0             = 0,    // File Identification
    EI_MAG1             = 1,
    EI_MAG2             = 2,
    EI_MAG3             = 3,
    EI_CLASS            = 4,    // File Class
    EI_DATA             = 5,    // Data Encoding
    EI_VERSION          = 6,    // File Version
    EI_OSABI            = 7,    // OS/ABI Identification
    EI_ABIVERSION       = 8,    // ABI Version
    EI_PAD              = 9,    // padding bytes in eIdent
};


//
// -- The following are the possible values for the ELF class, indicating what size the file objects
//    ----------------------------------------------------------------------------------------------
enum {
    ELFCLASS_NONE       = 0,    // Invalid
    ELFCLASS_32         = 1,    // 32-bit objects
    ELFCLASS_64         = 2,    // 64-bit objects
};


//
// -- The following are the possible values for the ELF Data encoding (big- or little-endian)
//    ---------------------------------------------------------------------------------------
enum {
    ELFDATA_NONE        = 0,    // Invalid
    ELFDATA_LSB         = 1,    // Binary values are in little endian order
    ELFDATA_MSB         = 2,    // Binary values are in big endian order
};


//
// -- The following are the possible values for the ELF Version, which only has 1 possible value
//    ------------------------------------------------------------------------------------------
enum {
    EV_CURRENT          = 1,    // This is the only valid value
};


//
// -- The following are the possible values for OS/ABI
//    ------------------------------------------------
enum {
    ELFOSABI_SYSV       = 0,    // System V ABI
    ELFOSABI_HPUX       = 1,    // HP-UX Operating system
    ELFOSABI_STANDALONE = 255,  // Standalone Application
};


//
// -- The following are the defined types
//    -----------------------------------
enum {
    ET_NONE             = 0,    // No file type
    ET_REL              = 1,    // Relocatable file
    ET_EXEC             = 2,    // Executable file
    ET_DYN              = 3,    // Dynamic or Shared object file
    ET_CORE             = 4,    // Core file
    ET_LOOS             = 0xfe00, // Environment-specific use
    ET_HIOS             = 0xfeff,
    ET_LOPROC           = 0xff00, // Processor-specific use
    ET_HIPROC           = 0xffff,
};


//
// -- These are the program segment flags
//    -----------------------------------
enum {
    PF_X                = 0x01,     // The segment is executable
    PF_W                = 0x02,     // The segment is writable
    PF_R                = 0x04,     // The segment is readable
};


//
// -- Architecture-independent check for the ELF signature
//    ----------------------------------------------------
#define HAS_ELF_MAGIC(x) ((x)->eIdent[EI_MAG0] == 0x7f && \
            (x)->eIdent[EI_MAG1] == 'E' && \
            (x)->eIdent[EI_MAG2] == 'L' && \
            (x)->eIdent[EI_MAG3] == 'F')


//
// -- This is the common part of the Elf File Header and will used to complete a sanity check before continuing
//    ---------------------------------------------------------------------------------------------------------
typedef struct ElfHdrCommon_t {
    unsigned char eIdent[ELF_NIDENT];
    elfHalf_t eType;
    elfHalf_t eMachine;
    elfWord_t eversion;
} __attribute__((packed)) ElfHdrCommon_t;


//
// -- This is the 32-bit ELF File Header Definition
//    ---------------------------------------------
typedef struct Elf32EHdr_t {
    unsigned char eIdent[ELF_NIDENT];
    elfHalf_t eType;
    elfHalf_t eMachine;
    elfWord_t eversion;
    elf32Addr_t eEntry;
    elf32Off_t ePhOff;          // Program Header offset
    elf32Off_t eShOff;          // Section Header offset
    elfWord_t eFlags;
    elfHalf_t eHSize;           // Program Header Size
    elfHalf_t ePhEntSize;       // Program Header Entry Size
    elfHalf_t ePhNum;           // Program Header Entry Count
    elfHalf_t eShEntSize;       // Section Header Entry Size
    elfHalf_t eShNum;           // Section Header Entry Count
    elfHalf_t eShStrNdx;        // Section Number for the string table
} __attribute__((packed)) Elf32EHdr_t;


//
// -- This is the 64-bit ELF File Header Definition
//    ---------------------------------------------
typedef struct Elf64EHdr_t {
    unsigned char eIdent[ELF_NIDENT];
    elfHalf_t eType;
    elfHalf_t eMachine;
    elfWord_t eversion;
    elf64Addr_t eEntry;
    elf64Off_t ePhOff;          // Program Header offset
    elf64Off_t eShOff;          // Section Header offset
    elfWord_t eFlags;
    elfHalf_t eHSize;           // Program Header Size
    elfHalf_t ePhEntSize;       // Program Header Entry Size
    elfHalf_t ePhNum;           // Program Header Entry Count
    elfHalf_t eShEntSize;       // Section Header Entry Size
    elfHalf_t eShNum;           // Section Header Entry Count
    elfHalf_t eShStrNdx;        // Section Number for the string table
} __attribute__((packed)) Elf64EHdr_t;


//
// -- This is the 32-bit ELF Program Header, which is needed to determine how to load the executable
//    ----------------------------------------------------------------------------------------------
typedef struct Elf32PHdr_t {
    elfWord_t pType;            // Type of segment
    elf32Off_t pOffset;         // Offset in file
    elf32Addr_t pVAddr;         // Virtual Address in Memory
    elf32Addr_t pPAddr;         // Reserved or meaningless
    elfWord_t pFileSz;          // Size of segment in file
    elfWord_t pMemSz;           // Size of segment in memory
    elfWord_t pFlags;           // Segment Attributes
    elfWord_t pAlign;           // Alignment of segment
} __attribute__((packed)) Elf32PHdr_t;


//
// -- This is the 64-bit ELF Program Header, which is needed to determine how to load the executable
//    ----------------------------------------------------------------------------------------------
typedef struct Elf64PHdr_t {
    elfWord_t pType;            // Type of segment
    elfWord_t pFlags;           // Segment Attributes
    elf64Off_t pOffset;         // Offset in file
    elf64Addr_t pVAddr;         // Virtual Address in Memory
    elf64Addr_t pPAddr;         // Reserved or meaningless
    elfXWord_t pFileSz;         // Size of segment in file
    elfXWord_t pMemSz;          // Size of segment in memory
    elfXWord_t pAlign;          // Alignment of segment
} __attribute__((packed)) Elf64PHdr_t;


#endif
