/* $Id: dma.c,v 1.2.2.4 2007/01/18 14:30:00 kuwa Exp $ */
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

/*
 * NOTE: H8/300H dependent code
 */

#include "sys.h"
#include "dma.h"

static u_char dmaMode[2];

/*
 * DMAC Initialize.
 */
void DMAInit(u_char ch, u_char mode)
{
	volatile struct st_fam *pDMAC = (ch == 0 ? &DMAC0 : &DMAC1);

	dmaMode[ch] = mode;

	/* Data transfer is disabled. */
	pDMAC->DTCRA.BIT.DTE = 0;
	pDMAC->DTCRB.BIT.DTME = 0;

	/* Word-size transfer. */
	pDMAC->DTCRA.BIT.DTSZ = mode & DMAC_MODE_WORD_TRANSFER ? 1 : 0;
	pDMAC->DTCRA.BIT.SAID = 0;
	pDMAC->DTCRB.BIT.DAID = 0;

	/* DEND interrupt is disabled. */
	pDMAC->DTCRA.BIT.DTIE = 0;

	/* Full Address Mode & Normal Mode. */
	pDMAC->DTCRA.BIT.DTS = 6;
	//pDMAC->DTCRB.BIT.TMS = 0;

	/* Auto-request (burst mode). */
	pDMAC->DTCRB.BIT.DTS = 0;
}

/*
 * DMA transfer len byte from memory to device.
 */
void DMAWrite(u_char ch, void *devAddr, void *mem, u_short len)
{
	volatile struct st_fam *pDMAC = (ch == 0 ? &DMAC0 : &DMAC1);

	/* MARA(source address) is incremented
	   by size (DTSZ) after each transfer. */
	pDMAC->DTCRA.BIT.SAIDE = 1;

	/* MARB(destination address) is held fixed
	   or incremented by size (DTSZ)
	   after each transfer. */
	pDMAC->DTCRB.BIT.DAIDE = dmaMode[ch] & DMAC_MODE_DEVADDR_FIXED ? 0 : 1;

	/* Source Address. */
	pDMAC->MARA = mem;

	/* Destination Address. */
	pDMAC->MARB = devAddr;

	/* Word or Byte count ? */
	pDMAC->ETCRA = dmaMode[ch] & DMAC_MODE_WORD_TRANSFER ? len >> 1 : len;

	/* Data transfer is enabled. */
	pDMAC->DTCRA.BIT.DTE = 1;
	pDMAC->DTCRB.BIT.DTME = 1;

	while (pDMAC->DTCRA.BIT.DTE);

	/* Data transfer is disabled. */
	pDMAC->DTCRB.BIT.DTME = 0;
}

/*
 * DMA transfer len byte from device to memory.
 */
void DMARead(u_char ch, void *devAddr, void *mem, u_short len)
{
	volatile struct st_fam *pDMAC = (ch == 0 ? &DMAC0 : &DMAC1);

	/* MARA(source address) is held fixed
	   or incremented by size (DTSZ)
	   after each transfer. */
	pDMAC->DTCRA.BIT.SAIDE = dmaMode[ch] & DMAC_MODE_DEVADDR_FIXED ? 0 : 1;

	/* MARB(destination address) is incremented
	   by size (DTSZ) after each transfer. */
	pDMAC->DTCRB.BIT.DAIDE = 1;

	/* Source Address. */
	pDMAC->MARA = devAddr;

	/* Destination Address. */
	pDMAC->MARB = mem;

	/* Word or Byte count ? */
	pDMAC->ETCRA = dmaMode[ch] & DMAC_MODE_WORD_TRANSFER ? len >> 1 : len;

	/* Data transfer is enabled. */
	pDMAC->DTCRA.BIT.DTE = 1;
	pDMAC->DTCRB.BIT.DTME = 1;

	while (pDMAC->DTCRA.BIT.DTE);

	/* Data transfer is disabled. (master) */
	pDMAC->DTCRB.BIT.DTME = 0;
}

/*
 * DMA transfer len byte from src to dst.
 */
void DMACopy(u_char ch, void *dst, void *src, u_short len)
{
	volatile struct st_fam *pDMAC = (ch == 0 ? &DMAC0 : &DMAC1);

	/* MARA(source address) is incremented
	   by size (DTSZ) after each transfer. */
	pDMAC->DTCRA.BIT.SAIDE = 1;

	/* MARB(destination address) is incremented
	   by size (DTSZ) after each transfer. */
	pDMAC->DTCRB.BIT.DAIDE = 1;

	/* Source Address. */
	pDMAC->MARA = src;

	/* Destination Address. */
	pDMAC->MARB = dst;

	/* Word or Byte count ? */
	pDMAC->ETCRA = dmaMode[ch] & DMAC_MODE_WORD_TRANSFER ? len >> 1 : len;

	/* Data transfer is enabled. */
	pDMAC->DTCRA.BIT.DTE = 1;
	pDMAC->DTCRB.BIT.DTME = 1;

	while (pDMAC->DTCRA.BIT.DTE);

	/* Data transfer is disabled. (master) */
	pDMAC->DTCRB.BIT.DTME = 0;
}
