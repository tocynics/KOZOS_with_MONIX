/* $Id: cmd.h,v 1.2.2.5 2007/01/24 17:46:03 kuwa Exp $ */
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

#ifndef __CMD_H__
#define __CMD_H__

#include "types.h"

/* Command Interpreter */
extern void CmdGetLine(char *s, u_short len);
extern void CmdParse(char *cmdline);

/* Each Commands */
extern int CmdLd(char *arg);
extern int CmdGo(char *arg);
extern int CmdDb(char *arg);
extern int CmdDw(char *arg);
extern int CmdEw(char *arg);
extern int CmdFb(char *arg);
extern int CmdFw(char *arg);
#ifdef GDB_STUB
extern int CmdStub(char *arg);
#endif
#ifdef ETHER
extern int CmdEtherInit(char *arg);
extern int CmdMac(char *arg);
extern int CmdIfConfig(char *arg);
extern int CmdArp(char *arg);
extern int CmdMBuf(char *arg);
#ifdef ICMP
extern int CmdPing(char *arg);
#endif
extern int CmdDhcp(char *arg);
extern int CmdTftp(char *arg);
#endif /* ETHER */
extern int CmdHelp(char *arg);

#endif /* __CMD_H__ */
