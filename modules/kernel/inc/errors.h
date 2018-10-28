//===================================================================================================================
//
// errors.h -- These are the error messages that can be reported by the kernel.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Oct-14  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#ifndef __ERRORS_H__
#define __ERRORS_H__


//
// -- A worker macro to help with defining the errors
//    -----------------------------------------------
#define ERR(x) ERR_##x = 0x##x


//
// -- These are the error messages that are available and used in the kernel.  They are separated by type.
//    ----------------------------------------------------------------------------------------------------
typedef enum {
    ERR(00000000),                  // This is not an error, but success
    ERR(00000001),                  // Reached an illegal location
    ERR(00000002),                  // Invalid parameter -- NULL

    //
    // -- Errors that begin with `0x80...` are process management errors
    //    --------------------------------------------------------------
    ERR(80000000),                  // This is an unspecified process error
    ERR(80000001),                  // PID tables are full
    ERR(80000002),                  // PID out of range
    ERR(80000003),                  // PID no longer has a valid Process_t structure
    ERR(80000004),                  // PID is already held
    ERR(80000005),                  // PID is ending
    ERR(80000006),                  // PID has invalid status
    ERR(80000007),                  // PID is not held
    ERR(80000008),                  // PID is held; cannot ready
    ERR(80000009),                  // PID has invalid priority
    ERR(8000000A),                  // PID is already ending

    //
    // -- Errors that begin with `0x90...` are heap management errors
    //    -----------------------------------------------------------
    ERR(90000000),                  // This is an unspecified heap error
    ERR(90000001),                  // Heap is out of memory
} Errors_t;


#define SUCCESS     ERR_00000000

#undef ERR


//
// -- From here, we set up macros to emit error messages
//    --------------------------------------------------
#define ERROR_00000001()    kprintf("Error (0x00000001): Invalid code location was reached\n")
#define ERROR_00000002(loc) kprintf("Warning (0x00000002): Invalid Parameter in function %s -- NULL\n")


#define ERROR_80000000()    kprintf("Error (0x80000000): An unspecified Process error has occurred\n")
#define ERROR_80000001()    kprintf("Fatal (0x80000001): Unable to allocate a new Process ID (PID); tables full\n" \
                            "Table Size: %d\nSystem halted!\n", MAX_NUM_PID)
#define ERROR_80000002(pid) kprintf("Error (0x80000002): Invalid PID %u in %s at line %lu\n", pid, __FILE__, __LINE__)
#define ERROR_80000003(pid) kprintf("Warning (0x80000003): PID %u does not have valid Process_t structure in %s " \
                            "at line %lu\n", pid, __FILE__, __LINE__)
#define ERROR_80000004(pid) kprintf("Warning (0x80000004): PID %ud is already held\n", pid)
#define ERROR_80000005(pid) kprintf("Warning (0x80000005): PID $u is ending\n", pid)
#define ERROR_80000006(pid) kprintf("Error (0x80000006): PID $u is not in a valid status\n", pid)
#define ERROR_80000007(pid) kprintf("Warning (0x80000007): PID $u is not held\n", pid)
#define ERROR_80000008(pid) kprintf("Warning (0x80000008): PID $u is held; cannot ready\n", pid)
#define ERROR_80000009(pid) kprintf("Error (0x80000009): PID $u does not have a valid priority\n", pid)
#define ERROR_8000000A(pid) kprintf("Warning (0x8000000A): PID $u is already ending\n", pid)


#define ERROR_90000000()    kprintf("Error (0x90000000): An unspecified Heap error has occurred\n")
#define ERROR_90000001(loc) kprintf("Error (0x90000001): Heap out of memory error in %s\n", loc)


#endif
