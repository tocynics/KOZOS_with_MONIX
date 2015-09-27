/* $Id: memutil.c,v 1.2.2.6 2007/11/10 17:39:25 kuwa Exp $ */
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

#include <stdarg.h>
#include <stdio.h>
#include "types.h"
#include "string.h"
#include "ctype.h"
#include "memutil.h"
#include "sys.h"

u_long xatoi(const char *cp)
{
	u_long val;
	char base;
	char c;

	/*
	 * 0x=hex
	 * 0=octal
	 * isdigit=decimal
	 */
	c = *cp;
	if (!isdigit(c))
		return 0;
	val = 0;
	base = 10;
	if (c == '0') {
		c = *++cp;
		if (c == 'x' || c == 'X') {
			base = 16;
			c = *++cp;
		} else
			base = 8;
	}
	for (;;) {
#if 0
		if (isascii(c) && isdigit(c)) {
#else
		if (isdigit(c)) {
#endif
			val = (val * base) + (c - '0');
			c = *++cp;
#if 0
		} else if (base == 16 && isascii(c) && isxdigit(c)) {
#else
		} else if (base == 16 && isxdigit(c)) {
#endif
			val = (val << 4) | (c + 10 - (islower(c) ? 'a' : 'A'));
			c = *++cp;
		} else
			break;
	}
	return val;
}

void MemDump(void *mem, u_long len, u_char wordAccess)
{
	u_long memaddr = (u_long)mem;
	u_long addr;
	u_long i = 0;
	u_char line[17];	/* character display buffer */
	u_char j, k, l;
	u_char even = 1;
	u_short w = 0;
	SYS_DECL_LOCK;

	if (wordAccess) {
		memaddr &= ~0x1;	/* make even */
		len <<= 1;	/* make double */
	}

	memset(line, 0, sizeof(line));
	l = 0;
	addr = memaddr & ~0x0f;
	if (len <= 0)
		return;

	SYS_LOCK(); /* not non-reentrant code but for clean display */
	do {
		if ((addr & 0x0f) == 0)
			printf("%08lx  ", addr);
		if (addr < memaddr) {
			printf("  ");
			line[l++] = ' ';
		} else {
			u_char b;
			if (wordAccess) {
				if (even) {
					w = *((u_short *)addr);
					b = (u_char)((w >> 8) & 0xff);
					even = 0;
				} else {
					b = (u_char)(w & 0xff);
					even = 1;
				}
			} else
				b = *((u_char *)addr);
			printf("%02x", (int)b);
			i++;
			//line[l++] =isspace(b) ? ' ' : (isprint(b) ? b : '.');
			line[l++] =  (b >= 0x20 && b <= 0x7e) ? b : '.';

		}
		addr++;
		if (i == len) {	/* end of mem */
			j = addr & 0x0f;
			if (j > 0) {	/* if start is in the middle of
					   the line */
				if ((j & 0x07) == 0)
					printf(" - ");	/* delimiter between
							   8 hex digits */
				else if ((j & 0x01) == 0)
					putchar(' ');	/* space between
							   4 hex digits */

				for (k = j + 1; k <= 16; k++) {
					/* append space and delimiter
					   until the end of line */
					printf("  ");
					line[l++] = ' ';
					if ((k & 0x0f) == 0) {
						line[l] = '\0';
						printf("  %s\n", line);
						l = 0;
					} else if ((k & 0x07) == 0)
						printf(" - ");
					else if ((k & 0x01) == 0)
						putchar(' ');
				}
			} else {
				line[l] = '\0';
				printf("  %s\n", line);
				l = 0;
			}
		} else {
			if ((addr & 0x0f) == 0) {
				line[l] = '\0';
				printf("  %s\n", line);
				l = 0;
			} else if ((addr & 0x07) == 0)
				printf(" - ");
			else if ((addr & 0x01) == 0)
				putchar(' ');
		}
	} while (i < len);
	SYS_UNLOCK();
}
