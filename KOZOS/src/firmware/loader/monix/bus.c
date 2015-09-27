/* $Id: bus.c,v 1.2.2.6 2007/04/22 04:07:04 kuwa Exp $ */
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
#include "monix.h" /* MonitorMode */

void BusInit(void)
{
	/* Setup vector */
	CurrentVector = MonitorVector;

#if defined(AKI_H8LAN)
	MonitorMode = 0x00;

	/* Setup DRAM controller */
	P1DDR = 0xff;	/* Enable A0  - A7  */
	P2DDR = 0xff;	/* Enable A8  - A15 */
	P3DDR = 0xff;	/* Enable D8  - D15 */

	P5DDR = 0x01;	/* Enable A16 */
	P8DDR = 0x0c;	/* Enable CS1 - CS2 */
	PBDDR = 0x10;	/* Enable Upper CAS */

	/* DRAM: 2,048/32ms: 32ms / 2,048 = 15.625us */
	/* 20MHz / 8 / 39 = 1 / 15.6us */
	BSC.RTCOR = (32 * (CPU_HZ * 1000)) / (8 * 2048);

	BSC.RTMCSR.BYTE = 0x10;	/* clock / 8 */
	BSC.DRCRB.BYTE = 0x90;	/* CA: 10 bit, A23-A10	*/
	BSC.DRCRA.BYTE = 0x28;	/* Enable Upper CAS	*/

	SysDelayUsec(CPU_HZ * 1000);	/* Wait for DRAM stable	*/
#endif
}
