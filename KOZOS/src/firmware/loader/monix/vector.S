/* $Id: vector.S,v 1.2.2.9 2007/04/30 07:01:26 kuwa Exp $ */
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

#include "intr.h"
	.h8300h

;----------------------------------------------------------------------------
; current vector pointer
; 
	.section ".mondata","ax"
	.global	_CurrentVector
_CurrentVector:
	.space	4

;----------------------------------------------------------------------------
; user vector
;
	.section ".uservec","ax"
	.global	_UserVector
_UserVector:
	.space	0x100

;----------------------------------------------------------------------------
; monitor vector
;
	.section ".monvec","ax"
	.align	4
	.global	_MonitorVector
_MonitorVector:
	.long	_startup	; 0 reset vector
	.long	0		; 1 reserved vector
	.long	0		; 2 reserved vector
	.long	0		; 3 reserved vector
	.long	0		; 4 reserved vector
	.long	0		; 5 reserved vector
	.long	0		; 6 reserved vector
	.long	0		; 7 nmi vector
	.long	_intrSyscall		; 8 trapa 0 vector
	.long	_intrSofterr		; 9 trapa 1 vector
	.long	_intrSofterr		; 10 trapa 2 vector
	.long	_intrSofterr		; 11 trapa 3 vector
	.long	0		; 12 irq0 vector ipra7
	.long	0		; 13 irq1 vector ipra6
	.long	0		; 14 irq2 vector ipra5
	.long	0		; 15 irq3 vector
#if defined(ETHER) && defined(AKI_H8LAN) && defined(EASYNIC)
	.long	_EtherIntr	; 16 irq4 vector ipra4
#else
	.long	0		; 16 irq4 vector ipra4
#endif
#if defined(ETHER) && defined(AKI_H8LAN) && !defined(EASYNIC)
	.long	_EtherIntr	; 17 irq5 vector
#else
	.long	0		; 17 irq5 vector
#endif
	.long	0		; 18 reserved vector
	.long	0		; 19 reserved vector
	.long	0		; 20 wovi wdt vector ipra3
	.long	0		; 21 cmi ref cont vector
	.long	0		; 22 reserved vector
	.long	0		; 23 reserved vector
	.long	0		; 24 imia0 itu0 vector ipra2
	.long	0		; 25 imib0 vector
	.long	0		; 26 ovi0 vector
	.long	0		; 27 reserved vector
	.long	0		; 28 imia1 itu1 vector ipra1
	.long	0		; 29 imib1 vector
	.long	0		; 30 ovi1 vector
	.long	0		; 31 reserved vector
	.long	0		; 32 imia2 itu2 vector ipra0
	.long	0		; 33 imib2 vector
	.long	0		; 34 ovi2 vector
	.long	0		; 35 reserved vector
	.long	0		; 36 imia3 itu3 vector iprb7
	.long	0		; 37 imib3 vector
	.long	0		; 38 ovi3 vector
	.long	0		; 39 reserved vector
	.long	0		; 40 imia4 itu4 vector iprb6
	.long	0		; 41 imib4 vector
	.long	0		; 42 ovi4 vector
	.long	0		; 43 reserved vector
	.long	0		; 44 dend0a dmac vector iprb5
	.long	0		; 45 dend0b vector
	.long	0		; 46 dend1a vector
	.long	0		; 47 dend1b vector
	.long	0		; 48 reserved vector
	.long	0		; 49 reserved vector
	.long	0		; 50 reserved vector
	.long	0		; 51 reserved vector
	.long	_intrSerial		; 52 eri0 sci0 vector iprb3
	.long	_intrSerial		; 53 rxi0 vector
	.long	_intrSerial		; 54 txi0 vector
	.long	_intrSerial		; 55 tei0 vector
	.long	_intrSerial		; 56 eri1 sci1 vector iprb2
	.long	_intrSerial		; 57 rxi1 vector
	.long	_intrSerial		; 58 txi1 vector
	.long	_intrSerial		; 59 tei1 vector
	.long	_intrSerial		; 60 eri2 sci2 vector iprb1
	.long	_intrSerial		; 61 rxi2 vector
	.long	_intrSerial		; 62 free vector
	.long	_intrSerial		; 63 free vector

;----------------------------------------------------------------------------
	.section .text
	.align	4
UndefinedVector:
;	push.l	er0
	jsr	@_UnhandledException
UndefinedVectorLoop:
	bra	UndefinedVectorLoop

;----------------------------------------------------------------------------
	.section .text
	.align	4
_IntrHandler:
	push.l	er0
	push.l	er1
	mov.l	@(8, er7), er0		; save return address (next address of '_Intr0xXX: jsr @_IntrHandler') to ER0
	sub.l	#_Intr0x04, er0		; save vector offset to ER0
	mov.w	r0, r1			; copy vector offset to R1
	mov.l	@_CurrentVector, er0
	mov.b	r1l, r0l
	mov.l	@er0, er1		; set vector to ER1
	beq	UndefinedVector:16	; if vector is zero then error
	stc.w	ccr, @(8, er7)		; set user CCR
	mov.w	r1, @(10, er7)		; set lower 16-bits of vector
	mov.w	e1, r1
	mov.b	r1l, @(9, er7)		; set upper 8-bits of vector
	pop.l	er1
	pop.l	er0
	rte				; jump to vector

;----------------------------------------------------------------------------
; real interrupt handler
	.section .text
	.align	4
