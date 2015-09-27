/* $Id: dma.h,v 1.2.2.4 2007/01/18 14:30:00 kuwa Exp $ */
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

#ifndef __DMA_H__
#define __DMA_H__

/*
 * NOTE:
 *
 * - len should be EVEN.
 * - No mutual exclusion:
 *   do not use the same DMA channel for multiple devices at the same time.
 */

#include "types.h"

/* DMA mode */
#define DMAC_MODE_DEVADDR_FIXED		0x1
#define DMAC_MODE_DEVADDR_INCREMENTED	0x2
#define DMAC_MODE_WORD_TRANSFER		0x4

/* API */
extern void DMAInit(u_char ch, u_char mode);
extern void DMAWrite(u_char ch, void *devAddr, void *mem, u_short len);
extern void DMARead(u_char ch, void *devAddr, void *mem, u_short len);
extern void DMACopy(u_char ch, void *dst, void *src, u_short len);

#endif /* __DMA_H__ */
