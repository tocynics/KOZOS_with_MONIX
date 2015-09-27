/* $Id: sci.h,v 1.2.2.4 2007/01/18 14:30:01 kuwa Exp $ */
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

#ifndef __H8_SCI_H__
#define __H8_SCI_H__

#include "sys.h"

#if CPU_HZ == 16
#define SCI_2400	207
#define SCI_4800	103
#define SCI_9600	51
#define SCI_19200	25
#define SCI_31250	15
#define SCI_38400	12
#define SCI_57600	8
#elif CPU_HZ == 20
#define SCI_4800	129
#define SCI_9600	64
#define SCI_19200	32
#define SCI_31250	19
#define SCI_38400	15
#define SCI_57600	10
#elif CPU_HZ == 25
#define SCI_4800	162
#define SCI_9600	80
#define SCI_19200	40
#define SCI_31250	24
#define SCI_38400	19
#define SCI_57600	13
#endif

extern void SCIInit(u_char port, u_char rate);
extern void SCIPutChar(u_char port, char c);
extern int SCIGetChar(u_char port);

#define SCIGetByte SCIGetChar
#define SCIPutByte SCIPutChar

#endif	/* __H8_SCI_H__ */
