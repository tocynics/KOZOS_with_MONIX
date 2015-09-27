/* $Id: crt0.S,v 1.2.2.5 2007/04/30 07:01:26 kuwa Exp $ */
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

	.h8300h
;----------------------------------------------------------------------------
	; Monitor mode
	.section ".mondata","ax"
	.global	_MonitorMode
_MonitorMode:
	.space	1

;----------------------------------------------------------------------------
	.section .text
	.align	4
	.global	_startup
_startup:
	; Initialize stack pointer
	mov.l	#_stack, er7

	; Initialize bus controller
	jsr	@_BusInit

	; Copy .data section
	mov.l	#___dtors_end, er0	; LMA
	mov.l	#_edata, er1
	mov.l	#___data, er2	; VMA
data_copy_loop:
	mov.w	@er0, r3
	mov.w	r3, @er2
	adds	#2, er0
	adds	#2, er2
	cmp.l	er1, er2
	blo	data_copy_loop

	; Clear .bss section
	mov.l	#_edata, er0
	mov.l	#_end, er1
	mov.w	#0, r2
bss_clear_loop:
	mov.w	r2, @er0
	adds	#2, er0
	cmp.l	er1, er0
	blo	bss_clear_loop

	; Jump to C main
	jsr	@_main
infinite_loop:
	sleep
	bra	infinite_loop

	.end
;----------------------------------------------------------------------------
