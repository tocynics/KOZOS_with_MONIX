/* $Id: sys.c,v 1.2.2.4 2007/01/18 14:30:01 kuwa Exp $ */
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

#include "types.h"
#include "sys.h"
#include "stdio.h"

/*
 * NOTE: H8/300H dependent code
 */

#if 0
int SysLockDepth;
#endif

void UnhandledException(u_long v)
{
	printf("Unhandled Exception (vector address=0x%lx)\n", v);
}

/* The number of states required for execution when the instruction
   and its operands are located in on-chip memory. */
#define MOVL_ERs_ERd_CYCLES 2
#define BEQ_d8_CYCLES 4
#define NOP_CYCLES 2
#define SUBS_1_ERd_CYCLES 2
#define BRA_d8_CYCLES 4

/* Need to adjust according to CPU_HZ value and assembly code. */
#define LOOP_USECS (CPU_HZ / (MOVL_ERs_ERd_CYCLES + BEQ_d8_CYCLES + \
		    NOP_CYCLES * 4 + SUBS_1_ERd_CYCLES + BRA_d8_CYCLES))

/*
 * Delay micro seconds.
 */
void SysDelayUsec(u_long usec)
{
	u_long count = usec * LOOP_USECS;
	u_long i;

	for (i = 0; i < count; i++) {
		asm volatile ("nop");
		asm volatile ("nop");
		asm volatile ("nop");
		asm volatile ("nop");
	}
}