_Intr0x04:	jsr	@_IntrHandler
_Intr0x08:	jsr	@_IntrHandler
_Intr0x0c:	jsr	@_IntrHandler
_Intr0x10:	jsr	@_IntrHandler
_Intr0x14:	jsr	@_IntrHandler
_Intr0x18:	jsr	@_IntrHandler
_Intr0x1c:	jsr	@_IntrHandler
_Intr0x20:	jsr	@_IntrHandler
_Intr0x24:	jsr	@_IntrHandler
_Intr0x28:	jsr	@_IntrHandler
_Intr0x2c:	jsr	@_IntrHandler
_Intr0x30:	jsr	@_IntrHandler
_Intr0x34:	jsr	@_IntrHandler
_Intr0x38:	jsr	@_IntrHandler
_Intr0x3c:	jsr	@_IntrHandler
_Intr0x40:	jsr	@_IntrHandler
_Intr0x44:	jsr	@_IntrHandler
_Intr0x48:	jsr	@_IntrHandler
_Intr0x4c:	jsr	@_IntrHandler
_Intr0x50:	jsr	@_IntrHandler
_Intr0x54:	jsr	@_IntrHandler
_Intr0x58:	jsr	@_IntrHandler
_Intr0x5c:	jsr	@_IntrHandler
_Intr0x60:	jsr	@_IntrHandler
_Intr0x64:	jsr	@_IntrHandler
_Intr0x68:	jsr	@_IntrHandler
_Intr0x6c:	jsr	@_IntrHandler
_Intr0x70:	jsr	@_IntrHandler
_Intr0x74:	jsr	@_IntrHandler
_Intr0x78:	jsr	@_IntrHandler
_Intr0x7c:	jsr	@_IntrHandler
_Intr0x80:	jsr	@_IntrHandler
_Intr0x84:	jsr	@_IntrHandler
_Intr0x88:	jsr	@_IntrHandler
_Intr0x8c:	jsr	@_IntrHandler
_Intr0x90:	jsr	@_IntrHandler
_Intr0x94:	jsr	@_IntrHandler
_Intr0x98:	jsr	@_IntrHandler
_Intr0x9c:	jsr	@_IntrHandler
_Intr0xa0:	jsr	@_IntrHandler
_Intr0xa4:	jsr	@_IntrHandler
_Intr0xa8:	jsr	@_IntrHandler
_Intr0xac:	jsr	@_IntrHandler
_Intr0xb0:	jsr	@_IntrHandler
_Intr0xb4:	jsr	@_IntrHandler
_Intr0xb8:	jsr	@_IntrHandler
_Intr0xbc:	jsr	@_IntrHandler
_Intr0xc0:	jsr	@_IntrHandler
_Intr0xc4:	jsr	@_IntrHandler
_Intr0xc8:	jsr	@_IntrHandler
_Intr0xcc:	jsr	@_IntrHandler
_Intr0xd0:	jsr	@_IntrHandler
_Intr0xd4:	jsr	@_IntrHandler
_Intr0xd8:	jsr	@_IntrHandler
_Intr0xdc:	jsr	@_IntrHandler
_Intr0xe0:	jsr	@_IntrHandler
_Intr0xe4:	jsr	@_IntrHandler
_Intr0xe8:	jsr	@_IntrHandler
_Intr0xec:	jsr	@_IntrHandler
_Intr0xf0:	jsr	@_IntrHandler
_Intr0xf4:	jsr	@_IntrHandler
_Intr0xf8:	jsr	@_IntrHandler
_Intr0xfc:	jsr	@_IntrHandler

;----------------------------------------------------------------------------
	; real vector
	.section ".vectors","ax"
	.align	4
	.global	_vector_table
_vector_table:
	.long	_startup
	.long	_Intr0x04
	.long	_Intr0x08
	.long	_Intr0x0c
	.long	_Intr0x10
	.long	_Intr0x14
	.long	_Intr0x18
	.long	_Intr0x1c
	.long	_intrSyscall
	.long	_intrSofterr
	.long	_intrSofterr
	.long	_intrSofterr
	.long	_Intr0x30
	.long	_Intr0x34
	.long	_Intr0x38
	.long	_Intr0x3c
	.long	_Intr0x40
	.long	_Intr0x44
	.long	_Intr0x48
	.long	_Intr0x4c
	.long	_Intr0x50
	.long	_Intr0x54
	.long	_Intr0x58
	.long	_Intr0x5c
	.long	_Intr0x60
	.long	_Intr0x64
	.long	_Intr0x68
	.long	_Intr0x6c
	.long	_Intr0x70
	.long	_Intr0x74
	.long	_Intr0x78
	.long	_Intr0x7c
	.long	_Intr0x80
	.long	_Intr0x84
	.long	_Intr0x88
	.long	_Intr0x8c
	.long	_Intr0x90
	.long	_Intr0x94
	.long	_Intr0x98
	.long	_Intr0x9c
	.long	_Intr0xa0
	.long	_Intr0xa4
	.long	_Intr0xa8
	.long	_Intr0xac
	.long	_Intr0xb0
	.long	_Intr0xb4
	.long	_Intr0xb8
	.long	_Intr0xbc
	.long	_Intr0xc0
	.long	_Intr0xc4
	.long	_Intr0xc8
	.long	_Intr0xcc
	.long	_intrSerial
	.long	_intrSerial
	.long	_intrSerial
	.long	_intrSerial
	.long	_intrSerial
	.long	_intrSerial
	.long	_intrSerial
	.long	_intrSerial
	.long	_intrSerial
	.long	_intrSerial
	.long	_intrSerial
	.long	_intrSerial
;----------------------------------------------------------------------------

	.end
