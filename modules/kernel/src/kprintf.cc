//===================================================================================================================
//  kprintf.cc -- Write a formatted string to the serial port COM1 (like printf)
//
//        Copyright (c)  2017-2018 -- Adam Clark
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
//
//===================================================================================================================


#include "types.h"
#include "serial-kernel.h"
#include "printf.h"


//
// -- Several flags
//    -------------
enum {
	ZEROPAD = 1<<0,			/* pad with zero */
	SIGN    = 1<<1,			/* unsigned/signed long */
	PLUS    = 1<<2,			/* show plus */
	SPACE   = 1<<3,			/* space if plus */
	LEFT    = 1<<4,			/* left justified */
	SPECIAL = 1<<5,			/* 0x */
	LARGE   = 1<<6,			/* use 'ABCDEF' instead of 'abcdef' */
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
	int printed = 0;
	const char *dig = digits;
	ptrsize_t val;

	va_list args;
	va_start(args, fmt);

	for ( ; *fmt; fmt ++) {
		// -- for any character not a '%', just print the character
		if (*fmt != '%') {
			SerialPutChar(*fmt);
			printed ++;
			continue;
		}

		// -- we know the character is a '%' char at this point
		fmt ++;
		if (!*fmt) return printed;
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
			SerialPutChar('%');
			printed ++;
			continue;

		case 's': {
			char *s = va_arg(args, char *);
			if (!s) s = (char *)"<NULL>";
			while (*s) SerialPutChar(*s ++);
			printed ++;
			continue;
		}

		case 'P':
			flags |= LARGE;
			dig = upper_digits;
			// fall through

		case 'p':
			val = va_arg(args, ptrsize_t);
			SerialPutS("0x");
			printed += 2;

			for (int j = sizeof(ptrsize_t) * 8 - 4; j >= 0; j -= 4) {
				SerialPutChar(dig[(val >> j) & 0x0f]);
				printed ++;
			}

			break;

		case 'X':
			flags |= LARGE;
			dig = upper_digits;
			// fall through

		case 'x':
			val = va_arg(args, ptrsize_t);
			SerialPutS("0x");
			printed += 2;

			bool allZero = true;

			for (int j = sizeof(ptrsize_t) * 8 - 4; j >= 0; j -= 4) {
				int ch = (val >> j) & 0x0f;
				if (ch != 0) allZero = false;
				if (!allZero || flags & ZEROPAD) {
					SerialPutChar(dig[ch]);
					printed ++;
				}
			}

			if (allZero && !(flags & ZEROPAD)) {
				SerialPutChar('0');
				printed ++;
			}

			break;
		}
	}

	va_end(args);

	return printed;
}
