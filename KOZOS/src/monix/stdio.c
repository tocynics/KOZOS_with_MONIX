/* $Id: stdio.c,v 1.2.2.5 2007/11/10 17:33:34 kuwa Exp $ */
/*
 * Copyright (C) 2005-2007 Shuji KUWAHARA. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials provided
 *    with the distribution.
 * 3. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "console.h"

#define FLAGS_LEFT	0x01	/* '-' */
#define FLAGS_SIGN	0x02	/* '+' */
#define FLAGS_SPACE	0x04	/* ' ' */
#define FLAGS_PREFIX	0x08	/* '#' */
#define FLAGS_ZEROFILL	0x10	/* '0' */

#define FLAGS_UPPER	0x20	/* 'X' or 'x' */
#define FLAGS_LONG	0x40
#define FLAGS_BYTE	0x80

static const char hexu[] = "0123456789ABCDEF";
static const char hexl[] = "0123456789abcdef";

#ifdef USE_SPRINTF
/* NOTE: [v]s[n]printf are not reentrant. */
static char *sbuf, *ebuf;

static int sputchar(int c)
{
	if (sbuf < ebuf)
		*sbuf++ = c;
	return c;
}
#endif /* USE_SPRINTF */

int putchar(int c)
{
	if (c == '\n')
		ConsPutChar('\r');
	ConsPutChar((char)c);
	return c;
}

int getchar(void)
{
	return (int)ConsGetChar();
}

