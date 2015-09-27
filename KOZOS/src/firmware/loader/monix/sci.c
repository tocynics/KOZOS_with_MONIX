/* $Id: sci.c,v 1.2.2.5 2007/04/22 04:07:06 kuwa Exp $ */
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

#include "sci.h"

/*
 * NOTE: H8/300H dependent code
 */

/*
 * SCI initialize
 */
void SCIInit(u_char port, u_char rate)
{
	int i;
	volatile struct st_sci *pSCI =
	  (port == 0 ? &SCI0 : (port == 1 ? &SCI1 : &SCI2));

	/* initialize SCI */
	pSCI->SCR.BYTE = 0x00;	/* use internal clock */

	/* set communication mode and data format */
	pSCI->SMR.BYTE = 0x00;	/* asynchronouse mode, 8bit data, 1stop bit,
				   no-parity, 1/1 clock */

	/* set bit rate */
	pSCI->BRR = rate;
	for (i = 0; i < 350; i++) ;	/* wait 1bit time */

	/* clear error status */
	pSCI->SSR.BIT.ORER = 0;
	pSCI->SSR.BIT.FER = 0;
	pSCI->SSR.BIT.PER = 0;

	/* enable SCI */
	pSCI->SCR.BYTE = 0x30;	/* TE(transmit enable), RE(receive enable) */
}

/*
 * Output one character
 */
void SCIPutChar(u_char port, char c)
{
	volatile struct st_sci *pSCI =
	  (port == 0 ? &SCI0 : (port == 1 ? &SCI1 : &SCI2));

	/* Wait for TDRE (transmit data register empty) */
	while ((pSCI->SSR.BYTE & 0x80) == 0) ;

	/* data */
	pSCI->TDR  = c;

	/* Wait for TEND (transmit end) */
	while ((pSCI->SSR.BYTE & 0x04) == 0) ;

	/* Clear TDRE */
	pSCI->SSR.BYTE &= 0x7f;
}

/*
 * Input one character
 */
int SCIGetChar(u_char port)
{
	volatile struct st_sci *pSCI =
	  (port == 0 ? &SCI0 : (port == 1 ? &SCI1 : &SCI2));
	u_char status;
	u_char c;

	/* Look at the status */
	status = pSCI->SSR.BYTE;
	if (status & 0x38) {
		/* Clear any errors */
		if (status & 0x20) {	/* overrun error */
			pSCI->SSR.BYTE &= ~0x20;
		}
		if (status & 0x10) {	/* framing error */
			c = pSCI->RDR;	/* receive one character */
			pSCI->SSR.BYTE &= ~0x10;
		}
		if (status & 0x08) {	/* parity error */
			c = pSCI->RDR;	/* receive one character */
			pSCI->SSR.BYTE &= ~0x08;
		}
	}

	/* Wait for a byte. */
	while (!(pSCI->SSR.BYTE & 0x40)) ;

	/* Get and return it. */
	c = pSCI->RDR;
	pSCI->SSR.BYTE &= ~0x40;
	return (int)c;
}
