/* $Id: cmd.c,v 1.5.2.14 2007/12/03 14:50:25 kuwa Exp $ */
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
#include <string.h>
#include <ctype.h>
#include "types.h"
#include "sys.h"
#include "monix.h"
#include "cmd.h"
#include "memutil.h"
#include "sci.h"

#define CTRL(x)	(x & 0x1f)
#define BS	CTRL('H')
#define TAB	CTRL('I')
#define LF	CTRL('J')
#define CR	CTRL('M')
#define ESC	CTRL('[')
#define DEL	0x7f

#ifdef USE_SREC_MODNAME_AND_ADDR
#define MODNAME_SIZE 30
void *StartAddress;
char ModuleName[MODNAME_SIZE];
#endif
#define LINE_SIZE 40

static char *CmdCurArg(char *arg);
static char *CmdNextArg(char *arg);
static int CmdHasArg(char *arg);
static void CmdGetLineNoEcho(char *s);
static int CmdBurnSrecLineIntoRAM(char *srec);


static const struct cmdtab {
	char *cmd;
	int (*func)(char *arg);
} cmdtab[] = {
	{"ld", CmdLd},
	{"go", CmdGo},
	{"db", CmdDb},
	{"fb", CmdFb},
	{"dw", CmdDw},
	{"ew", CmdEw},
	{"fw", CmdFw},
	{"help", CmdHelp},
	{"?", CmdHelp},
};

void CmdParse(char *cmdline)
{
	int i;
	size_t len = 0;

	for (i = 0; i < sizeof(cmdtab) / sizeof(*cmdtab); i++) {
		len = strlen(cmdtab[i].cmd);
		if (strncmp(cmdtab[i].cmd, cmdline, len) == 0) {
			cmdtab[i].func(&cmdline[len]);
			break;
		}
	}
	if (i == sizeof(cmdtab) / sizeof(*cmdtab) && len > 0 && *cmdline) {
		while (!isspace(*cmdline)) {
			putchar(*cmdline);
			cmdline++;
			if (!*cmdline)
				break;
		}
		printf(": command not found\n");
	}
}

static char *CmdCurArg(char *arg)
{
	while (isspace(*arg))
		arg++;
	return arg;
}

static char *CmdNextArg(char *arg)
{
	while (*arg && !isspace(*arg))
		arg++;
	return CmdCurArg(arg);
}

static int CmdHasArg(char *arg)
{
	return strlen(CmdCurArg(arg));
}

static void CmdGetLineNoEcho(char *s)
{
	int i = 0;
	int c = 0;

	for (;;) {
		c = getchar();
		switch (c) {
		case CR:
		case LF:
			s[i] = '\0';
			return;
		default:
			s[i] = c;
			i++;
			break;
		}
	}
}

void CmdGetLine(char *s, u_short len)
{
	int i = 0;
	int c = 0;

	if (len == 0)
		return;

	while ((c = getchar()) != EOF) {
		if (c == CR || c == LF) {
			putchar(c);
			s[i] = '\0';
			break;
		} else if (c == BS || c == DEL) {
			if (i >= 1) {
				putchar(c);
				putchar(' ');
				putchar(c);
				i--;
			}
		} else {
			if (i < len - 1) {
				putchar(c);
				s[i] = c;
				i++;
			}
		}
	}
}

int CmdHelp(char *arg)
{
	printf("\n%s\n", VERSION);
	printf("----------------------------------------------------------------------------\n"
	       "ld                                       load program via serial I/F\n"
	       "go [<ad>]                                execute program\n"
	       "dw [<ad> [<ln>]]                         dump word\n"
	       "db [<ad> [<ln>]]                         dump byte\n"
	       "ew [<ad>]                                enter word\n"
	       "fw [<ad>]                                fill word (without read)\n"
	       "fb [<ad> [<ln> [<in> [<ic> [<rp>]]]]]    fill byte\n"
	       "help                                     show this message\n"
	       "?                                        show this message\n"
	       "----------------------------------------------------------------------------\n");
	return 0;
}