int doprnt(int (*put)(int), const char *fmt, va_list ap)
{
	char c;
	int pos = 0;
	unsigned char flags;
	int width;
	int precision;
	int i;
	signed char sign;

	char buf[16];	/* number string buffer */
	char *bp;	/* pointer for buf */
	int bl;		/* length of buf */

	char fc;	/* fill char ' ' or '0' */
	int cl;		/* copy length */
	int fl;		/* fill length */

	const char *hexp;

	const char *ssave;
	unsigned long ulval;
	signed long slval;
	signed char scval;
	char *spval;
	const char *p;

	while ((c = *fmt) != 0) {
		if (c != '%') {
			put(c);
			fmt++;
			pos++;
			continue;
		}
		ssave = fmt;
		fmt++;

		/* parse flags */
		flags = 0;
		for (;;) {
			switch (*fmt) {
			case '-':
				flags |= FLAGS_LEFT;
				fmt++;
				continue;
			case '+':
				flags |= FLAGS_SIGN;
				fmt++;
				continue;
			case ' ':
				flags |= FLAGS_SPACE;
				fmt++;
				continue;
			case '#':
				flags |= FLAGS_PREFIX;
				fmt++;
				continue;
			case '0':
				flags |= FLAGS_ZEROFILL;
				fmt++;
				continue;
			default:
				break;
			}
			break;
		}
		if (flags & FLAGS_SIGN)
			flags &= ~FLAGS_SPACE;
#if 0
		if (flags & FLAGS_LEFT)
			flags &= ~FLAGS_ZEROFILL;
#endif
		/* parse width */
		width = -1;
		c = *fmt;
		if (c == '*') {
			width = va_arg(ap, int);
			fmt++;
		} else if (c >= '0' && c <= '9') {
			width = c - '0';
			fmt++;
			for (;;) {
				c = *fmt;
				if (c >= '0' && c <= '9') {
					width = width * 10 + (c - '0');
					fmt++;
					continue;
				}
				break;
			}
		}
		/* parse precision */
		precision = -1;
		if (*fmt == '.') {
			fmt++;
			c = *fmt;
			if (c == '*') {
				precision = va_arg(ap, int);
				fmt++;
			} else if (c >= '0' && c <= '9') {
				precision = c - '0';
				fmt++;
				for (;;) {
					c = *fmt;
					if (c >= '0' && c <= '9') {
						precision = precision * 10 +
								(c - '0');
						fmt++;
						continue;
					}
					break;
				}
			}
		}

		/* type prefix */
		switch (*fmt) {
		case 'l':
		case 'L':
			flags |= FLAGS_LONG;
			fmt++;
			break;
		case 'b':
		case 'B':
			flags |= FLAGS_BYTE;
			fmt++;
			break;
		}

		/* type */
		hexp = hexl;	/* lower for 'x' */
		bp = &buf[sizeof(buf)-1];
		*bp = '\0';
		bl = 0;

		switch (*fmt) {
		case 'd':
		case 'i':
			slval = (flags & FLAGS_LONG) ?
			  va_arg(ap, signed long) : va_arg(ap, signed int);
			sign = 1;
			if (slval < 0) {
				sign = -1;
				slval = -slval;
			}
			do {
				*--bp = ((slval % 10) + '0');
				bl++;
				slval /= 10;
			} while (slval != 0);

			if (precision > 0) {
				while (bl < precision) {
					*--bp = '0';
					bl++;
				}
				flags &= ~FLAGS_ZEROFILL;
			}

			if (flags & FLAGS_LEFT) {
				if (sign < 0) {
					*--bp = '-';
					bl++;
				} else {
					if (flags & FLAGS_SPACE) {
						*--bp = ' ';
						bl++;
					}
					if (flags & FLAGS_SIGN) {
						*--bp = '+';
						bl++;
					}
				}
				while ((c = *bp++) != 0) {
					put(c);
					pos++;
				}
				for (i = bl; i < width; i++) {
					put(' ');
					pos++;
				}
			} else {
				if (sign < 0 ||
				    flags & (FLAGS_SIGN | FLAGS_SPACE)) {
					if (flags & FLAGS_ZEROFILL) {
						while (bl < width - 1) {
							*--bp = '0';
							bl++;
						}
					}
					if (sign < 0)
						*--bp = '-';
					else {
						if (flags & FLAGS_SIGN)
							*--bp = '+';
						if (flags & FLAGS_SPACE)
							*--bp = ' ';
					}
					bl++;
					flags &= ~FLAGS_ZEROFILL;
				}

				fc = (flags & FLAGS_ZEROFILL) ? '0' : ' ';
				for (i = bl; i < width; i++) {
					put(fc);
					pos++;
				}

				/* copy from buf */
				while ((c = *bp++) != 0) {
					put(c);
					pos++;
				}
			}

			fmt++;
			break;

		case 'u':
			ulval = (flags & FLAGS_LONG) ?
			  va_arg(ap, unsigned long) : va_arg(ap, unsigned int);
			do {
				*--bp = ((ulval % 10) + '0');
				bl++;
				ulval /= 10;
			} while (ulval != 0);

			if (precision > 0) {
				while (bl < precision) {
					*--bp = '0';
					bl++;
				}
				flags &= ~FLAGS_ZEROFILL;
			}
			if (flags & FLAGS_LEFT) {
				while ((c = *bp++) != 0) {
					put(c);
					pos++;
				}
				for (i = bl; i < width; i++) {
					put(' ');
					pos++;
				}
			} else {
				fc = (flags & FLAGS_ZEROFILL) ? '0' : ' ';
				for (i = bl; i < width; i++) {
					put(fc);
					pos++;
				}
				while ((c = *bp++) != 0) {
					put(c);
					pos++;
				}
			}
			fmt++;
			break;

		case 'n':
			bp = va_arg(ap, void *);
			*(unsigned int *)bp = pos;
			fmt++;
			break;

		case 'p':	/* "%p" -> "%#lx" */
			ulval = (unsigned long)va_arg(ap, void *);
			do {
				*--bp = hexl[ulval & 0x0f];
				bl++;
				ulval >>= 4;
			} while (ulval != 0);

			if (precision > 0) {
				while (bl < precision) {
					*--bp = '0';
					bl++;
				}
				flags &= ~FLAGS_ZEROFILL;
			}

			if (flags & FLAGS_LEFT) {
				*--bp = 'x';
				bl++;
				*--bp = '0';
				bl++;
				while ((c = *bp++) != 0) {
					put(c);
					pos++;
				}
				for (i = bl; i < width; i++) {
					put(' ');
					pos++;
				}
			} else {
				if (flags & FLAGS_ZEROFILL) {
					while (bl < width - 2) {
						*--bp = '0';
						bl++;
					}
				}
				*--bp = 'x';
				bl++;
				*--bp = '0';
				bl++;
				for (i = bl; i < width; i++) {
					put(' ');
					pos++;
				}

				/* copy from buf */
				while ((c = *bp++) != 0) {
					put(c);
					pos++;
				}
			}
			fmt++;
			break;

		case 'X':
			hexp = hexu;	/* upper for 'X' */
		case 'x':
			ulval = (flags & FLAGS_LONG) ?
			  va_arg(ap, unsigned long) : va_arg(ap, unsigned int);
			do {
				*--bp = hexp[ulval & 0x0f];
				bl++;
				ulval >>= 4;
			} while (ulval != 0);

			if (precision > 0) {
				while (bl < precision) {
					*--bp = '0';
					bl++;
				}
				flags &= ~FLAGS_ZEROFILL;
			}

			if (flags & FLAGS_LEFT) {
				if (flags & FLAGS_PREFIX) {
					*--bp = *fmt;
					bl++;
					*--bp = '0';
					bl++;
				}
				while ((c = *bp++) != 0) {
					put(c);
					pos++;
				}
				for (i = bl; i < width; i++) {
					put(' ');
					pos++;
				}
			} else {
				if (flags & FLAGS_PREFIX) {
					if (flags & FLAGS_ZEROFILL) {
						while (bl < width - 2) {
							*--bp = '0';
							bl++;
						}
					}
					*--bp = *fmt;
					bl++;
					*--bp = '0';
					bl++;
					flags &= ~FLAGS_ZEROFILL;
				}

				fc = (flags & FLAGS_ZEROFILL) ? '0' : ' ';
				for (i = bl; i < width; i++) {
					put(fc);
					pos++;
				}

				/* copy from buf */
				while ((c = *bp++) != 0) {
					put(c);
					pos++;
				}
			}
			fmt++;
			break;

		case 'o':
			ulval = (flags & FLAGS_LONG) ?
			  va_arg(ap, unsigned long) : va_arg(ap, unsigned int);
			do {
				*--bp = (ulval & 0x07) + '0';
				bl++;
				ulval >>= 3;
			} while (ulval != 0);

			if (precision > 0) {
				while (bl < precision) {
					*--bp = '0';
					bl++;
				}
				flags &= ~FLAGS_ZEROFILL;
			}
			if (flags & FLAGS_PREFIX && *bp != '0') {
				*--bp = '0';
				bl++;
			}
			if (flags & FLAGS_LEFT) {
				while ((c = *bp++) != 0) {
					put(c);
					pos++;
				}
				for (i = bl; i < width; i++) {
					put(' ');
					pos++;
				}
			} else {
				fc = (flags & FLAGS_ZEROFILL) ? '0' : ' ';
				for (i = bl; i < width; i++) {
					put(fc);
					pos++;
				}
				while ((c = *bp++) != 0) {
					put(c);
					pos++;
				}
			}
			fmt++;
			break;

		case 'c':
			scval = (flags & FLAGS_LONG) ?
			  va_arg(ap, unsigned long) : va_arg(ap, unsigned int);
			if (flags & FLAGS_LEFT) {
				put(scval);
				pos++;
				for (i = 1; i < width; i++) {
					put(' ');
					pos++;
				}
			} else {
				for (i = 1; i < width; i++) {
					put(' ');
					pos++;
				}
				put(scval);
				pos++;
			}
			fmt++;
			break;

		case 's':
			spval = va_arg(ap, char *);
			cl = strlen(spval);
			if (precision >= 0 && precision < cl)
				cl = precision;
			if (width > cl)
				fl = width - cl;
			else
				fl = 0;
			pos += (cl + fl); /**/

			if (flags & FLAGS_LEFT) {
				while (cl--)
					put(*spval++);
				while (fl--)
					put(' ');
			} else {
				while (fl--)
					put(' ');
				while (cl--)
					put(*spval++);
			}
			fmt++;
			break;

		case '%':
			put('%');
			fmt++;
			break;

		default:
			for (p = ssave; p <= fmt; p++)
				put(*p);
			fmt++;
			break;
		}

	}
	return pos;
}

