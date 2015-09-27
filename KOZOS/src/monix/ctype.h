/* $Id: ctype.h,v 1.2.2.6 2007/01/18 14:30:00 kuwa Exp $ */
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

#ifndef __CTYPE_H__
#define __CTYPE_H__

#define	__C	0x01		/* Control */
#define	__D	0x02		/* Digit */
#define	__L	0x04		/* Lower */
#define	__P	0x08		/* Punct */
#define	__S	0x10		/* Space */
#define	__U	0x20		/* Upper */
#define	__X	0x40		/* X digit */

#define	isalnum(c)	((__ctype[(int) c] & (__U | __L | __D)) != 0)
#define	isalpha(c)	((__ctype[(int) c] & (__U | __L)) != 0)
#define	iscntrl(c)	((__ctype[(int) c] & __C) != 0)
#define	isdigit(c)	((__ctype[(int) c] & __D) != 0)
#define	isgraph(c)	((!(__ctype[(int) c] & (__C | __S))) != 0)
#define	islower(c)	((__ctype[(int) c] & __L) != 0)
#define	isprint(c)	((!(__ctype[(int) c] & __C)) != 0)
#define	ispunct(c)	((__ctype[(int) c] & __P) != 0)
#define	isspace(c)	((__ctype[(int) c] & __S) != 0)
#define	isupper(c)	((__ctype[(int) c] & __U) != 0)
#define	isxdigit(c)	((__ctype[(int) c] & __X) != 0)

#define	toascii(c)	((c) & 0x7F)
#define	isascii(c)	((!((c) & ~0x7F)) != 0)
#define	toupper(c)	(islower(c) ? (c) ^ 0x20 : (c))
#define	tolower(c)	(isupper(c) ? (c) ^ 0x20 : (c))

extern const unsigned char __ctype[];

#endif /* __CTYPE_H__ */