static int CmdBurnSrecLineIntoRAM(char *srec)
{
	u_char buf[LINE_SIZE];
	u_char count;
	u_char cksum;
	u_char *mem;
	int i;

	if (srec[0] != 'S') {
		printf("Invalid S-record (%02x)\n", srec[0]);
		return 1;
	}

#define HexCharToInt(hexChar) \
	((hexChar) >= 'A' ? (hexChar) + 10 - 'A' : (hexChar) - '0')

	count = HexCharToInt(srec[2]) << 4;
	count |= HexCharToInt(srec[3]);
	cksum = count;
	for (i = 1; i <= count; i++) {
		buf[i] = (HexCharToInt(srec[2 * i + 2]) << 4);
		buf[i] |= HexCharToInt(srec[2 * i + 3]);
		cksum += buf[i];
	}

	/* sum of the count, the address, the data and cksum == 0xFF */
	if (cksum != 0xff) {
		printf("S-record checksum error\n");
		return 0;
	}

	switch (srec[1]) {	/* type of record */
	case '0':	/* module name */
#ifdef USE_SREC_MODNAME_AND_ADDR
		for (i = 3; i < count; i++)
			ModuleName[i - 3] = buf[i];
		ModuleName[++i] = '\0';
#endif
		return 1;
	case '1':
		mem = (u_char *)((((u_long)buf[1]) << 8) | (u_long)buf[2]);
		for (i = 3; i < count; i++)
			*(mem + i - 3) = buf[i];
		return 1;
	case '2':
		mem = (u_char *)((((u_long)buf[1]) << 16) |
				 (((u_long)buf[2]) << 8) | (u_long)buf[3]);
		for (i = 4; i < count; i++)
			*(mem + i - 4) = buf[i];
		return 1;
	case '8':
#ifdef USE_SREC_MODNAME_AND_ADDR
		StartAddress = (void *)((((u_long)buf[1]) << 16) |
					(((u_long)buf[2]) << 8) |
					(u_long)buf[3]);
#endif
		return 0;
	case '9':
#ifdef USE_SREC_MODNAME_AND_ADDR
		StartAddress = (void *)(((u_long)(buf[1]) << 8) |
					(u_long) (buf[2]));
#endif
		return 0;
	default:
		return 1;
	}
}

int CmdLd(char *arg)
{
	char srec_line[LINE_SIZE * 2 + 2];

	do
		CmdGetLineNoEcho(srec_line);
	while (CmdBurnSrecLineIntoRAM(srec_line));
	return 0;
}

int CmdGo(char *arg)
{
	u_long addr;

	/* Disable interrupts */
	SYS_INTR_DISABLE();

# ifdef USE_SREC_MODNAME_AND_ADDR
	printf("StartAddress=0x%x\n", StartAddress);
	printf("ModuleName=%s\n", ModuleName);
# endif
	printf("MonitorVector=0x%x\n", (unsigned int)MonitorVector);
	printf("UserVector=0x%x\n", (unsigned int)UserVector);
	printf("CurrentVector=0x%x\n", (unsigned int)CurrentVector);
	CurrentVector = UserVector;	/* change vector */

	addr = CmdHasArg(arg) ? xatoi(CmdCurArg(arg)) : *(u_long *)UserVector;
	printf("Entry=0x%x\n", addr);
	((void (*)())addr)();	/* call entrypoint routine
				   (will not return...) */

	CurrentVector = MonitorVector;	/* restore vector */
	return 0;
}

int CmdDb(char *arg)
{
	u_long addr;
	u_long len;

	arg = CmdCurArg(arg);
	addr = xatoi(arg);	/* if (arg == 0) addr will be 0x0 */
	len = xatoi(CmdNextArg(arg));
	if (len == 0)
		len = 1;
	MemDump((void *)addr, len, 0);	/* byte access */
	return 0;
}

int CmdFb(char *arg)
{
	u_long addr;
	u_long len;
	u_char init;
	u_char inc;
	u_short repeat;
	u_short i;
	u_short repeat_count;
	u_char val;

	/* addr len init inc repeat */
	arg = CmdCurArg(arg);
	addr = xatoi(arg);	/* if (arg == 0) addr will be 0x0 */
	arg = CmdNextArg(arg);
	len = xatoi(arg);
	arg = CmdNextArg(arg);
	init = xatoi(arg);
	arg = CmdNextArg(arg);
	inc = xatoi(arg);
	arg = CmdNextArg(arg);
	repeat = xatoi(arg);

	if (len == 0)
		len = 256;
	val = init;
	repeat_count = 0;
	for (i = 0; i < len; i++) {
		*((u_char *)addr) = val;
		val += inc;
		addr++;
		repeat_count++;
		if (repeat_count >= repeat) {
			repeat_count = 0;
			val = init;
		}
	}
	return 0;
}

int CmdDw(char *arg)
{
	u_long addr;
	u_long len;

	arg = CmdCurArg(arg);
	addr = xatoi(arg);	/* if (arg == 0) addr will be 0x0 */
	len = xatoi(CmdNextArg(arg));
	if (len == 0)
		len = 1;
	MemDump((void *)addr, len, 1);	/* word access */
	return 0;
}

int CmdEw(char *arg)
{
	u_long addr;
	u_short val;
	char buf[7];

	arg = CmdCurArg(arg);
	addr = xatoi(arg);	/* if (arg == 0) addr will be 0x0 */
	printf("%08lx  %04x => ", addr, *((u_short *)addr));
	buf[0] = '0';
	buf[1] = 'x';
	CmdGetLine(&buf[2], sizeof(buf) - 2);
	putchar(LF);
	val = xatoi(buf);
	*((u_short *)addr) = val;
	return 0;
}

int CmdFw(char *arg)
{
	u_long addr;
	u_short val;
	char buf[7];

	arg = CmdCurArg(arg);
	addr = xatoi(arg);	/* if (arg == 0) addr will be 0x0 */
	printf("%08lx  ???? => ", addr);
	buf[0] = '0';
	buf[1] = 'x';
	CmdGetLine(&buf[2], sizeof(buf) - 2);
	putchar(LF);
	val = xatoi(buf);
	*((u_short *)addr) = val;
	return 0;
}

