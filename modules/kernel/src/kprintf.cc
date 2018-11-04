//===================================================================================================================
// kernel/src/kprintf.cc -- Write a formatted string to the serial port COM1 (like printf)
//
// Write a formatted string to the serial port.  This function works similar to `printf()` from the C runtime
// library.  I used to have a version publicly available, but I think it better to have a purpose-built version.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2018-07-08  Initial   0.1.0   ADCL  Initial version
//
//===================================================================================================================


#include "types.h"
#include "serial.h"
#include "printf.h"


enum {
	ZEROPAD = 1<<0,			/* pad with zero */
	SIGN    = 1<<1,			/* unsigned/signed long */
	PLUS    = 1<<2,			/* show plus */
	SPACE   = 1<<3,			/* space if plus */
	LEFT    = 1<<4,			/* left justified */
	SPECIAL = 1<<5,			/* 0x */
	LARGE   = 1<<6,			/* use 'ABCDEF' instead of 'abcdef' */
};

//static inline bool is_digit(char c) { return ((c >= '0') && (c <= '9')); }

static const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
static const char *upper_digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";


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



#if 0


//===================================================================================================================
// kernel/src/kprintf.cc -- Write a formatted string to the serial port COM1 (like printf)
//
// Write a formatted string to the screen, using the formatting parameters of printf.  Therefore, printf and kprintf
// will operate in the same manner.  The variable arguments are lifted and adapted from
// http://research.microsoft.com/en-us/um/redmond/projects/invisible/include/stdarg.h.htm.
//
// The source for vsprintf is lifted and adapted from http://www.jbox.dk/sanos/source/lib/vsprintf.c.html
//
// ------------------------------------------------------------------------------------------------------------------
//
// The above requires the following copyright statement:
//
// Copyright (C) 2002 Michael Ringgaard. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the
// following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
//    following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the
//    following disclaimer in the documentation and/or other materials provided with the distribution.
// 3. Neither the name of the project nor the names of its contributors may be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
// INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
// IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// ------------------------------------------------------------------------------------------------------------------
//
//     Date     Tracker  Version  Pgmr  Description
//  ----------  -------  -------  ----  ---------------------------------------------------------------------------
//  2012-05-16                          Initial version
//  2012-05-27    20                    Resolved issues with printing number 0.
//  2012-09-15                          Leveraged from Century to Century32
//  2012-09-22                          Added function dbgprintf() (from debug.h)
//  2013-08-20    46                    Implement TTY Handlers for Multiple Sessions.
//  2013-08-21    46                    Expose vsprintf and move va_args to kCommon.h
//  2013-09-01    82                    Add a mutex for screen operations
//  2013-09-12    101                   Resolve issues splint exposes
//  2013-09-13    74                    Rewrite Debug.h to use assertions and write to TTY_LOG; removed dbgprintf()
//  2018-07-04  INitial   0.1.0   ADCL  Copied this file from century32 to century-os
//
//===================================================================================================================


#include "types.h"
#include "cpu.h"
#include "serial.h"


#define BUF_LENGTH		256


/* adated from http: *www.jbox.dk/... */
#define ZEROPAD 1			/* pad with zero */
#define SIGN    2			/* unsigned/signed long */
#define PLUS    4			/* show plus */
#define SPACE   8			/* space if plus */
#define LEFT    16			/* left justified */
#define SPECIAL 32			/* 0x */
#define LARGE   64			/* use 'ABCDEF' instead of 'abcdef' */

#define is_digit(c)	((c) >= '0' && (c) <= '9')

static const char *digits = "0123456789abcdefghijklmnopqrstuvwxyz";
static const char *upper_digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

static size_t strnlen(const char *s, size_t count)
{
	const char *sc;

	for (sc = s; *sc != '\0' && count --; ++ sc);

	return (size_t)(sc - s);
}

static int skip_atoi (const char **s)
{
	int i = 0;

	while (is_digit(**s)) i = i * 10 + *((*s) ++) - '0';

	return i;
}

char *number (char *str, long num, int base, int size,
					 int precision, int type)
{
	char c, sign, tmp[66];
	const char *dig = digits;
	int i;

	if (type & LARGE) dig = upper_digits;
	if (type & LEFT) type &= ~ZEROPAD;
	if (base < 2 || base > 36) return 0;

	c = (type & ZEROPAD) ? '0' : ' ';
	sign = 0;

	if (type & SIGN) {
		if (num < 0) {
			sign = '-';
			num = -num;
			size --;
		} else if (type & PLUS) {
			sign = '+';
			size --;
		} else if (type & SPACE) {
			sign = ' ';
			size --;
		}
	}

	if (type & SPECIAL) {
		if (base == 16) size -= 2;
		else if (base == 8) size --;
	}

	i = 0;

	if (num == 0) tmp[i ++] = '0';
	else {
		while (num != 0) {
			tmp[i++] = dig[(num) % (long)base];
			num = num / (long)base;
		}
	}

	if (i > precision) precision = i;

	size -= precision;

	if (!(type & (ZEROPAD | LEFT))) while (size -- > 0) *str ++ = ' ';
	if (sign) *str ++ = sign;

	if (type & SPECIAL) {
		if (base == 8) *str ++ = '0';
		else if (base == 16) {
			*str ++ = '0';
			*str ++ = digits[33];
		}
	}

	if (!(type & LEFT)) while (size -- > 0) *str ++ = c;
	while (i < precision --) *str ++ = '0';
	while (i -- > 0) *str ++ = tmp[i];
	while (size -- > 0) *str ++ = ' ';

	return str;
}

