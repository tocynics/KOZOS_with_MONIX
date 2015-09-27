/* $Id: monix.c,v 1.2.2.5 2007/01/24 17:46:02 kuwa Exp $ */
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

#include "stdio.h"
#include "monix.h"
#include "sci.h"
#include "cmd.h"

char buf[128];

int main(void)
{
	/* Initialize monitor SCI port */
	SCIInit(1, SCI_57600);
	svInit();

	printf("\n%s\n", VERSION);
	printf("BootMode: 0x%02x\n", (int)MonitorMode);

	switch (MonitorMode) {
	case 0x00: /* Monitor session */
	default:
		for (;;) {
			putchar('1');
			putchar(':');
			CmdGetLine(buf, sizeof(buf));
			putchar('\n');
			CmdParse(buf);
		}
		break;

#if 0 /* Network boot */
	case 0xFF:
#ifdef ETHER
		CmdDhcp(""); /* Get IP */
		CmdIfConfig(""); /* Print interface */
		CmdTftp("192.168.0.4 maketest.elf 0x800000"); /* Do TFTP */
		CmdGo(""); /* Go */
#endif
		break;
#endif
	}

	return 0;
}
