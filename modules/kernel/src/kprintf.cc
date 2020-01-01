//===================================================================================================================
//  kprintf.cc -- Write a formatted string to the serial port COM1 (like printf)
//
//        Copyright (c)  2017-2020 -- Adam Clark
//        Licensed under "THE BEER-WARE LICENSE"
//        See License.md for details.
//
//  Write a formatted string to the serial port.  This function works similar to `printf()` from the C runtime
//  library.  I used to have a version publicly available, but I think it better to have a purpose-built version.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date      Tracker  Version  Pgmr  Description
//  -----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-Jul-08  Initial   0.1.0   ADCL  Initial version
//  2019-Feb-08  Initial   0.3.0   ADCL  Relocated
//
//===================================================================================================================


#include "types.h"
#include "serial.h"
#include "spinlock.h"
#include "printf.h"


//
// -- This is the spinlock that is used to ensure that only one process can output to the serial port at a time
//    ---------------------------------------------------------------------------------------------------------
Spinlock_t kprintfLock = {0};


//
// -- Several flags
//    -------------
enum {
    ZEROPAD = 1<<0,            /* pad with zero */
    SIGN    = 1<<1,            /* unsigned/signed long */
    PLUS    = 1<<2,            /* show plus */
    SPACE   = 1<<3,            /* space if plus */
    LEFT    = 1<<4,            /* left justified */
    SPECIAL = 1<<5,            /* 0x */
    LARGE   = 1<<6,            /* use 'ABCDEF' instead of 'abcdef' */
};


//
// -- Used for Hex numbers
//    --------------------
static const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
static const char *upper_digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

//
// -- This is a printf()-like function to print to the serial port
//    ------------------------------------------------------------
int kprintf(const char *fmt, ...)
{
    archsize_t flags = SPINLOCK_BLOCK_NO_INT(kprintfLock) {
        int printed = 0;
        const char *dig = digits;
        archsize_t val;

        va_list args;
        va_start(args, fmt);

        for ( ; *fmt; fmt ++) {
            // -- for any character not a '%', just print the character
            if (*fmt != '%') {
                SerialPutChar(&debugSerial, *fmt);
                printed ++;
                continue;
            }

            // -- we know the character is a '%' char at this point
            fmt ++;
            if (!*fmt) goto exit;
            int fmtDefn = 1;
            int flags = 0;

            // -- we need to check for the format modifiers, starting with zero-fill
            if (*fmt == '0') {
                flags |= ZEROPAD;
                fmtDefn ++;
                fmt ++;
            }

            // -- now, get to the bottom of a formatted value
            switch (*fmt) {
            default:
                fmt -= fmtDefn;
                // fall through

            case '%':
                SerialPutChar(&debugSerial, '%');
                printed ++;
                continue;

            case 's': {
                char *s = va_arg(args, char *);
                if (!s) s = (char *)"<NULL>";
                while (*s) SerialPutChar(&debugSerial, *s ++);
                printed ++;
                continue;
            }

            case 'P':
                flags |= LARGE;
                dig = upper_digits;
                // fall through

            case 'p':
                val = va_arg(args, archsize_t);
                SerialPutChar(&debugSerial, '0');
                SerialPutChar(&debugSerial, 'x');
                printed += 2;

                for (int j = sizeof(archsize_t) * 8 - 4; j >= 0; j -= 4) {
                    SerialPutChar(&debugSerial, dig[(val >> j) & 0x0f]);
                    printed ++;
                }

                break;

            case 'X':
                flags |= LARGE;
                dig = upper_digits;
                // fall through

            case 'x':
                {
                    val = va_arg(args, archsize_t);
                    SerialPutChar(&debugSerial, '0');
                    SerialPutChar(&debugSerial, 'x');
                    printed += 2;

                    bool allZero = true;

                    for (int j = sizeof(archsize_t) * 8 - 4; j >= 0; j -= 4) {
                        int ch = (val >> j) & 0x0f;
                        if (ch != 0) allZero = false;
                        if (!allZero || flags & ZEROPAD) {
                            SerialPutChar(&debugSerial, dig[ch]);
                            printed ++;
                        }
                    }

                    if (allZero && !(flags & ZEROPAD)) {
                        SerialPutChar(&debugSerial, '0');
                        printed ++;
                    }

                    break;
                }

            case 'd':
                {
                    val = va_arg(args, archsize_t);
                    char buf[30];
                    int i = 0;

                    if (val == 0) {
                        SerialPutChar(&debugSerial, '0');
                        printed ++;
                    } else {
                        while (val) {
                            buf[i ++] = (val % 10) + '0';
                            val /= 10;
                        }

                        while (--i >= 0) {
                            SerialPutChar(&debugSerial, buf[i]);
                            printed ++;
                        }
                    }

                    break;
                }
            }
        }

exit:
        va_end(args);

        SPINLOCK_RLS_RESTORE_INT(kprintfLock, flags);
        return printed;
    }
}
