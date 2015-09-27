/* $Id: sys.h,v 1.2.2.7 2007/11/10 17:29:17 kuwa Exp $ */
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

#ifndef __SYS_H__
#define __SYS_H__

#include "types.h"

#if defined(AKI_H8LAN)
#include "3069s.h"
//#define CPU_HZ 20	/* MHz */
#define CPU_HZ 25	/* MHz */
#endif

/*
 * Definitions for byte order,
 * according to byte significance from low address to high.
 */
#define	LITTLE_ENDIAN	1234		/* LSB first: i386, vax */
#define	BIG_ENDIAN	4321		/* MSB first: 68000, ibm, net */
#define	PDP_ENDIAN	3412		/* LSB first in word, MSW first in long */

#define	BYTE_ORDER	BIG_ENDIAN

/*
 * Delay.
 */
extern void SysDelayUsec(u_long usec);

/*
 * Mutual exclusion.
 */
/***
#define SYS_DECL_LOCK
#define SYS_LOCK()	{ asm("stc ccr,@-er7"); \
			asm("orc.b #0xC0,ccr"); }
#define SYS_UNLOCK()	asm("ldc  @er7+,ccr");
***/

typedef u_char PSW;

extern inline void PSWUnmask(PSW mask)
{
	asm volatile ("ldc %0l,ccr"::"r"(mask));
}

extern inline PSW PSWMask(void)
{
	unsigned char value;
	asm volatile ("stc ccr,%0l":"=g"(value):);
	asm volatile ("orc.b #0xC0,ccr");
	return value;
}

#define SYS_DECL_LOCK	PSW psw
#define SYS_LOCK()	psw = PSWMask()
#define SYS_UNLOCK()	PSWUnmask(psw)

#define SYS_INTR_DISABLE()	asm volatile ("orc.b #0x80,ccr")
#define SYS_INTR_ENABLE()	asm volatile ("andc.b #0x7F,ccr")

#endif /* __SYS_H__ */