#ifdef USE_SPRINTF
int vsnprintf(char *buf, size_t size, const char *fmt, va_list ap)
{
	sbuf = buf;
	ebuf = buf + size - 1;
	doprnt(sputchar, fmt, ap);
	*sbuf = '\0';
	return sbuf - buf;
}

int snprintf(char *buf, size_t size, const char *fmt, ...)
{
	va_list ap;
	int len;

	va_start(ap, fmt);
	len = vsnprintf(buf, size, fmt, ap);
	va_end(ap);
	return len;
}

int vsprintf(char *buf, const char *fmt, va_list ap)
{
	return vsnprintf(buf, -(size_t)buf, fmt, ap);
}

int sprintf(char *buf, const char *fmt, ...)
{
	va_list ap;
	int len;

	va_start(ap, fmt);
	len = vsnprintf(buf, -(size_t)buf, fmt, ap);
	va_end(ap);
	return len;
}
#endif /* USE_SPRINTF */

int vprintf(const char *fmt, va_list ap)
{
	return doprnt(putchar, fmt, ap);
}

int printf(const char *fmt, ...)
{
	va_list ap;
	int len;

	va_start(ap, fmt);
	len = vprintf(fmt, ap);
	va_end(ap);
	return len;
}

int puts(const char *p)
{
	char c;

	while ((c = *p++) != 0)
		putchar(c);
	putchar('\n');
	return 0;
}