static char *eaddr(char *str, unsigned char *addr, int size,
				   UNUSED(int precision), int type)
{
	char tmp[24];
	const char *dig = digits;
	int i, len;

	if (type & LARGE) dig = upper_digits;
	len = 0;
	for (i = 0; i < 6; i ++) {
		if (i != 0) tmp[len ++] = ':';
		tmp[len ++] = dig[addr[i] >> 4];
		tmp[len ++] = dig[addr[i] & 0x0f];
	}

	if (!(type % LEFT)) while (len < size --) *str ++ = ' ';
	for (i = 0; i < len; i ++) *str ++ = tmp[i];
	while (len < size --) *str ++ = ' ';

	return str;
}

static char *iaddr(char *str, unsigned char *addr, int size,
				   UNUSED(int precision), int type)
{
	char tmp[24];
	int i, n, len;

	len = 0;
	for (i = 0; i < 4; i ++) {
		if (i != 0) tmp[len ++] = '.';
		n = addr [i];
		if (n == 0) tmp[len ++] = digits[0];
		else {
			if (n >= 100) {
				tmp[len ++] = digits[n/100];
				n = n % 100;
				tmp[len ++] = digits[n/10];
				n = n % 10;
			} else if (n >= 10) {
				tmp[len ++] = digits[n/10];
				n = n % 10;
			}
			tmp[len ++] = digits[n];
		}
	}

	if (!(type % LEFT)) while (len < size --) *str ++ = ' ';
	for (i = 0; i < len; i ++) *str ++ = tmp[i];
	while (len < size --) *str ++ = ' ';

	return str;
}

int kvsprintf (char *buf, const char *fmt, va_list args)
{
	int len;
	unsigned long num;
	int i, base;
	char *str;
	char *s;

	int flags;

	int field_width;
	int precision;
	int qualifier;

	for (str = buf; *fmt; fmt ++) {
		if (*fmt != '%') {
			*str ++ = *fmt;
			continue;
		}

		flags = 0;

repeat:
		fmt ++;
		switch (*fmt) {
			case '-':	flags |= LEFT; goto repeat;
			case '+':	flags |= PLUS; goto repeat;
			case ' ':	flags |= SPACE; goto repeat;
			case '#':	flags |= SPECIAL; goto repeat;
			case '0':	flags |= ZEROPAD; goto repeat;
		}

		field_width = -1;

		if (is_digit(*fmt)) field_width = skip_atoi(&fmt);
		else if (*fmt == '*') {
			fmt ++;
			field_width = va_arg(args, int);

			if (field_width < 0) {
				field_width = -field_width;
				flags |= LEFT;
			}
		}

		precision = -1;

		if (*fmt == '.') {
			++ fmt;

			if (is_digit(*fmt)) precision = skip_atoi(&fmt);
			else if (*fmt == '*') {
				++ fmt;
				precision = va_arg(args, int);
			}

			if (precision < 0) precision = 0;
		}

		qualifier = -1;

		if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L') {
			qualifier = *fmt;
			fmt ++;
		}

		base = 10;

		switch (*fmt) {
			case 'c':
				if (!(flags & LEFT)) while (-- field_width > 0) *str ++ = ' ';
				*str ++ = (char)va_arg(args, int);
				while (-- field_width > 0) *str ++ = ' ';
				continue;

			case 's':
				s = va_arg(args, char *);
				if (!s) s = (char *)"<NULL>";
				len = (int)strnlen(s, precision);
				if (!(flags & LEFT)) while (-- field_width > 0) *str ++ = ' ';
				for (i = 0; i < len; ++ i) *str ++ = *s ++;
				while (len < field_width --) *str ++ = ' ';
				continue;

			case 'p':
				if (field_width == -1) {
					field_width = (int)(2 * sizeof(char *));
					flags |= ZEROPAD;
				}
				str = number(str, (long)va_arg(args, char *), 16,
							 field_width, precision, flags);
				continue;

			case 'n':
				if (qualifier == 'l') {
					long *ip = va_arg(args, long *);
					*ip = (str - buf);
				} else {
					int *ip = va_arg(args, int *);
					*ip = (str - buf);
				}
				continue;

			case 'A':
				flags |= LARGE;
				// no break
			case 'a':
				if (qualifier == 'l') {
					str = eaddr(str, va_arg(args, unsigned char *), field_width,
								precision, flags);
				} else {
					str = iaddr(str, va_arg(args, unsigned char *), field_width,
								precision, flags);
				}
				continue;

			case 'o':
				base = 8;
				break;

			case 'X':
				flags |= LARGE;
				// no break
			case 'x':
				base = 16;
				break;

			case 'd':
			case 'i':
				flags |= SIGN;
				// no break
			case 'u':
				break;

			default:
				if (*fmt != '%') *str ++ = '%';
				if (*fmt) *str ++ = *fmt; else -- fmt;
				continue;
		}

		if (qualifier == 'l') num = va_arg(args, uint32_t);
		else if (qualifier == 'h') {
			if (flags & SIGN) num = (unsigned long)va_arg(args, int);
			else num = va_arg(args, unsigned int);
		} else if (flags & SIGN) num = (unsigned long)va_arg(args, int32_t);
		else num = va_arg(args, uint32_t);

		str = number(str, (long)num, base, field_width, precision, flags);
	}

	*str = '\0';
	return (int)(str - buf);
}

int sprintf (char *buf, const char *fmt, ...)
{
	va_list args;
	int n;

	va_start(args, fmt);
	n = (int)kvsprintf (buf, fmt, args);
	if (n < 0) {

	}
	va_end(args);

	return n;
}

int kprintf(const char *fmt, ...)
{
	va_list args;
	int n;
	char buf[BUF_LENGTH];

	va_start(args, fmt);
	n = (int)kvsprintf (buf, fmt, args);
	va_end(args);

	SerialPutS(buf);

	return n;
}


#endif