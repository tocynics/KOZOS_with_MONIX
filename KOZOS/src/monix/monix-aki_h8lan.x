/* $Id: monix-aki_h8lan.x,v 1.2.2.4 2007/01/18 14:30:01 kuwa Exp $ */
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
OUTPUT_FORMAT("elf32-h8300")
OUTPUT_ARCH(h8300h)
ENTRY("_startup")
MEMORY
{
/* AKI-H8LAN */
	vectors : o = 0x000000,	l = 0x100
	monvec	: o = 0x000100, l = 0x100
	stubvec	: o = 0x000200, l = 0x100
	rom    	: o = 0x000300,	l = 0xFFD00
	uservec	: o = 0x400000, l = 0x100
	ram    	: o = 0xFFBF20, l = 0x4000 /* 16KB */
	softvec	: o = 0x400000, l = 0x000040	/* ソフトウェア割り込み領域 */
	stack	: o = 0xFFFF14, l = 0x4
	mondata	: o = 0xFFFF18, l = 0x8
	intrstack	: o = 0x502000, l = 0x001000
}
SECTIONS
{
.vectors : {
	*(.vectors)
	FILL(0xff)
	} > vectors

.monvec : {
	*(.monvec)
} > monvec

.stubvec : {
	*(.stubvec)
} > stubvec

.uservec : {
	*(.uservec)
} > uservec

.text : {
	*(.text)
	*(.strings)
	*(.rodata*)
   	 _etext = . ;
	} > rom

	.softvec : {
		_softvec = . ;
	} > softvec /* RAM領域に配置する */

.tors ALIGN(4): {
	___ctors = . ;
	*(.ctors)
	___ctors_end = . ;
	___dtors = . ;
	*(.dtors)
	___dtors_end = . ;
	} > rom
.data : AT ( ADDR(.tors) + SIZEOF(.tors) ) {
	___data = . ;
	*(.data)
	*(.tiny)
	 _edata = .;
	} > ram

/*
__mondata_start = . ;
.mondata   : AT(__mondata_start)
*/
.mondata :
{
	*(.mondata)
} > mondata

/*
.bss : AT ( LOADADDR(.data) + SIZEOF(.data) ) {
	 _bss_start = . ;
	*(.bss)
	*(COMMON)
	 _end = . ;
	} > ram
*/
.bss : AT ( ADDR(.data) + SIZEOF(.data) ) {
	 _bss_start = . ;
	*(.bss)
	*(COMMON)
	 _end = . ;
	} > ram
.stack : {
	 _stack = . ;
	*(.stack)
	} > stack
.stab 0 (NOLOAD) : {
	[ .stab ]
	}
.stabstr 0 (NOLOAD) : {
	[ .stabstr ]
	}
	.intrstack : {
		_intrstack = . ;
	} > intrstack
}
